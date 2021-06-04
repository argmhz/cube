#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"

class BouncyvTwo : public Animation {

  void draw(Cube *c){

    int wipex, wipey, wipez, ranr, rang, ranb, select, oldx[50], oldy[50], oldz[50];
     int x[50], y[50], z[50], addr, ledcount=20, direct, direcTwo;
     int xx[50], yy[50], zz[50];
     int xbit=1, ybit=1, zbit=1;
      for(addr=0; addr<ledcount+1; addr++){
        oldx[addr]=0;
        oldy[addr]=0;
        oldz[addr]=0;
        x[addr]=0;
        y[addr]=0;
        z[addr]=0;
        xx[addr]=0;
        yy[addr]=0;
        zz[addr]=0;

      }

     while(isRunning()){
       direct = random(3);

    for(addr=1; addr<ledcount+1; addr++){
    c->set(oldx[addr], oldy[addr],oldz[addr], 0,0,0);
    c->set(x[addr], y[addr], z[addr], xx[addr],yy[addr],zz[addr]);
    }

    for(addr=1; addr<ledcount+1; addr++){
    oldx[addr]=x[addr];
    oldy[addr]=y[addr];
    oldz[addr]=z[addr];
    }
    c->update();
    usleep(10000);


    //direcTwo=random(3);
    //if(direcTwo==1)



    if(direct==0)
    x[0]= x[0]+xbit;
    if(direct==1)
    y[0]= y[0]+ybit;
    if(direct==2)
    z[0]= z[0]+zbit;

    if(direct==3)
    x[0]= x[0]-xbit;
    if(direct==4)
    y[0]= y[0]-ybit;
    if(direct==5)
    z[0]= z[0]-zbit;





    if(x[0]>7){
    xbit=-1;
    x[0]=7;
    xx[0]=random(16);
    yy[0]=random(16);
    zz[0]=0;
    //wipe_out();
    }
    if(x[0]<0){
    xbit=1;
     x[0]=0;
    xx[0]=random(16);
    yy[0]=0;
    zz[0]=random(16);
    //wipe_out();
    }
    if(y[0]>7){
    ybit=-1;
    y[0]=7;
    xx[0]=0;
    yy[0]=random(16);
    zz[0]=random(16);
    //wipe_out();
    }
    if(y[0]<0){
    ybit=1;
     y[0]=0;
     xx[0]=0;
    yy[0]=random(16);
    zz[0]=random(16);
    //wipe_out();
    }
    if(z[0]>7){
    zbit=-1;
    z[0]=7;
    xx[0]=random(16);
    yy[0]=0;
    zz[0]=random(16);
    //wipe_out();
    }
    if(z[0]<0){
    zbit=1;
     z[0]=0;
     xx[0]=random(16);
    yy[0]=random(16);
    zz[0]=0;
    //wipe_out();
    }

    for(addr=ledcount; addr>0; addr--){
     x[addr]=x[addr-1];
     y[addr]=y[addr-1];
     z[addr]=z[addr-1];
     xx[addr]=xx[addr-1];
     yy[addr]=yy[addr-1];
     zz[addr]=zz[addr-1];
    }


     }//while

     c->clear();
     // c->update();
  }

};

extern "C" Animation * create() {
    return new BouncyvTwo;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
