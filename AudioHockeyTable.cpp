/*
 *  AudioHockeyTable.cpp
 *
 *  Created on: Nov 2, 2013
 *      Author: Chet Gnegy - chetgnegy@gmail.com
 *      
 *      
 *      Uses RtAudio Library and OpenGL
 */

#include <stdio.h>
#include <math.h>
#include <iostream>

#include "UGenChain.h"
#include "Disc.h"
#include "World.h"
#include "Graphics.h"
#include "Physics.h"
#include "Menu.h"

int main(int argc, char *argv[]) {
  srand (time(NULL));

  Graphics *myGraphics = new Graphics(960, 600);
  myGraphics->initialize(argc, argv);
  

  UGenChain *myChain = new UGenChain();
  //myChain->initialize();
  
  Chorus *d = new Chorus(44100);
  //myChain->add_effect(d);
  //while (true){
  //  usleep(1000000);
  //  d->set_params(.99*rand()/(1.0*RAND_MAX)+.01, .99*rand()/(1.0*RAND_MAX));
  //}
  
  Menu *myMenu = new Menu();
  World *myWorld = new World(30, 30, 9, 0);
  Disc *myDisc = new Disc(d, 1);
  Disc *myDisc2 = new Disc(d, 2);
  Disc *myDisc3 = new Disc(d, .5);
  
  
  Graphics::add_drawable(myMenu);
  Graphics::add_moveable(myMenu);
  

  Graphics::add_drawable(myWorld);
  Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);
  Graphics::add_drawable(myDisc);
  Graphics::add_drawable(myDisc2);
  Graphics::add_drawable(myDisc3);
  
  myDisc2->set_location(8,-6);
  myDisc->set_location(12,12);
  myDisc3->set_location(10,4);
  

  Graphics::add_moveable(myDisc);
  Graphics::add_moveable(myDisc2);
  Graphics::add_moveable(myDisc3);
  Physics::give_physics(myDisc);
  Physics::give_physics(myDisc2);
  Physics::give_physics(myDisc3);
  
  for (int i = 0; i < 200; i++){
    myDisc->orb_create();
    myDisc->orb_abandon();
    myDisc2->orb_create();
     myDisc2->orb_abandon();
    myDisc3->orb_create();
     myDisc3->orb_abandon();
  }
  
  myGraphics->start_graphics();
  
  
  delete myWorld;
  delete myDisc;
  delete myChain;
  return 1;

}