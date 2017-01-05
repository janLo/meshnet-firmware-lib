import abc
import asyncio
import logging
from typing import List

import serial

from meshnet.serio.util import to_hex

from meshnet.serio.messages import SerialMessageConsumer, SerialMessage

logger = logging.getLogger(__name__)


class SerialBuffer(object):
    def __init__(self):
        self._buff = bytearray()

    def put(self, data):
        if isinstance(data, (bytes, bytearray)):
            self._buff.extend(data)
        else:
            self._buff.append(data)

    def read(self, max_bytes):
        ret = self._buff[:max_bytes]
        self._buff = self._buff[max_bytes:]

        return bytes(ret)

    def available(self):
        return len(self._buff)


class MessageWriter(object, metaclass=abc.ABCMeta):
    @abc.abstractmethod
    def put_packet(self, packet: SerialMessage, key: bytes):
        pass


class MessageHandler(object, metaclass=abc.ABCMeta):
    @abc.abstractmethod
    def on_message(self, message: SerialMessage, writer: MessageWriter):
        pass

    @abc.abstractmethod
    def on_connect(self, writer: MessageWriter):
        pass

    @abc.abstractmethod
    def on_disconnect(self):
        pass


class AioSerialConnection(asyncio.Protocol, MessageWriter):
    def __init__(self):
        self._handlers = []  # type: List[MessageHandler]

        self._consumer = SerialMessageConsumer()
        self.transport = None
        self._buffer = SerialBuffer()

    def __call__(self):
        return self

    def register_handler(self, handler: MessageHandler):
        self._handlers.append(handler)

    def connection_made(self, transport):
        self.transport = transport
        logger.info('serial port opened: %s', transport)
        for handler in self._handlers:
            handler.on_connect(self)

    def data_received(self, data):
        logger.debug('data received: %s', to_hex(data))
        self._buffer.put(data)
        while self._buffer.available() > 0:
            packet = self._consumer.consume(self._buffer, max_len=self._buffer.available())
            if packet is not None:
                self._on_packet(packet)

    def _on_packet(self, packet):
        for handler in self._handlers:
            handler.on_message(packet, self)

    def put_packet(self, message: SerialMessage, key: bytes):
        out = message.framed(key)
        logger.debug("write data: %s", to_hex(out))
        self.transport.write(out)

    def connection_lost(self, exc):
        logger.warning("Serial port closed!")
        for handler in self._handlers:
            handler.on_disconnect()

    def pause_writing(self):
        logger.debug('pause writing, buffer=%d', self.transport.get_write_buffer_size())

    def resume_writing(self):
        logger.debug('resume writing, buffer=%d', self.transport.get_write_buffer_size())


class LegacyConnection(MessageWriter):
    def __init__(self, device):
        self._device = device
        self._conn = None
        self._consumer = SerialMessageConsumer()

        self._handlers = []  # type: List[MessageHandler]

    def register_handler(self, handler):
        self._handlers.append(handler)

    def connect(self):
        logger.info("Connect to %s", self._device)
        self._conn = serial.Serial(self._device, 115200, timeout=1)
        for handler in self._handlers:
            handler.on_connect(self)

    def read(self) -> bool:
        if self._conn.in_waiting == 0:
            return False
        pkt = self._consumer.consume(self._conn, self._conn.in_waiting)
        if pkt is not None:
            for handler in self._handlers:
                handler.on_message(pkt, self)
            return True
        return False

    def put_packet(self, message: SerialMessage, key: bytes):
        out = message.framed(key)
        self._conn.write(out)
        self._conn.flush()
