
#ifndef _COMMON_H_
#define _COMMON_H_

#include <Arduino.h>

#ifndef NO_DEBUG
#include <stdarg.h>

void _dbg_p(const char *fmt, ...) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
}

#define DEBUG_LOG(fmt, ...) _dbg_p("DEBUG:" fmt, ##__VA_ARGS__);
#else
#define DEBUG_LOG(...)
#endif

#endif /* end of include guard: _COMMON_H_ */
