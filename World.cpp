/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.cpp

*/

#import "World.h"



//Creates the world
World::World(double x, double y){
  x_ = x;
  y_ = y;
}

//Should clean up all discs inside too
World::~World(){}


//Draws the world in OpenGl
void World::draw(){
  glPushMatrix();
    glRotatef(90,1,0,0);//face up
    glScalef(x_,.4,y_);//makes whole thing thinner and scales to actual size
    glTranslatef(0, -.5, 0);
    glColor3f(1,0,0);
    glPushMatrix();
      glutSolidCube(1);
      glPopMatrix();  
    glTranslatef(0, 1, 0);//walls height
    
    //First two walls
    glColor4f(1,1,0,1);
    glTranslatef(-.5 * (1 - kWallThickness), 0, 0);
    glPushMatrix();
      glScalef(kWallThickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    glColor4f(1,1,0,1);
    glPushMatrix();
      glTranslatef(1 - kWallThickness, 0, 0);
      glScalef(kWallThickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    //Last Two walls
    glRotatef(90,0,1,0);
    glTranslatef(-.5 , 0, .5);
    glPushMatrix();
      glTranslatef(1- .5 * kWallThickness, 0, -.5 * kWallThickness);
      glScalef(kWallThickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    glPushMatrix();
      glTranslatef(.5 * kWallThickness, 0, -.5 * kWallThickness);
      glScalef(kWallThickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
  glPopMatrix();  


}

// Gets the current position of the world. The world doesn't need to move.
void World::get_origin(double &x, double &y, double &z){
  x=0; y=0; z=0;
}

// Gets the current orientation of the world.
void World::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}
