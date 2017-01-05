import logging

import sys

from meshnet.serio.messages import SerialMessageConsumer, SerialMessage, MessageType
from meshnet.serio.connection import LegacyConnection, MessageHandler, MessageWriter

logger = logging.getLogger(__name__)

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


class TestHandler(MessageHandler):
    def __init__(self, key: bytes):
        self._cnt = 0
        self.key = key

    def on_message(self, message: SerialMessage, writer: MessageWriter):
        if not message.verify(self.key):
            logger.warning("cannot verify checksum")
            return
        self._cnt += 1
        reply = SerialMessage(0, message.sender, MessageType.configure, None, message.session, self._cnt,
                              b'\x01\x01h\0')
        writer.put_packet(reply, self.key)

    def on_connect(self, writer: MessageWriter):
        pass

    def on_disconnect(self):
        pass


if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

    handler = TestHandler(KEY)
    conn = LegacyConnection(sys.argv[1])
    conn.register_handler(handler)
    consumer = SerialMessageConsumer()

    conn.connect()
    while True:
        conn.read(consumer)
