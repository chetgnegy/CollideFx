#ifndef GRAPHICSUTIL_H_
#define GRAPHICSUTIL_H_

double spectrum(double w, double &R, double &G, double &B) {
  if (w>1)w=1;
  if (w<0)w=0;
  
  w=w*(645-380)+380;
  
  if (w >= 380 && w < 440){
      R = -(w - 440.) /(440. - 350.);
      G = 0.0;
      B = 1.0;
  }
  else if (w >= 440 && w < 490){
      R = 0.0;
      G = (w - 440.) / (490. - 440.);
      B = 1.0;
  }
  else if (w >= 490 && w < 510){
      R = 0.0;
      G = 1.0;
      B = (510-w) / (510. - 490.);
  }
  else if (w >= 510 && w < 580){
      R = (w - 510.) / (580. - 510.);
      G = 1.0;
      B = 0.0;
  }
  else if (w >= 580 && w < 645){
      R = 1.0;
      G = -(w - 645.) / (645. - 580.);
      B = 0.0;
  }
  else if (w >= 645 && w <= 780){
      R = 1.0;
      G = 0.0;
      B = 0.0;
  }
  else{
      R = 0.0;
      G = 0.0;
      B = 0.0;
  }
}


void glDrawPrism(float x, float z, float height, float width)
{
    glBegin(GL_QUADS);
    glVertex3f(x, 0, z);                              glTexCoord2f (0, 0);
    glVertex3f(x, 0, z + width);                      glTexCoord2f (1, 0);
    glVertex3f(x, height, z + width);                 glTexCoord2f (1, 1);
    glVertex3f(x, height, z);                         glTexCoord2f (0, 1);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex3f(x, 0, z + width);                      glTexCoord2f (0, 0);
    glVertex3f(x + width, 0, z + width);              glTexCoord2f (1, 0);
    glVertex3f(x + width, height, z + width);         glTexCoord2f (1, 1);
    glVertex3f(x, height, z + width);                 glTexCoord2f (0, 1);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex3f(x + width, 0, z + width);              glTexCoord2f (0, 0);
    glVertex3f(x + width, 0, z);                      glTexCoord2f (1, 0);
    glVertex3f(x + width, height, z);                 glTexCoord2f (1, 1);
    glVertex3f(x + width, height, z + width);         glTexCoord2f (0, 1);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex3f(x + width, 0, z);                      glTexCoord2f (0, 0);
    glVertex3f(x, 0, z);                              glTexCoord2f (1, 0);
    glVertex3f(x, height, z);                         glTexCoord2f (1, 1);
    glVertex3f(x + width, height, z);                 glTexCoord2f (0, 1);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex3f(x, height, z);                         glTexCoord2f (0, 0);
    glVertex3f(x, height, z + width);                 glTexCoord2f (1, 0);
    glVertex3f(x + width, height, z + width);         glTexCoord2f (1, 1);
    glVertex3f(x + width, height, z);                 glTexCoord2f (0, 1);
    glEnd();
}




void DrawCubeFace(float fSize)
{
  fSize /= 2.0;
  glBegin(GL_QUADS);
  glVertex3f(-fSize, -fSize, fSize);    glTexCoord2f (0, 0);
  glVertex3f(fSize, -fSize, fSize);     glTexCoord2f (1, 0);
  glVertex3f(fSize, fSize, fSize);      glTexCoord2f (1, 1);
  glVertex3f(-fSize, fSize, fSize);     glTexCoord2f (0, 1);
  glEnd();
}
void DrawCubeWithTextureCoords (float fSize)
{
  glPushMatrix();
  DrawCubeFace (fSize);
  glRotatef (90, 1, 0, 0);
  DrawCubeFace (fSize);
  glRotatef (90, 1, 0, 0);
  DrawCubeFace (fSize);
  glRotatef (90, 1, 0, 0);
  DrawCubeFace (fSize);
  glRotatef (90, 0, 1, 0);
  DrawCubeFace (fSize);
  glRotatef (180, 0, 1, 0);
  DrawCubeFace (fSize);
  glPopMatrix();
}





//void drawSphere using glBegin
void get_sphere_vertices(double r, int lats, int longs) {
 int i, j;
 for(i = 0; i <= lats; i++) {
   double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
   double z0  = sin(lat0);
   double zr0 =  cos(lat0);
   
   double lat1 = M_PI * (-0.5 + (double) i / lats);
   double z1 = sin(lat1);
   double zr1 = cos(lat1);
   
   glBegin(GL_QUAD_STRIP);
   for(j = 0; j <= longs; j++) {
     double lng = 2 * M_PI * (double) (j - 1) / longs;
     double x = cos(lng);
     double y = sin(lng);
     
     // glNormal3f(x * zr0, y * zr0, z0);
     glColor4f(1.0, 1.0, 1.0, 1.0);
     glVertex3f( r * x * zr0,  r * y * zr0, r * z0);
     
     //glNormal3f(x * zr1, y * zr1, z1);
     glColor4f(1.0, 1.0, 1.0, 1.0);
     glVertex3f(r * x * zr1, r * y * zr1, r * z1);
   }
   glEnd();
 }
}


#endif