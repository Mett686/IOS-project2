
#include "sub_processes.h"

void log_action(char* fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(output_file, "%d: ", shared_mem->counter);
  vfprintf(output_file, fmt, args);
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


void dispatcher(unsigned id, args_t* args) {
  (void)id;
  sharedData_t* mem = shared_mem;
  sem_t* mutex = &mem->mutex;

  sem_wait(mutex);
  log_action("D: started\n");
  sem_post(mutex);

  while (!mem->closing) {
    sem_wait(mutex);

    // If all visitors entered a cart start closing
    if(mem->happy_customers_count == args->N) {
      log_action("D: closing\n");
      mem->closing = true;
      for(int i = 0; i < args->V; i++) {
        sem_post(&mem->cart_queue);
      }
      sem_post(mutex);
      break;
    }

    // Start loading next cart
    log_action("D: next cart\n");
    sem_post(&mem->cart_queue);
    sem_post(mutex);
    
    sem_wait(&mem->cart_left);
    usleep(args->O);
  }
  
  exit(EXIT_SUCCESS);
}

void cart(unsigned id, args_t* args) {
  sharedData_t* mem = shared_mem;
  cartsSemaphores_t* carts = &mem->carts_arr;
  sem_t* mutex = &mem->mutex;
  srandom((unsigned)getpid());

  sem_wait(mutex);
  log_action("V %d: started\n", id);
  sem_post(mutex);

  while(!mem->closing) {
    sem_wait(&mem->cart_queue);
    sem_wait(mutex);

    if(mem->closing) {
      sem_post(mutex);
      break;
    }

    log_action("V %d: boarding started\n", id);
    carts->arr_idx = id-1;

    for(int i = 0; i < args->K; i++) {
      sem_post(&mem->visitor_queue);
    }
    sem_post(mutex);


    sem_wait(&mem->boarding_complete);
    sem_wait(mutex);
    log_action("V %d: boarding complete\n", id);
    sem_post(&mem->cart_left);
    sem_post(mutex);

    sleep_random(args->TV/2, args->TV);

    sem_wait(&mem->leaving_cart_queue);
    sem_wait(mutex);
    log_action("V %d: leaving started\n", id);
    for(int i = 0; i < carts->count_arr[id-1]; i++) {
      sem_post(&carts->sem_arr[id-1]);
    }
    sem_post(mutex);

    sem_wait(&mem->visitors_left);
    sem_wait(mutex);
    log_action("V %d: leaving complete\n", id);
    sem_post(&mem->leaving_cart_queue);
    sem_post(mutex);
  }
  sem_wait(mutex);
  log_action("V %d: closed\n", id);
  sem_post(mutex);
  exit(EXIT_SUCCESS);
}


void visitor(unsigned id, args_t* args) {
  sharedData_t* mem = shared_mem;
  sem_t* mutex = &mem->mutex;
  cartsSemaphores_t* carts = &mem->carts_arr;
  srandom((unsigned)getpid());

  sem_wait(mutex);
  log_action("N %d: started\n", id);
  sem_post(mutex);

  sleep_random(0, args->TN);

  sem_wait(mutex);
  log_action("N %d: queue\n", id);
  sem_post(mutex);

  int cart_id;
  sem_wait(&mem->visitor_queue);
  sem_wait(mutex);
  log_action("N %d: boarding\n", id);
  cart_id = carts->arr_idx;
  carts->count_arr[cart_id]++;
  mem->happy_customers_count++;

  if( carts->count_arr[cart_id] == args->K || 
      mem->happy_customers_count == args->N ) {
    sem_post(&mem->boarding_complete);
  }
  sem_post(mutex);

  sem_wait(&carts->sem_arr[cart_id]);
  sem_wait(mutex);
  log_action("N %d: leaving\n", id);
  carts->count_arr[cart_id]--;
  int count = carts->count_arr[cart_id];
  if( count <= 0) {
    if(count < 0)
      error_exit("Number of visitors in cart reached negative value: %d\n", count);
    sem_post(&mem->visitors_left);
  }
  sem_post(mutex);

  exit(EXIT_SUCCESS);
}