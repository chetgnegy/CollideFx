/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.h
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/

#ifndef _MENU_H_
#define _MENU_H_

#include "RgbImage.h"
#include "Drawable.h"
#include "Moveable.h"
#include <iostream> // delete


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
  void convert_coords(double x, double y, int &a, int &b);
  GLuint loadTextureFromFile( const char * filename );
  void handle_click(int x, int y);
  
  GLuint menu_texture_ctrl_;
  GLuint menu_texture_fft_;
  bool menu_texture_loaded_;
  bool ctrl_menu_shown_;

  // Properties of the Menu bitmap
  int menu_row_pixels_;
  int menu_col_pixels_;
  float height_to_width_;

};

#endif