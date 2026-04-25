

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>
#include <stdbool.h>

typedef struct {
  sem_t mutex;
  unsigned counter;

  int boarding_person_count;
  sem_t boarding_person_queue;

  int boarding_cart_count;
  sem_t boarding_cart_queue;

  int leaving_cart_count;
  sem_t leaving_cart_queue;

  int happy_customers_count;
  bool closing;
} sharedData_t;

// map shared memory
sharedData_t* shared_mem_map();

// Initialize shared memory
sharedData_t* shared_mem_init();

// free shared memory
void shared_mem_free(sharedData_t* memory);

#endif