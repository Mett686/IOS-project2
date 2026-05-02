
#include "sub_processes.h"

// Prints formated string to output_file
void log_action(char* fmt, ...) {
  va_list args;
  va_start (args, fmt);
  fprintf(output_file, "%d: ", shared_mem->counter);
  vfprintf(output_file, fmt, args);
  shared_mem->counter++;
  va_end (args);
}

// Sleeps random interval from min to max micro-seconds
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

// Appends id to array that keeps track of order on the track
void append(unsigned* arr, unsigned id) {
  int i;
  for(i = 0; arr[i]; i++);
  arr[i] = id;
}

// Removes first id from the array and moves all one forward
void pop_first(unsigned* arr) {
  for(int i = 0; arr[i+1]; i++) {
    arr[i] = arr[i+1];
  }
}


void dispatcher(unsigned id, args_t* args) {
  (void)id;
  sharedData_t* mem = shared_mem;
  sem_t* mutex = &mem->mutex;

  sem_wait(mutex);
  log_action("D: started\n");
  sem_post(mutex);

  // Reading outside of crittical function,
  // the program should never get here because of break;
  // therefore should function as while(true)
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
  srandom((unsigned)getpid()*time(NULL));

  // crittical function starts here and ends at the start of main while cycle or after the while cycle
  sem_wait(mutex);
  log_action("V %d: started\n", id);

  while(!mem->closing) {
    sem_post(mutex);

    // Wait for your turn
    sem_wait(&mem->cart_queue);
    sem_wait(mutex);

    // check if should start boarding
    if(mem->closing) {
      break; // after exiting while loop logs and posts mutex
    }

    // Boarding section
    log_action("V %d: boarding started\n", id);
    carts->arr_idx = id-1;
    for(int i = 0; i < args->K; i++) {
      sem_post(&mem->visitor_queue);
    }
    sem_post(mutex);

    sem_wait(&mem->boarding_complete);
    sem_wait(mutex);
    log_action("V %d: boarding complete\n", id);
    sem_post(mutex);

    
    // Track section
    // Track the order in which carts go on the track
    pthread_mutex_lock(&carts->mutex);
    append(carts->order_arr, id);
    pthread_mutex_unlock(&carts->mutex);
    sem_post(&mem->cart_left);

    sleep_random(args->TV/2, args->TV);


    // Leaving / disembarking section
    // Pthread conditional wait for your turn to leave
    // using pthread functions here and different mutex
    pthread_mutex_lock(&carts->mutex);
    while(carts->order_arr[0] != id) {
      pthread_cond_wait(&carts->condition, &carts->mutex);
    }
    pop_first(carts->order_arr);
    pthread_cond_broadcast(&carts->condition);
    

    sem_wait(mutex);
    log_action("V %d: leaving started\n", id);
    for(int i = 0; i < carts->count_arr[id-1]; i++) {
      sem_post(&carts->sem_arr[id-1]);
    }
    sem_post(mutex);
    
    sem_wait(&mem->visitors_left);
    sem_wait(mutex); // post for this mutex is either at the beginnig of (or after this) while loop
    log_action("V %d: leaving complete\n", id);
    pthread_mutex_unlock(&carts->mutex); // this keeps the leaving station locked for current cart only 
  }

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

  // Board cart and save its id
  // see cartsSemaphores_t in shared_memory.h
  int cart_id;
  sem_wait(&mem->visitor_queue);
  sem_wait(mutex);
  log_action("N %d: boarding\n", id);
  cart_id = carts->arr_idx;
  carts->count_arr[cart_id]++;
  mem->happy_customers_count++;

  // check if visitor was last to fit or last today
  if( carts->count_arr[cart_id] == args->K || 
      mem->happy_customers_count == args->N ) {
    sem_post(&mem->boarding_complete);
  }
  sem_post(mutex);

  // wait in cart for ride to end
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