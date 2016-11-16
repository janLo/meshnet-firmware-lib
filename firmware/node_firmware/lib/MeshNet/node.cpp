#include "node.hpp"

Node::Node(uint8_t node_id)
  : radio(CE_PIN, CS_PIN), network(radio), mesh(radio, network), node_id(node_id)
{}


void Node::init() {
  mesh.setNodeID(0);
  mesh.begin();
}


void Node::update() {
  mesh.update();
}

void Node::checkConn() {
  if ( ! mesh.checkConnection() ) {
    //refresh the network address
    mesh.renewAddress();
  }
}


uint16_t Node::fetch(uint16_t * sender, messages_t * type, char * buffer, uint16_t len) {
  update();

  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    *sender = header.from_node;
    *type = (messages_t) header.type;
    return network.read(header, buffer, len);
  } else {
      return -1;
  }
}

bool Node::send(uint16_t reciever, messages_t type, char *buffer, uint16_t len) {
  for (uint8_t i = 0; i < 3; ++i) {
    if (mesh.write(reciever, buffer, type, len)) {
      return true;
    } else {
      checkConn();
    }
  }
  return false;
}


void Master::update() {
  mesh.update();
  mesh.DHCP();
}

void Master::checkConn() {}
