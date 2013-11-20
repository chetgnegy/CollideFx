/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.h
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#ifndef _DISC_H_
#define _DISC_H_

#include "RgbImage.h"
#include "UnitGenerator.h"
#include "Physics.h"
#include "Graphics.h"
#include "Drawable.h" //imports opengl stuff, too
#include "Moveable.h"
#include "Orb.h"
#include "Physical.h"
#include "vmath.h"


class Disc : public Drawable, public Moveable, public Physical{
public:
  static const int kNumParticles = 5;

  // Pairs the disc with a unit generator, can be set to ghost mode
  Disc(UnitGenerator *u, double radius, bool ghost, int initial_orbs = 0, int maintain_orbs = 0); 
  // Cleans up the unit generator
  ~Disc();
  
  // Sets the color of the disc -- changes material properties
  void set_color(float r, float g, float b);

  double get_radius(){return r_;} 

  // Changes the image on the face of the disc
  void set_texture(int i);

  // Changes the color scheme of any orbs that are created
  void delegate_orb_color_scheme(int color_scheme){
    orb_color_scheme_ = color_scheme;
  }

  // Creates a new orb to hang out around this disc
  void orb_create(int num_orbs = 1);

  // Passes the orb to another disc, d.
  bool orb_handoff(Disc *d);
  
  // Receives an orb from another Disc
  void orb_receive(Orb *);
  
  // Deletes the orb after removing all references to it
  bool orb_destroy();
  
  // Tells the particle to just fly away. It eventually deletes
  // itself. See Orb::self_destruct();
  bool orb_abandon();

  // Places disc at certain location
  void set_location(double x, double y);

  // Sets instantaneous velocity of the disc
  void set_velocity(double x, double y);

  UnitGenerator *get_ugen(){ return ugen_; }

  // Forwards request for parameter values
  double get_ugen_params(int param);

  void set_ugen_params(double param1, double param2);
  
  /* ----- Drawable ----- */

  // OpenGL instructions for drawing a unit disc centered at the origin
  void draw(void);
  
  // The actual location of the disc's center
  void get_origin(double &x, double &y, double &z);
  
  // The orientation of the disk
  void get_rotation(double &w, double &x, double &y, double &z);

  // Sets up the visual attributes for the Disc
  void set_attributes();
  
  // Pops the attributes matrix off the stack
  void remove_attributes();
  
  // initializes the textures
  void prepare_graphics(void);

  /* ----- Moveable ----- */

  //Responds to the user moving in the interface
  void move(double x, double y, double z);

  //Corrects for offset from center of object in user click
  void prepare_move(double x, double y, double z);
  
  //Checks if positions are within radius of center of object
  bool check_clicked(double x, double y, double z);
  
  //Signals that the disc is no longer clicked.
  void unclicked();

  void right_clicked();

  /* ----- Physics ----- */

  //Discs can have collisions with other discs
  bool has_collisions(){ return !ghost_; }

  //The discs have friction with the ground below them
  bool uses_friction(){ return !ghost_; }

  bool rotates(){return false;}
  
  double intersection_distance(){ return r_; }

  // The forces are handled here. This is called from Physics.cpp during the numerical integration step.
  Vector3d external_forces();

  // The torques are handled here. This is called from Physics.cpp during the numerical integration step.
  Vector3d external_torques();

  static Disc *spotlight_disc_;


private:
  // Reads in a bitmap file and uses it as a texture.
  GLuint loadTextureFromFile( const char * filename );

  UnitGenerator *ugen_;
  
  //position offsets
  double x_offset_, y_offset_;
  Vector3d pull_point_;
  bool is_clicked_;

  //radius
  double r_;
  
  //An object that is useful for drawing the cylinder
  GLUquadricObj *quadratic; 

  std::list<Orb *> orbs_;
  Vector3d color_;
  int initial_orbs_;
  int maintain_orbs_;

  bool ghost_;

  static GLuint *tex_;
  static bool texture_loaded_;
  int which_texture_;

  int orb_color_scheme_;


};


#endif
