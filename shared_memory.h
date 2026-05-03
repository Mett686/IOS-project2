// shared_memory.h
// Autor: Martin Knotek, FIT 3.5.2026
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>
#include <stdbool.h>

typedef struct {
  int size; // Size of array
  unsigned arr_idx; // Index to which semaphor in array should visitors start boarding
  int* count_arr; // Number of visitors in each cart
  sem_t* sem_arr; // Carts semaphores

  unsigned* order_arr;
  unsigned on_track_count;
  sem_t* on_track;
} cartsSemaphores_t;


typedef struct {
  sem_t mutex; // Main mutex
  unsigned counter; // Action counter

  sem_t visitor_queue; // Semaphore for boarding visitors

  sem_t cart_queue; // Semaphore for empty carts
  sem_t boarding_complete; 
  sem_t cart_left;

  sem_t leaving_station; // Semaphore for leaving

  sem_t visitors_left; // Signal that all visitors disembarked

  int visitors_to_board; // Number of visitors left to board
  bool closing; // Whether carts should start boarding or ending
  cartsSemaphores_t carts_arr;

} sharedData_t;

// Map array of cart semaphores
cartsSemaphores_t* shared_carts_map(sharedData_t* memory, int count);

// Initialize array of cart semaphores
cartsSemaphores_t* shared_carts_init(cartsSemaphores_t* arr);

// Free array of cart semaphores
void shared_carts_free(cartsSemaphores_t* arr);

// Map shared memory
sharedData_t* shared_mem_map();

// Initialize shared memory, carefull unsafe write
sharedData_t* shared_mem_init(sharedData_t* memory);

// Free shared memory
void shared_mem_free(sharedData_t* memory);

#endif