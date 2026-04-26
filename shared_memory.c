#include <stdlib.h>
#include <sys/mman.h>

#include "shared_memory.h"
#include "main.h"
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

cartsSemaphores_t* shared_carts_map(sharedData_t* memory, int count) {
  int* count_arr = mmap(NULL, sizeof(int)*count, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if(count_arr == MAP_FAILED)
    return NULL;

  sem_t* sem_arr = mmap(NULL, sizeof(sem_t)*count, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if(sem_arr == MAP_FAILED) {
    munmap(count_arr, sizeof(int)*count);
    return NULL;
  }
  memory->carts_arr.size = count;
  memory->carts_arr.cart_count_arr = count_arr;
  memory->carts_arr.cart_sem_arr = sem_arr;
  return &memory->carts_arr;
}

void shared_carts_free(cartsSemaphores_t* carts_arr) {
  munmap(&carts_arr->cart_count_arr, sizeof(int)*carts_arr->size);
  munmap(&carts_arr->cart_sem_arr, sizeof(sem_t)*carts_arr->size);
}


// Initialize shared memory, carefull unsafe write
sharedData_t* shared_mem_init(sharedData_t* memory) {
  if(memory) {
    if(sem_init(&memory->mutex, 1, 1) == -1) {
      return NULL;
    }
    if(sem_init(&memory->boarding_visitor_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->boarding_cart_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->leaving_cart_queue, 1, 0) == -1) {
      return NULL;
    }
    memory->counter = 1;

    memory->boarding_visitor_count = 0;
    
    memory->boarding_cart_count = 0;
    memory->leaving_cart_count = 0;

    memory->happy_customers_count = 0;
    memory->closing = false;
  }
  return memory;
}

void shared_mem_free(sharedData_t* memory) {
  sem_destroy(&memory->boarding_visitor_queue);
  sem_destroy(&memory->boarding_cart_queue);
  sem_destroy(&memory->leaving_cart_queue);
  munmap(memory, sizeof(sharedData_t));
}