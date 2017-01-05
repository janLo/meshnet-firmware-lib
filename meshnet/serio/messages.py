import logging
import struct
from enum import Enum
from siphashc import siphash
from typing import Optional

from meshnet.serio.util import to_hex

logger = logging.getLogger(__name__)


class MessageType(Enum):
    booted = 70
    configure = 71
    configured = 72
    set_state = 73
    get_state = 74
    reading = 75
    ping = 76
    pong = 77
    reset = 78


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
        hash_sum = "<not_calculated>"
        if self.hash_sum is not None:
            hash_sum = self.hash_sum.hex()
        return "SerialMessage<sender:{}, receiver={}, type={}, session={}, counter={}, hash={}, payload={}>".format(
            self.sender, self.receiver, self.msg_type, self.session, self.counter, hash_sum, self.payload)

    def _compute_hash(self, key):
        packed_data = struct.pack(">HHB", self.sender, self.receiver,
                                  self.msg_type.value) + self._proto_header() + self.payload
        return struct.pack(">Q", siphash(key, packed_data))

    def _proto_header(self):
        return struct.pack(">BHH", len(self.payload) + 5, self.session, self.counter)

    def _serio_header(self):
        return struct.pack(">HB", self.receiver, self.msg_type.value)

    def verify(self, key):
        ref_hash = self._compute_hash(key)
        result = self.hash_sum == ref_hash
        if not result:
            logger.warning("Invalid hash: %s != %s", to_hex(ref_hash), to_hex(self.hash_sum))
        return result

    def serialize(self, key):
        self.hash_sum = self._compute_hash(key)
        return (self._serio_header() +
                self._proto_header() +
                self.payload +
                self._compute_hash(key))

    @staticmethod
    def parse(data: bytes) -> 'Optional[SerialMessage]':
        logger.debug("parse packet: %s", to_hex(data))
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

        logger.debug("Payload: %s", to_hex(serial_payload, True))

        if len(serial_payload) < (5 + 8):
            logger.info("Packet too small to contain length, session, counter and hash: %d bytes", len(serial_payload))
            return None

        length, session, counter = struct.unpack(">BHH", serial_payload[:5])
        if (len(serial_payload) - 8) != length:
            logger.info("Wrong number of bytes from network")
            return None

        hash_sum = serial_payload[-8:]

        return SerialMessage(sender, 0, msg_type, hash_sum, session, counter, serial_payload[5:-8])

    def framed(self, key: bytes) -> bytes:
        content = self.serialize(key)
        return b"\xaf\xaf\x02" + struct.pack("B", len(content)) + content + b"\x03"


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
                self._read_bytes.clear()

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
            readed = bytes(self._read_bytes)
            self._read_bytes.clear()
            if source.read(1) == b"\x03":
                return SerialMessage.parse(readed)

        else:
            raise IndexError

        return
