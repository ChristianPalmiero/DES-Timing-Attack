#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include <utils.h>
//#include <des.h>
#include <rdtsc_timer.h>

int main (int argc, char* argv[]){
  uint64_t val[4] = {0x1, 0x3, 0x7, 0xF};
  uint64_t a, b, t, temp;
  double sum, H0;
  int i, j;

  sum = 0.0;
  for(i=0; i<100000; i++){
    a = get_rdtsc_timer();
    temp = des_p_ta(0x0);
    //printf("P output: %d\n", temp);    
    b = get_rdtsc_timer();
    t = b - a;
    sum += t;
    //printf("Time it takes for Hamming weight = 1: %d\n", t);
  }
  printf("Avg time H0: %f\n", sum/100000);

  for(j=0; j<4; j++){
    sum = 0.0;
    for(i=0; i<100000; i++){
      a = get_rdtsc_timer();
      temp = des_p_ta(val[j]);
      //printf("P output: %d\n", temp);    
      b = get_rdtsc_timer();
      t = b - a;
      sum += t;
      //printf("Time it takes for Hamming weight = 1: %d\n", t);
    }
    printf("Avg time H%d: %f\n", j+1, sum/100000);
  } 

  return 0;
}
