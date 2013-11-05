/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  GraphicsBox.h
  This is the object that interacts directly with OpenGL.
*/

#ifndef _GRAPHICSBOX_H_
#define _GRAPHICSBOX_H_

#include <list>
#include "Drawable.h"

#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif



class GraphicsBox{
public: 
  GraphicsBox(int w, int h);
  ~GraphicsBox();
  //Creates the OpenGL instance
  int initialize(int argc, char *argv[]);
  
  //Starts the main loop
  void start_graphics();

  void add_drawable(Drawable *);

private:
  void display_function();
  int w_, h_;
  std::list<Drawable *> items_;
  
};



#endif