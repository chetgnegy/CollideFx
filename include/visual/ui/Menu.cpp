/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Menu.cpp
  This is a menu for the OpenGL window. It is going to handle the creation of all discs, the linking of discs and unit generators, 
  the changing of parameters, and a specral display.
*/



#include "Menu.h"

// Converts a double from 0 to 1 to the visible light spectrum
void spectrum(double w, double &R, double &G, double &B);
// Tests to see if some coordinates are inside of a square
bool inSquare(int x, int y, int a, int b, int w);

Menu::Menu(){
  GLuint menu_texture_ctrl_ = 0;
  GLuint menu_texture_fft_ = 0;
  menu_texture_loaded_ = false;
  ctrl_menu_shown_ = true;
  menu_row_pixels_ = 0;
  menu_col_pixels_ = 0;
  height_to_width_ = 0;

  // We have to link a graph
  graph_ = NULL;

  // Candidate discs
  valid_disc_ = false;
  new_disc_ = NULL;
  last_disc_ = NULL; 

  // The parameter slider
  slider1_clicked_ = false;
  slider2_clicked_ = false;
  slider_initial_ = false;
  slider1_ = 0;
  slider2_ = 0;
  midi_active_ = false;
  selector_enabled_ = false;
}

Menu::~Menu(){}

// Allows MIDI disks to be made
void Menu::enable_midi(){ midi_active_ = true; }

// Links the menu to the audio module
void Menu::link_ugen_graph(UGenGraphBuilder *gb){ graph_= gb; }


// #-------------- Drawable ----------------#


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
  glPopAttrib();
  

  // Grey Midi if not enabled
  if (!midi_active_){
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
    glColor4f(0,0,0,.7);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.55*width, 0.75*height, 0);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.9*width, 0.75*height, 0);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.9*width, 0.53*height, 0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.55*width, 0.53*height, 0);
    glEnd();
    glPopAttrib();
  }

  // Grey arrow boxes if not in use
  selector_enabled_ = graph_->selector_activated();
  if (!selector_enabled_ && ctrl_menu_shown_){
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
    glColor4f(0,0,0,.7);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(0.17*width, -0.5*height, 0);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.73*width, -0.5*height, 0);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.73*width, -0.25*height, 0);
    glTexCoord2f(0.0, 0.0); glVertex3f(0.17*width, -0.25*height, 0);
    glEnd();
    glPopAttrib();
  }
  glTranslatef(0,-9,0);
    
    // Disc clicked, show slider
    if (Disc::spotlight_disc_ != NULL){
      // slider positions
      slider1_ = Disc::spotlight_disc_->get_ugen_params(1);
      slider2_ = Disc::spotlight_disc_->get_ugen_params(2);
      
      double text_x = -7.125;
      if (ctrl_menu_shown_){
        glPushMatrix();
          glTranslatef(text_x, 1.5, 0);
          draw_text(Disc::spotlight_disc_->get_ugen()->name(), true);
          glTranslatef(11.8, 1.8, 0);
          draw_text(graph_->text_box_content(), true);
          glTranslatef(-2.5, -2, 0);
          draw_text(graph_->text_box_label(), false);
          glPopMatrix();
      }
      
      glPushMatrix();
        // First Slider
        glPushMatrix();
          glTranslatef(text_x, -1.25, 0);
          draw_text(Disc::spotlight_disc_->get_ugen()->p_name(1), false);
          glTranslatef(9, 0, 0);
          draw_text(Disc::spotlight_disc_->get_ugen()->report_param(1), false);
          glPopMatrix();
        glTranslatef(-7 + slider1_ * 13.5, 0, 0);
        glutSolidSphere(.4,20,20);
        glPopMatrix();
      glPushMatrix();
        // Second Slider
        glPushMatrix();
          glTranslatef(text_x, -5, 0);
          draw_text(Disc::spotlight_disc_->get_ugen()->p_name(2), false);
          glTranslatef(9, 0, 0);
          draw_text(Disc::spotlight_disc_->get_ugen()->report_param(2), false);
          glPopMatrix();
        glTranslatef(-7 + slider2_ * 13.5, 0, 0);
        glTranslatef(0, -3.68, 0);
        glutSolidSphere(.4,20,20);
        glPopMatrix(); 
    }
    if (!ctrl_menu_shown_){
      if (Disc::spotlight_disc_ == NULL) glColor4f(0,0,0,1);
      else glColor4f(0,0,0,.8);
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glEnable(GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f(-8, -5.2, 0);
      glTexCoord2f(1.0, 1.0); glVertex3f(6.8, -5.2, 0);
      glTexCoord2f(1.0, 0.0); glVertex3f(8.5, 2, 0);
      glTexCoord2f(0.0, 0.0); glVertex3f(-8, 2, 0);
      glEnd();
      glPopAttrib();
    }
  
  if (!ctrl_menu_shown_ && Disc::spotlight_disc_ != NULL){
    // Draws the FFT
    complex *fft = graph_->get_fft();
    
    int bins = graph_->get_fft_length();
    double x = -9, y = -5.4;
    double bar_width = 5.32 / (1.0 * bins);
    double y_scale = 2.3;
    double R, G, B;
    double y_coord;
    glColor4f(1,1,1,0);
    glPushMatrix();
      glTranslatef(1, 4.25, 0);
      draw_text(Disc::spotlight_disc_->get_ugen()->name(), true);
      glPopMatrix();
    double col;
    glLineWidth(2);
    for (int i = 0; i < bins; ++i){
      // The color of the bin
      col = 0;
      for (int j = -5; j < 5; ++j){
        if (i+j>0 && i+j < bins)
        col += .4*log10(1+100*fft[i+j].normsq());
      }
      spectrum(col/10.0,R, G, B);
      glColor3f(R,G,B);
      glLineWidth(400*(log10(i*bar_width+1) -log10((i-1)*bar_width+1)));
      // Limit max coord
      y_coord = fmin(y_scale * .8*log10(1+400*fft[i].normsq()), 10.8);
      // The bar
      glBegin(GL_LINES);
      glVertex3f(x + 20*log10(i*bar_width+1), y, 0);
      glVertex3f(x + 20*log10(i*bar_width+1), y + y_coord,0);
      glEnd();
    }
  }
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

// Pops the attributes stack
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

void Menu::advance_time(double t){
  if (graph_->is_new_buffer()){
    graph_->lock_thread(true);
    graph_->update_graphics_dependencies();
    graph_->lock_thread(false);
  }
}


// #-------------- Moveable ----------------#


// Menu doesn't need to move, but a disc might!
void Menu::move(double x, double y, double z){
  if (valid_disc_){
    new_disc_->move(x,y,z);
  }
  if (slider1_clicked_){
    //World coordinates for sliders
    x = fmin(x,-8.695);
    x = fmax(x,-22.71);
    slider1_ = (x+22.71)/(22.71 - 8.695);
  }
  if (slider2_clicked_){
    x = fmin(x,-8.695);
    x = fmax(x,-22.71);
    slider2_ = (x+22.71)/(22.71 - 8.695); 
  }
  if (Disc::spotlight_disc_ != NULL && (slider1_clicked_ || slider2_clicked_)){
    Disc::spotlight_disc_->set_ugen_params(slider1_, slider2_);
  }
}

// Prepares any discs that are being created for motion
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

// Checks to see if the mouse is within the bounds of the menu
bool Menu::check_clicked(double x, double y, double z){
  //Check bounds of in
  if (abs(x - kXShift) < kScaleDimensions){
    if (y < kScaleDimensions * height_to_width_){
      return true;
    }
  }
  return false;
}

// Is called when the menu is no longer clicked. Often this needs delegated to the
// disc that is being moved/created.
void Menu::unclicked(){
  if (valid_disc_){
  
    if(Physics::is_clear_area(new_disc_->pos_.x,new_disc_->pos_.y, new_disc_->get_radius())){ 
    // unclicks the disc, a new disc is finalized!
      new_disc_->unclicked();
      // Add disc to the graph! 
      // Functions do internal type checking
      if (graph_->add_effect(new_disc_)){}
      else if (graph_->add_midi_ugen(new_disc_)){}
      else if (graph_->add_input(new_disc_)){}
      // Keeps an eye on this disc
      Disc::spotlight_disc_ = new_disc_;
          
    }
    else{
      // Remove the disc from the world
      Graphics::remove_drawable(new_disc_);
      Graphics::remove_moveable(new_disc_);
      Physics::take_physics(new_disc_);
      delete new_disc_;
    }
    
  }
  valid_disc_ = false;
  new_disc_ = 0;


  if (Disc::spotlight_disc_ != NULL && (slider1_clicked_ || slider2_clicked_)){
    slider1_clicked_ = false;
    slider2_clicked_ = false;
    Disc::spotlight_disc_->set_ugen_params(slider1_, slider2_);
  }
  
}


// #-------------- Private ----------------#


// Writes words to the screen
void Menu::draw_text(const char * p, bool large){
  glPushMatrix();
  glRasterPos2f(0.0f, 0.0f);
  if (large){
    for (; *p; p++){
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    }
  }
  else {
    for (; *p; p++){
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
    }
  }
  glPopMatrix();

}


// Converts the coordinates (x,y) from screen coordinates to
// image coordinates (a,b)
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



// Determines if the mouse click happened within the bounds of some square
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
  int y_help = 1252;
  int y_trash = 1341;

  int x_slider = 147, slider_length = 680;
  int y_slider1 = 1158, y_slider2 = 1330;

  if (y < y_but[2] + but_size){//Upper button
    //First row of buttons
    for (int i = 0; i < 5; ++i){
      if (inSquare(x -32, y - 21, x_but + i*(but_size+h_but_space), y_but[0], but_size)){
        switch (i){
          case 0: make_disc(100); return; // Input
          case 1: if (midi_active_) make_disc(101); return; // Sine
          case 2: if (midi_active_) make_disc(102); return; // Square
          case 3: if (midi_active_) make_disc(103); return; // Tri
          case 4: if (midi_active_) make_disc(104); return; // Saw
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
    // UP ARROW BUTTON
    if (selector_enabled_ && 
        inSquare(x - 16, y, x_arrow_but, y_up_but, sm_but_size)){
        graph_->handle_up_press();
        return;
    }
    // DOWN ARROW BUTTON
    if (selector_enabled_ && 
        inSquare(x - 16, y, x_arrow_but, y_down_but, sm_but_size)){
        graph_->handle_down_press();
        return;
    }
    // CTRL BUTTON
    if (inSquare(x - 16, y, x_pane_but, y_up_but, sm_but_size)){
      ctrl_menu_shown_ = true; return;
    }
    // FFT BUTTON
    if (inSquare(x - 16, y, x_pane_but, y_down_but, sm_but_size)){
      ctrl_menu_shown_ = false; return;
    }

    // HELP BUTTON
    if (inSquare(x - 16, y, x_pane_but, y_help, sm_but_size)){
      Graphics::show_splash_screen();
      return;
    }

    // DELETE BUTTON
    if (inSquare(x - 16, y, x_pane_but, y_trash, sm_but_size)){
      delete_spotlight();
      return;
    }
    // PARAMETER SLIDER
    if (x > x_slider && x < x_slider + slider_length && Disc::spotlight_disc_ != NULL){
      if (abs(y - y_slider1) < 30){
        slider1_clicked_ = true;
        return;
      }
      if (abs(y - y_slider2) < 30){
        slider2_clicked_ = true;
        return;
      }
    }
  }

}

void Menu::delete_spotlight(){
  if (Disc::spotlight_disc_ != NULL){
    while(Disc::spotlight_disc_->orb_abandon()){}
    Graphics::remove_drawable(Disc::spotlight_disc_);
    Graphics::remove_moveable(Disc::spotlight_disc_);
    Physics::take_physics(Disc::spotlight_disc_);

    //Graphics thread is also using this
    graph_->lock_thread(true);
    graph_->remove_disc(Disc::spotlight_disc_);
    graph_->rebuild();
    graph_->lock_thread(false);
    Disc::spotlight_disc_ = NULL;
  }
}

// Creates a new disc whenever a disc button is pressed.
void Menu::make_disc(int button){
  double rad = 1.15;
  switch (button){
    // Input
    case 100: {
          Input *u_input = new Input();
          new_disc_ = new Disc(u_input, rad, true, 200, 50);
          new_disc_->set_color(0.5, 0.5, 0.5);
          new_disc_->set_texture(0);
          new_disc_->delegate_orb_color_scheme(0);
          break;
          }
    // Sine 
    case 101:{ 
          Sine *u_sine = new Sine();
          new_disc_ = new Disc(u_sine, rad, true, 200, 50);
          new_disc_->set_color(0.9, 0.9, 0.3);
          new_disc_->set_texture(1);
          new_disc_->delegate_orb_color_scheme(6);
          break;
    }
    // Square
    case 102: {
          Square *u_square = new Square();
          new_disc_ = new Disc(u_square, rad, true, 200, 50);
          new_disc_->set_color(0.3, 0.9, 0.9);
          new_disc_->set_texture(2);
          new_disc_->delegate_orb_color_scheme(2);
          break;
    }
    // Tri
    case 103: {
          Tri *u_tri = new Tri();
          new_disc_ = new Disc(u_tri, rad, true, 200, 50);
          new_disc_->set_color(0.9, 0.9, 0.3);
          new_disc_->set_texture(3);
          new_disc_->delegate_orb_color_scheme(3);
          break; 
    }
    // Saw
    case 104: {
          Saw *u_saw = new Saw();
          new_disc_ = new Disc(u_saw, rad, true, 200, 50);
          new_disc_->set_color(0.9, 0.3, 0.9);
          new_disc_->set_texture(4);
          new_disc_->delegate_orb_color_scheme(4);
          break;
    }
    // BitCrusher
    case 200: {
          BitCrusher *u_bc = new BitCrusher();
          new_disc_ = new Disc(u_bc, rad, true);
          new_disc_->set_color(0.3, 0.9, 0.3);
          new_disc_->set_texture(5);
          break; 
    }
    // Chorus
    case 201: {
          Chorus *u_chorus = new Chorus();
          new_disc_ = new Disc(u_chorus, rad, true);
          new_disc_->set_color(0.3, 0.6, 0.9);
          new_disc_->set_texture(6);
          break; 
    }
    // Delay
    case 202: {
          Delay *u_delay = new Delay();
          new_disc_ = new Disc(u_delay, rad, true);
          new_disc_->set_color(0.7, 0.7, 0.3);
          new_disc_->set_texture(7);
          break; 
    }
    // Distortion
    case 203: {
          Distortion *u_dist = new Distortion();
          new_disc_ = new Disc(u_dist, rad, true);
          new_disc_->set_color(0.9, 0.6, 0.3);
          new_disc_->set_texture(8);
          break; 
    }
    // Filter
    case 204: {
          Filter *u_filt = new Filter();
          new_disc_ = new Disc(u_filt, rad, true);
          new_disc_->set_color(0.7, 0.7, 0.7);
          new_disc_->set_texture(9);
          break; 
    }
    // Granular
    case 205: {
          Granular *u_gn = new Granular();
          new_disc_ = new Disc(u_gn, rad, true);
          new_disc_->set_color(0.1, 0.1, 0.8);
          new_disc_->set_texture(10);
          break;
    }
    // Looper 
    case 206: {
          Looper *u_loop = new Looper();
          new_disc_ = new Disc(u_loop, rad, true, 0, 0);
          new_disc_->set_color(0.9, 0.0, 0.0);
          new_disc_->set_texture(11);
          new_disc_->delegate_orb_color_scheme(1);
          break;
    }
    // RingMod 
    case 207: {
          RingMod *u_rm = new RingMod();
          new_disc_ = new Disc(u_rm, rad, true);
          new_disc_->set_color(0.9, 0.0, 0.7);
          new_disc_->set_texture(12);
          break; 
    }
    // Reverb
    case 208: {
          Reverb *u_rev = new Reverb();
          new_disc_ = new Disc(u_rev, rad, true);
          new_disc_->set_color(0.7, 0.0, 0.9);
          new_disc_->set_texture(13);
          break; 
    }
    // Tremolo
    case 209: {
          Tremolo *u_trem = new Tremolo();
          new_disc_ = new Disc(u_trem, rad, true);
          new_disc_->set_color(0.0, 0.6, 0.6);
          new_disc_->set_texture(14);
          break; }
    }
  
  valid_disc_ = true;
  // Add the disc to the front of the list. It will speed deletion times
  // Chances are, it will be deleted anyhow.
  Graphics::add_drawable(new_disc_, 10);
  Graphics::add_moveable(new_disc_);
  Physics::give_physics(new_disc_);
}

void spectrum(double w, double &R, double &G, double &B){
  if (w>1)w=1;
  if (w<0)w=0;
  
  w=w*(645-380)+380;
  
  if (w >= 380 && w < 440){
      R = -(w - 440.) /(440. - 350.);  G = 0.0;  B = 1.0;}
  else if (w >= 440 && w < 490){
      R = 0.0; G = (w - 440.) / (490. - 440.);  B = 1.0;}
  else if (w >= 490 && w < 510){
      R = 0.0; G = 1.0; B = (510-w) / (510. - 490.);}
  else if (w >= 510 && w < 580){
      R = (w - 510.) / (580. - 510.); G = 1.0; B = 0.0;}
  else if (w >= 580 && w < 645){
      R = 1.0; G = -(w - 645.) / (645. - 580.); B = 0.0;}
  else if (w >= 645 && w <= 780){
      R = 1.0; G = 0.0; B = 0.0;}
  else{
      R = 0.0; G = 0.0; B = 0.0;}
}

