#include <iostream>
#include <thread>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"


Cube * cube = new Cube;


Cube::Color getColor(){
  Cube::Color c;
  c.red = rand() % 16;
  c.green = rand() % 16;
  c.blue = rand() % 16;
  return c;
}

void clear(int speed) {

  int direction = (rand() % 2) + 1;
  int axis = (rand() % 3) + 1;

  if(direction == 1) {
    for (int i = 0; i < 8; i++) {
      cube->clearPlane(axis, i);
      cube->update();
      usleep(speed);
    }
  }
  else
  {
    for (int i = 7; i >= 0; i--) {
      cube->clearPlane(axis, i);
      cube->update();
      usleep(speed);
    }
  }


}

void draw(int speed ){
  int direction = (rand() % 2) + 1;
  int axis = (rand() % 3) + 1;

  Cube::Color c = getColor();

  if(direction == 1){
    for (int i = 0; i < 8; i++) {
      cube->plane(axis, i, c);
      cube->update();
      usleep(speed);
    }
  }
  else
  {
    for (int i = 7; i >= 0; i--) {
      cube->plane(axis, i, c);
      cube->update();
      usleep(speed);
    }
  }



}

void animation(Cube *cube){

int speed = 50000;

cube->all(getColor());
cube->update();
usleep(speed);

while(cube->isRunning()){
    clear(speed);
    draw(speed);
}


}



int main(int argc, char *argv[]){
  setbuf(stdout, NULL);
  srand (time(NULL));

  std::thread t(animation, cube);
  std::thread c = cube->start();

  t.join();
  c.join();

}
