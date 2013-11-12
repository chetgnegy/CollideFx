/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.cpp
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/

#import "Menu.h"
#import <iostream>



GLuint loadTextureFromFile( const char * filename );

Menu::Menu(){
    menu_texture_loaded_ = false;
    ctrl_menu_shown_ = true;
}

Menu::~Menu(){}


// Draws the currently showing menu
void Menu::draw(){
    glBegin(GL_QUADS);
    float width = kScaleDimensions;
    float height = kScaleDimensions*7.42/4.71;
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
bool Menu::check_clicked(double x, double y, double z){}
void Menu::unclicked(){}

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
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
    theTexMap.GetNumCols(), theTexMap.GetNumRows(),
    GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

    return texture;
}