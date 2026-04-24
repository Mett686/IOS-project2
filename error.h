// error.h
// Řešení IJC-DU1, příklad b), 25.3.2026
// Autor: Martin Knotek, FIT
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#ifndef ERROR_H
#define ERROR_H

// Warn function, same inputs as printf()
// outputs to stderr: "Warning: ..." 
void warning(const char *fmt, ...);

// Error function, same inputs as printf()
// logs to stderr and exits program
// outputs to stderr: "Error: ..."
void error_exit(const char *fmt, ...);

#endif