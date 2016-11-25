#include "node.hpp"
#include "common.hpp"

#include "SoftwareReset.h"

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

uint16_t Node::fetch(uint16_t *sender, messages_t *type, Message *msg) {
  update();

  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    *sender = header.from_node;
    *type = (messages_t)header.type;

    // Read the data
    uint16_t size = network.read(header, msg->rawBuffer(), Message::maxLen());

    if (!msg->verify(key, header.from_node, header.to_node, header.type,
                     size)) {
      DEBUG_LOG("Cannot verify message");
      return -1;
    }

    session_t pkt_session = msg->getShort();
    if (pkt_session != session) {
      DEBUG_LOG("Wrong session: %d", pkt_session);
      return -1;
    }

    /**
      * reset the controller on reset request message.
      *
      * We do not check the counter here as the sender may
      * have restarted and lost its state.
      *
      * Should be enough to check the session id as this
      * can be adapted from the sender from our pong packets
      * and as we restart after this packet and negotiate a
      * new session replay should not be a problem.
      */
    if (*type == reset) {
      DEBUG_LOG("Reset controller after reset message");
      softwareReset(STANDARD);
    }

    /* check that we cot a packet with an incremented counter */
    counter_t pkt_cnt = msg->getShort();
    if (pkt_cnt <= other_id) {
      DEBUG_LOG("Wrong counter: %d", pkt_counter);
      return -1;
    }
    other_id = pkt_cnt;

    return msg->len();
  } else {
    return -1;
  }
}

bool Node::send(uint16_t reciever, messages_t type, Message *msg) {
  uint16_t len = msg->finalize(key, node_id, reciever, type);

  for (uint8_t i = 0; i < 3; ++i) {

    // Write the message
    if (mesh.write(reciever, msg->rawBuffer(), type, len)) {
      return true;
    } else {
      checkConn();
    }
  }
  return false;
}

void Node::setSession(session_t _session) {
  session = _session;
  own_id = 0;
  other_id = 0;
}

Message *Node::prepareSendMessage() {
  message.init(session, own_id++);
  return &message;
}

Message *Node::prepareRecieveMessage() {
  message.reset();
  return &message;
}
