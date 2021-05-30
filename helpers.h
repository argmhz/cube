#include <stdio.h>
#include <ctime>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

std::clock_t c_start = std::clock();


  int random(int min, int max) {

    return rand() % max + min;
  }

  int random(int max) {
    return random(0,max);
  }

  unsigned long millis(){
    std::clock_t c_end = std::clock();
    return 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC;
  }
