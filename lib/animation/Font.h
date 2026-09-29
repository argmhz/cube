#pragma once
#include <array>
#include <bitset>
#include <string>
#include <vector>
#include "../../resources/fonts.h"
// #include "../../resources/box.h"

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

  // The font table has 128 entries, but text from the web UI arrives as
  // UTF-8, where a Danish "å" is two bytes that are both >= 128. Indexing
  // fonts[] with either reads past the end of the table, and the character
  // comes out as two pieces of garbage. Transliterate the Danish letters
  // and drop anything else the font has no glyph for.
  static std::string renderable(const std::string &text) {
    std::string out;

    for (size_t i = 0; i < text.size(); i++) {
      const unsigned char byte = text[i];

      if (byte < 128) {
        out += static_cast<char>(byte);
        continue;
      }

      const unsigned char next = (i + 1 < text.size()) ? text[i + 1] : 0;
      if (byte == 0xC3) {
        switch (next) {
          case 0xA6: out += "ae"; i++; continue; // æ
          case 0x86: out += "AE"; i++; continue; // Æ
          case 0xB8: out += "oe"; i++; continue; // ø
          case 0x98: out += "OE"; i++; continue; // Ø
          case 0xA5: out += "aa"; i++; continue; // å
          case 0x85: out += "AA"; i++; continue; // Å
        }
      }

      // Unknown multi-byte character: skip the whole sequence rather than
      // let its continuation bytes fall through as more garbage.
      if (byte >= 0xF0) {
        i += 3;
      } else if (byte >= 0xE0) {
        i += 2;
      } else if (byte >= 0xC0) {
        i += 1;
      }
    }

    return out;
  }

};
