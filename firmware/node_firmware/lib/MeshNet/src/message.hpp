#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "config.h"

/**
  * Message:
  * [ len | session | counter | payload | hash ]
  */

class Message {
  char _buffer[MAX_MESSAGE_LEN];
  uint8_t _pos;
  uint8_t _len;

  bool inBound() { return _pos < _len; }

public:
  void finalize(node_t from, node_t to, type_t type);
  bool verify(const char *key, node_t from, node_t to, type_t type,
              uint16_t len);
  void init(session_t session, counter_t cnt);

  msg_size_t len() { return _len; }
  msg_size_t remain() { return _len - _pos; }

  uint8_t getByte() { return _buffer[_pos++]; }
  void setByte(uint8_t byte) { _buffer[_pos++] = byte; }

  uint16_t getShort() {
    return ((_buffer[_pos++] << 8) & 0xff) | _buffer[_pos++];
  }
  void setShort(uint16_t data) {
    _buffer[_pos++] = (data >> 8) & 0xff;
    _buffer[_pos++] = (data & 0xff);
  }

  char *getBytes(uint8_t len) {
    uint8_t tmp = _pos;
    _pos = tmp + len;
    return _buffer + tmp;
  }

  char *rawBuffer() { return _buffer; }

  static msg_size_t maxLen() { return MAX_MESSAGE_LEN; }
};

#endif /* end of include guard: _MESSAGE_H_ */
