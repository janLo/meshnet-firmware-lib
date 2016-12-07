#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Arduino.h"

#define CE_PIN 7
#define CS_PIN 8

#define session_t uint16_t
#define msg_size_t uint8_t
#define node_t uint16_t
#define counter_t uint16_t
#define type_t uint8_t

#define PAYLOAD_LEN 32
#define HASH_LEN 8
#define SESSION_LEN sizeof(session_t)

#define MAX_MESSAGE_LEN                                                        \
  PAYLOAD_LEN + HASH_LEN + sizeof(msg_size_t) + sizeof(counter_t)

#define PONG_INTERVAL 3000
#define CONFIG_TIMEOUT 5000
#define CHECK_INTERVAL 1000

#define MASTER_ADDR 0
#define KEY                                                                    \
  {                                                                            \
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,    \
        0x0c, 0x0d, 0x0e, 0x0f                                                 \
  }

#define MASTER_ID 0

#endif /* end of include guard: _CONFIG_H_ */
