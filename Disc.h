/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.h
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#ifndef _DISC_H_
#define _DISC_H_

#include "UnitGenerator.h"
#include "Drawable.h" //imports opengl stuff, too
#include "Moveable.h"
#include "Particle.h"

class Disc : public Drawable, public Moveable {
public:
  static const int kNumParticles = 5;

  // Pairs the disc with a unit generator
  Disc(UnitGenerator *u, double radius); 
  // Cleans up the unit generator
  ~Disc();
  
  // Places disc at certain location
  void set_location(double x, double y);
  // Sets instantaneous velocity of the disc
  void set_velocity(double x, double y);
  
  // OpenGL instructions for drawing a unit disc centered at the origin
  void draw(void);
  
  // The actual location of the disc's center
  void get_origin(double &x, double &y, double &z);
  
  // The orientation of the disk
  void get_rotation(double &x, double &y, double &z);

  // Sets up the visual attributes for the Disc
  void set_attributes();
  
  // Pops the attributes matrix off the stack
  void remove_attributes();
  
  // initializes the textures
  void prepare_graphics(void);

  // Moves the object to new coordinates
  void move(double x, double y, double z);

  void prepare_move(double x, double y, double z);
  
  bool check_clicked(double x, double y, double z);
  

private:
  // Draws the glowing, moving orbs
  void draw_particles();

  // Advances the position of the particles, or possibly triggers new ones
  void advance_particles();

  UnitGenerator *ugen_;
  //positions
  double x_, y_, x_offset_, y_offset_;
  //velocity
  double vx_, vy_;
  //forces
  double fx_, fy_;
  //radius
  double r_;
  //angular velocity
  double w_;
  //An object that is useful for drawing the cylinder
  GLUquadricObj *quadratic; 

  Particle *particles_;
  GLuint texture_[1]; 
};


#endif