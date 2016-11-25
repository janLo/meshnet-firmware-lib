#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "config.h"

/**
  * Message:
  * [ len | session | counter | payload | hash ]
  */

class Message {
  char buffer[MAX_MESSAGE_LEN];
  uint8_t pos;
  uint8_t len;

  bool inBound() { return pos < len; }

public:
  void finalize(node_t from, node_t to, type_t type);
  bool verify(const char *key, node_t from, node_t to, type_t type,
              uint16_t len);
  void init(session_t session, counter_t cnt);

  msg_size_t len() { return len; }
  msg_size_t remain() { return len - pos; }

  uint8_t getByte() { return buffer[pos++]; }
  void setByte(uint8_t byte) { buffer[pos++] = byte; }

  uint16_t getShort() { return ((buffer[pos++] << 8) & 0xff) | buffer[pos]; }
  void setShort(uint16_t data) {
    buffer[pos++] = (data >> 8) & 0xff;
    buffer[pos++] = (data & 0xff);
  }

  char *getBytes(uint8_t len) {
    uint8_t tmp = pos;
    pos = tmp + len;
    return buffer + tmp;
  }

  char *rawBuffer() { return buffer; }

  static msg_size_t maxLen() { return MAX_MESSAGE_LEN; }
};

#endif /* end of include guard: _MESSAGE_H_ */