#!/usr/bin/python3
import asyncio
import struct
import subprocess
from enum import Enum
import time

import logging
from functools import partial
from typing import Callable

import colorlog
from serial.aio import create_serial_connection

from meshnet.serio.connection import LegacyConnection, MessageHandler, MessageWriter, AioSerialConnection
from meshnet.serio.messages import MessageType, SerialMessageConsumer, SerialMessage

logger = logging.getLogger(__name__)

KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f'


def build_fakeio(port_a, port_b):
    return subprocess.Popen(["socat",
                             "PTY,link={:s},raw,wait-slave".format(port_a),
                             "PTY,link={:s},raw,wait-slave".format(port_b)])


class FakeState(Enum):
    new = 0
    configured = 1


class FakeDeviceMessage(SerialMessage):
    def _serio_header(self):
        return struct.pack(">HB", self.sender, self.msg_type.value)


class FakeRouter(MessageHandler):
    def __init__(self, key: bytes):
        self.key = key
        self.consumer = SerialMessageConsumer()
        self.devices = {}

    def register_device(self, device: 'FakeNode'):
        self.devices[device.node_id] = device

    def write_packet(self, writer: MessageWriter, message: SerialMessage):
        logger.debug("Write message: %s", message)
        writer.put_packet(message, self.key)

    def on_message(self, message: SerialMessage, writer: MessageWriter):
        sender = message.receiver
        message.receiver = message.sender
        message.sender = sender
        if not message.verify(KEY):
            logger.warning("cannot verify checksum")
            return

        if message.receiver not in self.devices:
            logger.warning("Unknown node id: %x", message.receiver)
            return

        dev = self.devices[message.receiver]
        dev.on_message(message, partial(self.write_packet, writer))

    def on_connect(self, writer: MessageWriter):
        for dev in self.devices.values():
            dev.on_connect(partial(self.write_packet, writer))

    def on_disconnect(self):
        pass


class FakeDevice(object):
    def __init__(self, dev_type):
        self.dev_type = dev_type


class FakeNode(object):
    def __init__(self, node_id: int):
        self.state = FakeState.new
        self.session = 0x12
        self._counter = 0
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

    def _dummy_handler(self, message: SerialMessage, write_func: Callable[[SerialMessage], None]):
        logger.warning("No actual handler for message %s defined.", message.msg_type.name)
        write_func(self.make_packet(MessageType.pong, b'1234'))

    def _config_handler(self):
        pass

    def on_message(self, message: SerialMessage, writer: FakeRouter):
        logger.info("Node %d got message %s", self.node_id, message)

        actual_handler = self.handlers.get(message.msg_type, self._dummy_handler)
        actual_handler(message, writer)

    def on_connect(self, write_func: Callable[[SerialMessage], None]):
        write_func(self.make_packet(MessageType.booted, b'1234'))

    @property
    def counter(self):
        tmp = self._counter
        self._counter = (self._counter + 1) % 0xffff
        return tmp

    def make_packet(self, msg_type: MessageType, payload: bytes) -> SerialMessage:
        return FakeDeviceMessage(self.node_id, 0, msg_type, counter=self.counter, payload=payload, session=self.session)


if __name__ == "__main__":
    handler = colorlog.StreamHandler()
    handler.setFormatter(colorlog.ColoredFormatter(
        '%(log_color)s%(levelname)s:%(name)s:%(message)s'))

    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG, handlers=[handler])

    proc = build_fakeio("/tmp/ttyS0", "/tmp/ttyS1")
    time.sleep(1)

    connection = AioSerialConnection()

    router = FakeRouter(KEY)
    router.register_device(FakeNode(1))
    connection.register_handler(router)

    loop = asyncio.get_event_loop()
    coro = create_serial_connection(loop, connection, "/tmp/ttyS0", baudrate=115200)
    loop.run_until_complete(coro)
    loop.run_forever()
    loop.close()
