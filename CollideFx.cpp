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

#define CLASSDEMO 0


int main(int argc, char *argv[]) {
  srand (time(NULL));

  if (!CLASSDEMO){
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
    if (UGenChain::has_midi()){ myMenu->enable_midi(); }

    Graphics::add_drawable(myWorld, 2);
    Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);
    myGraphics->start_graphics();
  
  }
  else {
    Graphics *myGraphics = new Graphics(960, 600);
    myGraphics->initialize(argc, argv);
    

    UGenChain *myChain = new UGenChain();
    myChain->initialize_audio();
    myChain->initialize_midi();
    
    Menu *myMenu = new Menu();
    World *myWorld = new World(30, 30, 9, 0);


    Graphics::add_moveable(myMenu);
    myMenu->link_ugen_graph(myChain->get_signal_graph());
    if (UGenChain::has_midi()){ myMenu->enable_midi(); }


    Input *i = new Input();
    Reverb *c = new Reverb();
    Delay *d = new Delay();
    
    Disc *inp = new Disc(i, 1.5, false);
    Disc *chor = new Disc(c, 1.5, false);
    Disc *dist = new Disc(d, 1.5, false);
    
    chor->set_texture(6);    
    dist->set_texture(7);
            
    Graphics::add_drawable(inp);
    Graphics::add_moveable(inp);
    Physics::give_physics(inp);
    Graphics::add_drawable(chor);
    Graphics::add_moveable(chor);
    Physics::give_physics(chor);
    Graphics::add_drawable(dist);
    Graphics::add_moveable(dist);
    Physics::give_physics(dist);

    myChain->get_signal_graph()->add_input(inp);
    myChain->get_signal_graph()->add_effect(chor);
    myChain->get_signal_graph()->add_effect(dist);

    inp->set_location(0,3);
    chor->set_location(2,-3);
    dist->set_location(0,0);
    inp->set_color(0.7, 0.7, 0.3);
    chor->set_color(0.2, 0.7, 0.3);
    dist->set_color(0.0, 0.7, 0.3);
    
   
    
    Graphics::add_drawable(myWorld, 2);
    Physics::set_bounds(30*(1-2*World::kWallThickness), 30*(1-2*World::kWallThickness), 9, 0);
    myGraphics->start_graphics();
  
  }
  
  return 1;

}

