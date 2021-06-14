#include <vector>
#include "../resources/fonts.cpp"


class Font {

public:

  static std::vector<Cube::Point> asPoints(char chr){
      int pos[8] = {7,6,5,4,3,2,1,0};
      std::vector<Cube::Point> points;

      for(int y=0;y<8;y++){
        std::bitset<8> bit(fonts[chr][y]);
        for (int x = 0;x<8; x++) {
          if(bit[x]){
            Cube::Point point;
            point.x = x;
            point.y = pos[y];
            point.z = 0;
            points.push_back(point);
          }
        }
      }
      return points;
  }

  static int * asArray(char chr){
    int pos[8] = {7,6,5,4,3,2,1,0};
    static int items[64];

    for(int y=0;y<8;y++){
      std::bitset<8> bit(fonts[chr][y]);
      for (int x = 0;x<8; x++) {
          items[x*8+pos[y]] = bit[x];
      }
    }
    return items;
  }



};
