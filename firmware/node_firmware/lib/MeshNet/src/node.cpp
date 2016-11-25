#include "node.hpp"
#include "common.hpp"

#include "SoftwareReset.h"

Node::Node(uint8_t node_id, const uint8_t *key)
    : radio(CE_PIN, CS_PIN), network(radio), mesh(radio, network),
      node_id(node_id), key(key), session(micros()), own_id(0), other_id(0),
      last_pong(millis()) {}

void Node::init() {
  mesh.setNodeID(0);
  mesh.begin();

  update();

  Message *boot_message = prepareSendMessage();
  boot_message->setShort(millis());

  if (!send(MASTER_ADDR, booted, boot_message)) {
    // Mesh not available? try again.
    softwareReset(STANDARD);
  }

  uint16_t last = millis();

  for (;;) {
    Message *recieve_message = prepareRecieveMessage();
    node_t sender;
    messages_t type;

    if (0 != fetch(&sender, &type, recieve_message)) {
      last = millis();
      switch (type) {
      case configure:
        registry.configure(recieve_message);
        sendPong();
        break;
      case configured:
        setSession(recieve_message->getShort());
        sendPong();
        return;
      default:
        break;
      }
      continue;
    }

    if (last + CONFIG_TIMEOUT < millis()) {
      // reset if we did not get an answer.
      softwareReset(STANDARD);
    }
  }
}

void Node::update() { mesh.update(); }

void Node::checkConn() {
  if (!mesh.checkConnection()) {
    // refresh the network address
    mesh.renewAddress();
  }
}

msg_size_t Node::fetch(uint16_t *sender, messages_t *type, Message *msg) {
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
      return 0;
    }

    session_t pkt_session = msg->getShort();
    if (pkt_session != session) {
      DEBUG_LOG("Wrong session: %d", pkt_session);
      return 0;
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
      return 0;
    }
    other_id = pkt_cnt;

    return msg->len();
  } else {
    return 0;
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

void Node::process() {
  update();

  // Handle incomming packets
  {
    Message *recieved = prepareRecieveMessage();
    uint16_t sender;
    messages_t type;
    msg_size_t recieved_len = fetch(&sender, &type, recieved);

    if (recieved_len > 0) {
      // XXX process packet
    }
  }

  // Send pong
  if (last_pong + PONG_INTERVAL < millis()) {
    // XXX Send pong
    last_pong = millis();
  }

  // Send out state
  Message *state_packet = prepareSendMessage();
  if (registry.nextState(state_packet)) {
    return;
  }

  // Update state of items
  registry.checkItems();
}

void Node::sendPong() {
  Message *pong_message = prepareSendMessage();
  pong_message->setShort(millis());
  send(MASTER_ADDR, pong, pong_message);
}
