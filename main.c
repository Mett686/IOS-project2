#include <stdlib.h>
#include <stdio.h>
#include "error.h"


int main(int argc, char** argv) {
  if(argc != 7)
  error_exit("Invalid number of arguments!\n");
  
  int carriage_count = (int)atoi(argv[1]);
  int visitor_count = (int)atoi(argv[2]);
  int carriage_capacity = (int)atoi(argv[3]);
  int ride_duration = (int)atoi(argv[4]);
  int queue_delay = (int)atoi(argv[5]);
  int carriage_interval = (int)atoi(argv[6]);
  
  // V: počet vozíků, 0<V<10
  if(!(0 < carriage_count && carriage_count < 10))
  error_exit("Invalid argument V: %d\n", carriage_count);
  
  // N: počet návštěvníků, 0<N<10000
  if(!(0 < visitor_count && visitor_count < 10000))
  error_exit("Invalid argument N: %d\n", visitor_count);
  
  // K: kapacita vozíku, 4<=K<=40
  if(!(4 <= carriage_capacity && carriage_capacity <= 40))
  error_exit("Invalid argument K: %d\n", carriage_capacity);
  
  // TV: Doba jízdy vozíku po dráze v mikrosekundách, 0<=TV<=1000
  if(!(0 <= ride_duration && ride_duration <= 1000))
  error_exit("Invalid argument TV: %d\n", ride_duration);
  
  // TN: Maximální doba, než návštěvník dojde do fronty v mikrosekundách, 0<=TN<=1000
  if(!(0 <= queue_delay && queue_delay <= 1000))
  error_exit("Invalid argument N: %d\n", queue_delay);
  
  // O: Minimální časový odstup vozíků v mikrosekundách, 0<O<=100
  if(!(0 < carriage_interval && carriage_interval <= 100))
    error_exit("Invalid argument V: %d\n", carriage_interval);
}