#include <cstring>
#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"
#include "../lib/animation/Font.h"
#include "../lib/vendor/json.hpp"

class Text : public Animation {

  std::string str = "";
  std::string tmp = "Topper 3D ";
  int color_speed = 22000;
  int speed = 80000;
  int r = 15;
  int g = 15;
  int b = 15;
public:

  void setText(char* string) {
    tmp = std::string(string);
  }

  void setText(std::string string){
    tmp = string;
  }

  void onDataUpdate(json data){
    if(data["text"].is_string()){
        tmp = data["text"].get<std::string>();
    }

    if(data["color_speed"].is_number()){
      int value = data["color_speed"].get<int>();
      if(value > 0){
        color_speed = value;
      }
    }

    if(data["speed"].is_number()){
      int value = data["speed"].get<int>();
      if(value > 0){
        speed = value;
      }
    }
  }

  void changeColor(){
    int i = 0;
    while(isRunning()){
      Cube::Color c = makeColorGradient(i);
      i++;
      r = c.red;
      g = c.green;
      b = c.blue;

      if(i == 255){
        i = 0;
      }
      usleep(color_speed);
    }
  }

  void draw(Cube *cube) {

    std::thread ColorChangerThread([this]{ this->changeColor(); });

      while(isRunning()){

        // Exactly one trailing space, whatever the text ends with, so the
        // last character never runs straight into the first one when the
        // text comes round again.
        str = Font::renderable(tmp);
        while(!str.empty() && str.back() == ' '){
          str.pop_back();
        }
        str += ' ';

        for(signed c=0;c<str.length();c++){

          std::array<std::array<int,8>,8> items = Font::asArray(str[c]);

          for (signed x = 0; x < 8; x++) {
            for (signed y = 0; y < 8; y++) {
               if(items[y][x]){
                 cube->set(7,y,7,r,g,b);
               }
            }
            cube->update();
            cube->shift(AXIS_X,-1);
            usleep(speed);
          }

        }
      }

      ColorChangerThread.join();
  }



};
extern "C" Animation * create() {
    return new Text;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
