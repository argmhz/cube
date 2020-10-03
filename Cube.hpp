#pragma once
#include <cstdint>
#include <iostream>
#include <dlfcn.h>
#include <bitset>
#include <sys/time.h>
#include "Animation.hpp"

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3
struct timeval timeStart, timeEnd;

class Cube {
  private:
   class Animation *animation;

   bool isRunning = false;

   char engine[8][4][24] = {};
   char* ep = &engine[0][0][0];

   std::bitset<4> frontBuffer[8][192] = {};
   int tr[192] = {
      7,  8,  3,  4,  5,  0,  1,  2,
     26, 25, 24, 29, 28, 27, 32, 31,
     17, 12, 13, 14,  9, 10, 11,  6,
     30, 35, 34, 33, 38, 37, 36, 41,
     21, 22, 23, 18, 19, 20, 15, 16,
     40, 39, 44, 43, 42, 47, 46, 45,
     69, 70, 71, 66, 67, 68, 63, 64,
     88, 87, 92, 91, 90, 95, 94, 93,
     65, 60, 61, 62, 57, 58, 59, 54,
     78, 83, 82, 81, 86, 85, 84, 89,
     55, 56, 51, 52, 53, 48, 49, 50,
     74, 73, 72, 77, 76, 75, 80, 79,
    103,104, 99,100,101, 96, 97, 98,
    122,121,120,125,124,123,128,127,
    113,108,109,110,105,106,107,102,
    126,131,130,129,134,133,132,137,
    117,118,119,114,115,116,111,112,
    136,135,140,139,138,143,142,141,
    165,166,167,162,163,164,159,160,
    184,183,188,187,186,191,190,189,
    161,156,157,158,153,154,155,150,
    174,179,178,177,182,181,180,185,
    151,152,147,148,149,144,145,146,
    170,169,168,173,172,171,176,175};

  public:

  	Cube(){
      clear();
    }
    ~Cube(){
      clear();
      delete animation;
    }

    void clear() {
      for (size_t i = 0; i < 192; i++) {
        for (size_t k = 0; k < 8; k++) {
          frontBuffer[k][i] = 0;
        }
      }
    }
    bool inBounce(int x,int y,int z){
        return ((x < 8 && x >= 0) && (y < 8 && y >= 0) && (z < 8 && z >= 0 ));
    }
    void set(int x, int y, int z, int r, int g, int b ){
      if(inBounce(x,y,z)){
        int index = (z*8+x)*3;
  			frontBuffer[y][tr[index]] = r;
        frontBuffer[y][tr[index+1]] = g;
  	    frontBuffer[y][tr[index+2]] = b;
      }
    }

    struct Color{
      int red;
      int green;
      int blue;
    };

    Color get(uint8_t x, uint8_t y, uint8_t z){
      Color c;
      int index = (z*8+x)*3;
      c.red = (int)frontBuffer[y][tr[index]].to_ulong();
      c.green = (int)frontBuffer[y][tr[index+1]].to_ulong();
      c.blue = (int)frontBuffer[y][tr[index+2]].to_ulong();
      return c;
    }
    void update() {

      gettimeofday(&timeStart, NULL);

          createFrame();


      gettimeofday(&timeEnd, NULL);

      std::cout << "This effing slow piece of code took " << ((timeEnd.tv_sec - timeStart.tv_sec) * 1000000 + timeEnd.tv_usec - timeStart.tv_usec) << " us to execute." << std::endl;
       // for (size_t i = 0; i < 8; i++) {
       //  for (size_t y = 0; y < 4; y++) {
       //    for (size_t r = 0; r < 24; r++) {
       //      std::bitset<8> b(engine[i][y][r]);
       //        std::cout <<  b << " ";
       //    }
       //    std::cout << "\n";
       //  }
       //    std::cout << "\n";
       // }
      // std::cout << leds->get(0,0,0).red << std::endl;

    }

    void run(){

      animation->draw(this);

      // push layer byte
      // push the 24 cube bytes
    }

    void loadAnimation(char const* name) {

      void* TestAnimation = dlopen(name, RTLD_NOW);

      if (!TestAnimation) {
            std::cerr << "Cannot open library: " << dlerror() << '\n';
            return;
      }

      dlerror();

      create_t* create_animation = (create_t*) dlsym(TestAnimation, "create");
      const char* dlsym_error = dlerror();

      if (dlsym_error) {
           std::cerr << "Cannot load symbol create: " << dlsym_error << '\n';
           return;
       }

      destroy_t* destroy_animation = (destroy_t*) dlsym(TestAnimation, "destroy");

      dlsym_error = dlerror();

      if (dlsym_error) {
         std::cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
         return;
      }

      if(animation){
        destroy_animation(animation);
      }

      animation = (Animation*)create_animation();
    }

    void plane(uint8_t axis,uint8_t index,uint8_t r,uint8_t g,uint8_t b);
  	void line(uint8_t x1,uint8_t y1,uint8_t z1,uint8_t x2,uint8_t y2,uint8_t z2,uint8_t r,uint8_t g,uint8_t b);
  	void shift(uint8_t axis,uint8_t direction);
    void rotate(double pitch,double roll,double yaw);

  private:
    void createFrame(){
      for (int s = 0; s < 8; ++s){
        for (int i = 0; i < 4; ++i){
          for (int j = 0; j < 24; ++j){
            engine[s][i][j] = 0;
            for (int bit = 0; bit < 8; ++bit) {
              engine[s][i][j] |= frontBuffer[s][j * 8 + bit][i] << bit;
            }
          }
        }
      }
    }


};
