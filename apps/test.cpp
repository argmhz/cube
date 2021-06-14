#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <cstring>
#include <bitset>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../lib/Font.cpp"

Cube * cube = new Cube;
 


void animationChanger(Cube *cube, char *str){

  cube->clear();
  cube->update();
  sleep(1);

  int strLength = strlen(str);

  for(int c=0;c<strLength;c++){

    int *items = Font::asArray(str[c]);
    int pos[8] = {0,8,16,24,32,40,48,56};

    for (size_t y = 0; y < 8; y++) {
      for (size_t x = 0; x < 8; x++) {
        std::cout << x;
        if(items[pos[x]+y]){
          std::cout << pos[y]+x << " ";
          cube->set(x,pos[x]+y,7,0,0,15);
        }
      }
      std::cout << std::endl;
      cube->shift(AXIS_X,-1);
      cube->update();
      usleep(100000);
    }


    // for (int x = 0; x < 8; x++) {
    //   for (int y = 0; y < 8; y++) {
    //     if(items[x*8+y])
    //       cube->set(x,y,7,0,0,15);
    //   }
    // }


  }

  cube->update();
}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animationChanger, cube,argv[1]);
  std::thread c = cube->start();

  t.join();
  c.join();
}
