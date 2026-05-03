#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>

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

  unsigned* order_arr = mmap(NULL, sizeof(int)*(count+1), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if(order_arr == MAP_FAILED) {
    munmap(count_arr, sizeof(int)*count);
    munmap(sem_arr, sizeof(sem_t)*count);
    return NULL;
  }
  sem_t* on_track = mmap(NULL, sizeof(sem_t)*count, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if(on_track == MAP_FAILED) {
    munmap(count_arr, sizeof(int)*count);
    munmap(sem_arr, sizeof(sem_t)*count);
    munmap(order_arr, sizeof(unsigned)*(count + 1));
    return NULL;
  }
  memory->carts_arr.size = count;
  memory->carts_arr.count_arr = count_arr;
  memory->carts_arr.sem_arr = sem_arr;
  memory->carts_arr.order_arr = order_arr;
  memory->carts_arr.on_track = on_track;
  return &memory->carts_arr;
}

// Initialize array of cart semaphores
cartsSemaphores_t* shared_carts_init(cartsSemaphores_t* arr) {
  for(int i = 0; i < arr->size; i++) {
    bool failed = sem_init(&arr->sem_arr[i], 1, 0) == -1;
    failed = (sem_init(&arr->on_track[i], 1, 0) == -1) || failed;
    // on fail destroy all previously initialized
    if(sem_init(&arr->sem_arr[i], 1, 0) == -1) {
      for(int j = 0; j < i; j++) {
        sem_destroy(&arr->sem_arr[j]);
        sem_destroy(&arr->on_track[j]);
      }
      return NULL;
    }
    arr->count_arr[i] = 0;
    arr->arr_idx = 0;
    arr->on_track_count = 0;
  }
  return arr;
}

// Free array of cart semaphores
void shared_carts_free(cartsSemaphores_t* arr) {
  for(int i = 0; i < arr->size; i++) {
    sem_destroy(&arr->sem_arr[i]);
    sem_destroy(&arr->on_track[i]);
  }
  munmap(arr->count_arr, sizeof(arr->count_arr[0])*arr->size);
  munmap(arr->sem_arr, sizeof(arr->sem_arr[0])*arr->size);
  munmap(arr->order_arr, sizeof(arr->order_arr[0])*(arr->size+1));
  munmap(arr->on_track, sizeof(arr->on_track[0])*arr->size);
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
    if(sem_init(&memory->leaving_station, 1, 1) == -1) {
      return NULL;
    }
    if(sem_init(&memory->visitors_left, 1, 0) == -1) {
      return NULL;
    }

    memory->counter = 1;

    memory->visitors_to_board = 0;
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
  sem_destroy(&memory->leaving_station);
  sem_destroy(&memory->visitors_left);
  munmap(memory, sizeof(sharedData_t));
}