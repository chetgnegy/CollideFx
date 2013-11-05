/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.cpp

*/

#import "World.h"
#import <stdio.h>//remove

//Creates the world
World::World(double x, double y){
  x_ = x;
  y_ = y;
}

//Should clean up all discs inside too
World::~World(){}


//Draws the world in OpenGl
void World::draw(){
  printf("Trying to draw the world;");
}
void World::get_origin(double &x, double &y, double &z){
  x=0; y=0; z=0;
}
void World::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}