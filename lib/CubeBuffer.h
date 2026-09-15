#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <cstddef>

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

#define MAX_COLOR 15
#define MIN_COLOR 0

// Pure 8x8x8 voxel buffer + geometry helpers.
// No hardware I/O of any kind (no bcm2835, no threads) so it can be built
// and unit tested on any machine, not just on the Raspberry Pi.
class CubeBuffer {
  protected:

   int tr[192] = {
     189,190,191,186,187,188,175,184,
     185,172,173,174,169,170,171,158,
     159,168,155,156,157,152,153,154,
     178,177,176,181,180,179,160,183,
     182,163,162,161,166,165,164,145,
     144,167,148,147,146,151,150,149,
     109,110,111,106,107,108,127,104,
     105,124,125,126,121,122,123,142,
     143,120,139,140,141,136,137,138,
     98,97,96,101,100,99,112,103,102,
     115,114,113,118,117,116,129,128,
     119,132,131,130,135,134,133, 93,
      94, 95, 90, 91, 92, 79, 88, 89,
      76, 77, 78, 73, 74, 75, 62, 63,
      72, 59, 60, 61, 56, 57, 58, 82,
      81, 80, 85, 84, 83, 64, 87, 86,
      67, 66, 65, 70, 69, 68, 49, 48,
      71, 52, 51, 50, 55, 54, 53, 13,
      14, 15, 10, 11, 12, 31,  8,  9,
      28, 29, 30, 25, 26, 27, 46, 47,
      24, 43, 44, 45, 40, 41, 42,  2,
      1, 0, 5,  4,  3, 16,  7,  6, 19,
      18, 17, 22, 21, 20, 33, 32, 23,
      36, 35, 34, 39, 38, 37};

   std::bitset<4> frontBuffer[8][192] = {};

  public:
    struct Color{
      int red = 0;
      int green = 0;
      int blue = 0;

      void random(){
        red = rand()%16;
        green = rand()%16;
        blue = rand()%16;
      }

      void set(int r, int g, int b){
        red = r;
        green = g;
        blue = b;
      }
    };

    struct Point {
      int x,y,z;
    };

    CubeBuffer(){
      clear();
    }

    void clear() {
      for (size_t i = 0; i < 192; i++) {
        for (size_t k = 0; k < 8; k++) {
          frontBuffer[k][i] = 0;
        }
      }
    }

    bool isOff(int x,int y,int z){
      Color c = get(x,y,z);
      return (c.red == 0 && c.green == 0 && c.blue == 0);
    }

    bool inBounce(int x,int y,int z){
        return ((x < 8 && x >= 0) && (y < 8 && y >= 0) && (z < 8 && z >= 0 ));
    }

    void set(int x, int y, int z, int r, int g, int b ){
      if(inBounce(x,y,z)){
        int index = (z*8+x)*3;
      	frontBuffer[y][tr[index]] = r;
        frontBuffer[y][tr[index+1]] = g;
  	    frontBuffer[y][tr[index+2]] = b;
      }
    }

    void setIndex(int layer, int index, int r, int g, int b){
      if(layer < 8 && layer >= 0 && index < 64 && index >= 0){
        int i = index*3;
        frontBuffer[layer][tr[i]] = r;
        frontBuffer[layer][tr[i+1]] = g;
        frontBuffer[layer][tr[i+2]] = b;
      }
    }

    void set(int x, int y, int z, Color color){
      set(x,y,x, color.red, color.green, color.blue);
    }

    Color get(uint8_t x, uint8_t y, uint8_t z){
      Color c;
      int index = (z*8+x)*3;

      if(inBounce(x,y,z)){
        c.red = (int)frontBuffer[y][tr[index]].to_ulong();
        c.green = (int)frontBuffer[y][tr[index+1]].to_ulong();
        c.blue = (int)frontBuffer[y][tr[index+2]].to_ulong();
      }

      return c;
    }

    void setDirect(int layer, int pos){
      frontBuffer[layer][pos] = 15;
    }

    void setIndex(int layer, int pos, int value){
      frontBuffer[layer][tr[pos]] = value;
    }

    void swapint(int & one, int & two) {
	  	one = one^two;
	  	two = one^two;
	  	one = one^two;
  	}

    void clear(int x,int y,int z){
		    set(x,y,z,0,0,0);
	  }

    int roundClostest(int numerator, int denominator) {
  	  	numerator = (numerator << 1)/denominator;
  	  	int output = (numerator>>1) + (numerator % 2);
  	  	return output;
  	}

    void plane(int axis,int index,int r,int g,int b);
    void plane(int axis,int index,Color color);
  	void line(int x1,int y1,int z1,int x2,int y2,int z2,int r,int g,int b);
  	void shift(int axis,int direction);
    void shiftPlane(int axis,int index,int direction);
    void clearPlane(int axis,int index);
    void sphere(int x,int y,int z,int radius,int r, int g,int b);
    void sphere(int x,int y,int z,int radius,Color color);
    void all(int r, int g,int b);
    void all(Color color);
    void box(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g, int b);
    void boxOutline(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g,int b);
    void hollowBox(int startx, int starty, int startz, int endx, int endy, int endz, int r, int g,int b);
    void rotateZ(int degrees);
    void rotate(int axis, int degree);
};
