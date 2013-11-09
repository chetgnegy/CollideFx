/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UnitGenerator.h
  This file the UnitGenerator class. This class is used as modules 
  for input and effects. UnitGenerator is an abstract class and should 
  only be subclassed
*/

#ifndef _UNITGENERATOR_H_
#define _UNITGENERATOR_H_


#include <cmath>
#include <stdio.h>//remove
#include <list>
#include "DigitalFilter.h"

class UnitGenerator{
public: 
  // Processes a single sample in the unit generator
  virtual double tick(double in){};
  
  // Allows user to set the generic parameters. Override this for more
  // complex behavior.  Parameters are forced to be positive by default
  virtual void set_params(double p1, double p2);

  // Gets the interpolated value between two samples of array
  double interpolate(double *array, int length, double index);
  float interpolate(float *array, int length, double index);
  
  // generic parameters for the unit generator. It is up 
  // to the subclass to define these and make them meaningful
  double param1_;
  double param2_;
  double mix_;
  
};





/*
The input unit gen just returns what it was given
  parameters are ignored
*/
class Input : public UnitGenerator {
public:
  Input(){}
  ~Input(){}
  // Processes a single sample in the unit generator
  double tick(double in){return in;}  
};







/*
The bitcrusher effect quantizes and downsamples the input 
  param1 = bits (casted to an int)
  param2 = downsampling factor
*/
class BitCrusher : public UnitGenerator {
public:
  BitCrusher(int p1 = 8, int p2 = 2);
  ~BitCrusher();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);
private:
  //Quantizes the double to the number of bits specified by param1
  double quantize(double in);
  
  double sample_;
  double sample_count_;
};






/*
The chorus effect delays the signal by a variable amount
  param1 = rate of the chorus LFO
  param2 = depth of the chorus effect
*/
class Chorus : public UnitGenerator {
public:
  static const double kMaxDelay = 0.0125;// s
  static const double kDelayCenter = 0.0175;// s
  static const double kMaxFreq = 10.0;// Hz
  static const double kMinFreq = .020;// Hz
  
  Chorus(int sample_rate, double p1 = 0.5, double p2 = 0.5);
  ~Chorus();
  // Processes a single sample in the unit generator
  double tick(double in);  

  // restricts parameters to range (0,1) and calculates other params
  void set_params(double p1, double p2);
private:
  double *buffer_;
  int buf_write_;
  double rate_hz_, depth_;
  double sample_count_;
  int buffer_size_;
  int sample_rate_;
};





/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
class Delay : public UnitGenerator {
public:
  static const int kShortestDelay = 50;
  
  Delay(int sample_rate, double p1 = 0.5, double p2 = 0.5);
  ~Delay();
  // Processes a single sample in the unit generator
  double tick(double in);  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);
private:
  float *buffer_;
  int buf_write_;
  int buffer_size_;
  int sample_rate_;
};





/*
The distortion effect clips the input to a speficied level
  param1 = pre clip gain
  param2 = post clip gain
*/
class Distortion : public UnitGenerator {
public:
  
  Distortion(double p1 = 18.0, double p2 = 0.6);
  ~Distortion();
  // Processes a single sample in the unit generator
  double tick(double in);  
  
};





/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per second
  param2 = number of beats
*/
class Looper : public UnitGenerator {
public:
  
  Looper(int sample_rate, double param1, double param2);
  ~Looper();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);
  
  // Starts counting down beats until recording starts 
  void start_countdown();
  
  // Cue Loop to start recording
  void start_recording();
  
  // Cue Loop to stop recording and start playing
  void stop_recording();
  
  // Cue for a single beat
  void pulse();

private:
  float *buffer_;
  int buf_write_, buf_read_;
  int buffer_size_;
  int sample_rate_;
  int this_beat_;
  int beat_count_;
  bool counting_down_;
  bool is_recording_, has_recording_;
};





/*
The reverb effect convolves the signal with an impulse response
  param1 = room size
  param2 = damping
*/
class Reverb : public UnitGenerator {
public:
  static const int kCombDelays[];
  static const int kAllPassDelays[];
  
  Reverb(double p1 = 0.8, double p2 = 0.2);
  ~Reverb();
  // Processes a single sample in the unit generator
  double tick(double in);  
  //Updates the filter variables
  void set_params(double p1, double p2);
private:
  FilterBank *fb_;
  std::list<AllpassApproximationFilter *> aaf_;
  
};





/*
The tremolo effect modulates the amplitude of the signal
  param1 = rate
  param2 = depth
*/
class Tremolo : public UnitGenerator {
public:
  static const double kMaxFreq = 10.0;// Hz
  static const double kMinFreq = .020;// Hz
  
  Tremolo(int sample_rate, double p1 = 0.5, double p2 = 0.5);
  ~Tremolo();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  //Updates the parameters
  void set_params(double p1, double p2);
  
private:
  int sample_count_;
  int sample_rate_;
  double rate_hz_;

};

#endif