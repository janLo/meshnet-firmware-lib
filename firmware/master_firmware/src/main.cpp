#include "Arduino.h"

#include "RF24.h"
#include "RF24Mesh.h"
#include "RF24Network.h"
#include <SPI.h>
// Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setup() {
  Serial.begin(115200);

  mesh.setNodeID(0);

  // Connect to the mesh
  mesh.begin();
}

void loop() {

  mesh.update();
  mesh.DHCP();
}
