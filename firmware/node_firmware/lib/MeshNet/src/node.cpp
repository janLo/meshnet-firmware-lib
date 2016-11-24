#include "node.hpp"
#include "common.hpp"

#include <SipHash_2_4.h>

static uint8_t *computeHash(const uint8_t *key, uint16_t sender,
                            uint16_t reciever, unsigned char type,
                            uint16_t msg_id, const char *message,
                            uint8_t message_len) {
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

Node::Node(uint8_t node_id, const uint8_t *key)
    : radio(CE_PIN, CS_PIN), network(radio), mesh(radio, network),
      node_id(node_id), key(key), session(0) {}

void Node::init() {
  mesh.setNodeID(0);
  mesh.begin();
}

void Node::update() { mesh.update(); }

void Node::checkConn() {
  if (!mesh.checkConnection()) {
    // refresh the network address
    mesh.renewAddress();
  }
}

uint16_t Node::fetch(uint16_t *sender, messages_t *type, char *buffer,
                     uint16_t len) {
  update();

  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    *sender = header.from_node;
    *type = (messages_t)header.type;

    // Read the data
    uint16_t size = network.read(header, buffer, len);

    // Check that we have at least the hash and the session_id
    if (size < (HASH_LEN + SESSION_LEN)) {
      DEBUG_LOG("Message too short: %d", size);
      return -1;
    }

    // Compute the hash
    uint8_t *hash =
        computeHash(key, header.from_node, header.to_node, header.type,
                    header.id, buffer, size - HASH_LEN);

    // Compare hash from packet with sent hash
    const char *send_hash = buffer + (len - HASH_LEN);
    for (uint8_t pos = 0; pos < 8; pos++) {
      if (hash[pos] != send_hash[pos]) {
        DEBUG_LOG("Hash does not match!");
        return -1;
      }
    }

    // Check the session id
    const char *send_session_id = buffer + (len - HASH_LEN - SESSION_LEN);
    for (uint8_t pos = 0; pos < SESSION_LEN; ++pos) {
      if ((uint8_t)send_session_id[pos] != ((session >> pos) & 0xff)) {
        DEBUG_LOG("Session id does not match!");
        return -1;
      }
    }

    return size;
  } else {
    return -1;
  }
}

bool Node::send(uint16_t reciever, messages_t type, char *buffer,
                uint16_t len) {

  if (len <= PAYLOAD_LEN + SESSION_LEN) {
    DEBUG_LOG("Buffer too small")
    return false;
  }

  // Write session id before the hash
  char *meta_buffer = buffer + (len - HASH_LEN - SESSION_LEN);
  for (uint8_t pos = 0; pos < SESSION_LEN; ++pos) {
    meta_buffer[pos] = (session >> pos) & 0xff;
  }

  for (uint8_t i = 0; i < 3; ++i) {
    // Compute hash
    uint8_t *hash =
        computeHash(key, node_id, reciever, type, RF24NetworkHeader::next_id,
                    buffer, len - HASH_LEN);

    // Put hash in the buffer
    for (uint8_t pos = 0; pos < 8; ++pos) {
      meta_buffer[pos + SESSION_LEN] = hash[pos];
    }

    // Write the message
    if (mesh.write(reciever, buffer, type, len)) {
      return true;
    } else {
      checkConn();
    }
  }
  return false;
}

/*
bool Node::idValid(uint16_t id) {
  if (id <= last_id) {
    return false;
  } else {
    last_id = id;
    return true;
  }
}
*/
void Node::setSession(uint16_t node_id, session_t _session) {
  session = _session;

  // Reset the frame id to prevent overflow.
  RF24NetworkHeader::next_id = 1;
}
