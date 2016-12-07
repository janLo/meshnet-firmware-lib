from enum import Enum

import serial
import struct
from siphashc import siphash


def _hash(key: bytes, sender: int, receiver: int, msg_type: int, data: bytes):
    packed_data = struct.pack(">hhB", sender, receiver, msg_type) + data
    return struct.pack(">Q", siphash(key, packed_data))





class SerialMessage(object):
    def __init__(self):
        pass

    def serialize(self):
        pass


class _MessageState(Enum):
    preamble = 0
    start = 1
    length = 2
    message = 4
    end = 5


class MessageConsumer(object):
    def __init__(self):
        self._state = _MessageState.preamble
        self._preamble_cnt = 0
        self._read_bytes = bytearray()
        self._to_read = 0
        self._actual_read = 0

    def consume(self, source, max_len):
        assert max_len >= 1

        self._to_read = 0

        if self._state == _MessageState.preamble:
            self._read_bytes = self._read_bytes[-1:] + source.read(1)
            if self._read_bytes == b"\xAF\xAF":
                self._state = _MessageState.start

        elif self._state == _MessageState.start:
            if source.read(1) == b"\x02":
                self._state == _MessageState.length
            else:
                self._state = _MessageState.preamble

        elif self._state == _MessageState.length:
            self._to_read = struct.unpack("B", source.read(1))[0]
            self._state = _MessageState.end
            self._read_bytes = bytearray()

        elif self._state == _MessageState.message:
            self._read_bytes.append(min(max_len, self._to_read - len(self._read_bytes)))
            if len(self._read_bytes) == self._to_read:
                self._state == _MessageState.end

        elif self._state == _MessageState.message:
            if source.read(1) == b"\x03":
                pass
                # XXX Emit packet here

            self._state = _MessageState.preamble
        else:
            raise IndexError

        return


class Connection(object):
    def __init__(self, device):
        self._device = device
        self._conn = None

    def connect(self):
        self._conn = serial.Serial(self._device, 115200)
