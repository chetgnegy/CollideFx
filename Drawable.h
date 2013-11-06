/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Drawable.cpp
  An interface for things that are to be passed to OpenGL
*/
#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif

class Drawable {
public: 
  virtual void draw() = 0;
  virtual void get_origin(double &x, double &y, double &z) = 0;
  virtual void get_rotation(double &x, double &y, double &z) = 0;
};


#endif