
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class DiamondWave : public Animation {

  void draw(Cube *cube) {

    int z1,z;

    while(isRunning()){
      for (int j=0; j<10; j++){  // go up and down 10 times
     /*   colorCount=colorCount+10;  // change color with each pass
        if (colorCount>180) {
          colorCount=10;
        }
    */
        for (int i=0; i<18; i++){ // moving upward

          for (int xx=0; xx<8; xx++){ // Here we generate both the upper and lower pyramid
            for (int yy=0; yy<8; yy++){ // If actually starts out as a cone, but flairs out to a square at its widest.
              z= int(i - 9.5 + sqrt((3.5-xx)*(3.5-xx)+(3.5-yy)*(3.5-yy)));
              z1= int(i - 0.5 - sqrt((3.5-xx)*(3.5-xx)+(3.5-yy)*(3.5-yy)));
              if (z>-1 && z<8) {
                cube->set(xx,yy, z, 0,0,15);
              }
              if (z1>-1 && z1<8) {
                cube->set(xx,yy, z1, 0,0,15);
              }

            }
          }
          cube->update();
          usleep(30000);       // Increase or decrease to change speed of this animation
          cube->clear();    // clear the cube
        }
        for (int i=16; i>0; i--){ //moving downward

          for (int xx=0; xx<8; xx++){ //
            for (int yy=0; yy<8; yy++){
              z= int(i - 9.5 + sqrt((3.5-xx)*(3.5-xx)+(3.5-yy)*(3.5-yy)));
              z1= int(i - 0.5 - sqrt((3.5-xx)*(3.5-xx)+(3.5-yy)*(3.5-yy)));
              if (z>-1 && z<8) {
                cube->set(xx,yy, z, 0,0,15);
              }
              if (z1>-1 && z1<8) {
                cube->set(xx,yy, z1, 0,0,15);
              }
            }

          }
          cube->update();
          usleep(30000);       // Increase or decrease to change speed of this animation
          cube->clear();    // clear the cube
        }
      }
  }


  }

};
extern "C" Animation * create() {
    return new DiamondWave;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
