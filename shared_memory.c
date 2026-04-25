#include <stdlib.h>
#include <sys/mman.h>

#include "main.h"
#include "shared_memory.h"
#include "error.h"

// Map shared memory
sharedData_t* shared_mem_map() {
  sharedData_t* memory = NULL;
  memory = mmap(NULL, sizeof(sharedData_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if(memory == MAP_FAILED) {
    return NULL;
  }
  return memory;
}

// Initialize shared memory, carefull unsafe write
sharedData_t* shared_mem_init(sharedData_t* memory) {
  if(memory) {
    if(sem_init(&memory->mutex, 1, 1) == -1) {
      return NULL;
    }
    if(sem_init(&memory->boarding_person_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->boarding_cart_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->leaving_cart_queue, 1, 0) == -1) {
      return NULL;
    }
    memory->counter = 1;

    memory->boarding_person_count = 0;
    
    memory->boarding_cart_count = 0;
    memory->leaving_cart_count = 0;

    memory->happy_customers_count = 0;
    memory->closing = false;
  }
  return memory;
}

void shared_mem_free(sharedData_t* memory) {
  sem_destroy(&memory->boarding_person_queue);
  sem_destroy(&memory->boarding_cart_queue);
  sem_destroy(&memory->leaving_cart_queue);
  munmap(memory, sizeof(sharedData_t));
}