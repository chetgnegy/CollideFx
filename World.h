/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.h
*/
#ifndef _DISC_H_
#define _DISC_H_

#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif


class World {
public:
  //Creates the world
  World(double x, double y);
  //Should clean up all discs inside too
  ~World();
  //Draws the world in OpenGl
  void draw();
  
private:
  //size of the map
  double x_, y_;

};


#endif