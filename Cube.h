#pragma once

#include <bcm2835.h>
// #include <cstdint>
#include <iostream>
#include <dlfcn.h>
#include <bitset>
#include <unistd.h>
// #include <sys/time.h>
#include "Animation.hpp"
#include <time.h>
// #include <chrono>
#include <thread>

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3



class Cube {
  private:
   class Animation *animation;

   bool isRunning = true;

   int bam_counter = 0;
   int bam_bit = 0;

   char layers[8] = {128,64,32,16,8,4,2,1};

   char engine[8][4][24] = {};
   char r_engine[24] = {};

   std::bitset<4> frontBuffer[8][192] = {};

   int tr[192] = {
     189,190,191,186,187,188,175,184,
     185,172,173,174,169,170,171,158,
     159,168,155,156,157,152,153,154,
     178,177,176,181,180,179,160,183,
     182,163,162,161,166,165,164,145,
     144,167,148,147,146,151,150,149,
     109,110,111,106,107,108,127,104,
     105,124,125,126,121,122,123,142,
     143,120,139,140,141,136,137,138,
     98,97,96,101,100,99,112,103,102,
     115,114,113,118,117,116,129,128,
     119,132,131,130,135,134,133, 93,
      94, 95, 90, 91, 92, 79, 88, 89,
      76, 77, 78, 73, 74, 75, 62, 63,
      72, 59, 60, 61, 56, 57, 58, 82,
      81, 80, 85, 84, 83, 64, 87, 86,
      67, 66, 65, 70, 69, 68, 49, 48,
      71, 52, 51, 50, 55, 54, 53, 13,
      14, 15, 10, 11, 12, 31,  8,  9,
      28, 29, 30, 25, 26, 27, 46, 47,
      24, 43, 44, 45, 40, 41, 42,  2,
      1, 0, 5,  4,  3, 16,  7,  6, 19,
      18, 17, 22, 21, 20, 33, 32, 23,
      36, 35, 34, 39, 38, 37};


  public:

  	Cube(){
      initBcm2835();
      clear();
    }
    ~Cube(){
      clear();

      bcm2835_spi_end();
      bcm2835_close();

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

      if(index >= 192){
        index = 191;
      }
      if(index < 0){
        index = 0;
      }

      c.red = (int)frontBuffer[y][tr[index]].to_ulong();
      c.green = (int)frontBuffer[y][tr[index+1]].to_ulong();
      c.blue = (int)frontBuffer[y][tr[index+2]].to_ulong();
      return c;
    }

    void update() {
      createFrame();
    }

    void run(){

      while (isRunning) {

        if(bam_counter == 1 || bam_counter == 3 || bam_counter == 7){
          bam_bit++;
        }

        bam_counter++;
        for (size_t layer = 0; layer < 8; layer++) {
          // disable shift registers
          bcm2835_gpio_write(RPI_GPIO_P1_15, HIGH);
          // transfer layer select byte
          bcm2835_spi_transfer(layers[layer]);
          // transfer layer data
          bcm2835_spi_transfernb(engine[layer][bam_bit],r_engine,24);
          // enable shift registers
          bcm2835_gpio_write(RPI_GPIO_P1_15, LOW);

          // latch pin
          bcm2835_gpio_write(RPI_GPIO_P1_11, LOW);
          usleep(1);
          bcm2835_gpio_write(RPI_GPIO_P1_11, HIGH);

        }

        if(bam_counter == 15){
          bam_bit = 0;
          bam_counter = 0;
        }

      }
      clear();
      update();
    }

    void setDirect(int layer, int pos){
      frontBuffer[layer][pos] = 15;
    }

    void setIndex(int layer, int pos, int value){
      frontBuffer[layer][tr[pos]] = value;
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

    void stop(){
      isRunning = false;
    }

    void start(){
      std::thread cubeThread([this] {
        this->run();
      });
      isRunning = true;
      animation->draw(this);
      cubeThread.join();
    }

    void plane(int axis,int index,int r,int g,int b);

    void swapint(int & one, int & two) {
	  	one = one^two;
	  	two = one^two;
	  	one = one^two;
  	}

    void clear(int x,int y,int z){
		    set(x,y,z,0,0,0);
	  }

    int roundClostest(int numerator, int denominator) {
  	  	numerator = (numerator << 1)/denominator;
  	  	int output = (numerator>>1) + (numerator % 2);
  	  	return output;
  	}

  	void line(int x1,int y1,int z1,int x2,int y2,int z2,int r,int g,int b);
  	void shift(int axis,int direction);
    void rotate(double pitch,double roll,double yaw);
    void all(int r, int g,int b);
  private:
    void createFrame(){
      for (int s = 0; s < 8; ++s){
        for (int i = 0; i < 4; ++i){
          for (int j = 0; j < 24; ++j){
            char &c = engine[s][i][j];
            c = 0;
            for (int bit = 0; bit < 8; ++bit) {
              c |= frontBuffer[s][j * 8 + bit][i] << bit;
            }
          }
        }
      }
    }

    bool initBcm2835(){

      if (!bcm2835_init()) {
        printf("init failed\n");
        return false;
      }

      if(!bcm2835_spi_begin()){
        printf("bcm2835_spi_begin failed. Are you running as root??\n");
        return false;
      }

      bcm2835_gpio_fsel(RPI_GPIO_P1_11, BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_fsel(RPI_GPIO_P1_15, BCM2835_GPIO_FSEL_OUTP);
      bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);

      return true;
    }

};
