/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  GraphicBox.cpp
  This file the object that interacts with OpenGL.
*/
#include <iostream>
#include "GraphicsBox.h"
#include "Physics.h"
#include <sys/time.h>
#include <time.h>


void display();
void glInitialize();
void idle();
void mouse(int, int, int, int);
void mouseMotion(int, int);
void recoverClick(int, int, double &, double &);
void reshape(int width, int height);

std::list<Drawable *> *draw_list;
std::list<Moveable *> *move_list;
Moveable *clicked;
bool valid_clicked;
long time_now;
struct timeval timer; 
float distance = -20.0;
float scale = .45;

GraphicsBox::GraphicsBox(int w, int h){
  w_ = w;
  h_ = h;
  draw_list = &draw_items_;
  move_list = &move_items_;
}

GraphicsBox::~GraphicsBox(){
  draw_list = NULL;
}

int GraphicsBox::initialize(int argc, char *argv[]){
  // initialize GLUT
  glutInit(&argc, argv);
  // initialize the window size
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  glutInitWindowSize(w_, h_);
  // set the window postion
  //glutInitWindowPosition(350, 350);
  // create the window
  glutCreateWindow("AudioHockeyTable");
  //glutEnterGameMode();

  glInitialize();
  
  // set the idle function - called when idle
  glutIdleFunc(idle);
  // set the display function - called when redrawing
  glutDisplayFunc(display);
  // set the reshape function - called when client area changes
  glutReshapeFunc(reshape);
  // set the keyboard function - called on keyboard events
  //glutKeyboardFunc(keyboard);
  // set the mouse function - called on mouse stuff
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  //glutSpecialFunc(special);


  time_now = -1;
  return 0;
}

//Starts the main loop
void GraphicsBox::start_graphics(){ glutMainLoop(); }

void GraphicsBox::add_drawable(Drawable *k){ 
  k->prepare_graphics();
  draw_items_.push_back(k); 
}

void GraphicsBox::add_moveable(Moveable *k){ move_items_.push_back(k); }

//Is the main loop. Runs repeatedly.
void display() {
  usleep(1000);

  gettimeofday(&timer, NULL);  
  long new_time = (long)(timer.tv_sec*1000000+timer.tv_usec);
  long time_diff =  new_time - time_now;
  if (time_now > 0){
    Physics::update(time_diff*1.0e-6);
  }
  
  //char input;
  //std::cin.get(input);
  
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  glLoadIdentity();
  glPushMatrix();
  
  //The world scaling that is applied to everything.
  glTranslatef(0, 0, distance);
  glScalef(scale,scale,scale);
  
  double w,x,y,z;
  

  //Draws every drawable that is on the list
  std::list<Drawable *> items = *draw_list;
  if (items.size() > 0) {
    std::list<Drawable *>::iterator it;
    it = items.begin();
    //Process each effect in chain
    while (it != items.end()) {
      
      glPushMatrix();
      if (time_now > 0){
        (*it)->advance_time(time_diff*1.0e-6);
      }
      (*it)->set_attributes();
      (*it)->get_origin(x,y,z);
      glTranslatef(x,y,z);
      (*it)->get_rotation(w,x,y,z);
      glRotatef(w,x,y,z);
      (*it)->draw();
      (*it)->remove_attributes();
      glPopMatrix();
      
      ++it;
    }
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
        std::list<Moveable *> items = *move_list;
        if (items.size() > 0) {
          std::list<Moveable *>::iterator it;
          it = items.begin();
          while (it != items.end()) {
            if ((*it)->check_clicked(coordX, coordY, -distance)){
                valid_clicked = true;
                clicked = *it;
                clicked->prepare_move(coordX, coordY, -distance);
            }
            ++it;
          }
        }
      } else {
        if (valid_clicked) clicked->unclicked();
        valid_clicked = false;
        clicked = 0;
      }


    } else if (button == GLUT_RIGHT_BUTTON) {
      // when right mouse button down, move right
      if (state == GLUT_DOWN) {
      } else {
      }
    } else {
    }

  glutPostRedisplay();
}

void mouseMotion(int x, int y){
    double oX, oY;
    recoverClick(x,y, oX, oY); 
    if (valid_clicked){
      clicked->move(oX, oY, -distance);
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
  GLfloat t = (posZ1 - distance - 0.5) / (posZ1 - posZ2);

  // so here are the desired (x, y) coordinates
  oX = (posX1 + (posX2 - posX1) * t) / scale;
  oY = (posY1 + (posY2 - posY1) * t) / scale;
}




void glInitialize() {
  /*GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glShadeModel (GL_SMOOTH);
  */

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