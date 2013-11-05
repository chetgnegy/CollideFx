/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  GraphicBox.cpp
  This file the object that interacts with OpenGL.
*/
#include <iostream>
#include <math.h>
#include "GraphicsBox.h"

void displayFunc();
void glInitialize();
void idleFunc();
void reshapeFunc(int width, int height);

std::list<Drawable *> *draw_list;

GraphicsBox::GraphicsBox(int w, int h){
  w_ = w;
  h_ = h;
  draw_list = &items_;
}

GraphicsBox::~GraphicsBox(){
  draw_list = NULL;
}

int GraphicsBox::initialize(int argc, char *argv[]){
  // initialize GLUT
  glutInit(&argc, argv);
  // initialize the window size
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);

  glutInitWindowSize(w_, h_);
  // set the window postion
  //glutInitWindowPosition(350, 350);
  // create the window
  glutCreateWindow("AudioHockeyTable");
  //glutEnterGameMode();

  // set the idle function - called when idle
  //glutIdleFunc(idleFunc);
  // set the display function - called when redrawing
  glutDisplayFunc(displayFunc);
  // set the reshape function - called when client area changes
  //glutReshapeFunc(reshapeFunc);
  // set the keyboard function - called on keyboard events
  //glutKeyboardFunc(keyboardFunc);
  // set the mouse function - called on mouse stuff
  //glutMouseFunc(mouseFunc);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  //glutSpecialFunc(specialFunc);
  
  return 0;
}

//Starts the main loop
void GraphicsBox::start_graphics(){
  glutMainLoop();
}

void GraphicsBox::add_drawable(Drawable *k){
  items_.push_back(k);

}
//Is the main loop. Runs repeatedly.
void displayFunc() {
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  glLoadIdentity();
  
  //Draws every drawable that is on the list
  std::list<Drawable *> items = *draw_list;
  if (items.size() > 0) {
    std::list<Drawable *>::iterator it;
    it = items.begin();
    //Process each effect in chain
    while (it != items.end()) {
      (*it)->draw();
      ++it;
    }
  }
  
  //Draws a test sphere
  glPushMatrix();
  glTranslatef(0,0,1.8);
  glColor4f(1.0, 0.0, 1.0, 1.0);
  glutSolidSphere(1, 20, 20);
  glPushMatrix();
  
  
  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();
}



void glInitialize() {
  // seed random number generator
  srand(time(NULL));
  // set fill mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Enable transparency
  glEnable (GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable (GL_COLOR_MATERIAL);
  glEnable (GL_LIGHT0);
  glEnable (GL_DEPTH_TEST);
  
}


void reshapeFunc(int x, int y) {
  // save the new window size
  if (y == 0 || x == 0)
    return;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(39.0, (GLdouble) x / (GLdouble) y, 0.6, 21.0);
  glMatrixMode (GL_MODELVIEW);
  glViewport(0, 0, x, y);
}


void idleFunc() {
  // render the scene
  glutPostRedisplay();
}

