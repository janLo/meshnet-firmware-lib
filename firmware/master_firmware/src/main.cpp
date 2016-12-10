#include "Arduino.h"

#include <RF24.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
#include <SPI.h>
// Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#include "config.h"

RF24 radio(CE_PIN, CS_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

typedef enum { preamble, start, len, end } ser_state_t;

char dat_net[64];
char dat_serial[64];
ser_state_t ser_state;
uint8_t serial_len;
uint8_t serial_pos;

uint8_t preamble_buff[2];
uint8_t preamble_cnt = 0;

void setup() {
  Serial.begin(115200);

  // if (mesh.getNodeID() != MASTER_ADDR) {
  mesh.setNodeID(MASTER_ADDR);
  //}

  // Connect to the mesh
  mesh.begin();
}

void sendData(const char *buffer, uint8_t buffer_len) {
  uint16_t node = ((buffer[0] << 8) & 0xff) | buffer[1];
  uint8_t type = buffer[2];

  for (uint8_t i = 0; i < mesh.addrListTop; i++) {
    if (mesh.addrList[i].nodeID == node) {
      RF24NetworkHeader header(mesh.addrList[i].address, type);
      network.write(header, buffer + 3, buffer_len - 3);
      break;
    }
  }
}

void loop() {

  mesh.update();
  mesh.DHCP();
  delay(2);
  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
    uint8_t pkt_len = network.read(header, dat_net, sizeof(dat_net));
    uint16_t node_id = mesh.getNodeID(header.from_node);

    // Preamble
    Serial.write(0xaf);
    Serial.write(0xaf);

    // Start
    Serial.write(0x02);

    // Data
    Serial.write((pkt_len + sizeof(node_id) + sizeof(header.type)) & 0xff);
    Serial.write((node_id >> 8) & 0xff);
    Serial.write(node_id & 0xff);
    Serial.write(header.type);
    Serial.write(dat_net, pkt_len);

    // End
    Serial.write(0x03);
  }

  for (;;) {
    int available = Serial.available();

    if (available > 0) {

      switch (ser_state) {
      case preamble:
        preamble_cnt = (preamble_cnt + 1) % 2;
        preamble_buff[preamble_cnt] = Serial.read();

        if (preamble_buff[0] == 0xaf && preamble_buff[1] == 0xaf) {
          preamble_buff[0] = 0x00;
          preamble_buff[1] = 0x00;
          ser_state = start;
        }
        break;
      case start:
        if (Serial.read() == 0x02) {
          ser_state = len;
        } else {
          ser_state = preamble;
        }
        break;
      case len:
        serial_len = Serial.read() + 1;
        serial_pos = 0;
        ser_state = end;
        break;
      case end:

        serial_pos += Serial.readBytes(dat_serial + serial_pos,
                                       min(serial_len - serial_pos, available));
        if (serial_pos == serial_len) {
          ser_state = preamble;
          if (dat_serial[serial_len - 1] != 0x03) {
            return;
          }
          sendData(dat_serial, serial_len - 1);
        }
        break;
      }
    } else {
      break;
    }
  }
}
