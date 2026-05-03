// subprocesses.h
// Autor: Martin Knotek, FIT 3.5.2026
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#ifndef SUB_PROCESSES_H
#define SUB_PROCESSES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "error.h"
#include "main.h"

void dispatcher(unsigned id, args_t* args);

void cart(unsigned id, args_t* args);

void visitor(unsigned id, args_t* args);

#endif