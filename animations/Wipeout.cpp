#include "../lib/Cube.cpp"
#include "../lib/Animation.hpp"
#include "../lib/helpers.h"


class Wipeout : public Animation {


  int speed = 12000;


  int xxx=0, yyy=0, zzz=0;
  int fx=random(8), fy=random(8), fz=random(8), direct, fxm=1, fym=1, fzm=1, fxo=0, fyo=0, fzo=0;
  int  ftx=random(8), fty=random(8), ftz=random(8), ftxm=1, ftym=1, ftzm=1, ftxo=0, ftyo=0, ftzo=0;
  int select, rr, gg, bb, rrt, ggt, bbt;


  void onDataUpdate(json data){

    if(data["speed"].is_number()){
      speed = data["speed"].get<int>();
    }

    if(data["random_color"].is_number()){
      changeColor();
    }

  }

  void changeColor(){

    select=random(3);
 if(select==0){
   rr=random(15);
   gg=random(15);
   bb=0;}
  if(select==1){
   rr=random(15);
   gg=0;
   bb=random(15);}
  if(select==2){
   rr=0;
   gg=random(15);
   bb=random(15);}

    select=random(3);
 if(select==0){
   rrt=random(15);
   ggt=random(15);
   bbt=0;}
  if(select==1){
   rrt=random(15);
   ggt=0;
   bbt=random(15);}
  if(select==2){
   rrt=0;
   ggt=random(15);
   bbt=random(15);}
  }

  void draw(Cube *c) {

  c->clear();
  c->update();

  changeColor();


  while(isRunning()){ 

    c->set(fxo, fyo, fzo, 0, 0, 0);
    c->set(fxo, fyo, fzo+1, 0, 0, 0);
    c->set(fxo, fyo, fzo-1, 0, 0, 0);
    c->set(fxo+1, fyo, fzo, 0, 0, 0);
    c->set(fxo-1, fyo, fzo, 0, 0, 0);
    c->set(fxo, fyo+1, fzo, 0, 0, 0);
    c->set(fxo, fyo-1, fzo, 0, 0, 0);

    c->set(ftxo, ftyo, ftzo, 0, 0, 0);
    c->set(ftxo, ftyo, ftzo+1, 0, 0, 0);
    c->set(ftxo, ftyo, ftzo-1, 0, 0, 0);
    c->set(ftxo+1, ftyo, ftzo, 0, 0, 0);
    c->set(ftxo-1, ftyo, ftzo, 0, 0, 0);
    c->set(ftxo, ftyo+1, ftzo, 0, 0, 0);
    c->set(ftxo, ftyo-1, ftzo, 0, 0, 0);

    c->set(ftx, fty, ftz, rr, gg, bb);
    c->set(ftx, fty, ftz+1, rr, gg, bb);
    c->set(ftx, fty, ftz-1,  rr, gg, bb);
    c->set(ftx+1, fty, ftz, rr, gg, bb);
    c->set(ftx-1, fty, ftz, rr, gg, bb);
    c->set(ftx, fty+1, ftz, rr, gg, bb);
    c->set(ftx, fty-1, ftz, rr, gg, bb);

    c->set(fx, fy, fz, rrt, ggt, bbt);
    c->set(fx, fy, fz+1, rrt, ggt, bbt);
    c->set(fx, fy, fz-1, rrt, ggt, bbt);
    c->set(fx+1, fy, fz, rrt, ggt, bbt);
    c->set(fx-1, fy, fz, rrt, ggt, bbt);
    c->set(fx, fy+1, fz, rrt, ggt, bbt);
    c->set(fx, fy-1, fz, rrt, ggt, bbt);

    c->update();
     usleep(speed);

    fxo=fx;
    fyo=fy;
    fzo=fz;

    ftxo=ftx;
    ftyo=fty;
    ftzo=ftz;

    direct=random(3);
    if(direct==0)
    fx= fx+fxm;
    if(direct==1)
    fy= fy+fym;
    if(direct==2)
    fz= fz+fzm;
  if(fx<0){
    fx=0; fxm=1;}
  if(fx>7){
    fx=7; fxm=-1;}
  if(fy<0){
    fy=0; fym=1;}
  if(fy>7){
    fy=7; fym=-1;}
  if(fz<0){
    fz=0; fzm=1;}
  if(fz>7){
    fz=7; fzm=-1;}

      direct=random(3);
    if(direct==0)
    ftx= ftx+ftxm;
    if(direct==1)
    fty= fty+ftym;
    if(direct==2)
    ftz= ftz+ftzm;
  if(ftx<0){
    ftx=0; ftxm=1;}
  if(ftx>7){
    ftx=7; ftxm=-1;}
  if(fty<0){
    fty=0; ftym=1;}
  if(fty>7){
    fty=7; ftym=-1;}
  if(ftz<0){
    ftz=0; ftzm=1;}
  if(ftz>7){
    ftz=7; ftzm=-1;}
  }//while


  c->clear();

  }

};

extern "C" Animation * create() {
    return new Wipeout;
}

extern "C" void destroy(Animation * p) {
    delete p;
}
