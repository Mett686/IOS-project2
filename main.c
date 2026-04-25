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

sharedData_t* shared_mem = NULL;
static pid_t main_pid;

// Array for tracking child processes
typedef struct {
  unsigned count;
  unsigned size;
  pid_t* pids;
} pidArr_t;

// Close file on end
void main_close_file(int status, void* arg) {
  (void)status;
  if(getpid() == main_pid) {
    fclose((FILE*)arg);
  }
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

void pid_arr_free(int status, void* arg) {
  (void)status;
  if(getpid() == main_pid) {
    pidArr_t* arr_struct = (pidArr_t*)arg;
    free(arr_struct->pids);
  }
}

// rework this shit
void pid_arr_add(pidArr_t* pid_arr, pid_t pid) {
  if(pid_arr->count < pid_arr->size) {
    pid_arr->pids[pid_arr->count] = pid;
    pid_arr->count++;
  } else {
    error_exit("pid array full!\n");
  }
}

void free_shared_memory(int status, void* arg) {
  (void)status;
  if(getpid() == main_pid) {
    shared_mem_free((sharedData_t*)arg);
  }
}

// Start sub-process
pid_t start_sub_process(args_t* args, unsigned id, FILE* file, void (*process_function)(unsigned id, args_t* args, FILE* file)) {
  pid_t pid = fork();
  if(pid == -1) {
    error_exit("Failed to fork main!\n");
  } else if(pid) {
    return pid;
  } else {
    process_function(id, args, file);
    error_exit("Process function did not end process!\n");
  }
  return -1;
}

void kill_all_children(int status, void* arg) {
  if(status == EXIT_FAILURE && getpid() == main_pid) {
    pidArr_t* pid_arr = arg;
    for(unsigned i = 0; i < pid_arr->count; i++) {
      kill(pid_arr->pids[i], SIGTERM);
    }
  }
}

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
  FILE* file = fopen("proj2.out", "w");
  if(!file) {
    error_exit("Failed to open output file!\n");
  }
  setbuf(file, NULL); // Turn off write buffer, needed for multiple process write
  on_exit(main_close_file, (void*)file); // close file on exit
  
  // Initialize pid array
  pid_t main_pid = getpid();
  pidArr_t pid_arr;
  if(!pid_arr_init(&pid_arr, 1 + args.V + args.N)) {
    error_exit("Process ID array allocation failed!\n");
  }
  on_exit(pid_arr_free, &pid_arr);
  // Initialize shared memory
  if(!(shared_mem = shared_mem_map())) {
    error_exit("Failed to map shared memory.\n");
  }
  on_exit(free_shared_memory, (void*)shared_mem);
  if(!shared_mem_init(shared_mem)) {
    error_exit("Failed to initialize shared memory!\n");
  }


  printf("Main pid: %d\n", (int)main_pid);



  on_exit(kill_all_children, (void*)&pid_arr);
  // Start dispatcher
  int pid = start_sub_process(&args, 0, file, dispatcher);
  pid_arr_add(&pid_arr, pid);

  // Start carts
  for(int i = 1; i <= args.V; i++) {
    pid = start_sub_process(&args, i, file, cart);
    pid_arr_add(&pid_arr, pid);
  }

  // Start visitors
  for(int i = 1; i <= args.N; i++) {
    pid = start_sub_process(&args, i, file, visitor);
    pid_arr_add(&pid_arr, pid);
  }

  printf("%d\n",shared_mem->counter);

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

  shared_mem_free(shared_mem);
  return(EXIT_SUCCESS);
}