#include "../lib/core/Cube.h"
#include "../lib/animation/Animation.h"
#include "../lib/helpers.h"


class Fireworks : public Animation {

  void draw(Cube *c) {

    c->clear();
    int n = 7;
    int _delay = 10000;
    int i,f,e;

    float origin_x = 3;
    float origin_y = 3;
    float origin_z = 3;

    int rand_y, rand_x, rand_z;

    float slowrate, gravity;

    // Particles and their position, x,y,z and their movement, dx, dy, dz
    float particles[n][6];

    while(isRunning()){

        origin_x = rand()%4;
        origin_y = rand()%4;
        origin_z = rand()%2;
        origin_z +=5;
        origin_x +=2;
        origin_y +=2;

        // shoot a particle up in the air
        for (e=0;e<origin_z;e++)
        {
            c->set(origin_x,origin_y,e,0,0,15);
            c->update();
            usleep(600+500*e);
            c->clear();
        }

        // Fill particle array
        for (f=0; f<n; f++)
        {
            // Position
            particles[f][0] = origin_x;
            particles[f][1] = origin_y;
            particles[f][2] = origin_z;

            rand_x = rand()%200;
            rand_y = rand()%200;
            rand_z = rand()%200;

            // Movement
            particles[f][3] = 1-(float)rand_x/100; // dx
            particles[f][4] = 1-(float)rand_y/100; // dy
            particles[f][5] = 1-(float)rand_z/100; // dz
        }

        // explode
        for (e=0; e<25; e++)
        {
            slowrate = 1+tan((e+0.1)/20)*10;

            gravity = tan((e+0.1)/20)/2;

            for (f=0; f<n; f++)
            {
                particles[f][0] += particles[f][3]/slowrate;
                particles[f][1] += particles[f][4]/slowrate;
                particles[f][2] += particles[f][5]/slowrate;
                particles[f][2] -= gravity;

                c->set(particles[f][0],particles[f][1],particles[f][2],15,15,15);
                c->update();
            }

            usleep(_delay);
            c->clear();
        }

    }
  }


};

extern "C" Animation * create() {
    return new Fireworks;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
