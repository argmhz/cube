#include <iostream>
#include <thread>
#include <cstring>
#include <vector>

#include "../lib/core/Cube.h"
#include "../lib/helpers.h"
#include "../lib/animation/Font.h"

Cube * cube = new Cube;
int r,g,b;


void animationChanger(Cube *cube, char *str){

  cube->clear();
  cube->update();
  sleep(1);

  int strLength = strlen(str);

  // cube->plane(AXIS_X,0,15,0,0);
  // cube->update();

  while(cube->isRunning()){
    for(int c=0;c<strLength;c++){

      std::array<std::array<int,8>,8> items = Font::asArray(str[c]);

      for (signed x = 0; x < 8; x++) {
        for (signed y = 0; y < 8; y++) {
           if(items[y][x]){
             // cube->set(7,y,4,r,g,b);
             cube->set(7,y,0,r,g,b);
             cube->set(7,y,1,r,g,b);
             cube->set(7,y,2,r,g,b);
             cube->set(7,y,3,r,g,b);
             cube->set(7,y,4,r,g,b);
             cube->set(7,y,5,r,g,b);
             cube->set(7,y,6,r,g,b);
             cube->set(7,y,7,r,g,b);
           }

        }

        cube->shift(AXIS_X,-1);
        cube->update();
        usleep(80000);
      }

    }

    // cube->clearPlane(AXIS_X,4);
    cube->clearPlane(AXIS_X,5);
    cube->update();
    sleep(1);
  }

}

void changeColor(){
  int i = 0;
  while(true){
    Cube::Color c = makeColorGradient(i);
    i++;
    r = c.red;
    g = c.green;
    b = c.blue;

    if(i == 255){
      i = 0;
    }
    usleep(20000);
  }
}

int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));
  // std::cout << argc;
  if(argc <= 1){
    std::cout << "No text!" << std::endl;
    return 1;
  }


  std::thread t(animationChanger, cube,argv[1]);
  std::thread c = cube->start();
  std::thread cc(changeColor);

  t.join();
  c.join();
  cc.join();
}
