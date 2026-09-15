#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/vendor/json.hpp"

class Rain : public Animation {

  int speed = 65000;
  int max_drops = 4;
  int tens = 3;

  void onDataUpdate(json data){

    if(data["speed"].is_number()){
      speed = data["speed"].get<int>();
    }

    if(data["max_drops"].is_number()){
      max_drops = data["max_drops"].get<int>();
    }

    if(data["tens"].is_number()){
      tens = data["tens"].get<int>();
    }
  }

  void draw(Cube *c) {

    int i;

    int target = 1;

    while(isRunning()){
      int prev[max_drops][3] = {};

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
          for (int z = 0; z < 8; z++) {
            if(!c->isOff(x,0,z)){
              c->set(x,0,z,15,0,15);
            }
          }
        }

        usleep(speed);
    }
  }

};
extern "C" Animation * create() {
    return new Rain;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
