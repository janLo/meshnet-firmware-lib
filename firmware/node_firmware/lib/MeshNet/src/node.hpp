#ifndef _NODE_HPP_
#define _NODE_HPP_

#include "config.h"

#include <Arduino.h>

#include <RF24.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include <SPI.h>
// Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#include "items.hpp"
#include "message.hpp"
#include "protocol.hpp"

class Node {
protected:
  RF24 radio;
  RF24Network network;
  RF24Mesh mesh;
  const uint8_t node_id;
  const uint8_t *key;

  session_t session;
  uint16_t own_id;
  uint16_t other_id;
  uint16_t last_pong;

  ItemRegistry registry;
  Message message;

  void update();
  void checkConn();
  void sendPong();

public:
  Node(uint8_t node_id, const uint8_t *key);

  void init();
  msg_size_t fetch(uint16_t *sender, messages_t *type, Message *msg);
  bool send(uint16_t reciever, messages_t type, Message *msg);
  void setSession(session_t _session);

  Message *prepareSendMessage();
  Message *prepareRecieveMessage();

  void process();
};

#endif /* end of include guard: _NODE_HPP_ */
