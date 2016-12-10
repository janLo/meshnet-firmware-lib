#include "message.hpp"

#include "common.hpp"

#include <SipHash_2_4.h>
#include <string.h>

static uint8_t *computeHash(const uint8_t *key, uint16_t sender,
                            uint16_t reciever, unsigned char type,
                            const uint8_t *message, uint8_t message_len) {

  DEBUG_LOG("Hash %d bytes (%d bytes content)",
            message_len + sizeof(node_t) + sizeof(node_t) + 1, message_len);

#ifndef NO_DEBUG
  {
    Serial.write(" -- Content:");
    char tmp[10];
    uint8_t __i;
    for (__i = 0; __i < message_len; ++__i) {
      snprintf(tmp, 10, "0x%02x ", message[__i] & 0xff);
      Serial.write(tmp);
    }
    Serial.write('\n');
  }
#endif

  sipHash.initFromRAM(key);

  sipHash.updateHash((sender >> 8) & 0xff);
  sipHash.updateHash(sender & 0xff);

  sipHash.updateHash((reciever >> 8) & 0xff);
  sipHash.updateHash(reciever & 0xff);

  sipHash.updateHash(type);

  for (int i = 0; i < message_len; i++) {
    sipHash.updateHash((byte)message[i]);
  }
  sipHash.finish();

#ifndef NO_DEBUG
  {
    Serial.write(" -- Hash:");
    char tmp[10];
    uint8_t __i;
    for (__i = 0; __i < 8; ++__i) {
      snprintf(tmp, 10, "0x%02x ", sipHash.result[__i] & 0xff);
      Serial.write(tmp);
    }
    Serial.write('\n');
  }
#endif

  return sipHash.result;
}

void Message::init(session_t session, counter_t counter) {
  _pos = 1;
  _len =
      PAYLOAD_LEN + sizeof(msg_size_t) + sizeof(session_t) + sizeof(counter_t);

  setShort(session);
  setShort(counter);
}

msg_size_t Message::finalize(const uint8_t *key, node_t from_node,
                             node_t to_node, type_t type) {

  _buffer[0] = _pos;
  uint8_t *hash = computeHash(key, from_node, to_node, type, _buffer, _pos);
  memcpy(_buffer + _pos, hash, HASH_LEN);

  _len = _pos + HASH_LEN;

  return _len;
}

bool Message::verify(const uint8_t *key, node_t from, node_t to, type_t type,
                     uint16_t len) {
  if (len < SESSION_LEN + sizeof(counter_t) + HASH_LEN + sizeof(msg_size_t)) {
    DEBUG_LOG("Message too short: %d !", len);
    return false;
  }

  if (len != (uint16_t)(_buffer[0] + HASH_LEN)) {
    DEBUG_LOG("bytes read does not match len: len(%d) != read(%d)", _buffer[0],
              len);
    return false;
  }

  _len = _buffer[0];
  _pos = 1;

  uint8_t *hash = computeHash(key, from, to, type, _buffer, _len);

  uint8_t *ref_hash = (uint8_t *)_buffer + _len;
  for (uint8_t i = 0; i < HASH_LEN; i++) {
    if (hash[i] != ref_hash[i]) {
      DEBUG_LOG("Byte %d of hash does not match: hash(%x) != ref(%x)", hash[i],
                ref_hash[i]);
      return false;
    }
  }

  return true;
}
