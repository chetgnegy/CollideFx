/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.h
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/

#ifndef _MENU_H_
#define _MENU_H_

#import "RgbImage.h"
#import "Drawable.h"
#import "Moveable.h"
#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif


class Menu : public Drawable, public Moveable {
public:
  static const float kXShift = -16;
  static const float kScaleDimensions = 9.7;
    
  Menu();
  ~Menu();

  // Draws the currently showing menu
  void draw();

  // Shifts the menu into the left side of the screen
   void get_origin(double &x, double &y, double &z);

  // The menu does not rotate. This always returns zero
  void get_rotation(double &w, double &x, double &y, double &z);

  //Sets the current texture and some special parameters for 2D drawing
  void set_attributes(void);

  // Pops the attriubtes stack
  void remove_attributes(void);

  // Loads the graphics, but only on the very first time
  void prepare_graphics(void);

  void move(double x, double y, double z);
  void prepare_move(double x, double y, double z);
  bool check_clicked(double x, double y, double z);
  void unclicked();
private:
  GLuint loadTextureFromFile( const char * filename );

  GLuint menu_texture_ctrl_;
  GLuint menu_texture_fft_;
  bool menu_texture_loaded_;
  bool ctrl_menu_shown_;

};

#endif