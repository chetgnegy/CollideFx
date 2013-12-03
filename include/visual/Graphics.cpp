/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  GraphicBox.cpp
  This file the object that interacts with OpenGL.
*/

#include "Graphics.h"

void display();
void glInitialize();
void idle();
void mouse(int, int, int, int);
void mouseMotion(int, int);
void recoverClick(int, int, double &, double &);
void keyboard(unsigned char key, int, int);
void reshape(int width, int height);

std::list<Drawable *> Graphics::draw_list_;
std::list<int> Graphics::draw_priority_;
std::list<Moveable *> Graphics::move_list_;

Moveable *clicked;
bool valid_clicked;
long time_now;
struct timeval timer; 
float z_distance = -20.0;
float scale = .45;
bool fullscreen = false;

GLuint Graphics::splash_ = 300;
bool Graphics::show_splash_ = false;
bool Graphics::splash_loaded_ = false;


Graphics::Graphics(int w, int h){
  w_ = w;
  h_ = h;
  GLuint splash_;
  bool show_splash_ = false;
  bool splash_loaded_ = false;

}

Graphics::~Graphics(){}

int Graphics::initialize(int argc, char *argv[]){
  // initialize GLUT
  glutInit(&argc, argv);
  // initialize the window size
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  glutInitWindowSize(w_, h_);
  // set the window postion
  //glutInitWindowPosition(350, 350);
  // create the window
  glutCreateWindow("CollideFx");
  //glutEnterGameMode();

  glInitialize();
    
  // set the idle function - called when idle
  glutIdleFunc(idle);
  // set the display function - called when redrawing
  glutDisplayFunc(display);
  // set the reshape function - called when client area changes
  glutReshapeFunc(reshape);
  // set the mouse function - called on mouse stuff
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutKeyboardFunc(keyboard);
  time_now = -1;


  return 0;
}

//Starts the main loop
void Graphics::start_graphics(){ glutMainLoop(); }


void Graphics::add_drawable(Drawable *k, int priority){ 
  k->prepare_graphics();
  if (priority < 999999){

    std::list<Drawable *>::iterator it;
    std::list<int>::iterator pri_it;
    it = Graphics::draw_list_.begin();
    pri_it = Graphics::draw_priority_.begin();
    while (it != Graphics::draw_list_.end()) {
      if (*pri_it >= priority){
        Graphics::draw_list_.insert(it, k);
        Graphics::draw_priority_.insert(pri_it, priority); 
        return;
      }
      ++it; ++pri_it;
    }
  }
  Graphics::draw_list_.push_back(k); 
  Graphics::draw_priority_.push_back(priority); 
  
}

// Removes an item from the draw list
bool Graphics::remove_drawable(Drawable *k){ 
  if (Graphics::draw_list_.size() > 0) {
    std::list<Drawable *>::iterator it;
    std::list<int>::iterator pri_it;
    it = Graphics::draw_list_.begin();
    pri_it = Graphics::draw_priority_.begin();
    
    while (it != Graphics::draw_list_.end()) {
      if (*it == k){
        Graphics::draw_list_.erase(it);
        Graphics::draw_priority_.erase(pri_it);
        return true;
      }
      ++it; ++pri_it;
    }
  }
  return false;
}


void Graphics::add_moveable(Moveable *k){ move_list_.push_back(k); }

// Removes an item from the move list
bool Graphics::remove_moveable(Moveable *k){ 
  if (Graphics::move_list_.size() > 0) {
    std::list<Moveable *>::iterator it;
    it = Graphics::move_list_.begin();
    while (it != Graphics::move_list_.end()) {
      if (*it == k){
        Graphics::move_list_.erase(it);
        return true;
      }
      ++it;
    }
  }
  return false;
}


 void Graphics::show_splash_screen(){
    if (!splash_loaded_) {
      glGenTextures( 1, &splash_ );
      glBindTexture(GL_TEXTURE_2D, splash_);

      RgbImage theTexMap( "graphics/splash.bmp" ); // instantiation

      // Set the interpolation settings to best quality.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
      theTexMap.GetNumCols(), theTexMap.GetNumRows(),
      GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );
      splash_loaded_ = true;
    }
    show_splash_ = true;
 }
// #----------GLUT CODE -----------------#



//Is the main loop. Runs repeatedly.
void display() {
  usleep(2000);

  gettimeofday(&timer, NULL);  
  long new_time = (long)(timer.tv_sec*1000000+timer.tv_usec);
  long time_diff =  new_time - time_now;
  if (time_now > 0){
    Physics::update(time_diff*1.0e-6);
  }
  
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  glLoadIdentity();
  glPushMatrix();
  
  if (fullscreen) scale = .43;
  else scale = .45;

  //The world scaling that is applied to everything.
  glTranslatef(0, 0, z_distance);
  glScalef(scale,scale,scale);
  
  double w,x,y,z;
  //Draws every drawable that is on the list
  if (Graphics::draw_list_.size() > 0) {
    std::list<Drawable *>::iterator it;
    std::list<Drawable *>::iterator backup;
    it = Graphics::draw_list_.begin();
    //Process each effect in chain
    int count = 0;
    while (it != Graphics::draw_list_.end()) {
      if (time_now > 0){
        (*it)->advance_time(time_diff*1.0e-6);
      }
      glPushMatrix();
      (*it)->set_attributes();
      (*it)->get_origin(x,y,z);
      glTranslatef(x,y,z);
      (*it)->get_rotation(w,x,y,z);
      glRotatef(w,x,y,z);
      (*it)->draw();
      (*it)->remove_attributes();
      // Handles the case where clean_up removes 
      // the instance from the list
      backup = it++;
      (*backup)->clean_up();
      glPopMatrix();
    }
  }
  
  if (Graphics::show_splash_){
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glColor4f(0,0,0,.7);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-29, 19, 0);
    glTexCoord2f(1.0, 1.0); glVertex3f(29, 19, 0);
    glTexCoord2f(1.0, 0.0); glVertex3f(29, -19, 0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-29, -19, 0);
    glEnd();
    glPopAttrib();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);  // disable depth-testing
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, Graphics::splash_);
    
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-26, 16.5, 0);
    glTexCoord2f(1.0, 1.0); glVertex3f(26, 16.5, 0);
    glTexCoord2f(1.0, 0.0); glVertex3f(26, -15.5, 0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-26, -15.5, 0);
    glEnd();
    glPopAttrib();

  }
  glPopMatrix();

  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();

  time_now = new_time;
  
}




void mouse(int button, int state, int x, int y) {
    double coordX, coordY;
    recoverClick(x,y, coordX, coordY);

    if (button == GLUT_LEFT_BUTTON) {
    // when left mouse button is down, move left

      if (state == GLUT_DOWN) {
        Graphics::show_splash_ = false;
        //glDeleteTextures(1, &Graphics::splash_);
        //Graphics::splash_loaded_ = false;

        if (Graphics::move_list_.size() > 0) {
          std::list<Moveable *>::iterator it;
          it = Graphics::move_list_.begin();
          while (it != Graphics::move_list_.end()) {
            if ((*it)->check_clicked(coordX, coordY, -z_distance)){
                // Found the right object under the cursor
                valid_clicked = true;
                clicked = *it;
                clicked->prepare_move(coordX, coordY, -z_distance);
                clicked->move(coordX, coordY, -z_distance);
                break;
            } ++it;
          }
        }
      } else {
        // Object is no longer clicked
        if (valid_clicked) clicked->unclicked();
        valid_clicked = false;
        clicked = NULL;
      }


    } else if (button == GLUT_RIGHT_BUTTON) {
      // when right mouse button down, move right
      if (state == GLUT_DOWN) {
        if (Graphics::move_list_.size() > 0) {
          std::list<Moveable *>::iterator it;
          it = Graphics::move_list_.begin();
          while (it != Graphics::move_list_.end()) {
            if ((*it)->check_clicked(coordX, coordY, -z_distance)){
                (*it)->right_clicked();
                break;
            } ++it;
          }
        }
      }
    } else {
    }

  glutPostRedisplay();
}

void mouseMotion(int x, int y){
    double oX, oY;
    recoverClick(x,y, oX, oY); 
    if (valid_clicked){
      clicked->move(oX, oY, -z_distance);
    }
    glutPostRedisplay();
}

void recoverClick(int iX, int iY, double &oX, double &oY){
  // http://www.3dbuzz.com/forum/threads/191296-OpenGL-gluUnProject-ScreenCoords-to-WorldCoords-problem
  GLdouble posX1, posY1, posZ1, posX2, posY2, posZ2, modelview[16], projection[16];
  GLint viewport[4];

  // Get matrices
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);
 
  // Create ray
  gluUnProject(iX, viewport[1] + viewport[3] - iY, 0, modelview, projection, viewport, &posX1, &posY1, &posZ1);  // Near plane
  gluUnProject(iX, viewport[1] + viewport[3] - iY, 1, modelview, projection, viewport, &posX2, &posY2, &posZ2);  // Far plane

  // This is a little bit hacky. The top of the discs is .5 above the board. 
  //This offset keeps the mouse in the plane of the top of the disc.
  GLfloat t = (posZ1 - z_distance - 0.5) / (posZ1 - posZ2);

  // so here are the desired (x, y) coordinates
  oX = (posX1 + (posX2 - posX1) * t) / scale;
  oY = (posY1 + (posY2 - posY1) * t) / scale;
}


void keyboard(unsigned char key, int x, int y){
  switch (key){
    case ('x'):
      exit(0);
    break;
    case ('f'):
      if (!fullscreen){
        glutFullScreen();
        fullscreen = true;
      }
      else {
        glutReshapeWindow(1100, 600);
        glutPositionWindow(0,0);
        fullscreen = false;
      }
    break;
  }
}

void glInitialize() {
  GLfloat light_position[] = { -2.0, 1.0, 0.8, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);

  glShadeModel (GL_SMOOTH);
  
}


void reshape(int w, int h) {
  if (h == 0 || w == 0)
    return;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(39.0, (GLdouble) w / (GLdouble) h, 0.6, 40.0);
  glMatrixMode (GL_MODELVIEW);
  glViewport(0, 0, w, h);
}


void idle() {
  // render the scene
  glutPostRedisplay();
}
