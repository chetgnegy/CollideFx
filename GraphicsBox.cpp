/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  GraphicBox.cpp
  This file the object that interacts with OpenGL.
*/
#include <iostream>
#include "GraphicsBox.h"

void display();
void glInitialize();
void idle();
void reshape(int width, int height);

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
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

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
  //glutMouseFunc(mouse);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  //glutSpecialFunc(special);
  
  
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
void display() {
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  glLoadIdentity();
  glPushMatrix();
  
  glTranslatef(0, 0, -20);
  glScalef(.45,.45,.45);
  
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
  
  glPopMatrix();
  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();
}



void glInitialize() {
  // seed random number generator
  srand(time(NULL));

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_ambient[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat mat_shininess[] = { 50.0 };
  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_SMOOTH);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  
  //glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  
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

