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
  m_ = radius * radius;
  x_offset_ = 0; 
  y_offset_ = 0;
  is_clicked_ = false;

}

// Cleans up the unit generator
Disc::~Disc(){
  delete ugen_;
}


// Creates a new orb to hang out around this disc
void Disc::orb_create(){
  Orb *roy_orbison = new Orb(&pos_, 2*r_);
  orbs_.push_back(roy_orbison);
  Graphics::add_drawable(roy_orbison);
  Physics::give_physics(roy_orbison);
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

// OpenGL instructions for drawing a unit disc centered at the origin
void Disc::draw(){
 glPushMatrix();
    glRotatef(90,1,0,0);//face up
    
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
void Disc::get_rotation(double &w, double &x, double &y, double &z){
  w=ang_pos_.length(); x=ang_pos_.x; y=ang_pos_.y; z=ang_pos_.z;
}

// Sets up the visual attributes for the Disc
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
}

// Pops the most recent set of attributes off of the stack
void Disc::remove_attributes(){
  glPopAttrib();
}

//We're just drawing cylinders, nothing needs to be done.
void Disc::prepare_graphics(void){
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
void Disc::unclicked(){ is_clicked_ = false;}


// The forces are handled here. This is called from Physics.cpp during the numerical integration step.
Vector3d Disc::external_forces(){
  //.5 * 2 * r * Cd * p * v^2 ~= r * v^2
  Vector3d drag = -vel_ * vel_.length() * .05 * r_;
  if (is_clicked_){
    Vector3d anchor(pos_.x + x_offset_, pos_.y + y_offset_, 0);
    //Drag, spring force and dampling
    return drag + (pull_point_-anchor)*75.0 - vel_*10; 
  }
  return drag;
}

// The torques are handled here. This is called from Physics.cpp during the numerical integration step.
Vector3d Disc::external_torques(){
  return Vector3d(0,0,0) * 180.0 / 3.1415926535;
}

