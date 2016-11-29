#include "common.hpp"

#include "SoftwareReset.h"
#include <stdarg.h>

void _dbg_p_fn(const char *fmt, ...) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
  Serial.flush();
}

void _do_reset() {
  delay(10);
  softwareReset(SKETCH);
}
