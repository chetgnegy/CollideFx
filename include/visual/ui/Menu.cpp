/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.cpp
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/


#include "Menu.h"

Menu::Menu(){
  menu_texture_loaded_ = false;
  ctrl_menu_shown_ = true;
  valid_disc_ = false;
  new_disc_ = NULL;
}

Menu::~Menu(){}


// Draws the currently showing menu
void Menu::draw(){
  glBegin(GL_QUADS);
  float width = kScaleDimensions;
  float height = kScaleDimensions * height_to_width_;
  glTexCoord2f(0.0, 1.0); glVertex3f(-width, height, 0);
  glTexCoord2f(1.0, 1.0); glVertex3f(width, height, 0);
  glTexCoord2f(1.0, 0.0); glVertex3f(width, -height, 0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-width, -height, 0);
  glEnd();
}

// Shifts the menu into the left side of the screen
void Menu::get_origin(double &x, double &y, double &z){
  x=kXShift; y=0; z=0;
}

// The menu does not rotate. This always returns zero
void Menu::get_rotation(double &w, double &x, double &y, double &z){
  w=0; x=0; y=0; z=0;
}

//Sets the current texture and some special parameters for 2D drawing
void Menu::set_attributes(void){
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable(GL_BLEND);
  glShadeModel(GL_FLAT);
  glDepthMask(GL_TRUE);  // disable writes to Z-Buffer
  glDisable(GL_DEPTH_TEST);  // disable depth-testing
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  if (ctrl_menu_shown_){
    glBindTexture(GL_TEXTURE_2D, menu_texture_ctrl_);
  }
  else{
    glBindTexture(GL_TEXTURE_2D, menu_texture_fft_);
  }
}

// Pops the attriubtes stack
void Menu::remove_attributes(void){
  glPopAttrib();
}

// Loads the graphics, but only on the very first time
void Menu::prepare_graphics(void){
  if (!menu_texture_loaded_){
    menu_texture_ctrl_ = loadTextureFromFile( "graphics/menu_ctrl.bmp" );
    menu_texture_fft_ = loadTextureFromFile( "graphics/menu_fft.bmp" );
    menu_texture_loaded_ = true;
  }
}

void Menu::move(double x, double y, double z){
  if (valid_disc_){
    new_disc_->move(x,y,z);
  }
}

void Menu::prepare_move(double x, double y, double z){
  int a, b;
  convert_coords(x, y, a, b);
  // Sees which button was clicked
  handle_click(a, b);
  if (valid_disc_){
    // Delegates movement to the new disk
    new_disc_->set_location(x,y);
    new_disc_->prepare_move(x,y,z);
  }
}

bool Menu::check_clicked(double x, double y, double z){
  //Check bounds of in
  if (abs(x - kXShift) < kScaleDimensions){
    if (y < kScaleDimensions * height_to_width_){
      return true;
    }
  }
  return false;
}

void Menu::unclicked(){
  if (valid_disc_){
  
    if(Physics::is_clear_area(new_disc_->pos_.x,new_disc_->pos_.y, new_disc_->get_radius()))
    { // unclicks the disc, a new disc is finalized!
      new_disc_->unclicked();
    }
    else{
      Graphics::remove_drawable(new_disc_);
      Graphics::remove_moveable(new_disc_);
      Physics::take_physics(new_disc_);
    }
    
  }
  valid_disc_ = false;
  new_disc_ = 0;
}


void Menu::convert_coords(double x, double y, int &a, int &b){
  double norm_x = 0.5 * (x - kXShift + kScaleDimensions)/kScaleDimensions;
  double norm_y = 0.5 * (1 - y/kScaleDimensions/height_to_width_);
  a = round(norm_x * menu_col_pixels_);
  b = round(norm_y * menu_row_pixels_);
}


// Loads a texture from a file, must be in bmp format
GLuint Menu::loadTextureFromFile( const char * filename ){
  GLuint texture;
  glGenTextures( 1, &texture );
  glBindTexture(GL_TEXTURE_2D, texture);

  RgbImage theTexMap( filename ); // instantiation

  // Set the interpolation settings to best quality.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  menu_row_pixels_ = theTexMap.GetNumRows();
  menu_col_pixels_ = theTexMap.GetNumCols();
  height_to_width_ = menu_row_pixels_/(1.0*menu_col_pixels_);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
  theTexMap.GetNumCols(), theTexMap.GetNumRows(),
  GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

  return texture;
}


bool inSquare(int x, int y, int a, int b, int w){
  //  A---B   Tell if input (x,y) is in square ABCD
  //  |   |   A = (a,b)   B = (a+w,b)    
  //  C---D   C = (a,b+w) D = (a+w,b+w)
  if (x < a || x > a+w) return false;
  if (y < b || y > b+w) return false;
  return true; 
}

// Requires image coordinates. Handles all button clicks
void Menu::handle_click(int x, int y){
  // Coords for full size 942x1447 image buttons
  int but_size = 147, sm_but_size= 68;
  int h_but_space = 23;
  int y_but[3] = {180, 495, 671};
  int x_but = 53;
  int y_up_but = 914, y_down_but = 994;
  int x_arrow_but = 579;
  int x_pane_but = 823;
  int y_trash = 1341;

  if (y < y_but[2] + but_size){//Upper button
    //First row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x -32, y - 21, x_but + i*(but_size+h_but_space), y_but[0], but_size)){
        switch (i){
          case 0: make_disc(100); return; // Input
          case 1: make_disc(101); return; // Sine
          case 2: make_disc(102); return; // Square
          case 3: make_disc(103); return; // Tri
          case 4: make_disc(104); return; // Saw
        }
      }
    }
    //Second row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x - 32, y - 21, x_but + i*(but_size+h_but_space), y_but[1], but_size)){
        switch (i){
          case 0: make_disc(200); return; // BitCrusher
          case 1: make_disc(201); return; // Chorus
          case 2: make_disc(202); return; // Delay
          case 3: make_disc(203); return; // Distortion
          case 4: make_disc(204); return; // Filter
        }
      }
    }
    //Third row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x - 32, y -21, x_but + i*(but_size+h_but_space), y_but[2], but_size)){
        switch (i){
          case 0: make_disc(205); return; // Bandpass
          case 1: make_disc(206); return; // Looper
          case 2: make_disc(207); return; // RingMod
          case 3: make_disc(208); return; // Reverb
          case 4: make_disc(209); return; // Tremolo
        }
      }
    }
  }
  else {
    if (inSquare(x - 16, y, x_arrow_but, y_up_but, sm_but_size)){
      std::cout << "Clicked Up" << std::endl; return;
    }
    if (inSquare(x - 16, y, x_arrow_but, y_down_but, sm_but_size)){
      std::cout << "Clicked Down" << std::endl; return;
    }
    if (inSquare(x - 16, y, x_pane_but, y_up_but, sm_but_size)){
      std::cout << "Clicked CTRL" << std::endl; ctrl_menu_shown_ = true; return;
    }
    if (inSquare(x - 16, y, x_pane_but, y_down_but, sm_but_size)){
      std::cout << "Clicked FFT" << std::endl; ctrl_menu_shown_ = false; return;
    }
    if (inSquare(x - 16, y, x_pane_but, y_trash, sm_but_size)){
      std::cout << "Clicked Trash" << std::endl; return;
    }
  }

}


void Menu::make_disc(int button){
  double rad = 1.15;
  switch (button){
    // Input
    case 100: 
      new_disc_ = new Disc(NULL, rad, true, 200, 50);
      new_disc_->set_color(0.5, 0.5, 0.5);
      new_disc_->set_texture(0);
      break; 

    // Sine 
    case 101: 
      new_disc_ = new Disc(NULL, rad, true, 200, 50);
      new_disc_->set_color(0.9, 0.9, 0.3);
      new_disc_->set_texture(1);
      break;

    // Square
    case 102: 
      new_disc_ = new Disc(NULL, rad, true, 200, 50);
      new_disc_->set_color(0.3, 0.9, 0.9);
      new_disc_->set_texture(2);
      break;

    // Tri
    case 103: 
      new_disc_ = new Disc(NULL, rad, true, 200, 50);
      new_disc_->set_color(0.9, 0.9, 0.3);
      new_disc_->set_texture(3);
      break; 

    // Saw
    case 104: 
      new_disc_ = new Disc(NULL, rad, true, 200, 50);
      new_disc_->set_color(0.9, 0.3, 0.9);
      new_disc_->set_texture(4);
      break;

    // BitCrusher
    case 200: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.3, 0.9, 0.3);
      new_disc_->set_texture(5);
      break; 
    
    // Chorus
    case 201: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.3, 0.6, 0.9);
      new_disc_->set_texture(6);
      break; 

    // Delay
    case 202: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.7, 0.7, 0.3);
      new_disc_->set_texture(7);
      break; 

    // Distortion
    case 203: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.9, 0.6, 0.3);
      new_disc_->set_texture(8);
      break; 

    // Filter
    case 204:   
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.7, 0.7, 0.7);
      new_disc_->set_texture(9);
      break; 

    // Bandpass
    case 205: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.5, 0.5, 0.5);
      new_disc_->set_texture(10);
      break;

    // Looper 
    case 206: 
      new_disc_ = new Disc(NULL, rad, true, 50, 50);
      new_disc_->set_color(0.9, 0.0, 0.0);
      new_disc_->set_texture(11);
      break;

    // RingMod 
    case 207: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.9, 0.0, 0.7);
      new_disc_->set_texture(12);
      break; 

    // Reverb
    case 208: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.7, 0.0, 0.9);
      new_disc_->set_texture(13);
      break; 

    // Tremolo
    case 209: 
      new_disc_ = new Disc(NULL, rad, true);
      new_disc_->set_color(0.0, 0.6, 0.6);
      new_disc_->set_texture(14);
      break; 
  }
  
  valid_disc_ = true;
  // Add the disc to the front of the list. It will speed deletion times
  // Chances are, it will be deleted anyhow.
  Graphics::add_drawable(new_disc_, 10);
  Graphics::add_moveable(new_disc_);
  Physics::give_physics(new_disc_);
}

