// main.c
// Autor: Martin Knotek, FIT 3.5.2026
// Přeloženo: gcc version 15.2.1 20260123 (Red Hat 15.2.1-7) (GCC)

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "main.h"
#include "error.h"
#include "shared_memory.h"
#include "sub_processes.h"

#define ARG_COUNT 7 // Expectd number of arguments

// Array for tracking child processes
typedef struct {
  unsigned count;
  unsigned size;
  pid_t* pids;
} pidArr_t;

// Globals:
FILE* output_file; // File to log into
sharedData_t* shared_mem = NULL;
static pid_t main_pid; // For on_exit logic
pidArr_t pid_arr; // Needs to be available for on_exit after main stack frame is destroyed


// Close file on end
void close_file_wrapper(int status, void* arg) {
  (void)status;
  fclose((FILE*)arg);
}

// Initialize pidArr structure;
pidArr_t* pid_arr_init(pidArr_t* pid_arr, unsigned size) {
  pid_arr->pids = calloc((size_t)size, sizeof(pid_t));
  if(pid_arr->pids) {
    pid_arr->size = size;
    pid_arr->count = 0;
    return pid_arr;
  }
  return NULL;
}

// Free pidArr structure
void pid_arr_free(int status, void* arg) {
  (void)status;
  pidArr_t* pid_arr = arg;
  free(pid_arr->pids);
  pid_arr->size = 0;
}

// Add pid to pid_arr
void pid_arr_add(pidArr_t* pid_arr, pid_t pid) {
  if(pid_arr->count < pid_arr->size) {
    pid_arr->pids[pid_arr->count] = pid;
    pid_arr->count++;
  } else {
    error_exit("pid array full!\n");
  }
}

// Free memory mapped for carts_arr in shared mem
void free_carts_arr_wrapper(int status, void* arg) {
  (void)status;
  if(getpid() == main_pid) {
    shared_carts_free(arg);
  }
}

// Free shared memory
void free_mem_wrapper(int status, void* arg) {
  (void)status;
  if(getpid() == main_pid) {
    shared_mem_free(arg);
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

// Kill all subprocesses on failure
void kill_all_children(int status, void* arg) {
  if(status == EXIT_FAILURE && getpid() == main_pid) {
    pidArr_t* pid_arr = arg;
    for(unsigned i = 0; i < pid_arr->count; i++) {
      kill(pid_arr->pids[i], SIGTERM);
    }
  }
}

// Cherge range of argument and strtol it
int get_arg(const char *str, int idx, int low, int high) {
  char *endptr;
  long val = strtol(str, &endptr, 10);

  if (str == endptr || *endptr != '\0') {
    error_exit("Argument at position %d has invalid format: %s\n", idx, str);
  }
  if (val < low || val > high) {
    error_exit("Argument at position %d is out of range from %d to %d: %d\n", idx, low, high, val);
  }

  return (int)val;
}


int main(int argc, char** argv) {
  // Check and parse arguments
  if(argc != ARG_COUNT)
  error_exit("Invalid number of arguments %d, should be %d.\n", argc, ARG_COUNT);

  int bounds_low[] =  { 1, 1,     4,  0,    0,    1   };
  int bounds_high[] = { 9, 9999,  40, 1000, 1000, 100 };
  int parsed_values[ARG_COUNT-1];

  for(int i = 1; i < ARG_COUNT; i++) {
    parsed_values[i-1] = get_arg(argv[i], i, bounds_low[i-1], bounds_high[i-1]);
  }

  args_t args;
  args.V  = parsed_values[0];
  args.N  = parsed_values[1];
  args.K  = parsed_values[2];
  args.TV = parsed_values[3];
  args.TN = parsed_values[4];
  args.O  = parsed_values[5];


  // Open file
  output_file = fopen("proj2.out", "w");
  if(!output_file) {
    error_exit("Failed to open output file!\n");
  }
  setbuf(output_file, NULL); // Turn off write buffer, needed for multiple process write
  on_exit(close_file_wrapper, (void*)output_file);
  
  // Initialize pid array
  main_pid = getpid();
  if(!pid_arr_init(&pid_arr, 1 + args.V + args.N)) {
    error_exit("Process ID array allocation failed!\n");
  }
  on_exit(pid_arr_free, &pid_arr);

  // Initialize shared memory
  if(!(shared_mem = shared_mem_map())) {
    error_exit("Failed to map shared memory.\n");
  }
  on_exit(free_mem_wrapper, (void*)shared_mem);
  if(!shared_mem_init(shared_mem)) {
    error_exit("Failed to initialize shared memory!\n");
  }

  // Initialize semaphore array for carts in shared mem
  if(!shared_carts_map(shared_mem, args.V)) {
    error_exit("Failed to map shared carts array!\n");
  }
  on_exit(free_carts_arr_wrapper, &shared_mem->carts_arr);
  if(!shared_carts_init(&shared_mem->carts_arr)) {
    error_exit("Failed to initialize cart array in shared memory!\n");
  }

  // Start sub processes
  on_exit(kill_all_children, (void*)&pid_arr);
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

  // Wait for all children to end
  int status;
  while(wait(&status) > 0) {
    if(WIFEXITED(status)) {
      int code = WEXITSTATUS(status);

      if(code) {
        error_exit("Child process ended with error code %d.\n", code);
      }
    } else if(WIFSIGNALED(status)) {
      error_exit("Child process was terminated by %d signal.\n", WTERMSIG(status));
    }
  }

  return(EXIT_SUCCESS);
}