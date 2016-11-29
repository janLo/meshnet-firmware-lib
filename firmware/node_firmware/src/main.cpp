

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

const uint8_t key[] = KEY;

Node node(1, key);

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Start init");
  node.init();
}

void loop() { node.process(); }
