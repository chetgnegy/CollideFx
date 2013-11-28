/*
 *  AudioHockeyTable.cpp
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
  myMenu->link_ugen_graph(myChain->get_signal_graph());
  if (UGenChain::has_midi()){ myMenu->enable_midi(); }
    


  Graphics::add_drawable(myWorld, 2);
  Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);

/*
  Chorus *c = new Chorus();
  Distortion *d = new Distortion();
  Disc *chor = new Disc(c, 2, false);
  Disc *dist = new Disc(d, 4, false);
  chor->set_texture(6);    
  dist->set_texture(8);
          
  Graphics::add_drawable(chor);
  Graphics::add_moveable(chor);
  Physics::give_physics(chor);
  Graphics::add_drawable(dist);
  Graphics::add_moveable(dist);
  Physics::give_physics(dist);

  chor->set_location(0,9);
  chor->set_velocity(0,-3);
  chor->ang_vel_.z = 8;
  dist->ang_vel_.z = -2;
  dist->set_location(0,0);
  dist->set_color(0.0, 0.7, 0.3);
  chor->set_color(0.2, 0.7, 0.3);
  dist->set_velocity(0,3);
        */  
  myGraphics->start_graphics();
  
  return 1;

}

