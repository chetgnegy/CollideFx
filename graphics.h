/*
 * graphics.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: chetgnegy
 */
#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>

#else
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#endif

#include <iostream>
#include <math.h>
#include "graphicsutil.h"

using namespace std;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

// initial window size (in pixels)
GLsizei g_width = 800;
GLsizei g_height = 600;

//-----------------------------------------------------------------------------
// Defines a point in a 3D space (coords x, y and z)
//-----------------------------------------------------------------------------
struct pt3d {
  pt3d(GLfloat _x, GLfloat _y, GLfloat _z)
      : x(_x),
        y(_y),
        z(_z) {
  }
  ;

  float x;
  float y;
  float z;
};

// =======================
// = Function prototypes =
// =======================
void idleFunc();
void displayFunc();
void reshapeFunc(int width, int height);
void keyboardFunc(unsigned char, int, int);
void mouseFunc(int button, int state, int x, int y);
void specialFunc(int key, int x, int y);
void initialize();
void changeLookAt(pt3d look_from, pt3d look_to, pt3d head_up);

// Camera global variables
pt3d g_look_from(0, 0, 1);
pt3d g_look_to(0, 0, 0);
pt3d g_head_up(0, 1, 0);

// Entry point
int startGraphics(int argc, char *argv[]) {
  // initialize GLUT
  glutInit(&argc, argv);
  // initialize the window size
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);

  glutInitWindowSize(g_width, g_height);
  // set the window postion
  //glutInitWindowPosition(350, 350);
  // create the window
  glutCreateWindow("Smellovision");
  //glutEnterGameMode();

  // set the idle function - called when idle
  glutIdleFunc(idleFunc);
  // set the display function - called when redrawing
  glutDisplayFunc(displayFunc);
  // set the reshape function - called when client area changes
  glutReshapeFunc(reshapeFunc);
  // set the keyboard function - called on keyboard events
  glutKeyboardFunc(keyboardFunc);
  // set the mouse function - called on mouse stuff
  glutMouseFunc(mouseFunc);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  glutSpecialFunc(specialFunc);
  return 0;
}

// ============
// = GL Stuff =
// ============

//-----------------------------------------------------------------------------
// Name: initialize( )
// Desc: sets initial OpenGL states
//       also initializes any application data
//-----------------------------------------------------------------------------
void glInitialize() {
  // seed random number generator
  srand(time(NULL));
  // set the front faces of polygons
  // set fill mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Enable transparency
  glEnable (GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 0.0 };  //color of shiny part
  GLfloat mat_shininess[] = { 10.0 };  //shininess of the objects
  GLfloat light_position[] = { 2.0, 4.0, 3.0, 0.0 };  //moves the lighting
  glClearColor(0.0, 0.0, 0.0, 1.0);  //sets the background color
  glShadeModel (GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable (GL_COLOR_MATERIAL);
  glEnable (GL_LIGHT0);
  glEnable (GL_DEPTH_TEST);
}

//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc() {
  // render the scene
  glutPostRedisplay();
}


//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc() {
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  
  
  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();
}

//-----------------------------------------------------------------------------
// name: changeLookAt()
// desc: changes the "camera"
//-----------------------------------------------------------------------------
void changeLookAt(pt3d look_from, pt3d look_to, pt3d head_up) {
  gluLookAt(look_from.x, look_from.y, look_from.z, look_to.x, look_to.y,
            look_to.z, head_up.x, head_up.y, head_up.z);
}

//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc(unsigned char key, int x, int y) {
  switch (key) {
    case 'Q':
    case 'q':
      exit(1);
      break;
    case 'L':
    case 'l':
      g_look_from = pt3d(-1, 0, 0);
      cerr << "Looking from the left" << endl;
      break;
    case 'R':
    case 'r':
      g_look_from = pt3d(1, 0, 0);
      cerr << "Looking from the right" << endl;
      break;
    case 'F':
    case 'f':
      g_look_from = pt3d(0, 0, 1);
      cerr << "Looking from the front" << endl;
      break;
  }

  glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    // when left mouse button is down, move left
    if (state == GLUT_DOWN) {
    } else {
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

//-----------------------------------------------------------------------------
// Name: specialFunc( )
// Desc: handles special function keys
//-----------------------------------------------------------------------------
void specialFunc(int key, int x, int y) {
  if (key == GLUT_KEY_LEFT) {
    std::cerr << "Left arrow" << std::endl;
  }
  if (key == GLUT_KEY_RIGHT) {
    std::cerr << "Right arrow" << std::endl;
  }
  if (key == GLUT_KEY_DOWN) {
    std::cerr << "Down arrow" << std::endl;
  }
  if (key == GLUT_KEY_UP) {
    std::cerr << "Up arrow" << std::endl;
  }
  if (key == GLUT_KEY_PAGE_UP) {
    std::cerr << "PageUp arrow" << std::endl;
  }
  if (key == GLUT_KEY_PAGE_DOWN) {
    std::cerr << "PageDown arrow" << std::endl;
  }

  glutPostRedisplay();
}

