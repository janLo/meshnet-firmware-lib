#ifndef _NODE_HPP_
#define _NODE_HPP_

#include <Arduino.h>

#include <RF24Network.h>
#include <RF24.h>
#include <RF24Mesh.h>
#include <SPI.h>
//Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#include "protocol.hpp"

#define CE_PIN 7
#define CS_PIN 8

class Node {
protected:
  RF24 radio;
  RF24Network network;
  RF24Mesh mesh;
  const uint8_t node_id;

  virtual void update();
  virtual void checkConn();

public:
  Node(uint8_t node_id);

  void init();
  uint16_t fetch(uint16_t *sender, messages_t * type, char * buffer, uint16_t len);
  bool send(uint16_t reciever, messages_t type, char* buffer, uint16_t len);
};

class Master : public Node {
protected:
  virtual void update();
  virtual void checkConn();

public:
  Master() : Node(0) { }

};


#endif /* end of include guard: _NODE_HPP_ */
