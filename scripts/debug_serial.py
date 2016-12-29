#!/usr/bin/python3
import struct
import subprocess
from enum import Enum
import time

import logging
import colorlog

from meshnet.serio.connection import LegacyConnection
from meshnet.serio.messages import MessageType, SerialMessageConsumer, SerialMessage

logger = logging.getLogger(__name__)

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


def build_fakeio(portA, portB):
    return subprocess.Popen(["socat",
                             "PTY,link={:s},raw,wait-slave".format(portA),
                             "PTY,link={:s},raw,wait-slave".format(portB)])


class FakeState(Enum):
    new = 0
    configured = 1


class FakeDeviceMessage(SerialMessage):
    def _serio_header(self):
        return struct.pack(">HB", self.sender, self.msg_type.value)


class FakeRouter(object):
    def __init__(self, tty_dev: str, key: bytes):
        self.key = key
        self.conn = LegacyConnection(tty_dev)
        self.consumer = SerialMessageConsumer()

        self.devices = {}

    def register_device(self, device: 'FakeNode'):
        self.devices[device.node_id] = device

    def write_packet(self, message: SerialMessage):
        logger.debug("Write message: %s", message)
        self.conn.write(message, self.key)

    def run(self):
        self.conn.connect()
        for dev in self.devices.values():
            dev.on_connect(self)

        while True:
            pkt = self.conn.read(self.consumer)
            if pkt is not None:
                sender = pkt.receiver
                pkt.receiver = pkt.sender
                pkt.sender = sender
                if not pkt.verify(KEY):
                    logger.warning("cannot verify checksum")
                    continue

                if pkt.receiver not in self.devices:
                    logger.warning("Unknown node id: %x", pkt.receiver)
                    continue

                dev = self.devices[pkt.receiver]
                dev.on_message(pkt, self)
            else:
                time.sleep(0.3)


class FakeDevice(object):
    def __init__(self, dev_type):
        self.dev_type = dev_type


class FakeNode(object):
    def __init__(self, node_id: int):
        self.state = FakeState.new
        self.session = 0x12
        self.counter = 0
        self.node_id = node_id

        self.items = []

        self.handlers = {
            MessageType.booted: self._dummy_handler,
            MessageType.configure: self._dummy_handler,
            MessageType.configured: self._dummy_handler,
            MessageType.set_state: self._dummy_handler,
            MessageType.get_state: self._dummy_handler,
            MessageType.reading: self._dummy_handler,
            MessageType.ping: self._dummy_handler,
            MessageType.pong: self._dummy_handler,
            MessageType.reset: self._dummy_handler,
        }

    def _dummy_handler(self, message: SerialMessage, writer: FakeRouter):
        logger.warning("No actual handler for message %s defined.", message.msg_type.name)
        writer.write_packet(self.make_packet(MessageType.pong, b'1234'))

    def _config_handler(self):
        pass

    def on_message(self, message: SerialMessage, writer: FakeRouter):
        logger.info("Node %d got message %s", self.node_id, message)

        handler = self.handlers.get(message.msg_type, self._dummy_handler)
        handler(message, writer)

    def on_connect(self, writer: FakeRouter):
        writer.write_packet(self.make_packet(MessageType.booted, b'1234'))

    def make_packet(self, msg_type: MessageType, payload: bytes) -> SerialMessage:
        cnt = self.counter
        self.counter += 1
        return FakeDeviceMessage(self.node_id, 0, msg_type, counter=cnt, payload=payload, session=self.session)


if __name__ == "__main__":
    handler = colorlog.StreamHandler()
    handler.setFormatter(colorlog.ColoredFormatter(
        '%(log_color)s%(levelname)s:%(name)s:%(message)s'))

    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG, handlers=[handler])

    proc = build_fakeio("/tmp/ttyS0", "/tmp/ttyS1")
    time.sleep(1)

    router = FakeRouter("/tmp/ttyS0", KEY)
    router.register_device(FakeNode(1))

    router.run()
