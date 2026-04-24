#include <stdio.h>
#include <stdlib.h>
#include "sub_processes.h"


void dispatcher(unsigned id, args_t* args) {
  (void)id;
  printf("D: started\n");
  printf("D: delay: %d\n", args->O);
  printf("D: closing\n");
  exit(EXIT_SUCCESS);
}

void cart(unsigned id, args_t* args) {
  printf("V %d: started\n", id);
  
  printf("V %d: capacity: %d\n", id, args->K);
  printf("V %d: delay: %d\n", id, args->TV);

  printf("V %d: closing\n", id);
  exit(EXIT_SUCCESS);
}

void visitor(unsigned id, args_t* args) {
  printf("N %d: started\n", id);
  printf("N %d: delay: %d\n", id, args->TN);
  printf("N %d: leaving\n", id);
  exit(EXIT_SUCCESS);
}