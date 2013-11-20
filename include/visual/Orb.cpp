/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Orb.cpp
  Orb is a class for small drawable objects that glow!
*/

#include "Orb.h"


// A constructor that links the orb to a disc
Orb::Orb(Vector3d *v, double hover){
  //We must give the orb a home or it will fly off and die
  hover_dist_ = hover;
  if (v == NULL) unassign();
  else pos_ = Vector3d(v->x + hover_dist_, v->y, 0);
  
  anchor_point_ = v;
  
  particle_size_ = 0.4;
  m_ = 1; I_ = 0;
  scheduled_death_ = false;
  death_timer_ = 99;
  angle_ = 360.0 * rand()/(1.0*RAND_MAX);
  wander_ = Vector3d(16.0*(rand()/(1.0*RAND_MAX)-.5),
                     16.0*(rand()/(1.0*RAND_MAX)-.5),
                     16.0*(rand()/(1.0*RAND_MAX)-.5) );
  time_ = 0;
  
  use_color_scheme(0);

  
}

Orb::~Orb(){}

// Changes the equilibrium distance of the orb to its anchor point
void Orb::change_hover_distance(double dist){
  hover_dist_ = dist;
}

// Forces the orb's color scheme to be from the list. Out of range goes to normal
void Orb::use_color_scheme(int color_scheme){
  // case 0   Normal
  float rr = 1.0, gr = 1.0, br = 1.0, ro = 0.0, go = 0.0, bo = 0.0; 
  
  switch (color_scheme){
    case 1:  //Fire
      rr = 0.6, gr = .5, br = 0.0, ro = 0.4, go = 0.0, bo = 0.0;
      break;
    case 2:  //Ocean
      rr = 0.9, gr = 0.9, br = 0.0, ro = 0.0, go = 0.0, bo = 1.0;
      break;
    case 3:  //Yellow Jacket
      rr = 0.15, gr = 0.15, br = 0.7, ro = 0.5, go = 0.3, bo = 0.0;
      break;
    case 4:  //Purple Rain
      rr = 0.4, gr = 0.9, br = 0.3, ro = 0.6, go = 0.0, bo = 0.7;
      break;
    case 5:  //Monochrome
      rr = 1.0,  ro = 0.0;
      r_ = rr*rand()/(1.0*RAND_MAX) + ro; g_ = r_; b_ = r_; 
      return;
    case 6:  //Bright
      rr = 0.5, gr = 0.5, br = 0.5, ro = 0.5, go = 0.5, bo = 0.5;
      break;
  
  }
  // Uses the ranges and offsets to make new color schemes
  r_ = rr*rand()/(1.0*RAND_MAX) + ro; 
  g_ = gr*rand()/(1.0*RAND_MAX) + go; 
  b_ = br*rand()/(1.0*RAND_MAX) + bo;
}

// Removes anchor point and schedules destruction.
// Particles fly in all directions!
void Orb::unassign(){
  anchor_point_ = NULL;
  scheduled_death_ = true;
  death_timer_ = kDeathTime;
}

//Do not call this if the orb is associated with a Disc's list!!
void Orb::self_destruct(){
  Graphics::remove_drawable(this);
  Physics::take_physics(this);
  delete this;
}

// Changes the disc that the orb is assigned to
void Orb::reassign(Vector3d *pos){
  anchor_point_ = pos;
}

// Draws the orb at location (0,0,0)
void Orb::draw(){
  double x,y,z;
  double  reduction;
  x = pos_.x;
  y = pos_.y;
  z = pos_.z;

  double alpha = scheduled_death_ ? death_timer_/kDeathTime : 1; 
  alpha = alpha < 0 ? 0 : alpha;
  
  glColor4f(r_, g_, b_, alpha);
  glPushMatrix();
  
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2d(1,1); // Top Right
  glVertex3f(particle_size_, particle_size_, 0); 
  glTexCoord2d(0,1); // Top Left
  glVertex3f(-particle_size_, particle_size_, 0); 
  glTexCoord2d(1,0); // Bottom Right
  glVertex3f(particle_size_, -particle_size_, 0); 
  glTexCoord2d(0,0); // Bottom Left
  glVertex3f(-particle_size_, -particle_size_, 0); 
  glEnd();

  glPopMatrix();
}

// The location of the particle center
void Orb::get_origin(double &x, double &y, double &z){
  x = pos_.x; y = pos_.y; z = pos_.z;
}

// The current orientation of the particle
void Orb::get_rotation(double &w, double &x, double &y, double &z){
  w=angle_; x=0; y=0; z=1;
}

// Sets up the visual attributes for the Orb
void Orb::set_attributes(void){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glClearDepth(1.0f);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable( GL_TEXTURE_2D );
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE);
  glBindTexture (GL_TEXTURE_2D, texture_);

}

//Pops the attributes stack in OpenGL
void Orb::remove_attributes(void){  
  glPopAttrib();
}

GLuint particle_texture; 
bool set_particle_texture = false;
// Loads the textures into the object
void Orb::prepare_graphics(void){
  if (!set_particle_texture){
    GLubyte *tex = new GLubyte[256 * 256 * 3];
    FILE *tf = fopen ( "graphics/dustbunny.raw", "rb" );
    fread ( tex, 256 * 256 * 3, 1, tf );
    fclose ( tf );

    glGenTextures(1, &particle_texture);
    glBindTexture(GL_TEXTURE_2D, particle_texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, tex);
    delete [] tex;
  }
  texture_ = particle_texture;
  set_particle_texture = true;

}

// Advances the internal clock of the particles. This
// currently only adjusts the orientation of the particles
void Orb::advance_time(double t){ 
  time_ += t; 
    
  if (scheduled_death_){ death_timer_ -= t;

    // Flies up!
    wander_.x += .6*rand()/(1.0*RAND_MAX)-.3;
    wander_.y += .6*rand()/(1.0*RAND_MAX)-.3;
    wander_.z += rand()/(1.0*RAND_MAX)-.45;  
  }
  else {
    //Random
    wander_.z += rand()/(1.0*RAND_MAX)-.5;
    wander_.x += rand()/(1.0*RAND_MAX)-.5;
    wander_.y += rand()/(1.0*RAND_MAX)-.5;
  
  }
}

// If a particle is unassigned, this could result in a call to 
// self_destruct. Be careful with self_destruct.
void Orb::clean_up(){
  if (death_timer_ < 0) self_destruct();
}


// The forces are handled here and called from Physics.cpp during nunerical integration
Vector3d Orb::external_forces(){
  if (anchor_point_ == NULL) {
    return wander_;
  }
  // The vector pointing to the anchor point
  Vector3d axis = *anchor_point_ - pos_;
  double norm_distance = axis.length()/hover_dist_;
  double parabolic = -1.0/pow(norm_distance, 2)+ pow(norm_distance, 2);
  axis.normalize();
  return axis * 40 * parabolic - vel_ * 2 + wander_;
}
