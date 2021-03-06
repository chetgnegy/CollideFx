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
  static void integrate_translational(double timestep, Physical* object);

  // Velocity Verlet algorithm applied to the angular motion
  static void integrate_rotational(double timestep, Physical* object);

  // Uses vector projections to make sure things don't get too close to each other
  static void collision_prevention();

  // Handles a collision using conservation of linear momentum
  static void collide(Physical* a, Physical* b);

  // Handles collision detection with world
  static void check_in_bounds();

  // Defines the bounds for the collision detection with the walls
  static void set_bounds(double size_x, double size_y, double x, double y);

  // Checks to see if an area is within the bounds of the world and also
  // clear of discs
  static bool is_clear_area(double x, double y, double r);


private:
  static std::list<Physical *> all_;
  static double x_max_,x_min_,y_max_,y_min_;
  
};

#endif
