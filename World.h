/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.h
*/
#ifndef _WORLD_H_
#define _WORLD_H_

#include <stdlib.h>
#include <iostream>
#include <algorithm>
#import "Drawable.h" //imports opengl stuff, too

typedef struct {
    bool active;
    float life;
    float fade;
    float r, g, b;
    float x, y, z;
    float dx, dy, dz;
    int enroute;
} Particle;

class World : public Drawable {
public:
  static const double kWallThickness = 0.02;
  static const int kNumParticles = 5;
  static const int kNumLines = 15;
  
  //Creates the world
  World(double sx, double sy, double x, double y);
  //Should clean up all discs inside too
  ~World();
  //Draws the world in OpenGL
  void draw();
  
  void set_attributes();
  
  void remove_attributes();

  // Gets the current position of the world. The world doesn't need to move.
  void get_origin(double &x, double &y, double &z);
  
  // Gets the current orientation of the world.
  void get_rotation(double &x, double &y, double &z);
  
  void prepare_graphics(void);

  void handle_particles();
  
private:
  //size of the map
  double size_x_, size_y_;
  double x_, y_;
  void draw_lines();
  void tron_effect();
  Particle *particles_;
  GLuint texture_[3]; 
};



#endif