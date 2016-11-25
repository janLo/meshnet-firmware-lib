#include "node.hpp"
#include "common.hpp"

Node::Node(uint8_t node_id, const uint8_t *key)
    : radio(CE_PIN, CS_PIN), network(radio), mesh(radio, network),
      node_id(node_id), key(key), session(0), own_id(0), other_id(0) {}

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

uint16_t Node::fetch(uint16_t *sender, messages_t *type, Message &msg) {
  update();

  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    *sender = header.from_node;
    *type = (messages_t)header.type;

    // Read the data
    uint16_t size = network.read(header, msg, Message::maxLen());

    if (!msg.verify(key, header.from_node, header.to_node, header.type, size)) {
      DEBUG_LOG("Cannot verify message");
      return -1;
    }

    if ((session_t pkt_session = msg.getShort()) != session) {
      DEBUG_LOG("Wrong session: %d", pkt_session);
      return -1;
    }

    counter_t pkt_cnt = msg.getShort();
    if (pkt_cnt <= other_id) {
      DEBUG_LOG("Wrong counter: %d", pkt_counter);
      return -1;
    }
    other_id = pkt_cnt;

    return msg.len();
  } else {
    return -1;
  }
}

bool Node::send(uint16_t reciever, messages_t type, Message &msg) {
  msg.finalize(node_id, reciever, type);

  for (uint8_t i = 0; i < 3; ++i) {

    // Write the message
    if (mesh.write(reciever, msg.rawBuffer(), type, msg.len())) {
      return true;
    } else {
      checkConn();
    }
  }
  return false;
}

void Node::setSession(uint16_t node_id, session_t _session) {
  session = _session;
  own_id = 0;
  other_id = 0;
}

Message *Node::getMessage() {
  message.init(session, own_id++);
  return &message;
}
