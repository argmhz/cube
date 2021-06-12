#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class Rain : public Animation {

  void draw(Cube *c) {

    int i;

    int target = 1;
    int max_drops = 4;
    int tens = 3;
    int prev[max_drops][3] = {};
    int speed = 80000;

    while(isRunning()){

        for(i =0; i<max_drops;i++){
          if(prev[i][0] || prev[i][1] || prev[i][2] ){
            c->set(prev[i][0],prev[i][1],prev[i][2],0,0,3);
          }

          prev[i][0] = 0;
          prev[i][1] = 0;
          prev[i][2] = 0;
        }

        for(i =0; i<max_drops;i++){

          if((rand()%tens) != target){
            continue;
          }

          int x = rand() % 8;
          int z = rand() % 8;
          int y = 7;

          prev[i][0] = x;
          prev[i][1] = y;
          prev[i][2] = z;

          c->set(x,y,z,0,0,15);
        }

        c->update();
        c->shift(AXIS_Y,-1);

        for(int x = 0;x<8;x++){
          for (int z = 0; z < 8; z++)
          {
              //int index = (x+z*8+7*64)*3+2;
              // Cube::Color color = c->get(x,7,z);
              // if(color.r != 0 || color.g != 0 || color.b != 0){
              if(!c->isOff(x,0,z)){
                c->set(x,0,z,15,0,15);
              }

              // }

          }
        }

        usleep(speed);
    }
  }

  // void onDataUpdate(std::vector<std::string> data){
  //
  // }
};
extern "C" Animation * create() {
    return new Rain;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
