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
  //Draws a test sphere
  double wall_thickness = .02;
  glPushMatrix();
    glRotatef(90,1,0,0);//face up
    glScalef(x_,.1,y_);//makes whole thing thinner and scales to actual size
    glTranslatef(0, -.5, 0);
    glColor3f(1,0,0);
    glPushMatrix();
      glutSolidCube(1);
      glPopMatrix();  
    glTranslatef(0, 1, 0);//walls height
    
    //First two walls
    glColor4f(1,1,0,1);
    glTranslatef(-.5 * (1 - wall_thickness), 0, 0);
    glPushMatrix();
      glScalef(wall_thickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    glColor4f(1,1,0,1);
    glPushMatrix();
      glTranslatef(1 - wall_thickness, 0, 0);
      glScalef(wall_thickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    //Last Two walls
    glRotatef(90,0,1,0);
    glTranslatef(-.5 , 0, .5);
    glPushMatrix();
      glTranslatef(1- .5 * wall_thickness, 0, -.5 * wall_thickness);
      glScalef(wall_thickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
    glPushMatrix();
      glTranslatef(.5 * wall_thickness, 0, -.5 * wall_thickness);
      glScalef(wall_thickness,1,1);
      glutSolidCube(1);
      glPopMatrix();
  glPopMatrix();  


}
void World::get_origin(double &x, double &y, double &z){
  x=0; y=0; z=0;
}
void World::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}
