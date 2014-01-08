/*
 *  CollideFx.cpp
 *
 *  Created on: Nov 2, 2013
 *      Author: Chet Gnegy - chetgnegy@gmail.com
 *      
 *      make 
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
/*
Known Bugs/Issues-- 
  Can't change buffer size without crash

Things to add: 
  Better distortion algorithm! // This is started

  Rectangle to select multiple
  
  auto-select
  
*/

int main(int argc, char *argv[]) {
  srand (time(NULL));

  Graphics *myGraphics = new Graphics(1100, 600);
  myGraphics->initialize(argc, argv);

  myGraphics->show_splash_screen();

  UGenChain *myChain = new UGenChain();
  myChain->initialize_audio();
  myChain->initialize_midi();

  Menu *myMenu = new Menu();
  World *myWorld = new World(30, 30, 9, 0);


  Graphics::add_drawable(myMenu, 1);
  Graphics::add_moveable(myMenu);
  myMenu->link_ugen_graph(myChain->get_signal_graph());
  if (UGenChain::has_midi()){ 
    myMenu->enable_midi(); 
  }

  Graphics::add_drawable(myWorld, 2);
  Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);
  myGraphics->start_graphics();

  return 1;

}

