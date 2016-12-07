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


class Connection(object):
    def __init__(self, device):
        self._device = device
        self._conn = None

    def connect(self):
        self._conn = serial.Serial(self._device, 115200)
