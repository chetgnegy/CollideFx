/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Orb.h
  Orb is a class for small drawable objects that glow!
*/

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "Drawable.h"
#include "Physical.h"
#include "vmath.h"



class Orb : public Drawable, public Physical {
public:

  // A constructor that links the orb to a disc
  Orb(Vector3d *v = 0);
  ~Orb();

  // Changes the disc that the orb is assigned to
  void reassign(Vector3d *v);

  // Draws the orb at location (0,0,0)
  void draw();

  // The location of the particle center
  void get_origin(double &x, double &y, double &z);

  // The current orientation of the particle
  void get_rotation(double &w, double &x, double &y, double &z);
  
  // Sets up the visual attributes for the Particle
  void set_attributes(void);

  // Pops the attributes off of the stack in OpenGL
  void remove_attributes(void);  

  // Loads the textures into the object
  void prepare_graphics(void);

  // Advances the internal clock of the particles. This
  // currently only adjusts the orientation of the particles
  void advance_time(double t);

  //Particles don't interact with anything but their anchor point
  bool has_collisions(){return false;}
  bool uses_friction(){return false;}
  double intersection_distance(){return 0;}

  // The forces are handled here and called from Physics.cpp during nunerical integration
  Vector3d external_forces();

  // Randomly turns the orbs for visual effect
  Vector3d external_torques(){return Vector3d(0, 0, rand()/(1.0*RAND_MAX)-.5);}
  
private:
  // The anchor point (the position of the parent disc)
  Vector3d *anchor_point_;
  // Random motion even when discs are at rest
  Vector3d wander_;
  // The internal timer
  double time_;

  // The color of the orb
  float r_,g_,b_;
  // equilibrium distance from the discs
  double hover_dist_;

  // The display size for the orbs
  double particle_size_;

  // The mask and image for the particles
  GLuint texture_[2]; 
};
#endif