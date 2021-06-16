#include <iostream>
#include <thread>
#include <vector>
#include "../lib/Cube.cpp"
#include "../lib/helpers.h"
#include <math.h>
#define PI 3.14159265

Cube * cube = new Cube;
void init_LUT(unsigned char LUT[65])
{
	unsigned char i;
	float sin_of,sine;
	for (i=0;i<65;i++)
	{
		sin_of=i*PI/64; // Just need half a sin wave
		sine=sin(sin_of);
		// Use 181.0 as this squared is <32767, so we can multiply two sin or cos without overflowing an int.
		LUT[i]=sine*181.0;
	}
}
int totty_sin(unsigned char LUT[65],int sin_of)
{
	unsigned char inv=0;
	if (sin_of<0)
	{
		sin_of=-sin_of;
		inv=1;
	}
	sin_of&=0x7f; //127
	if (sin_of>64)
	{
		sin_of-=64;
		inv=1-inv;
	}
	if (inv)
		return -LUT[sin_of];
	else
		return LUT[sin_of];
}

void animation(Cube *cube){
  int iterations = 1000;
  // 16 values for square root of a^2+b^2.  index a*4+b = 10*sqrt
  // This gives the distance to 3.5,3.5 from the point
  unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
  //LUT_START // Macro from new tottymath.  Commented and replaced with full code
  unsigned char LUT[65];
  init_LUT(LUT);
  int i;
  unsigned char x,y,height,distance;
  while (cube->isRunning())
  {
    i+=4;
    cube->clear();

    for (x=0;x<4;x++)
      for(y=0;y<4;y++)
      {
        // x+y*4 gives no. from 0-15 for sqrt_LUT
        distance=sqrt_LUT[x+y*4];// distance is 0-50 roughly
        // height is sin of distance + iteration*4
        //height=4+totty_sin(LUT,distance+i)/52;
        height=(196+totty_sin(LUT,distance+i))/49;
        // Use 4-way mirroring to save on calculations
        cube->set(x,y,height, 0,0,15);
        cube->set(7-x,y,height, 0,0,15);
        cube->set(x,7-y,height, 0,0,15);
        cube->set(7-x,7-y,height, 0,0,15);

      }
      cube->update();
    usleep(10000);
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
