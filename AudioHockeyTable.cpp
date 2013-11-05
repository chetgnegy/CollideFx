/*
 *  AudioHockeyTable.cpp
 *
 *  Created on: Nov 2, 2013
 *      Author: Chet Gnegy - chetgnegy@gmail.com
 *      Assignment: https://ccrma.stanford.edu/wiki/256a-fall-2013/hw3
 *
 *      
 *      Uses RtAudio Library, and OpenGL
 */

#include <stdio.h>
#include <math.h>
#include <iostream>

#include "UGenChain.h"
#include "World.h"


int main(int argc, char *argv[]) {
  srand (time(NULL));
  UGenChain *myChain = new UGenChain();
  myChain->initialize();
  
  Reverb *d = new Reverb();
  myChain->add_effect(d);
  
  
  World *myWorld = new World(20, 20);
  while (true){
    usleep(1000000);
    d->set_params(.99*rand()/(1.0*RAND_MAX)+.01, .99*rand()/(1.0*RAND_MAX));
  }
  char input;
  std::cin.get(input); //Pause after a single frame
  
  
  
  delete myWorld;
  delete myChain;
  return 1;

}