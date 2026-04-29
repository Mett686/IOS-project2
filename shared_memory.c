#include <stdlib.h>
#include <sys/mman.h>

#include "shared_memory.h"
#include "main.h"
#include "error.h"


// Map array of cart semaphores
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
  memory->carts_arr.count_arr = count_arr;
  memory->carts_arr.sem_arr = sem_arr;
  return &memory->carts_arr;
}

// Initialize array of cart semaphores
cartsSemaphores_t* shared_carts_init(cartsSemaphores_t* arr) {
  for(int i = 0; i < arr->size; i++) {
    // on fail destroy all previously initialized
    if(sem_init(&arr->sem_arr[i], 1, 0) == -1) {
      for(int j = 0; j < i; j++) {
        sem_destroy(&arr->sem_arr[j]);
      }
      return NULL;
    }
    arr->count_arr[i] = 0;
    arr->arr_idx = 0;
  }
  return arr;
}

// Free array of cart semaphores
void shared_carts_free(cartsSemaphores_t* arr) {
  for(int i = 0; i < arr->size; i++) {
    sem_destroy(&arr->sem_arr[i]);
  }
  munmap(&arr->count_arr, sizeof(arr->count_arr[0])*arr->size);
  munmap(&arr->sem_arr, sizeof(arr->sem_arr[0])*arr->size);
}

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
    if(sem_init(&memory->visitor_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->cart_queue, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->boarding_complete, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->cart_left, 1, 0) == -1) {
      return NULL;
    }
    if(sem_init(&memory->leaving_cart_queue, 1, 1) == -1) {
      return NULL;
    }
    if(sem_init(&memory->visitors_left, 1, 0) == -1) {
      return NULL;
    }

    memory->counter = 1;

    memory->happy_customers_count = 0;
    memory->closing = false;
  }
  return memory;
}

// Free shared memory
void shared_mem_free(sharedData_t* memory) {
  sem_destroy(&memory->mutex);
  sem_destroy(&memory->visitor_queue);
  sem_destroy(&memory->cart_queue);
  sem_destroy(&memory->boarding_complete);
  sem_destroy(&memory->cart_left);
  sem_destroy(&memory->leaving_cart_queue);
  sem_destroy(&memory->visitors_left);
  munmap(memory, sizeof(sharedData_t));
}