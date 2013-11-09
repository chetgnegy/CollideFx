/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.h
*/
#ifndef _WORLD_H_
#define _WORLD_H_

#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>//remove
#include "Drawable.h" //imports opengl stuff, too
#include "Particle.h"


class World : public Drawable {
public:
  static const double kWallThickness = 0.02;
  static const int kNumParticles = 5;
  static const int kNumLines = 15;
  static const double kAnimationFrequency = .25;
  
  // Creates the world
  World(double sx, double sy, double x, double y);
  // Should clean up all discs inside too
  ~World();
  // Draws the world in OpenGL
  void draw();
  
  // Sets up the transparency that is necessary for the texture
  void set_attributes();
  
  // Pops the attributes matrix off the stack
  void remove_attributes();

  // Gets the current position of the world. The world doesn't need to move.
  void get_origin(double &x, double &y, double &z);
  
  // Gets the current orientation of the world.
  void get_rotation(double &w, double &x, double &y, double &z);
  
  // initializes the textures
  void prepare_graphics(void);

  // Advances the graphics
  void advance_time(double time);
  
  
private:
  // Draws a wireframe wall and a pulsing interior
  void draw_wall(int size);
  
  // Draws the glowing lines and the moving orbs
  void draw_lines();
  
  // Draws the glowing, moving orbs
  void draw_particles();
  
  // Advances the position of the particles, or possibly triggers new ones
  void advance_particles();
  
  // size of the map
  double size_x_, size_y_;
  double x_, y_;
  Particle *particles_;
  GLuint texture_[2]; 
  // Ticks the graphics counter
  double ticky_;
};



#endif