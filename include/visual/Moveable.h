/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Moveable.h
  An interface for things that are to be moved in OpenGL
*/

#ifndef _MOVEABLE_H_
#define _MOVEABLE_H_

class Moveable {
public: 
  virtual void move(double x, double y, double z) = 0;
  virtual void prepare_move(double x, double y, double z) = 0;
  virtual bool check_clicked(double x, double y, double z) = 0;
  virtual void unclicked(){}
};


#endif