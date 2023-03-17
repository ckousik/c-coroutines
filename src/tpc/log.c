#include "log.h"

#include <stdarg.h>
#include <stdio.h>

int log_debug(const char *fmt, ...) {
#ifdef __TPC_DEBUG__
  va_list args;
  va_start(args, fmt);
  printf("DEBUG ");
  va_start(args, fmt);
  int result = vprintf(fmt, args);
  va_end(args);
  printf("\n");
  return result;
#else
  return 0;
#endif
}

int log_warn(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  printf("WARN ");
  va_start(args, fmt);
  int result = vprintf(fmt, args);
  va_end(args);
  printf("\n");
  return result;
}

int log_error(const char *fmt, ...) {
  va_list args;
  printf("ERROR ");
  va_start(args, fmt);
  int result = vprintf(fmt, args);
  va_end(args);
  printf("\n");
  return result;
}
