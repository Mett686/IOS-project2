// error.h
// Řešení IJC-DU1, příklad b), 25.3.2026
// Autor: Martin Knotek, FIT
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define warning(fmt, ...) \
  func_warning(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define error_exit(fmt, ...) \
  func_error_exit(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


// Warn function, same inputs as printf()
// outputs to stderr: "Warning: ..." 
void func_warning(const char *file, int line, const char *fmt, ...);

// Error function, same inputs as printf()
// logs to stderr and exits program
// outputs to stderr: "Error: ..."
void func_error_exit(const char *file, int line, const char *fmt, ...);

#endif