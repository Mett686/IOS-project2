#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "error.h"
#include "sub_processes.h"

// Function to log to file, needs to be called inside crittical section!!!
void log_action(FILE* file, char* fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(file, "%d: ", shared_mem->counter);
  vfprintf(file, fmt, args);
  shared_mem->counter++;
  va_end (args);
}

void sleep_random(int min, int max) {
  if(min > max) {
    warning("sleep_random received min greater than max!\n");
    return;
  }

  int delay = min;
  if (max > delay) {
    delay += min + (rand() % (max-min+1));
  }

  usleep((useconds_t)delay);
}


void dispatcher(unsigned id, args_t* args, FILE* file) {
  (void)id;

  sem_t* mutex = &shared_mem->mutex;
  sem_t* next_cart = &shared_mem->boarding_cart_queue;

  sem_wait(mutex);
  log_action(file, "D: started\n");
  sem_post(mutex);

  while(!shared_mem->closing) {
    sem_wait(mutex);
    
    if(shared_mem->happy_customers_count == args->N) {
      log_action(file, "D: closing\n");
      shared_mem->closing = true;
    }
    log_action(file, "D: next cart\n");
    shared_mem->boarding_cart_count--;
    sem_post(next_cart);
    
    sem_post(mutex);
    usleep(args->O);
  }
  exit(EXIT_SUCCESS);
}

void cart(unsigned id, args_t* args, FILE* file) {
  srandom((unsigned)getpid());
  sem_t* mutex = &shared_mem->mutex;
  sem_t* boarding = &shared_mem->boarding_cart_queue;
  sem_t* leaving = &shared_mem->leaving_cart_queue;

  sem_wait(mutex);
  log_action("V %d: started\n", id);
  shared_mem->boarding_cart_count++;
  sem_post(mutex);

  
  sem_wait(boarding);
  
  log_action("V %d: capacity: %d\n", id, args->K);
  log_action("V %d: delay: %d\n", id, args->TV);

  log_action("V %d: closing\n", id);
  exit(EXIT_SUCCESS);
}

void visitor(unsigned id, args_t* args, FILE* file) {
  log_action("N %d: started\n", id);
  log_action("N %d: delay: %d\n", id, args->TN);
  log_action("N %d: leaving\n", id);
  exit(EXIT_SUCCESS);
}