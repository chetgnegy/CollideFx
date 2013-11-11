/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Particle.cpp
  Particles are classes for small drawable objects that glow!
*/

#include "Particle.h"

Orb::Orb(Vector3d *v){
  pull_point_ = v;
  
  angle_ = 0;
  hover_dist_ = 2.0;
  particle_size_ = 0.4;
  m_ = 1;
  wander_ = Vector3d(16.0*(rand()/(1.0*RAND_MAX)-.5),
                     16.0*(rand()/(1.0*RAND_MAX)-.5),
                     16.0*(rand()/(1.0*RAND_MAX)-.5) );
  time_ = 0;
  p_.active = true; p_.life = 0.5; p_.fade = 0;
  p_.r = rand()/(1.0*RAND_MAX); 
  p_.g = rand()/(1.0*RAND_MAX); 
  p_.b = rand()/(1.0*RAND_MAX);
  //These are not used at all inside of hte orbs
  p_.x = 0; p_.y = 0; p_.z = 0;
  p_.dx = 0; p_.dy = 0; p_.dz = 0;
}

Orb::~Orb(){}

void Orb::reassign(Vector3d *pos){
  pull_point_ = pos;
}

void Orb::draw(){
  double x,y,z;
  if(p_.active){
      double  reduction;
      x = pos_.x;
      y = pos_.y;
      z = pos_.z;

      glColor4f(p_.r, p_.g, p_.b, 1);
      glPushMatrix();
      //glTranslatef(x,y,z);
      //glRotatef(90,1,0,0);
      //Draws the particles
      
      /*
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
      glBindTexture (GL_TEXTURE_2D, texture_[0]);

      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f(x + particle_size_, 0, z + particle_size_); // Top Right
      glTexCoord2d(0,1); glVertex3f(x - particle_size_, 0, z + particle_size_); // Top Left
      glTexCoord2d(1,0); glVertex3f(x + particle_size_, 0, z - particle_size_); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(x - particle_size_, 0, z - particle_size_); // Bottom Left
      glEnd();
     */
      
      glBlendFunc (GL_ONE, GL_ONE);//works //additive blending
      glBindTexture (GL_TEXTURE_2D, texture_[1]);
    
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f(particle_size_, particle_size_, 0); // Top Right
      glTexCoord2d(0,1); glVertex3f(- particle_size_, particle_size_, 0); // Top Left
      glTexCoord2d(1,0); glVertex3f(particle_size_, - particle_size_, 0); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(- particle_size_, - particle_size_, 0); // Bottom Left
      glEnd();
    
    
      glPopMatrix();
    }
}

// The location of the particle center
void Orb::get_origin(double &x, double &y, double &z){
  x = pos_.x; y = pos_.y; z = pos_.z;
}

void Orb::get_rotation(double &w, double &x, double &y, double &z){
  w=ang_pos_.length(); x=ang_pos_.x; y=ang_pos_.y; z=ang_pos_.z;
}

// Sets up the visual attributes for the Orb
void Orb::set_attributes(void){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glClearDepth(1.0f);
  glDisable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable( GL_TEXTURE_2D );
  glEnable (GL_BLEND);
}

//Pops the attributes stack in OpenGL
void Orb::remove_attributes(void){  
  glPopAttrib();
}


void Orb::prepare_graphics(void){
  GLubyte *tex = new GLubyte[256 * 256 * 3];
  FILE *tf;
  tf = fopen ( "graphics/dustbunny_mask.raw", "rb" );
  fread ( tex, 256 * 256 * 3, 1, tf );
  fclose ( tf );
  
  glGenTextures(1, &texture_[0]);
  glBindTexture(GL_TEXTURE_2D, texture_[0]);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, tex);
  delete [] tex;

  tex = new GLubyte[256 * 256 * 3];
  tf = fopen ( "graphics/dustbunny.raw", "rb" );
  fread ( tex, 256 * 256 * 3, 1, tf );
  fclose ( tf );

  glGenTextures(1, &texture_[1]);
  glBindTexture(GL_TEXTURE_2D, texture_[1]);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, tex);
  
  delete [] tex;
}
void Orb::advance_time(double t){ 
  time_ += t; 
  wander_.x += rand()/(1.0*RAND_MAX)-.5;
  wander_.y += rand()/(1.0*RAND_MAX)-.5;
  wander_.z += rand()/(1.0*RAND_MAX)-.5;
}

Vector3d Orb::external_forces(){
  if (pull_point_ == 0) 
    return wander_;
  Vector3d axis = *pull_point_ - pos_;
  double norm_distance = axis.length()/hover_dist_;
  double parabolic = -1.0/pow(norm_distance, 2)+ pow(norm_distance, 2);
  axis.normalize();
  return axis * 40 * parabolic - vel_ * 2 + wander_;
}