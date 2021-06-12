
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"
#include <algorithm>

class UpdownColor : public Animation {

  void draw(Cube *cube) {

    int nc = 3;

    int c[nc][3] = {
        {15,0,0},
        {0,15,0},
        {0,0,15}
    };

    cube->plane(AXIS_Y,0,c[nc-1][0],c[nc-1][1],c[nc-1][2]);
    cube->update();
    sleep(1);
    int leds[64];
    int i,x,z,cindex = 0;

    for(i=0;i<64;i++){
        leds[i] = i;
    }

    // std::srand(time(0));

    while(isRunning()){
        std::random_shuffle(std::begin(leds),std::end(leds));

        for (i = 0; i < 64; ++i)
        {

            x = (int)leds[i]/8;
            z = leds[i] % 8;

            for(int y = 0; y<8;y++){
                cube->clear(x,y-1,z);

                cube->set(x,y,z,c[cindex][0],c[cindex][1],c[cindex][2]);

                cube->update();
                usleep(30000);
            }
            usleep(100000);
        }

        if(cindex < nc-1){
            cindex++;
        }else{
            cindex=0;
        }

        std::random_shuffle(std::begin(leds),std::end(leds));

        for (i = 64; i >= 0; i--)
        {

            x = (int)leds[i]/8;
            z = leds[i] % 8;

            for(int y = 8; y>=0;y--){
                cube->set(x,y+1,z,0,0,0);
                cube->set(x,y,z,c[cindex][0],c[cindex][1],c[cindex][2]);

                cube->update();
                usleep(3000);
            }
            usleep(1000);
        }

        if(cindex < nc-1){
            cindex++;
        }else{
            cindex=0;
        }

    }




  }

};














extern "C" Animation * create() {
    return new UpdownColor;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
