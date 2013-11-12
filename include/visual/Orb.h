/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Orb.h
  Orb is a class for small drawable objects that glow!
*/

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "Physics.h"
#include "Graphics.h"
#include "Drawable.h"
#include "Physical.h"
#include "vmath.h"


class Orb : public Drawable, public Physical {
public:
  static const double kDeathTime = 15.0;

  // A constructor that links the orb to a disc. If we don't link it
  // it will fly off and die.
  Orb(Vector3d *v = NULL, double hover = 2.0);
  ~Orb();

  void change_hover_distance(double dist);

  // Changes the disc that the orb is assigned to
  void reassign(Vector3d *v);

  // Removes anchor point and schedules destruction.
  // Particles fly in all directions!
  void unassign();

  //Makes a call to delete self. Be careful with this!
  void self_destruct();

  /* ----- Drawable ----- */

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

  // If a particle is unassigned, this could result in a call to 
  // self_destruct. Be careful with self_destruct.
  void clean_up();

  // Advances the internal clock of the particles. This
  // adjusts the orientation of the particles. 
  void advance_time(double t);

  /* ----- Physics ----- */

  //Particles don't interact with anything but their anchor point
  bool has_collisions(){return false;}
  bool uses_friction(){return false;}
  bool rotates(){return false;}
  double intersection_distance(){return 0;}

  // The forces are handled here and called from Physics.cpp during nunerical integration
  Vector3d external_forces();

  // Not used. Speeds up computation
  Vector3d external_torques(){return Vector3d(0, 0, 0);}
  
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
  GLuint texture_; 

  double angle_;

  // If the anchor point is unassigned, we 
  bool scheduled_death_;
  double death_timer_;
};
#endif