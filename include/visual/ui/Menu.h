/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.h
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/

#ifndef _MENU_H_
#define _MENU_H_

#include "math.h"

#include "Disc.h"
#include "Drawable.h"
#include "Moveable.h"
#include "RgbImage.h"
#include "UGenGraphBuilder.h"

#include <iostream> // delete

class Menu : public Drawable, public Moveable {
public:
  const float kXShift = -16;
  const float kScaleDimensions = 9.7;
  
  Menu();
  ~Menu();


  // Links the menu to the audio module
  void link_ugen_graph(UGenGraphBuilder *u);
  
  // Allows MIDI discs to be created
  void enable_midi();


  /* ----- Drawable ----- */


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

  void advance_time(double t);

  /* ----- Moveable ----- */


  // Menu doesn't need to move, but a disc might!
  void move(double x, double y, double z);

  // Prepares any discs that are being created for motion
  void prepare_move(double x, double y, double z);

  // Checks to see if the mouse is within the bounds of the menu
  bool check_clicked(double x, double y, double z);

  // Is called when the menu is no longer clicked. Often this needs delegated to the
  // disc that is being moved/created.
  void unclicked();

  // The menu doesn't do anything when right clicked.
  void right_clicked(){}

 
private:
  // Converts the coordinates (x,y) from screen coordinates to
  // image coordinates (a,b)
  void convert_coords(double x, double y, int &a, int &b);

  // Writes words to the screen. large will increase the size a bit
  void draw_text(const char * p, bool large);

  // Loads a texture from a file, must be in bmp format
  GLuint loadTextureFromFile( const char * filename );

  // Requires image coordinates. Finds out if the clicker is located
  // inside of a button.
  void handle_click(int x, int y);
  
  // Creates a new disc whenever a disc button is pressed.
  void make_disc(int button);




  // The textures for all of the menus.
  GLuint menu_texture_ctrl_;
  GLuint menu_texture_fft_;
  bool menu_texture_loaded_;
  bool ctrl_menu_shown_;
    // Properties of the Menu bitmap
  int menu_row_pixels_;
  int menu_col_pixels_;
  float height_to_width_;

  // The signal graph that we are linked with
  UGenGraphBuilder *graph_;

  // The newly created disc.
  bool valid_disc_;
  Disc *new_disc_;
  // Used to see if spotlight disc has changed from frame to frame
  Disc *last_disc_;
  
  // The parameter slider
  double slider1_, slider2_;
  bool slider1_clicked_, slider2_clicked_;
  bool slider_initial_; 
  bool show_slider_;

  // If Midi has been activated
  bool midi_active_;
  bool selector_enabled_;
};

#endif
