

/** RF24Mesh_Example_Master.ino by TMRh20
 *
 *
 * This example sketch shows how to manually configure a node via RF24Mesh as a
 * master node, which
 * will receive all data from sensor nodes.
 *
 * The nodes can change physical or logical position in the network, and
 * reconnect through different
 * routing nodes as required. The master node manages the address assignments
 * for the individual nodes
 * in a manner similar to DHCP.
 *
 */

#include "items.hpp"
#include "node.hpp"

const uint8_t key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

Node node(1, key);

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Start init");
  node.init();
}

void loop() { node.process(); }
