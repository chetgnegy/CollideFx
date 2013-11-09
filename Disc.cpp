/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.cpp
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#import "Disc.h"

// Pairs the disc with a unit generator
Disc::Disc(UnitGenerator *u, double radius){
  ugen_ = u;
  r_ = radius;
  mass_ = 1;
  x_offset_ = 0; 
  y_offset_ = 0;
  particles_ = new Particle[kNumParticles];
  double theta;
  for (int i = 0; i < kNumParticles; ++i)
  {
      theta = 6.283185*rand()/(1.0*RAND_MAX);
      particles_[i].active = false;
      particles_[i].life = 0.5;
      particles_[i].fade = 0;
      particles_[i].r = 1.0;
      particles_[i].g = 0.0;
      particles_[i].b = 0.5;
      particles_[i].x = 2 * cos(theta);
      particles_[i].y = 0;
      particles_[i].z = 2 * sin(theta);
      particles_[i].dx = 0;
      particles_[i].dy = 0;
      particles_[i].dz = 0;
  }
}

// Cleans up the unit generator
Disc::~Disc(){
  delete[] particles_;
  delete ugen_;
}

// Places disc at certain location  
void Disc::set_location(double x, double y){
  pos_.x = x;
  pos_.y = y;
  pos_.z = 0;
};

// Sets instantaneous velocity of the disc
void Disc::set_velocity(double x, double y){
  vel_.x = x;
  vel_.y = y;
  vel_.z = 0;
};

// OpenGL instructions for drawing a unit disc centered at the origin
void Disc::draw(){

  glPushMatrix();
    glRotatef(90,1,0,0);//face up
    
    draw_particles();
    glPopAttrib();//Pops after drawing particles

    glRotatef(-90,1,0,0);//draw at old angle
    glScalef(r_, r_, 1);

    quadratic=gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
    gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
    gluQuadricTexture(quadratic, GL_TRUE);
    glPushMatrix();
      glColor3f(0,0,0);
      glTranslatef(0,0,.01);
      gluDisk(quadratic,0.70f,1.0f,32,32);
      glPopMatrix();
    glColor3f(1,0,0);
    gluCylinder(quadratic,1.0f,1.0f,1.0f,32,32);
    //Draw the faces
    gluDisk(quadratic,0.0f,1.0f,32,32);
    glTranslatef(0,0,1);
    gluDisk(quadratic,0.0f,1.0f,32,32);

    glPushMatrix();
      glScalef(1,1,.4);
      for (int i = 0; i < 6; ++i){
        glRotatef(60,0,0,1);
          glPushMatrix();
          glTranslatef(.8,0,0);
          glutSolidSphere(.2,6,6);
          glPopMatrix();
      }
      glPopMatrix();
    glPopMatrix();
}

// The current location of the disc's center
void Disc::get_origin(double &x, double &y, double &z){
  x=pos_.x; y=pos_.y; z=0;
}

// The current orientation of the disk
void Disc::get_rotation(double &x, double &y, double &z){
  x=0; y=0; z=0;
}

void Disc::set_attributes(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  
  GLfloat mat_specular[] = { 0.256777, 0.137622,  0.086014, 1.0 };
  GLfloat mat_diffuse[] = { 0.7038,  0.27048, 0.0828, 1.0 };
  GLfloat mat_ambient[] = { 0.19125, 0.0735,  0.0225, 1.0 };
  GLfloat mat_shininess[] = { .928};
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);


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

void Disc::remove_attributes(){
  glPopAttrib();
}


void Disc::prepare_graphics(void){
  GLubyte *tex = new GLubyte[256 * 256 * 3];
  FILE *tf;
  tf = fopen ( "dustbunny_mask.raw", "rb" );
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
  tf = fopen ( "dustbunny.raw", "rb" );
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

//Responds to the user moving in the interface
void Disc::move(double x, double y, double z){
  pos_.x = x - x_offset_;
  pos_.y = y - y_offset_;

}

//Corrects for offset from center of object in user click
void Disc::prepare_move(double x, double y, double z){
  x_offset_ = x - pos_.x;
  y_offset_ = y - pos_.y;
}

//Checks if positions are within radius of center of object
bool Disc::check_clicked(double x, double y, double z){
  if (pow(x-pos_.x,2) + pow(y-pos_.y,2) < pow(r_,2)) return true;
  return false;
}


// Draws the glowing, moving orbs
void Disc::draw_particles(){
  advance_particles();
  double x,y,z;
  double particle_size = 0.4;
  for ( int i=0; i<kNumParticles; ++i ) {
    if(particles_[i].active){
      
      double  reduction;
      x = particles_[i].x ;
      y = particles_[i].y;
      z = particles_[i].z;

      glColor4f(particles_[i].r,
                particles_[i].g,
                particles_[i].b,
                1 );
      glPushMatrix();
      glTranslatef(x,y,z);
      //Draws the particles
      
      /*
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
      glBindTexture (GL_TEXTURE_2D, texture_[0]);

      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f(x + particle_size, 0, z + particle_size); // Top Right
      glTexCoord2d(0,1); glVertex3f(x - particle_size, 0, z + particle_size); // Top Left
      glTexCoord2d(1,0); glVertex3f(x + particle_size, 0, z - particle_size); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(x - particle_size, 0, z - particle_size); // Bottom Left
      glEnd();
     */
      

      glBlendFunc (GL_ONE, GL_ONE);//works //additive blending
      glBindTexture (GL_TEXTURE_2D, texture_[1]);
    
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f(x + particle_size, 0, z + particle_size); // Top Right
      glTexCoord2d(0,1); glVertex3f(x - particle_size, 0, z + particle_size); // Top Left
      glTexCoord2d(1,0); glVertex3f(x + particle_size, 0, z - particle_size); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(x - particle_size, 0, z - particle_size); // Bottom Left
      glEnd();
      
    
      glPopMatrix();
    }
  }
}

// Advances the position of the particles, or possibly triggers new ones
void Disc::advance_particles(){

  double speed = 0.0007;
  
  for (int i = 0; i<kNumParticles; ++i)
    {
      if (particles_[i].active){
        particles_[i].x += particles_[i].dx;
        particles_[i].y += particles_[i].dy;
        particles_[i].z += particles_[i].dz;
      }
      else {
        //Move them somewhere?
        particles_[i].active = true;
        
        

      }

      //going out of bounds
      if (fabs(particles_[i].x) > .5 && fabs(particles_[i].x) < fabs(particles_[i].x + particles_[i].dx)){
        particles_[i].active = false;
      }
      if (fabs(particles_[i].z) > .5 && fabs(particles_[i].z) < fabs(particles_[i].z + particles_[i].dz)){
        particles_[i].active = false;
      }


    }

}


Vector3d Disc::external_forces(){
  return Vector3d(0,0,0);
}