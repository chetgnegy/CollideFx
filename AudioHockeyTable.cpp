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
  myChain->initialize_audio();
  myChain->initialize_midi();
  
  
  Menu *myMenu = new Menu();
  World *myWorld = new World(30, 30, 9, 0);
  
  Graphics::add_drawable(myMenu, 1);
  Graphics::add_moveable(myMenu);
  myMenu->link_ugen_chain(myChain);
  
  Graphics::add_drawable(myWorld, 2);
  Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);
  

  

  
  // Disc *myDisc = new Disc(d, 4, false);
  // Graphics::add_drawable(myDisc);



  myGraphics->start_graphics();
  
  
  return 1;

}