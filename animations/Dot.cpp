
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"

class Dot : public Animation {

  int x = 3;
  int y = 3;
  int z = 3;

  Cube::Color color;
  std::vector<Cube::Point> snake;
  Cube::Point pos;
  Cube::Point mouse;
  int speed = 1000000;
  int direction = 1;


  void onDataUpdate(json data){

    if(data["com"].is_number()){
      direction = data["com"].get<int>();


    }

  }

  void moveDirection() {

    switch (direction) {
      case 1:
        pos.x++;
      break;
      case 2:
        pos.x--;
      break;
      case 3:
        pos.y++;
      break;
      case 4:
        pos.y--;
      break;
      case 5:
        pos.z++;
      break;
      case 6:
        pos.z--;
      break;
    }

    if(pos.x > 7){
      pos.x = 0;
    }

    if(pos.x < 0){
      pos.x = 7;
    }

    if(pos.y > 7){
      pos.y = 0;
    }

    if(pos.y < 0){
      pos.y = 7;
    }

    if(pos.z > 7){
      pos.z = 0;
    }

    if(pos.z < 0){
      pos.z = 7;
    }
  }

  void generateMouse(){
    bool collision;
    do {
      mouse.x = rand() % 8;
      mouse.y = rand() % 8;
      mouse.z = rand() % 8;

      collision = false;
      for (auto&& point : snake) {
        if(point.x == mouse.x && point.y == mouse.y && point.z == mouse.z){
          collision = true;
          break;
        }
      }
    } while (collision);
  }

  bool isMouseHit(){
    return (pos.x == mouse.x && pos.y == mouse.y && pos.z == mouse.z);
  }

  void popSnake(){
    if (snake.size() > 1) {
        snake.erase(snake.begin());
    }
  }

  bool checkCollision(){

    for (size_t i = 1; i < snake.size(); i++) {
      if(pos.x == snake[i].x && pos.y == snake[i].y && pos.z == snake[i].z) {
          return true;
      }
    }

    return false;
  }

  void gameOver(Cube *cube){

    pos.x = x;
    pos.y = y;
    pos.z = z;

    color.set(0,0,15);
    direction = rand() % 6;
    snake = {};

    for (size_t i = 0; i < 5; i++) {
      cube->clear();
      cube->boxOutline(0,0,0,7,7,7,15,15,15);
      cube->update();
      usleep(300000);
    }

  }

  void draw(Cube *cube) {

    pos.x = x;
    pos.y = y;
    pos.z = z;

    color.set(0,0,15);
    direction = rand() % 6;

    generateMouse();

    while(isRunning()){
      cube->clear();

      moveDirection();

      if(checkCollision()){
        gameOver(cube);
        continue;
      }

      snake.push_back(pos);

      cube->set(mouse.x,mouse.y,mouse.z,15,15,0);

      if(!isMouseHit()){
        popSnake();

      }
      else
      {
        speed = speed / 1.1;
        generateMouse();
      }

      for (auto&& point : snake){
        cube->set(point.x,point.y,point.z,color.red,color.green,color.blue);
      }



      cube->update();
      usleep(speed);
    }

  }

};




extern "C" Animation * create() {
    return new Dot;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
