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

#endif
