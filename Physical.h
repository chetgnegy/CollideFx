/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Physical.h
  An interface for any object that is to interact via the physics engine.
  */

#ifndef _PHYSICAL_H_
#define _PHYSICAL_H_

#include "vmath.h" //http://bartipan.net/vmath/

class Physical{

public:

  virtual bool has_collisions() = 0;
  virtual bool uses_friction() = 0;
  virtual double intersection_distance() = 0;
  virtual Vector3d external_forces() = 0;
  virtual Vector3d external_torques() = 0;
  Vector3d pos_;
  Vector3d vel_;
  Vector3d acc_;
  double mass_;
  Vector3d ang_pos_;
  Vector3d ang_vel_;
  Vector3d ang_acc_;
  
};


#endif