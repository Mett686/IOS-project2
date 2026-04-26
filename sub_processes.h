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