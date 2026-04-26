// error.c
// Autor: Martin Knotek, FIT
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#include "error.h"

void func_warning(const char *file, int line, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s:%d: Warning: ", file, line);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void func_error_exit(const char *file, int line, const char *fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(stderr, "%s:%d: Error: ", file, line);
  vfprintf(stderr, fmt, args);
  va_end (args);
  exit(EXIT_FAILURE);
}