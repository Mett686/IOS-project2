// error.c
// Řešení IJC-DU1, příklad b), 25.3.2026
// Autor: Martin Knotek, FIT
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void warning(const char *fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(stderr, "Warning: ");
  vfprintf(stderr, fmt, args);
  va_end (args);
}

void error_exit(const char *fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, fmt, args);
  va_end (args);
  exit(EXIT_FAILURE);
}