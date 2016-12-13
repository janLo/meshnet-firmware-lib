import asyncio

import logging

from meshnet.serial.messages import SerialMessageConsumer

logger = logging.getLogger(__name__)


class SerialBuffer(object):
    def __init__(self):
        self._buff = bytearray()

    def put(self, data):
        self._buff.append(data)

    def read(self, max_bytes):
        ret = self._buff[:max_bytes]
        self._buff = self._buff[max_bytes:]

        return bytes(ret)

    def available(self):
        return len(self._buff)


class AioSerial(asyncio.Protocol):
    def __init__(self):
        self._consumer = SerialMessageConsumer()
        self.transport = None
        self._buffer = SerialBuffer()

    def connection_made(self, transport):
        self.transport = transport
        logger.info('serial port opened: %s', transport)

    def data_received(self, data):
        logger.debug('data received', repr(data))
        self._buffer.put(data)
        while self._buffer.available() > 0:
            packet = self._consumer.consume(self._buffer, max_len=self._buffer.available())
            if packet is not None:
                self._on_packet(packet)

    def _on_packet(self, packet):
        # XXX call packet handlers here
        pass

    def connection_lost(self, exc):
        logger.warning("Serial port closed!")

    def pause_writing(self):
        logger.debug('pause writing, buffer=%d', self.transport.get_write_buffer_size())

    def resume_writing(self):
        logger.debug('resume writing, buffer=%d', self.transport.get_write_buffer_size())
