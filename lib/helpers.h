#include <stdio.h>
#include <ctime>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <math.h>

std::clock_t c_start = std::clock();


int random(int min, int max) {
  return rand() % max + min;
}

int random(int max) {
  return random(0,max);
}

unsigned long millis(){
  std::clock_t c_end = std::clock();
  return 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC;
}

double dmap(double in, double inMin, double inMax, double outMin, double outMax){
    double out;
    out = (in-inMin)/(inMax-inMin)*(outMax-outMin) + outMin;
    return out;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * Creates an voctor with rgb values between the to input colors
*/
std::vector<Cube::Color> fadeColor( int r1,int g1,int b1,int r2,int g2,int b2,int n_steps = 16) {

    std::vector<Cube::Color>  colors;
    Cube::Color color;

    int red_diff   = r2 - r1;
    int green_diff = g2 - g1;
    int blue_diff  = b2 - b1;

    for (int i = 0; i < n_steps; ++i){

        color.red = r1 + i * red_diff / n_steps;
        color.green = g1 + i * green_diff / n_steps;
        color.blue = b1 + i * blue_diff/ n_steps;
        colors.push_back(color);
    }

  return colors;
}

Cube::Color makeColorGradient(int index){

    float frequency1 = .1;
    float frequency2 = .1;
    float frequency3 = .1;

    int phase1 = 0;
    int phase2 = 2;
    int phase3 = 4;

    int center = 128;
    int width = 127;

    Cube::Color colors;
    colors.red = map(sin(frequency1*index + phase1) * width + center,0,255,0,15);
    colors.green = map(sin(frequency2*index + phase2) * width + center,0,255,0,15);
    colors.blue = map(sin(frequency3*index + phase3) * width + center,0,255,0,15);
    return colors;
}
