#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>
#include <cstring>
#include <bitset>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include "../resources/fonts.cpp"





Cube * cube = new Cube;

void font(Cube *c,char chr, int x,int y, int z,int r,int g,int b){

  int pos[8] = {7,6,5,4,3,2,1,0};

  for(int _y=0;_y<8;_y++){
    std::bitset<8> bit(fonts[chr][_y]);
    for (int _x = 0;_x<8; _x++) {
      if(bit[_x]){
        c->set(_x+x,pos[_y]+y,z,r,g,b);
      }
    }
  }
}

void animationChanger(Cube *c, char *str){

  c->clear();
  c->update();
  sleep(1);

  int strLength = strlen(str);

  for(int x=0;x<strLength;x++){

      for (signed i = -3; i < 8; i++) {
        font(c,str[x],i,0,7,15,15,15);
        c->update();
        usleep(500000);
        c->clear();
      }

  }

  c->update();
}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animationChanger, cube,argv[1]);
  std::thread c = cube->start();

  t.join();
  c.join();
}
