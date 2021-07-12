#pragma once
#include <vector>
#include "../resources/fonts.cpp"
// #include "../resources/box.cpp"

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

  static std::array<std::array<int, 8>,8> asArray(char chr){

    std::array<std::array<int, 8>,8> items;
    int pos[8] = {7,6,5,4,3,2,1,0};

    for(int y=0;y<8;y++){
      std::bitset<8> bit(fonts[chr][y]);
      for (int x = 0;x<8; x++) {
        items[pos[y]][x] = bit[x];
      }
    }

    return items;
  }

};
