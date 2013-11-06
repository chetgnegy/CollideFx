/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.cpp
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#import "Disc.h"

//Pairs the disc with a unit generator
Disc::Disc(UnitGenerator *u, double radius){
  ugen_ = u;
  r_ = radius;
  x_ = 0; 
  y_ = 0;
}

//Cleans up the unit generator
Disc::~Disc(){
  delete ugen_;
}


void Disc::set_location(double x, double y){

};

void Disc::set_velocity(double y, double z){};

void Disc::draw(){
  glPushMatrix();
    glScalef(r_, r_, 1);
    quadratic=gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
    gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
    gluQuadricTexture(quadratic, GL_TRUE);
    gluCylinder(quadratic,1.0f,1.0f,1.0f,32,32);
    //Draw the faces
    gluDisk(quadratic,0.0f,1.0f,32,32);
    glTranslatef(0,0,1);
    gluDisk(quadratic,0.0f,1.0f,32,32);
    glPopMatrix();

}
void Disc::get_origin(double &x, double &y, double &z){
  x=x_; y=y_; z=0;
}
void Disc::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}