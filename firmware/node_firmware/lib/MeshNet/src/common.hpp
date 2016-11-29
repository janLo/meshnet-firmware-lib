#ifndef _COMMON_H_
#define _COMMON_H_

#include <Arduino.h>

//#define NO_DEBUG 1

#ifndef NO_DEBUG

void _dbg_p_fn(const char *fmt, ...);

#define DEBUG_LOG(fmt, ...) _dbg_p_fn("DEBUG:" fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

void _do_reset();
#define RESET() _do_reset()

#endif /* end of include guard: _COMMON_H_ */
