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
#include "Disc.h"
#include "World.h"
#include "GraphicsBox.h"
#include "Physics.h"

int main(int argc, char *argv[]) {

  srand (time(NULL));
  UGenChain *myChain = new UGenChain();
  //myChain->initialize();
  
  Chorus *d = new Chorus(44100);
  //myChain->add_effect(d);
  //while (true){
  //  usleep(1000000);
  //  d->set_params(.99*rand()/(1.0*RAND_MAX)+.01, .99*rand()/(1.0*RAND_MAX));
  //}
  
  
  World *myWorld = new World(30, 30, 9, 0);
  Disc *myDisc = new Disc(d, 1);
  Disc *myDisc2 = new Disc(d, 1);
  Disc *myDisc3 = new Disc(d, 2);
  
  GraphicsBox *myGraphics = new GraphicsBox(960, 600);
  
  myGraphics->initialize(argc, argv);
  myGraphics->add_drawable(myWorld);

  myGraphics->add_drawable(myDisc);
  myGraphics->add_drawable(myDisc2);
  myGraphics->add_drawable(myDisc3);
  myGraphics->add_moveable(myDisc);
  myGraphics->add_moveable(myDisc2);
  myDisc->set_velocity(3,3);
  myDisc2->set_location(-3,3);
  myDisc2->set_velocity(7,0);
  myDisc3->set_location(3,12);
  myDisc3->set_velocity(-2,-7);
  Physics::give_physics(myDisc);
  Physics::give_physics(myDisc2);
  Physics::give_physics(myDisc3);
  
  myGraphics->start_graphics();
  
  
  delete myWorld;
  delete myDisc;
  delete myChain;
  return 1;

}