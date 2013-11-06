/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.h
*/
#ifndef _WORLD_H_
#define _WORLD_H_

#import "Drawable.h" //imports opengl stuff, too

class World : public Drawable {
public:
  //Creates the world
  World(double x, double y);
  //Should clean up all discs inside too
  ~World();
  //Draws the world in OpenGl
  void draw();
  void get_origin(double &x, double &y, double &z);
  void get_rotation(double &x, double &y, double &z);
  
private:
  //size of the map
  double x_, y_;

};


#endif