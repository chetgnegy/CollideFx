/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.cpp
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#import "Disc.h"

// Pairs the disc with a unit generator
Disc::Disc(UnitGenerator *u, double radius){
  ugen_ = u;
  r_ = radius;
  x_ = 0; 
  y_ = 0;
  x_offset_ = 0; 
  y_offset_ = 0;
}

// Cleans up the unit generator
Disc::~Disc(){
  delete ugen_;
}

// Places disc at certain location  
void Disc::set_location(double x, double y){
  x_ = x;
  y_ = y;
};

// Sets instantaneous velocity of the disc
void Disc::set_velocity(double y, double z){};

// OpenGL instructions for drawing a unit disc centered at the origin
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
    //delete quadratic;
    glPopMatrix();

}

// The current location of the disc's center
void Disc::get_origin(double &x, double &y, double &z){
  x=x_; y=y_; z=0;
}

// The current orientation of the disk
void Disc::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}

void Disc::move(double x, double y, double z){
  x_ = x - x_offset_;
  y_ = y - y_offset_;

}

void Disc::prepare_move(double x, double y, double z){
  x_offset_ = x - x_;
  y_offset_ = y - y_;
}

bool Disc::check_clicked(double x, double y, double z){
  if (pow(x-x_,2) + pow(y-y_,2) < pow(r_,2)) return true;
  return false;
}