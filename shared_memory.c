#include <stdlib.h>
#include <sys/mman.h>

#include "args.h"
#include "shared_memory.h"
#include "error.h"

sharedData_t* shared_mem_init(args_t* args) {
  sharedData_t* memory = NULL;
  memory = mmap(NULL, sizeof(sharedData_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

  if(memory == MAP_FAILED) {
    error_exit("Shared memory allocation failed!\n");
  }
  memory->counter = 1;

  memory->boarding_person_count = 0;
  memory->leaving_person_count = 0;
  
  memory->boarding_cart_count = 0;
  memory->leaving_cart_count = 0;

  memory->closing = false;

  return memory;
}

void shared_mem_free(sharedData_t* memory) {
  munmap(memory, sizeof(sharedData_t));
}