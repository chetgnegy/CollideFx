/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.h
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#ifndef _DISC_H_
#define _DISC_H_

#include "UnitGenerator.h"

class Disc{
public:
  //Pairs the disc with a unit generator
  Disc(UnitGenerator *u, double radius); 
  //Cleans up the unit generator
  ~Disc();
  
  //Places disc at certain location
  void set_location(double x, double y);
  //Sets instantaneous velocity of the disc
  void set_velocity(double x, double y);
  
private:
  UnitGenerator *ugen_;
  //positions
  double x_, y_;
  //velocity
  double vx_, vy_;
  //forces
  double fx_, fy_;
  //radius
  double r_;

};


#endif