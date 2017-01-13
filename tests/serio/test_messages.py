import unittest

from meshnet.serio.messages import SerialMessage, MessageType

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


class TestSerialMessage(unittest.TestCase):
    def test_serialize(self):
        message = SerialMessage(0, 1, MessageType.booted, None, 12, 1, b"jsif")
        self.assertEqual(b"\x00\x01F\t\x00\x0c\x00\x01jsif\xae\x9a\xc1S\x88\x9d\xbc\xa4",
                         message.serialize(KEY))

    def test_framed(self):
        message = SerialMessage(0, 1, MessageType.booted, None, 12, 1, b"jsif")
        self.assertEqual(b"\xaf\xaf\x02\x14\x00\x01F\t\x00\x0c\x00\x01jsif\xae\x9a\xc1S\x88\x9d\xbc\xa4\x03",
                         message.framed(KEY))


class TestSerialMessageConsumer(unittest.TestCase):
    pass
