#pragma once
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "Animation.hpp"


class AniManager {
private:
  class Animation *animation = nullptr;
  class Cube *cube;
public:

  AniManager(Cube *cube){
    this->cube = cube;
  }

  ~AniManager(){
    delete animation;
  }

  bool isReady(){
    if(animation){
      return true;
    }
    return false;
  }

  void loadAnimation(Animation *a){
    animation = a;
  }

  void loadAnimation(char const* name) {

    void* TestAnimation = dlopen(name, RTLD_NOW);

    if (!TestAnimation) {
          std::cerr << "Cannot open library: " << dlerror() << '\n';
          // throw "Cannot open library";
          return;
    }

    dlerror();

    create_t* create_animation = (create_t*) dlsym(TestAnimation, "create");
    const char* dlsym_error = dlerror();

    if (dlsym_error) {
         std::cerr << "Cannot load symbol create: " << dlsym_error << '\n';
         // throw "Cannot load symbol create";
         return;
     }

    destroy_t* destroy_animation = (destroy_t*) dlsym(TestAnimation, "destroy");

    dlsym_error = dlerror();

    if (dlsym_error) {
       std::cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
       // throw "Cannot load symbol destroy";
       return;
    }

    if(animation){
      destroy_animation(animation);
    }

    animation = (Animation*)create_animation();
  }

  void stopAnimation(){
    if(animation){
      animation->stop();
    }
  }

  Animation &getAnimation(){
    return *animation;
  }

  std::vector<std::string> getAnimationsFiles(std::string path){

    std::vector<std::string> files;

    for (const auto & entry : std::filesystem::directory_iterator(path)){
      files.push_back(entry.path());
    }

    return files;
  }

};
