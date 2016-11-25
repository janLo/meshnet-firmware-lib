#ifndef _CONFIG_H_
#define _CONFIG_H_

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
  PAYLOAD_LEN + HASH_LEN + sizeof(msg_size_t) + sizeof(counter_t);

#endif /* end of include guard: _CONFIG_H_ */
