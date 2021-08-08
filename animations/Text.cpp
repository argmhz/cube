#include <cstring>
#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"
#include "../lib/Font.cpp"
#include "../lib/json.hpp"
// #include "../lib/remotehelpers.cpp"

class Text : public Animation {

  std::string str = "";
  std::string tmp = "Topper 3D ";
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


  }

  std::string getPropertiesString(){
    return R"({ "text" : { "type" : "string" } })";
  }

  void draw(Cube *cube) {

      int r = rand()%16;
      int g = rand()%16;
      int b = rand()%16;

      while(isRunning()){

        str = tmp;

        for(signed c=0;c<str.length();c++){

          std::array<std::array<int,8>,8> items = Font::asArray(str[c]);

          for (signed x = 0; x < 8; x++) {
            for (signed y = 0; y < 8; y++) {
               if(items[y][x]){
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

        cube->clearPlane(AXIS_X,5);
        cube->update();
        sleep(1);
      }

  }

};
extern "C" Animation * create() {
    return new Text;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
