/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Physics.h
  A physics engine to move the discs, and particles around within the world.
  */

#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include <list>
#include "Physical.h"

class Physics {
public:
  static const double kTimestep = 0.01; // seconds
  static const double kGravity = 9.81; // m/s^2 
  // Adds an object for which physics will be computed
  static void give_physics(Physical *object);
  
  // Removes physics from an object
  static bool take_physics(Physical *object);

  // Updates the positions of all objects 
  static void update(double timestep);

  // Uses Velocity Verlet integration to compute the next positions of the object
  static void velocity_verlet(double timestep, Physical* object);

  // Velocity Verlet algorithm applied to the angular motion
  static void angular_verlet(double timestep, Physical* object);

  // Uses vector projections to make sure things don't get too close to each other
  static void collision_prevention();

  // Check to see if two objects are so close that we should reduce the timestep
  static bool check_reduce_timestep();

private:
  static std::list<Physical *> all_;

};

#endif