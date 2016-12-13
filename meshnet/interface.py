import logging

import sys

from meshnet.serial.messages import SerialMessageConsumer, SerialMessage, MessageType
from meshnet.serial.connection import LegacyConnection

logger = logging.getLogger(__name__)

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'

if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

    conn = LegacyConnection(sys.argv[1])
    consumer = SerialMessageConsumer()

    conn.connect()
    cnt = 0
    while True:
        pkt = conn.read(consumer)
        if pkt is not None:
            if not pkt.verify(KEY):
                logger.warning("cannot verify checksum")
                continue
            print(pkt)
            cnt += 1
            reply = SerialMessage(0, pkt.sender, MessageType.configure, None, pkt.session, cnt, b'\x01\x01h\0')
            conn.write(reply, KEY)
