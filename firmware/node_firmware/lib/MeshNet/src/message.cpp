#include "message.hpp"

#include "common.hpp"

#include <SipHash_2_4.h>
#include <string.h>

static uint8_t *computeHash(const uint8_t *key, uint16_t sender,
                            uint16_t reciever, unsigned char type,
                            const char *message, uint8_t message_len) {
  sipHash.initFromRAM(key);

  sipHash.updateHash(sender & 0xff);
  sipHash.updateHash((sender >> 8) & 0xff);

  sipHash.updateHash(reciever & 0xff);
  sipHash.updateHash((reciever >> 8) & 0xff);

  sipHash.updateHash(msg_id & 0xff);
  sipHash.updateHash((msg_id >> 8) & 0xff);

  sipHash.updateHash(type);

  for (int i = 0; i < message_len; i++) {
    sipHash.updateHash((byte)message[i]);
  }
  sipHash.finish();
  return sipHash.result;
}

void Message::init(session_t session, counter_t counter) {
  pos = 1;
  len =
      PAYLOAD_LEN + sizeof(msg_size_t) + sizeof(session_t) + sizeof(counter_t);

  setShort(session);
  setShort(counter);
}

msg_size_t Message::finalize(const char *key, node_t from_node, node_t to_node,
                             type_t type) {

  uint8_t *hash = computeHash(key, from_node, to_node, type, buffer, pos);
  memcpy(buffer + pos, hash, HASH_LEN);

  buffer[0] = pos;

  len = pos + HASH_LEN;

  return len;
}

bool Message::verify(const char *key, node_t from, node_t to, type_t type,
                     uint16_t len) {
  if (len < SESSION_LEN + sizeof(counter_t) + HASH_LEN + sizeof(msg_size_t)) {
    DEBUG_LOG("Message too short: %d !", len);
    return false;
  }

  if (len != (buffer[0] + HASH_LEN)) {
    DEBUG_LOG("bytes read does not match len: len(%d) != read(%d)", buffer[0],
              len);
    return false;
  }

  len = buffer[0];
  pos = 1;

  uint8_t *hash = computeHash(key, from, to, type, buffer, len);

  uint8_t ref_hash = buffer + len;
  for (uint8_t i = 0; i < HASH_LEN; i++) {
    if (hash[i] != ref_hash[i]) {
      DEBUG_LOG("Byte %d of hash does not match: hash(%x) != ref(%x)", hash[i],
                ref[i]);
      return false;
    }
  }

  return true;
}
