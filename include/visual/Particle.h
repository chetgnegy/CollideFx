/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Particle.h
  Particles are classes for small drawable objects that glow!
*/

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "Drawable.h"
#include "Physical.h"
#include "vmath.h"

typedef struct {
    bool active;
    float life;
    float fade;
    float r, g, b;
    float x, y, z;
    float dx, dy, dz;
} Particle;

class Orb : public Drawable, public Physical {
public:
  Orb(Vector3d *v = 0);
  ~Orb();

  void reassign(Vector3d *v);

  void draw();

  // The location of the particle center
  void get_origin(double &x, double &y, double &z);
  void get_rotation(double &w, double &x, double &y, double &z);
  
  // Sets up the visual attributes for the Particle
  void set_attributes(void);
  void remove_attributes(void);  
  void prepare_graphics(void);
  void advance_time(double t);

  bool has_collisions(){return false;}
  bool uses_friction(){return false;}
  double intersection_distance(){return 0;}
  Vector3d external_forces();
  Vector3d external_torques(){return Vector3d(0, 0, rand()/(1.0*RAND_MAX)-.5);}
  
private:
  double angle_;
  Vector3d *pull_point_;
  Vector3d wander_;
  double time_;
  Particle p_;
  double hover_dist_;
  double particle_size_;
  GLuint texture_[2]; 
};
#endif