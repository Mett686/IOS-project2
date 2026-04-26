

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>
#include <stdbool.h>


typedef struct {
  int size; // Size of array
  unsigned arr_idx; // Index to which semaphor in array should visitors start boarding
  int* cart_count_arr; // Number of visitors in each cart 
  sem_t* cart_sem_arr; // Carts semaphores
} cartsSemaphores_t;


typedef struct {
  sem_t mutex; // Main mutex
  unsigned counter; // Action counter

  int boarding_visitor_count; // Number of visitors waiting in boarding queue
  sem_t boarding_visitor_queue; // Semaphore for boarding visitors

  int boarding_cart_count; // Number of carts waiting for boarding
  sem_t boarding_cart_queue; // Semaphore for empty carts

  int leaving_cart_count; // Count of carts waiting to be emptied
  sem_t leaving_cart_queue; // Semaphore for carts waiting to be emptied

  int happy_customers_count; // Number of satisfied visitors
  bool closing; // Whether carts should start boarding or ending

  cartsSemaphores_t carts_arr;
} sharedData_t;

// map shared memory
sharedData_t* shared_mem_map();

// Initialize shared memory
sharedData_t* shared_mem_init();

// free shared memory
void shared_mem_free(sharedData_t* memory);

#endif