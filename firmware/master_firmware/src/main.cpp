#include "Arduino.h"

#include "RF24.h"
#include "RF24Mesh.h"
#include "RF24Network.h"
#include <SPI.h>
// Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#include "config.h"

RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setup() {
  Serial.begin(115200);

  mesh.setNodeID(0);

  // Connect to the mesh
  mesh.begin();
}

void loop() {
  char dat[64];

  mesh.update();
  mesh.DHCP();

  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    Serial.write(header.from_node);
    Serial.write(header.type);
    uint8_t len = network.read(header, &dat, sizeof(dat));
    Serial.write(len);
    Serial.write(dat, len);
    Serial.write('\n');
  }

  if (Serial.available() > 4) {
    uint16_t len = Serial.readBytesUntil('\n', dat, sizeof(dat));
    if (len < 4 && len < 4 + dat[3]) {
      return;
    }
    uint16_t node = dat[1];
    node |= (dat[0] << 8) & 0xff;

    uint8_t type = dat[2];
    uint8_t data_len = dat[3];

    mesh.write(dat + 4, type, data_len, node);
  }
}
