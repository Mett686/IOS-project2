
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
  sem_t* next_cart = &shared_mem->cart_storage;

  sem_wait(mutex);
  log_action("D: started\n");
  sem_post(mutex);

  //for(int i = 0; i < 20; i++) {
  while(!shared_mem->closing) {
    sem_wait(mutex);

    if(shared_mem->visitor_queue_len == 0 &&
      shared_mem->happy_customers_count == args->N) {
      log_action("D: closing\n");
      shared_mem->closing = true;
      for(int i = 0; i < args->V; i++) {
        printf("posted next for closing\n");
        sem_post(next_cart);
      }
      sem_post(mutex);
      continue;
    }
    log_action("D: next cart\n");
    shared_mem->cart_count--;
    sem_post(next_cart);
    sem_post(mutex);

    sem_wait(&shared_mem->cart_left);
    usleep(args->O);  
  }
  exit(EXIT_SUCCESS);
}

void cart(unsigned id, args_t* args) {
  srandom((unsigned)getpid());
  sem_t* mutex = &shared_mem->mutex;
  sem_t* boarding_carts = &shared_mem->cart_storage;
  sem_t* boarding_visitors = &shared_mem->visitor_queue;
  sem_t* leaving = &shared_mem->leaving_cart_queue;
  cartsSemaphores_t* carts = &shared_mem->carts_arr;

  sem_wait(mutex);
  log_action("V %d: started\n", id);
  shared_mem->cart_count++;
  sem_post(mutex);
  
  
  while(!shared_mem->closing) {
    
    sem_wait(boarding_carts);
    sem_wait(mutex);
    shared_mem->cart_count--;
    log_action("V %d: boarding started\n", id);
    carts->arr_idx = id-1;

    int rest = args->N - shared_mem->happy_customers_count;
    for(int i = 0; i < args->K && i < rest; i++) {
      sem_post(boarding_visitors);
    }
    sem_post(mutex);


    sem_wait(&shared_mem->boarding_complete);
    sem_wait(mutex);
    log_action("V %d: boarding complete\n", id);
    sem_post(&shared_mem->cart_left);
    sem_post(mutex);
    
    sleep_random(args->TV/2, args->TV);

    sem_wait(leaving);
    sem_wait(mutex);
    log_action("V %d: leaving started\n", id);
    for(int i = 0; i < carts->count_arr[id-1]; i++) {
      sem_post(&carts->sem_arr[id-1]);
    }
    sem_post(mutex);

    sem_wait(&shared_mem->leaving_complete);
    sem_wait(mutex);
    log_action("V %d: leaving complete\n", id);
    shared_mem->cart_count++;
    sem_post(leaving);
    sem_post(mutex);
  }

  log_action("V %d: closed\n", id);
  exit(EXIT_SUCCESS);
}

void visitor(unsigned id, args_t* args) {
  sem_t* mutex = &shared_mem->mutex;
  sem_t* boarding_visitors = &shared_mem->visitor_queue;
  cartsSemaphores_t* carts = &shared_mem->carts_arr;

  sem_wait(mutex);
  log_action("N %d: started\n", id);
  sem_post(mutex);

  sleep_random(0, args->TN);

  sem_wait(mutex);
  log_action("N %d: queue\n", id);
  shared_mem->visitor_queue_len++;
  sem_post(mutex);

  int cart_id;
  sem_wait(boarding_visitors);
  sem_wait(mutex);
  log_action("N %d: boarding\n", id);
  cart_id = carts->arr_idx;
  carts->count_arr[cart_id]++;
  shared_mem->visitor_queue_len--;

    printf("shared_mem->visitor_queue_len: %d\n", shared_mem->visitor_queue_len);
    printf("shared_mem->happy_customers_count: %d\n", shared_mem->happy_customers_count);
    printf("visitors in cart: %d\n", carts->count_arr[cart_id]);

  if( carts->count_arr[cart_id] >= args->K ||
      carts->count_arr[cart_id] == (args->N - shared_mem->happy_customers_count)) {
    if(carts->count_arr[cart_id] > args->K) {
      error_exit("Number of visitors in cart exceeded cart capacity.\n");
    }

    printf("cart full\n");

    sem_post(&shared_mem->boarding_complete);
  }
  sem_post(mutex);

  sem_wait(&carts->sem_arr[cart_id]);
  sem_wait(mutex);
  log_action("N %d: leaving\n", id);
  carts->count_arr[cart_id]--;
  shared_mem->happy_customers_count++;
  if(carts->count_arr[cart_id] == 0) {
    if(carts->count_arr[cart_id] < 0) {
      carts->count_arr[cart_id] = 0;
      error_exit("Number of visitors in cart reached negative value.\n");
    }
    sem_post(&shared_mem->leaving_complete);
  }
  sem_post(mutex);
  
  exit(EXIT_SUCCESS);
}