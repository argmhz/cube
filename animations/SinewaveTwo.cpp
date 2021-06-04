#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"


class SinewaveTwo : public Animation {

  void draw(Cube *c) {

    int sinewavearray[8], addr, sinemult[8], colselect, rr=0, gg=0, bb=15, addrt;
    int sinewavearrayOLD[8], select, subZ=-7, subT=7, multi=0;//random(-1, 2);
    sinewavearray[0]=0;
    sinemult[0]=1;
     sinewavearray[1]=1;
    sinemult[1]=1;
      sinewavearray[2]=2;
    sinemult[2]=1;
      sinewavearray[3]=3;
    sinemult[3]=1;
      sinewavearray[4]=4;
    sinemult[4]=1;
      sinewavearray[5]=5;
    sinemult[5]=1;
      sinewavearray[6]=6;
    sinemult[6]=1;
      sinewavearray[7]=7;
    sinemult[7]=1;

while(isRunning()){
  for(addr=0; addr<8; addr++){
    if(sinewavearray[addr]==7){
    sinemult[addr]=-1;
    }
    if(sinewavearray[addr]==0){
    sinemult[addr]=1;
    }
    sinewavearray[addr] = sinewavearray[addr] + sinemult[addr];
  }//addr
   if(sinewavearray[0]==7){
   select=random(3);
  if(select==0){
    rr=random(1, 16);
    gg=random(1, 16);
    bb=0;}
   if(select==1){
    rr=random(1, 16);
    gg=0;
    bb=random(1, 16);}
   if(select==2){
    rr=0;
    gg=random(1, 16);
    bb=random(1, 16);}
 /*
if(multi==1)
multi=0;
else
multi=1;
*/

}



    for(addr=0; addr<8; addr++){
  c->set(sinewavearrayOLD[addr], addr, 0, 0, 0, 0);
  c->set(sinewavearrayOLD[addr], 0, addr, 0, 0, 0);
  c->set(sinewavearrayOLD[addr], subT-addr, 7, 0, 0, 0);
  c->set(sinewavearrayOLD[addr], 7, subT-addr, 0, 0, 0);
 c->set(sinewavearray[addr], addr, 0, rr, gg, bb);
 c->set(sinewavearray[addr], 0, addr, rr, gg, bb);
 c->set(sinewavearray[addr], subT-addr,7, rr, gg, bb);
 c->set(sinewavearray[addr], 7, subT-addr, rr, gg, bb);
  }//}

     for(addr=1; addr<7; addr++){
  c->set(sinewavearrayOLD[addr+multi*1], addr, 1, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*1], 1, addr, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*1], subT-addr, 6, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*1], 6, subT-addr, 0, 0, 0);
 c->set(sinewavearray[addr+multi*1], addr, 1, rr, gg, bb);
 c->set(sinewavearray[addr+multi*1], 1, addr, rr, gg, bb);
 c->set(sinewavearray[addr+multi*1], subT-addr,6, rr, gg, bb);
 c->set(sinewavearray[addr+multi*1], 6, subT-addr, rr, gg, bb);
     }

      for(addr=2; addr<6; addr++){
  c->set(sinewavearrayOLD[addr+multi*2], addr, 2, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*2], 2, addr, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*2], subT-addr, 5, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*2], 5, subT-addr, 0, 0, 0);
 c->set(sinewavearray[addr+multi*2], addr, 2, rr, gg, bb);
 c->set(sinewavearray[addr+multi*2], 2, addr, rr, gg, bb);
 c->set(sinewavearray[addr+multi*2], subT-addr,5, rr, gg, bb);
 c->set(sinewavearray[addr+multi*2], 5, subT-addr, rr, gg, bb);
     }
           for(addr=3; addr<5; addr++){
  c->set(sinewavearrayOLD[addr+multi*3], addr, 3, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*3], 3, addr, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*3], subT-addr, 4, 0, 0, 0);
  c->set(sinewavearrayOLD[addr+multi*3], 4, subT-addr, 0, 0, 0);
 c->set(sinewavearray[addr+multi*3], addr, 3, rr, gg, bb);
 c->set(sinewavearray[addr+multi*3], 3, addr, rr, gg, bb);
 c->set(sinewavearray[addr+multi*3], subT-addr,4, rr, gg, bb);
 c->set(sinewavearray[addr+multi*3], 4, subT-addr, rr, gg, bb);
     }

   for(addr=0; addr<8; addr++){
     sinewavearrayOLD[addr]=sinewavearray[addr];
      c->update();
      usleep(8000);

   }



}//while
  c->clear();

  }
};

extern "C" Animation * create() {
    return new SinewaveTwo;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
