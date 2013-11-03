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
  
  UGenChain *myChain = new UGenChain();
  myChain->initialize();
  
  Chorus *c = new Chorus(44100);
  c->set_params(.8,.9);
  myChain->add_effect(c);
  
  World *myWorld = new World(20, 20);

  char input;
  std::cin.get(input); //Pause after a single frame
  
  
  
  delete myWorld;
  delete myChain;
  return 1;

}