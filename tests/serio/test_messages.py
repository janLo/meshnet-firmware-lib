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

    def test_repr(self):
        message = SerialMessage(0, 1, MessageType.booted, None, 12, 1, b"jsif")
        self.assertEqual(repr(message),
                         "SerialMessage<sender:0, receiver=1, type=MessageType.booted, session=12, counter=1, hash=<not_calculated>, payload=b'jsif'>")

        message.hash_sum = message._compute_hash(KEY)
        self.assertEqual(repr(message),
                         "SerialMessage<sender:0, receiver=1, type=MessageType.booted, session=12, counter=1, hash=ae 9a c1 53 88 9d bc a4, payload=b'jsif'>")

    def test_verify(self):
        message = SerialMessage(0, 1, MessageType.booted, b'\xae\x9a\xc1\x53\x88\x9d\xbc\xa4', 12, 1, b"jsif")
        self.assertTrue(message.verify(KEY))
        message = SerialMessage(0, 1, MessageType.booted, b'\xae\x9a\xc1\x53\x99\x9d\xbc\xa4', 12, 1, b"jsif")
        self.assertFalse(message.verify(KEY))


class TestSerialMessageConsumer(unittest.TestCase):
    pass
