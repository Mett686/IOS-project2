

#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include <semaphore.h>
#include <stdbool.h>

typedef struct {
  sem_t mutex;
  unsigned counter;

  int boarding_person_count;
  sem_t boarding_person_queue;

  int leaving_person_count;
  sem_t leaving_person_queue;

  int boarding_cart_count;
  sem_t boarding_cart_queue;

  int leaving_cart_count;
  sem_t leaving_cart_queue;

  bool closing;
} sharedData_t;

// Initialize shared memory
sharedData_t* shared_mem_init();

// free shared memory
void shared_mem_free(sharedData_t* memory);

#endif