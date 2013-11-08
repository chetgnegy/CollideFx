#ifndef _GRAPHICSUTIL_H_
#define _GRAPHICSUTIL_H_

double spectrum(double w, double &R, double &G, double &B){
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

#endif