// main.h
// Autor: Martin Knotek, FIT 3.5.2026
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

#include "shared_memory.h"

// V: počet vozíků, 0<V<10
// N: počet návštěvníků, 0<N<10000
// K: kapacita vozíku, 4<=K<=40
// TV: Doba jízdy vozíku po dráze v mikrosekundách, 0<=TV<=1000
// TN: Maximální doba, než návštěvník dojde do fronty v mikrosekundách, 0<=TN<=1000
// O: Minimální časový odstup vozíků v mikrosekundách, 0<O<=100
typedef struct args {
  int V; // počet vozíků, 0<V<10
  int N; // počet návštěvníků, 0<N<10000
  int K; // kapacita vozíku, 4<=K<=40
  int TV; // Doba jízdy vozíku po dráze v mikrosekundách, 0<=TV<=1000
  int TN; // Maximální doba, než návštěvník dojde do fronty v mikrosekundách, 0<=TN<=1000
  int O; // Minimální časový odstup vozíků v mikrosekundách, 0<O<=100
} args_t;

extern FILE* output_file;
extern sharedData_t* shared_mem;

#endif