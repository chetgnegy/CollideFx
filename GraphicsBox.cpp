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
void mouse(int, int, int, int);
void mouseMotion(int, int);
void recoverClick(int, int, double &, double &);
void reshape(int width, int height);

std::list<Drawable *> *draw_list;
std::list<Moveable *> *move_list;
Moveable *clicked;
bool valid_clicked;

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
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  //glutSpecialFunc(special);
  return 0;
}

//Starts the main loop
void GraphicsBox::start_graphics(){ glutMainLoop(); }

void GraphicsBox::add_drawable(Drawable *k){ draw_items_.push_back(k); }

void GraphicsBox::add_moveable(Moveable *k){ move_items_.push_back(k); }

//Is the main loop. Runs repeatedly.
void display() {
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  glLoadIdentity();
  glPushMatrix();
  
  //The world scaling that is applied to everything.
  glTranslatef(0, 0, distance);
  glScalef(scale,scale,scale);
  
  double x,y,z;
      
  //Draws every drawable that is on the list
  std::list<Drawable *> items = *draw_list;
  if (items.size() > 0) {
    std::list<Drawable *>::iterator it;
    it = items.begin();
    //Process each effect in chain
    while (it != items.end()) {
      
      glPushMatrix();
      (*it)->get_origin(x,y,z);
      glTranslatef(x,y,z);
      (*it)->draw();
      glPopMatrix();
      
      ++it;
    }
  }
  
  glPopMatrix();
  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();
}




void mouse(int button, int state, int x, int y) {
    double coordX, coordY;
    recoverClick(x,y, coordX, coordY);

    if (button == GLUT_LEFT_BUTTON) {
    // when left mouse button is down, move left

      if (state == GLUT_DOWN) {
        //Draws every drawable that is on the list
        std::list<Moveable *> items = *move_list;
        if (items.size() > 0) {
          std::list<Moveable *>::iterator it;
          it = items.begin();
          //Process each effect in chain
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

  // Vector3f rayPoint1((float)posX1, (float)posY1, (float)posZ1);
  // Vector3f rayPoint2((float)posX2, (float)posY2, (float)posZ2);

  // This is a little bit hacky. The top of the discs is .5 above the board. 
  //This offset keeps the mouse in the plane of the top of the disc.
  GLfloat t = (posZ1 - distance - 0.5) / (posZ1 - posZ2);

  // so here are the desired (x, y) coordinates
  oX = (posX1 + (posX2 - posX1) * t) / scale;
  oY = (posY1 + (posY2 - posY1) * t) / scale;
}




void glInitialize() {
  // seed random number generator
  srand(time(NULL));

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_ambient[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat mat_shininess[] = { 10.0 };
  GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_SMOOTH);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);

  glEnable (GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
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