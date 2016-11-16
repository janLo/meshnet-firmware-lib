#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <Arduino.h>

#define MAX_ID_LEN 16
#define MAX_ITEMS 16

typedef enum {
  booted = 70,
  configure,
  configured,
  set_state,
  get_state,
  reading,
  ping,
  pong
} messages_t;

typedef enum {
  bin_switch = 0,
  bin_sensor = 1,
  analog_sensor = 3,
  one_wire = 4,
  rgb_lamp = 5,
  dimmer = 6
} items_t;

#endif
