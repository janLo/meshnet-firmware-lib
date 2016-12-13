from enum import Enum
from typing import Optional

import logging
import serial
import struct
import time
# noinspection PyPackageRequirements
from siphashc import siphash

logger = logging.getLogger(__name__)


class MessageType(Enum):
    booted = 70
    configure = 71
    configured = 72
    set_state = 73
    get_state = 74
    reading = 75
    oing = 76
    pong = 77
    reset = 78


KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


def _hex(hh, for_c=False):
    hex_values = ("{:02x}".format(c) for c in hh)
    if for_c:
        return "{" + ", ".join("0x{}".format(x) for x in hex_values) + "}"
    else:
        return " ".join(hex_values)


class SerialMessage(object):
    def __init__(self, sender: int, receiver: int, msg_type: MessageType, hash_sum: bytes = None, session: int = 0,
                 counter: int = 0,
                 payload=None):
        self.sender = sender
        self.receiver = receiver
        self.msg_type = msg_type
        self.hash_sum = hash_sum
        self.session = session
        self.counter = counter
        self.payload = payload

    def __repr__(self):
        return "SerialMessage<sender:{}, receiver={}, type={}, session={}, counter={}, hash={}, payload={}>".format(
            self.sender, self.receiver, self.msg_type, self.session, self.counter, self.hash_sum.hex(), self.payload)

    def _compute_hash(self, key):
        packed_data = struct.pack(">HHB", self.sender, self.receiver,
                                  self.msg_type) + self._proto_header() + self.payload
        return struct.pack(">Q", siphash(key, packed_data))

    def _proto_header(self):
        return struct.pack(">BHH", len(self.payload) + 5, self.session, self.counter)

    def verify(self, key):
        ref_hash = self._compute_hash(key)
        return self.hash_sum == ref_hash

    def serialize(self, key):
        self.hash_sum = self._compute_hash(key)
        return (struct.pack(">HB", self.receiver, self.msg_type) +
                self._proto_header() +
                self.payload +
                self._compute_hash(key))

    @staticmethod
    def parse(data: bytes) -> 'Optional[SerialMessage]':
        print("Packet: ", _hex(data))
        if len(data) < 4:
            logger.info("Not enough data received for serial packet: %d bytes", len(data))
            return None
        serial_header = data[:3]
        serial_payload = data[3:]

        sender, msg_type = struct.unpack(">HB", serial_header)

        try:
            msg_type = MessageType(msg_type)
        except ValueError:
            logger.warning("Unknown message type: %d", msg_type)
            return None

        print("Payload:", _hex(serial_payload, True))

        if len(serial_payload) < (5 + 8):
            logger.info("Packet too small to contain length, session, counter and hash: %d bytes", len(serial_payload))
            return None

        length, session, counter = struct.unpack(">BHH", serial_payload[:5])
        if (len(serial_payload) - 8) != length:
            logger.info("Wrong number of bytes from network")
            return None

        hash_sum = serial_payload[-8:]

        return SerialMessage(sender, 0, msg_type, hash_sum, session, counter, serial_payload[5:-8])


class _MessageState(Enum):
    preamble = 0
    start = 1
    length = 2
    message = 4
    end = 5


class SerialMessageConsumer(object):
    def __init__(self):
        self._state = _MessageState.preamble
        self._preamble_cnt = 0
        self._read_bytes = bytearray()
        self._to_read = 0
        self._actual_read = 0

    def consume(self, source, max_len: int) -> Optional[SerialMessage]:
        assert max_len >= 1

        if self._state == _MessageState.preamble:
            self._to_read = 0
            self._read_bytes = self._read_bytes[-1:] + source.read(1)
            if self._read_bytes == b"\xaf\xaf":
                self._state = _MessageState.start

        elif self._state == _MessageState.start:
            if source.read(1) == b"\x02":
                self._state = _MessageState.length
            else:
                self._state = _MessageState.preamble

        elif self._state == _MessageState.length:
            self._to_read = struct.unpack("B", source.read(1))[0]
            self._state = _MessageState.message
            self._read_bytes = bytearray()

        elif self._state == _MessageState.message:
            new_data = source.read(min(max_len, self._to_read - len(self._read_bytes)))

            self._read_bytes.extend(new_data)

            if len(self._read_bytes) == self._to_read:
                self._state = _MessageState.end

        elif self._state == _MessageState.end:
            self._state = _MessageState.preamble
            if source.read(1) == b"\x03":
                return SerialMessage.parse(bytes(self._read_bytes))

        else:
            raise IndexError

        return


class Connection(object):
    def __init__(self, device):
        self._device = device
        self._conn = None

    def connect(self):
        logger.info("Connect to %s", self._device)
        self._conn = serial.Serial(self._device, 115200, timeout=1)

    def read(self, consumer: SerialMessageConsumer) -> Optional[SerialMessage]:
        while self._conn.in_waiting == 0:
            time.sleep(0.1)
        return consumer.consume(self._conn, self._conn.in_waiting)

    def write(self, message: SerialMessage, key: bytes):
        content = message.serialize(key)
        out = b"\xaf\xaf\x02" + struct.pack("B", len(content)) + content + b"\x03"
        self._conn.write(out)
        self._conn.flush()
