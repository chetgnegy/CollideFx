/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Disc.cpp
  This file the Disc class. This class is used as the graphical representation of a unit generator
*/

#import "Disc.h"



Disc* Disc::spotlight_disc_ = NULL;
bool Disc::texture_loaded_ = false;
GLuint *Disc::tex_ = new GLuint[15];

// Pairs the disc with a unit generator
Disc::Disc(UnitGenerator *u, double radius, bool ghost, int initial_orbs, int maintain_orbs){
  ugen_ = u;
  r_ = radius;
  m_ = radius * radius;
  I_ = 0.5 * m_ * m_; // 0.5*M*R^2
  x_offset_ = 0; 
  y_offset_ = 0;
  is_clicked_ = false;
  texture_loaded_ = false;
  initial_orbs_ = initial_orbs;
  maintain_orbs_ = maintain_orbs;
  color_ = Vector3d(0,0,1);
  ghost_ = ghost;
  which_texture_ = -1;
  orb_color_scheme_ = 0;
}

// Cleans up the unit generator
Disc::~Disc(){
  delete ugen_;
}

// Sets the color of the disc -- changes material properties
void Disc::set_color(float r, float  g, float b){
  color_.x = r; color_.y = g; color_.z = b;
}

// Changes the image on the face of the disc
void Disc::set_texture(int i){
  which_texture_ = i;
}

// Creates a new orb to hang out around this disc
void Disc::orb_create(int num_orbs){
  for (int i = 0; i < num_orbs; ++i){
    Orb *roy_orbison = new Orb(&pos_, 2*r_);
    roy_orbison->use_color_scheme(orb_color_scheme_);
    orbs_.push_back(roy_orbison);
    Graphics::add_drawable(roy_orbison);
    Physics::give_physics(roy_orbison);
  }
}

// Passes the orb to another disc, d.
bool Disc::orb_handoff(Disc *d){
  if (orbs_.size() > 0) {
      d->orb_receive(*orbs_.begin());
      orbs_.erase(orbs_.begin());  
      return true;
  }
  return false;
}

// Receives an orb from another Disc
void Disc::orb_receive(Orb *roy_orbison){
  roy_orbison->reassign(&pos_);
  roy_orbison->change_hover_distance(r_);
  orbs_.push_back(roy_orbison);
}

// Deletes the orb after removing all instances of it
bool Disc::orb_destroy(){
  if (orbs_.size() > 0) {
    Graphics::remove_drawable(*orbs_.begin());
    Physics::take_physics(*orbs_.begin());
    delete *orbs_.begin();
    orbs_.erase(orbs_.begin());  
    return true;
  }
  return false;
}

// Tells the particle to just fly away. It eventually deletes
// itself. See Orb::self_destruct();
bool Disc::orb_abandon(){
  if (orbs_.size() > 0) {
    (*orbs_.begin())->unassign();
    orbs_.erase(orbs_.begin());
    return true;
  }
  return false;
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


// Forwards request for parameter values
double Disc::get_ugen_params(int param){
  return ugen_->get_normalized_param(param);
}
// Forwards request to set parameter values
void Disc::set_ugen_params(double param1, double param2){
  ugen_->set_normalized_param(param1, param2);
}

// OpenGL instructions for drawing a unit disc centered at the origin
void Disc::draw(){
 // Ghost mode makes it partially transparent
 double alpha = ghost_? 0.3 : 1.0;

 glPushMatrix();
    glScalef(r_, r_, 1);
    int res = 16;

    quadratic=gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
    gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
    gluQuadricTexture(quadratic, GL_TRUE);
    glPushMatrix();
      glColor4f(0,0,0, alpha);
      glTranslatef(0,0,.01);
      gluDisk(quadratic,0.70f,1.0f,res,res);
      glPopMatrix();
    glColor4f(color_.x,color_.y,color_.z, alpha);
    gluCylinder(quadratic,1.0f,1.0f,1.0f,res,res);
    //Draw the faces
    gluDisk(quadratic,0.0f,1.0f,res,res);
    glTranslatef(0,0,1);
    
    // Prepare attributes for top face
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if (which_texture_>=0){
      glEnable(GL_BLEND);
      glShadeModel(GL_FLAT);
      glDepthMask(GL_TRUE);  // disable writes to Z-Buffer
      glDisable(GL_DEPTH_TEST);  // disable depth-testing
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, tex_[which_texture_]);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    // Draw top design
    gluDisk(quadratic,0.0f,1.0f,res,res);

    glPopAttrib();

    glPopMatrix();
}

// The current location of the disc's center
void Disc::get_origin(double &x, double &y, double &z){
  x=pos_.x; y=pos_.y; z=0;
}

// The current orientation of the disk
void Disc::get_rotation(double &w, double &x, double &y, double &z){
  w=ang_pos_.length()*180.0 / 3.1415926535; 
  x=ang_pos_.x; y=ang_pos_.y; z=ang_pos_.z;
}

// Sets up the visual attributes for the Disc
void Disc::set_attributes(){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  GLfloat mat_specular[] = { .5*color_.x, .5*color_.y,  .5*color_.z, 1.0 };
  GLfloat mat_diffuse[] = { color_.x, color_.y,  color_.z, 1.0 };
  GLfloat mat_ambient[] = { color_.x, color_.y,  color_.z, 1.0 };
  GLfloat mat_shininess[] = { .0};
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

  glClearDepth(1.0f);
  glEnable (GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 
  
}

// Pops the most recent set of attributes off of the stack
void Disc::remove_attributes(){
  glPopAttrib();
}

// Imports the textures for the icons
void Disc::prepare_graphics(void){
  if (!texture_loaded_){
    tex_[0] = loadTextureFromFile( "graphics/input.bmp" );
    tex_[1] = loadTextureFromFile( "graphics/sine.bmp" );
    tex_[2] = loadTextureFromFile( "graphics/square.bmp" );
    tex_[3] = loadTextureFromFile( "graphics/triangle.bmp" );
    tex_[4] = loadTextureFromFile( "graphics/saw.bmp" );
    tex_[5] = loadTextureFromFile( "graphics/bitcrusher.bmp" );
    tex_[6] = loadTextureFromFile( "graphics/chorus.bmp" );
    tex_[7] = loadTextureFromFile( "graphics/delay.bmp" );
    tex_[8] = loadTextureFromFile( "graphics/distortion.bmp" );
    tex_[9] = loadTextureFromFile( "graphics/filter.bmp" );
    tex_[10] = loadTextureFromFile( "graphics/bandpass.bmp" );
    tex_[11] = loadTextureFromFile( "graphics/looper.bmp" );
    tex_[12] = loadTextureFromFile( "graphics/ringmod.bmp" );
    tex_[13] = loadTextureFromFile( "graphics/reverb.bmp" );
    tex_[14] = loadTextureFromFile( "graphics/tremolo.bmp" );
    texture_loaded_ = true;
  }
}

//Responds to the user moving in the interface
void Disc::move(double x, double y, double z){
  is_clicked_ = true;
  pull_point_ = Vector3d(x,y,0);

}

//Corrects for offset from center of object in user click
void Disc::prepare_move(double x, double y, double z){
  x_offset_ = x - pos_.x;
  y_offset_ = y - pos_.y;
}

//Checks if positions are within radius of center of object
bool Disc::check_clicked(double x, double y, double z){
  if (pow(x-pos_.x,2) + pow(y-pos_.y,2) < pow(r_,2)){ 
    return true;
  }
  return false;
}

//Signals that the disc is no longer clicked.
void Disc::unclicked(){ 
  is_clicked_ = false;
  if (ghost_){ 
    orb_create(initial_orbs_);
    ghost_ = false;
  }
}


// Broadcasts the disc that has been selected so the menu can see it.
void Disc::right_clicked(){
  spotlight_disc_ = this;
}

// The forces are handled here. This is called from Physics.cpp during the numerical integration step.
Vector3d Disc::external_forces(){
  //.5 * 2 * r * Cd * p * v^2 ~= r * v^2
  Vector3d drag = -vel_ * vel_.length() * .05 * r_;
  if (is_clicked_){
    Vector3d anchor(pos_.x + x_offset_, pos_.y + y_offset_, 0);
    
    // Before it is placed it should stick closely to the cursor
    if (ghost_) return (pull_point_-anchor)*1200.0 - vel_*70;
    
    //Drag, spring force and dampling
    return drag + (pull_point_-anchor)*75.0 - vel_*10; 
  }
  return drag;
}

// The torques are handled here. This is called from Physics.cpp during the numerical integration step.
Vector3d Disc::external_torques(){
  return Vector3d(0,0,0) * 180.0 / 3.1415926535;
}


// Loads a texture from a file, must be in bmp format
GLuint Disc::loadTextureFromFile( const char * filename ){
  GLuint texture;
  glGenTextures( 1, &texture );
  glBindTexture(GL_TEXTURE_2D, texture);

  RgbImage theTexMap( filename ); // instantiation

  // Set the interpolation settings to best quality.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
  theTexMap.GetNumCols(), theTexMap.GetNumRows(),
  GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

  return texture;
}
