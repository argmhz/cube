#pragma once

#include <bcm2835.h>
#include <unistd.h>
#include <thread>
#include "CubeBuffer.h"

// Drives the physical LED cube: owns the bit-angle-modulation render loop
// and the SPI/GPIO I/O. All voxel buffer state and geometry live in
// CubeBuffer, which this class inherits so callers keep using cube->set(),
// cube->line() etc. exactly as before.
class Cube : public CubeBuffer {
  private:

   bool running = true;

   int bam_counter = 0;
   int bam_bit = 0;

   char layers[8] = {128,64,32,16,8,4,2,1};

   char engine[8][4][24] = {};
   char r_engine[24] = {};

  public:
  	Cube(){
      initBcm2835();
      clear();
    }
    ~Cube(){
      clear();

      bcm2835_spi_end();
      bcm2835_close();
    }

    bool isRunning(){
      return running;
    }

    void update() {
      createFrame();
    }

    void run(){

      while (running) {

        if(bam_counter == 1 || bam_counter == 3 || bam_counter == 7){
          bam_bit++;
        }

        bam_counter++;
        for (size_t layer = 0; layer < 8; layer++) {
          // disable shift registers output
          bcm2835_gpio_write(RPI_GPIO_P1_15, HIGH);
          // transfer layer select byte
          bcm2835_spi_transfer(layers[layer]);
          // transfer layer data
          bcm2835_spi_transfernb(engine[layer][bam_bit],r_engine,24);
          // enable shift registers output
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

    void stop(){
      running = false;
    }

    std::thread start(){
      running = true;
      std::thread cubeThread([this] {
        this->run();
      });
      return cubeThread;
    }

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
