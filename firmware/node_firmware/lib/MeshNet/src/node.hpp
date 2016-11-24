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

#include "protocol.hpp"

class Node {
protected:
  RF24 radio;
  RF24Network network;
  RF24Mesh mesh;
  const uint8_t node_id;
  const uint8_t *key;

  session_t session;
  uint16_t last_id;
  uint16_t next_id;

  void update();
  void checkConn();

public:
  Node(uint8_t node_id, const uint8_t *key);

  void init();
  uint16_t fetch(uint16_t *sender, messages_t *type, char *buffer,
                 uint16_t len);
  bool send(uint16_t reciever, messages_t type, char *buffer, uint16_t len);
  void setSession(uint16_t node_id, session_t _session);
};

#endif /* end of include guard: _NODE_HPP_ */
