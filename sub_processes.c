
#include "sub_processes.h"

// Function to log to file, needs to be called inside crittical section!!!
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

  sem_t* mutex = &shared_mem->mutex;
  sem_t* next_cart = &shared_mem->boarding_cart_queue;

  sem_wait(mutex);
  log_action("D: started\n");
  sem_post(mutex);

  //for(int i = 0; i < 20; i++) {
  while(!shared_mem->closing) {
    sem_wait(mutex);
    
    if(shared_mem->happy_customers_count == args->N) {
      log_action("D: closing\n");
      shared_mem->closing = true;
      sem_post(mutex);
    } else {
      log_action("D: next cart\n");
      shared_mem->boarding_cart_count--;
      sem_post(next_cart);
      sem_post(mutex);
      usleep(args->O);
    }  
  }
  exit(EXIT_SUCCESS);
}

void cart(unsigned id, args_t* args) {
  srandom((unsigned)getpid());
  sem_t* mutex = &shared_mem->mutex;
  sem_t* boarding_carts = &shared_mem->boarding_cart_queue;
  sem_t* boarding_visitors = &shared_mem->boarding_visitor_queue;
  sem_t* leaving = &shared_mem->leaving_cart_queue;
  cartsSemaphores_t* carts = &shared_mem->carts_arr;

  sem_wait(mutex);
  log_action("V %d: started\n", id);
  shared_mem->boarding_cart_count++;
  sem_post(mutex);


  while(!shared_mem->closing) {

    sem_wait(boarding_carts);
    sem_wait(mutex);
    log_action("V %d: boarding started\n");
    carts->arr_idx = id-1;
    for(int i = 0; i < args->K; i++) {
      sem_post(boarding_visitors);
    }
    sem_post(mutex);


    sem_wait(&shared_mem->boarding_complete);
    sem_wait(mutex);
    log_action("V %d: boarding complete\n", id);
    sem_post(mutex);
    
    sleep_random(args->TV/2, args->TV);

    sem_wait(leaving);
    sem_wait(mutex);
    log_action("V %d: leaving started\n");
    for(int i = 0; i < carts->count_arr[id-1]; i++) {
      sem_post(&carts->sem_arr[id-1]);
    }
    sem_post(mutex);

    sem_wait(&shared_mem->leaving_complete);
    sem_wait(mutex);
    log_action("V %d: leaving complete\n", id);
    sem_post(mutex);
  }

  log_action("V %d: closed\n", id);
  exit(EXIT_SUCCESS);
}

void visitor(unsigned id, args_t* args) {
  sem_t* mutex = &shared_mem->mutex;
  sem_t* boarding_carts = &shared_mem->boarding_cart_queue;
  sem_t* boarding_visitors = &shared_mem->boarding_visitor_queue;
  sem_t* leaving = &shared_mem->leaving_cart_queue;
  cartsSemaphores_t* carts = &shared_mem->carts_arr;


  sem_wait(mutex);
  log_action("N %d: started\n", id);
  sem_post(mutex);

  sleep_random(0, args->TN);

  sem_wait(mutex);
  log_action("N %d: queue\n", id);
  sem_post(mutex);

  int cart_id;
  sem_wait(boarding_visitors);
  sem_wait(mutex);
  log_action("N %d: boarding\n", id);
  cart_id = carts->arr_idx;
  carts->count_arr[cart_id]++;
  sem_post(mutex);

  sem_wait(&carts->sem_arr[cart_id]);
  sem_wait(mutex);
  log_action("N %d: leaving\n", id);
  sem_post(mutex);
  exit(EXIT_SUCCESS);
}