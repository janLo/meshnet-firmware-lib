import asyncio
import logging
import sys

import colorlog
from serial.aio import create_serial_connection

from meshnet.serio.connection import MessageHandler, MessageWriter, AioSerialConnection
from meshnet.serio.messages import SerialMessage, MessageType

logger = logging.getLogger(__name__)

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


class TestHandler(MessageHandler):
    def __init__(self, key: bytes):
        self._cnt = 0
        self.key = key

    def next_cnt(self):
        self._cnt = (self._cnt + 1) % 0xffff
        return self._cnt

    def on_message(self, message: SerialMessage, writer: MessageWriter):
        if not message.verify(self.key):
            logger.warning("cannot verify checksum")
            return
        reply = SerialMessage(0, message.sender, MessageType.configure, None, message.session, self.next_cnt(),
                              b'\x01\x01h\0')
        writer.put_packet(reply, self.key)

    def on_connect(self, writer: MessageWriter):
        pass

    def on_disconnect(self):
        pass


if __name__ == "__main__":
    handler = colorlog.StreamHandler()
    handler.setFormatter(colorlog.ColoredFormatter(
        '%(log_color)s%(levelname)s:%(name)s:%(message)s'))

    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG, handlers=[handler])

    handler = TestHandler(KEY)

    conn = AioSerialConnection()
    conn.register_handler(handler)

    loop = asyncio.get_event_loop()
    coro = create_serial_connection(loop, conn, sys.argv[1], baudrate=115200)
    loop.run_until_complete(coro)
    loop.run_forever()
    loop.close()
