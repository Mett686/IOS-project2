#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "args.h"
#include "error.h"
#include "shared_memory.h"
#include "sub_processes.h"

//static bool successfull = false;

// Shared memory
static sharedData_t* shared_mem = NULL;
void free_shared_mem() {
  shared_mem_free(shared_mem);
}

// Static array for tracking child processes
typedef struct pidArr{
  unsigned count;
  unsigned size;
  pid_t* arr;
} pidArr_t;
static pidArr_t pid_arr;

// Initialize pidArr structure;
void pid_arr_init(pidArr_t* process_ids, unsigned size) {
  process_ids->size = size;
  process_ids->count = 0;
  process_ids->arr = calloc((size_t)size, sizeof(pid_t));
  if(!process_ids->arr) {
    error_exit("Process ID array allocation failed!\n");
  }
}

void pid_arr_add(pidArr_t* pid_arr, pid_t pid) {
  if(pid_arr->count < pid_arr->size) {
    pid_arr->arr[pid_arr->count] = pid;
    pid_arr->count++;
  } else {
    error_exit("pid array full!\n");
  }
}

// Start sub-process
pid_t start_sub_process(args_t* args, unsigned id, void (*process_function)(unsigned id, args_t* args)) {
  pid_t pid = fork();
  if(pid == -1) {
    error_exit("Failed to fork main!\n");
  } else if(pid) {
    return pid;
  } else {
    process_function(id, args);
    error_exit("Process function did not end process!\n");
  }
  return -1;
}

// Free all processes logged in static pid_arr
void free_all_processes() {
  for(unsigned i = 0; i < pid_arr.count; i++) {
    kill(pid_arr.arr[i], SIGINT);
  }
  free(pid_arr.arr);
}

void wait_for_all_processes(pidArr_t* pid_arr) {
  int status;
  for(unsigned i = 0; i < pid_arr->count; i++) {
    waitpid(pid_arr->arr[i], &status, 0);
  }
}



int main(int argc, char** argv) {
  if(argc != 7)
  error_exit("Invalid number of arguments!\n");
  
  args_t args;
  args.N = (int)atoi(argv[2]);
  args.K = (int)atoi(argv[3]);
  args.TV = (int)atoi(argv[4]);
  args.TN = (int)atoi(argv[5]);
  args.O = (int)atoi(argv[6]);
  
  // V: počet vozíků, 0<V<10
  args.V = (int)atoi(argv[1]);
  if(!(0 < args.V && args.V < 10))
    error_exit("Invalid argument V: %d\n", args.V);
  
  // N: počet návštěvníků, 0<N<10000
  if(!(0 < args.N && args.N < 10000))
    error_exit("Invalid argument N: %d\n", args.N);
  
  // K: kapacita vozíku, 4<=K<=40
  if(!(4 <= args.K && args.K <= 40))
    error_exit("Invalid argument K: %d\n", args.K);
  
  // TV: Doba jízdy vozíku po dráze v mikrosekundách, 0<=TV<=1000
  if(!(0 <= args.TV && args.TV <= 1000))
    error_exit("Invalid argument TV: %d\n", args.TV);
  
  // TN: Maximální doba, než návštěvník dojde do fronty v mikrosekundách, 0<=TN<=1000
  if(!(0 <= args.TN && args.TN <= 1000))
    error_exit("Invalid argument TN: %d\n", args.TN);
  
  // O: Minimální časový odstup vozíků v mikrosekundách, 0<O<=100
  if(!(0 < args.O && args.O <= 100))
    error_exit("Invalid argument O: %d\n", args.O);
  
  // Initialize shared_mem and pid_arr
  shared_mem = shared_mem_init();
  pid_arr_init(&pid_arr, 1 + args.V + args.N);

  // Start dispatcher
  int pid = start_sub_process(&args, 0, dispatcher);
  pid_arr_add(&pid_arr, pid);

  // Start carts
  for(int i = 1; i <= args.V; i++) {
    pid = start_sub_process(&args, i, cart);
    pid_arr_add(&pid_arr, pid);
  }

  // Start visitors
  for(int i = 1; i <= args.N; i++) {
    pid = start_sub_process(&args, i, visitor);
    pid_arr_add(&pid_arr, pid);
  }

  wait_for_all_processes(&pid_arr);
  shared_mem_free(shared_mem);
  return(EXIT_SUCCESS);
}