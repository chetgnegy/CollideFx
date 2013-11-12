/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.cpp
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/

#include "math.h"
#include "Menu.h"

Menu::Menu(){
  menu_texture_loaded_ = false;
  ctrl_menu_shown_ = true;
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
  glDepthMask(GL_FALSE);  // disable writes to Z-Buffer
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

void Menu::move(double x, double y, double z){}
void Menu::prepare_move(double x, double y, double z){}

bool Menu::check_clicked(double x, double y, double z){
  //Check bounds of in
  if (abs(x - kXShift) < kScaleDimensions){
    if (y < kScaleDimensions * height_to_width_){
      int a, b;
      convert_coords(x, y, a, b);
      handle_click(a, b);
      return true;
    }
  }

  
}
void Menu::unclicked(){}


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

void Menu::handle_click(int x, int y){
  // Coords for full size 942x1447 image buttons
  int but_size = 147, sm_but_size= 68;
  int h_but_space = 24;
  int y_but[3] = {180, 495, 671};
  int x_but = 53;
  int y_up_but = 914, y_down_but = 994;
  int x_arrow_but = 579;
  int x_pane_but = 823;
  int y_trash = 1341;

  if (y < y_but[2] + but_size){//Upper button
    //First row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x, y, x_but + i*(but_size+h_but_space), y_but[0], but_size)){
        switch (i){
          case 0: std::cout << "Clicked Input" << std::endl; return;
          case 1: std::cout << "Clicked Sine" << std::endl; return;
          case 2: std::cout << "Clicked Square" << std::endl; return;
          case 3: std::cout << "Clicked Tri" << std::endl; return;
          case 4: std::cout << "Clicked Saw" << std::endl; return;
        }
      }
    }
    //Second row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x, y, x_but + i*(but_size+h_but_space), y_but[1], but_size)){
        switch (i){
          case 0: std::cout << "Clicked BitCrusher" << std::endl; return;
          case 1: std::cout << "Clicked Chorus" << std::endl; return;
          case 2: std::cout << "Clicked Delay" << std::endl; return;
          case 3: std::cout << "Clicked Distortion" << std::endl; return;
          case 4: std::cout << "Clicked Filter" << std::endl; return;
        }
      }
    }
    //Third row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x, y, x_but + i*(but_size+h_but_space), y_but[2], but_size)){
        switch (i){
          case 0: std::cout << "Clicked BandPass" << std::endl; return;
          case 1: std::cout << "Clicked Looper" << std::endl; return;
          case 2: std::cout << "Clicked RingMod" << std::endl; return;
          case 3: std::cout << "Clicked Reverb" << std::endl; return;
          case 4: std::cout << "Clicked Tremolo" << std::endl; return;
        }
      }
    }
  }
  else {
    if (inSquare(x, y, x_arrow_but, y_up_but, sm_but_size)){
      std::cout << "Clicked Up" << std::endl; return;
    }
    if (inSquare(x, y, x_arrow_but, y_down_but, sm_but_size)){
      std::cout << "Clicked Down" << std::endl; return;
    }
    if (inSquare(x, y, x_pane_but, y_up_but, sm_but_size)){
      std::cout << "Clicked CTRL" << std::endl; ctrl_menu_shown_ = true; return;
    }
    if (inSquare(x, y, x_pane_but, y_down_but, sm_but_size)){
      std::cout << "Clicked FFT" << std::endl; ctrl_menu_shown_ = false; return;
    }
    if (inSquare(x, y, x_pane_but, y_trash, sm_but_size)){
      std::cout << "Clicked Trash" << std::endl; return;
    }
  }

}

