/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Drawable.cpp
  An interface for things that are to be passed to OpenGL
*/
#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

class Drawable{
public: 
  virtual void draw() = 0;
  virtual void get_origin(double &x, double &y, double &z) = 0;
  virtual void get_rotation(double &x, double &y, double &z) = 0;
};


#endif