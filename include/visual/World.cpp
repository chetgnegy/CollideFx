/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  World.cpp

*/
#include "World.h"

// Creates the world
World::World(double sx, double sy, double x, double y){
  size_x_ = sx;
  size_y_ = sy;
  x_ = x;
  y_ = y;
  particles_ = new Particle[kNumParticles];
  ticky_ = 0;
  for (int i = 0; i<kNumParticles; ++i)
    {
        particles_[i].active = false;
        particles_[i].life = 0.5;// - float(rand()%100)/100.0f;
        particles_[i].fade = 0;//float(rand()%50)/1000.0f;
        particles_[i].r = 0;
        particles_[i].g = 0.8 +rand()/(5.0*RAND_MAX);
        particles_[i].b = 0.2 +rand()/(5.0*RAND_MAX);
        particles_[i].x = 0;
        particles_[i].y = 0;
        particles_[i].z = 0;
        particles_[i].dx = 0;
        particles_[i].dy = 0;
        particles_[i].dz = 0;
    }
}

// Should clean up all discs inside too
World::~World(){
  delete[] particles_;
}


// Draws the world in OpenGl
void World::draw(){
  glPushMatrix();
    glRotatef(90,1,0,0);//face up
    glScalef(size_x_,.4,size_y_);//makes whole thing thinner and scales to actual size
    glTranslatef(0, -.5, 0);

    draw_lines();
    
    glColor3f(.1,.1,.1);
    glTranslatef(0, 1, 0);//walls height
    
    //First two walls
    glTranslatef(-.5 * (1 - kWallThickness), 0, 0);
    glPushMatrix();
      glScalef(kWallThickness,1,1);
      draw_wall(1);
      glPopMatrix();
    glPushMatrix();
      glTranslatef(1 - kWallThickness, 0, 0);
      glScalef(kWallThickness,1,1);
      draw_wall(1);
      glPopMatrix();
    //Last Two walls
    glRotatef(90,0,1,0);
    glTranslatef(-.5 , 0, .5);
    glPushMatrix();
      glTranslatef(1- .5 * kWallThickness, 0, -.5 * kWallThickness);
      glScalef(kWallThickness,1,1);
      draw_wall(1);
      glPopMatrix();
    glPushMatrix();
      glTranslatef(.5 * kWallThickness, 0, -.5 * kWallThickness);
      glScalef(kWallThickness,1,1);
      draw_wall(1);
      glPopMatrix();
      
  glPopMatrix();  
}



// Sets up the transparency that is necessary for the texture
void World::set_attributes(){
  glClearColor( 0, 0, 0, 1 );
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable( GL_COLOR_MATERIAL );
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glClearDepth(1.0f);
  glDisable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texture_[1]);  

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glBindTexture(GL_TEXTURE_2D,texture_[0]);  
}

// Pops the attributes matrix off the stack
void World::remove_attributes(){
  glPopAttrib();
}

// Gets the current position of the world. The world doesn't need to move.
void World::get_origin(double &x, double &y, double &z){
  x=x_; y=y_; z=0;
}

// Gets the current orientation of the world.
void World::get_rotation(double &w, double &x, double &y, double &z){
   w=0; x=0; y=0; z=0;
}

// initializes the textures
void World::prepare_graphics(void){
    GLubyte *tex = new GLubyte[32 * 32 * 3];
    FILE *tf;
    tf = fopen ( "graphics/particle.raw", "rb" );
    fread ( tex, 1, 32 * 32 * 3, tf );
    fclose ( tf );

    glGenTextures(1, &texture_[0]);

    glBindTexture(GL_TEXTURE_2D, texture_[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);
    delete [] tex;
 
    unsigned char rgba[] = { 0, 0, 0, 0,
                             0, 0, 0, 0,  
                             50, 50, 50, 10,
                             120, 120, 120, 100 ,   
                             255, 255, 255, 255 ,
                             120, 120, 120, 100 , 
                             50, 50, 50, 10,
                             0, 0, 0, 0,
                             0, 0, 0, 0 };

    glGenTextures(1, &texture_[1]);

      
    glBindTexture(GL_TEXTURE_2D, texture_[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 9, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

}

void World::advance_time(double time){
  ticky_ += time;
  if (ticky_ > 1/kAnimationFrequency) {
    ticky_ -= 1/kAnimationFrequency ;

  }
  
};


// Draws a wireframe wall and a pulsing interior
void World::draw_wall(int size){
  glColor4f(0, 1, .5, 1);
  glutWireCube(size);
  //The pulsing walls
  glColor4f(.3, 1, .1, 0.1*(1-sin(kAnimationFrequency*6.283185*ticky_))+.05);
  glutSolidCube(size);
}


// Draws the glowing lines and the moving orbs
void World::draw_lines(){
  glPushMatrix();
    glTranslatef(0.0, 0.5, 0.0);
    //Draws the glowy orbs
    draw_particles();
    glPopAttrib();

    //The glowing of the lines
    glColor4f(0.2, 1.0, 0.0, 0.2*sin(kAnimationFrequency*6.283185*ticky_)+0.5);
    
    for (int i = 1 ; i < kNumLines; ++i){
      //Draws Horizontal Lines
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f(i/(1.0*kNumLines) - 0.49, 0.0,  0.5); // Top Right
      glTexCoord2d(0,1); glVertex3f(i/(1.0*kNumLines) - 0.51, 0.0,  0.5); // Top Left
      glTexCoord2d(1,0); glVertex3f(i/(1.0*kNumLines) - 0.49, 0.0, -0.5); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(i/(1.0*kNumLines) - 0.51, 0.0, -0.5); // Bottom Left
      glEnd();
      //Draws veritcal lines
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(1,1); glVertex3f( .5, 0, i/(1.0*kNumLines) - 0.49); // Top Right
      glTexCoord2d(0,1); glVertex3f( .5, 0, i/(1.0*kNumLines) - 0.51); // Top Left
      glTexCoord2d(1,0); glVertex3f(-.5, 0, i/(1.0*kNumLines) - 0.49); // Bottom Right
      glTexCoord2d(0,0); glVertex3f(-.5, 0, i/(1.0*kNumLines) - 0.51); // Bottom Left
  
      glEnd();

    }
    glPopAttrib();
    glPopMatrix(); 
}

// Draws the glowing, moving orbs
void World::draw_particles(){
  advance_particles();

  double particle_size = .017;
  for ( int i=0; i<kNumParticles; ++i ) {
    if(particles_[i].active){
      
      double tempx, tempz, reduction;
      for (int k = 0; k < 10; ++k){
        
        tempx = particles_[i].x - 1000*particles_[i].dx * particle_size*k;
        tempz = particles_[i].z - 1000*particles_[i].dz * particle_size*k;

        if (fabs(tempx)<.5 && fabs(tempz)<.5){
          reduction = pow(10-k,2)/100.0;
          glColor4f(particles_[i].r,
                    particles_[i].g,
                    particles_[i].b,
                    reduction + .2*sin(ticky_));
          //Draws the particles
          glBegin(GL_TRIANGLE_STRIP);
          glTexCoord2d(1,1); glVertex3f(tempx + particle_size, 0, tempz + particle_size); // Top Right
          glTexCoord2d(0,1); glVertex3f(tempx - particle_size, 0, tempz + particle_size); // Top Left
          glTexCoord2d(1,0); glVertex3f(tempx + particle_size, 0, tempz - particle_size); // Bottom Right
          glTexCoord2d(0,0); glVertex3f(tempx - particle_size, 0, tempz - particle_size); // Bottom Left
          glEnd();
        }
      }
    }
  }
}

// Advances the position of the particles, or possibly triggers new ones
void World::advance_particles(){

  double speed = 0.002;
  for (int i = 0; i<kNumParticles; ++i)
    {
      if (particles_[i].active){
        particles_[i].x += particles_[i].dx;
        particles_[i].y += particles_[i].dy;
        particles_[i].z += particles_[i].dz;
      }
      else if (rand()/(1.0*RAND_MAX) < .0003){
        particles_[i].active = true;
        int k = rand()%4;
        int j = rand()%(kNumLines-1) + 1;
        particles_[i].r = 0.0;
        particles_[i].g = 0.5 +rand()/(2.0*RAND_MAX);
        particles_[i].b = 0.1 +rand()/(4.0*RAND_MAX);
        
        switch(k){
          case 1:
            particles_[i].x = - 0.5;
            particles_[i].y = 0.0;
            particles_[i].z = j/(1.0*kNumLines) - 0.5;
            particles_[i].dx = speed*(1+rand()/(1.0*RAND_MAX));
            particles_[i].dy = 0;
            particles_[i].dz = 0;
            break;
          case 2:
            particles_[i].x = 0.5;
            particles_[i].y = 0.0;
            particles_[i].z = j/(1.0*kNumLines) - 0.5;
            particles_[i].dx = -speed*(1+rand()/(1.0*RAND_MAX));
            particles_[i].dy = 0;
            particles_[i].dz = 0;
            break;
          case 3:
            particles_[i].x = j/(1.0*kNumLines) - 0.5;
            particles_[i].y = 0.0;
            particles_[i].z = 0.5;
            particles_[i].dx = 0;
            particles_[i].dz = 0;
            particles_[i].dz = -speed*(1+rand()/(1.0*RAND_MAX));
            break;
          case 0:
            particles_[i].x = j/(1.0*kNumLines) - 0.5;
            particles_[i].y = 0.0;
            particles_[i].z = -0.5;
            particles_[i].dx = 0;
            particles_[i].dy = 0;
            particles_[i].dz = speed*(1+rand()/(1.0*RAND_MAX));
            break;
        }
        

      }
      //going out of bounds
      
      if (fabs(particles_[i].x) > 0.8 && fabs(particles_[i].x) < fabs(particles_[i].x + particles_[i].dx)){
        particles_[i].active = false;
      }
      if (fabs(particles_[i].z) > 0.8 && fabs(particles_[i].z) < fabs(particles_[i].z + particles_[i].dz)){
        particles_[i].active = false;
      }

    }

}


