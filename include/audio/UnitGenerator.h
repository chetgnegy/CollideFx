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
#include <string.h>
#include <list>
#include <algorithm>
#include "ClassicWaveform.h"
#include "DigitalFilter.h"
#include "complex.h"

class UnitGenerator{
public: 
  // Processes a single sample in the unit generator
  virtual double tick(double in){};
  
  // Allows user to set the generic parameters, bounds must already be set
  virtual void set_params(double p1, double p2);

  // Scales the input to the range 0 - 1
  double get_normalized_param(int param);

  // Sets the input using the range 0 - 1, requires maximum 
  // and minimum parameters to be set
  void set_normalized_param(double param1, double param2);

  // Can things lead into is unit generator or is it the first
  // in the chain?
  virtual bool is_input() = 0;
  virtual bool is_midi() = 0;

  const char * name() { return name_;}
  const char * p_name(int i) {return i==1 ? param1_name_ : param2_name_;}
protected:
  // Sets the bounds on the parameters of the ugen
  void set_limits(double min1, double max1, double min2, double max2);

  // Uses the specified minimum and maximum bounds to restrict parameter to
  // valid range
  double clamp(double param_in);

  // generic parameters for the unit generator. It is up 
  // to the subclass to define these and make them meaningful
  double param1_, max_param1_, min_param1_;
  double param2_, max_param2_, min_param2_;
  const char *name_, *param1_name_, *param2_name_;
};

class MidiUnitGenerator: public UnitGenerator{
public:

  // Adds a single note to the instruent's play list
  void play_note(int MIDI_pitch, int velocity);

  // Searches for a note of the same pitch and stops it.
  void stop_note(int MIDI_pitch);

  // Gets the next sample from the instrument
  double tick();

  // A wrapper for the UnitGenerator's tick function
  double tick(double in){ return tick(); }

  bool is_input(){ return true; }
  bool is_midi(){ return true; }

protected:
  ClassicWaveform *myCW_;
};



/*
The input unit gen just returns what it was given
  parameters are ignored
*/
class Input : public UnitGenerator {
public:
  Input(){
    name_ = "Input";
    param1_name_ = "Volume";
    param2_name_ = "Not Used";
    set_limits(0, 1, 0, 0);
    set_params(1, 0);
  
  }
  ~Input(){}
  // Processes a single sample in the unit generator
  double tick(double in){ return current_value_; }  
  bool is_input(){ return true; }
  bool is_midi(){ return false; }
  double set_sample(double val){ current_value_ = val; }

private:
  double current_value_;
};


/*
The sine wave listens to the midi controller
  param1 = attack
  param2 = sustain
*/
class Sine : public MidiUnitGenerator {
public:
  Sine(double p1 = 0.001, double p2 = .2, int sample_rate = 44100);
  ~Sine();
  void set_params(double p1, double p2);
};


/*
The square wave listens to the midi controller
  param1 = attack
  param2 = sustain
*/
class Square : public MidiUnitGenerator  {
public:
  Square(double p1 = 0.25, double p2 = 2, int sample_rate = 44100);
  ~Square();
  void set_params(double p1, double p2);
};


/*
The tri wave listens to the midi controller
  param1 = attack
  param2 = sustain
*/
class Tri : public MidiUnitGenerator  {
public:
  Tri(double p1 = 0.25, double p2 = 2, int sample_rate = 44100);
  ~Tri();
  void set_params(double p1, double p2);
};


/*
The saw wave listens to the midi controller
  param1 = attack
  param2 = sustain
*/
class Saw : public MidiUnitGenerator  {
public:
  Saw(double p1 = 0.25, double p2 = 2, int sample_rate = 44100);
  ~Saw();
  void set_params(double p1, double p2);
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

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
private:
  // Quantizes the double to the number of bits specified by param1
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
  
  Chorus(double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100);
  ~Chorus();
  // Processes a single sample in the unit generator
  double tick(double in);  

  // restricts parameters to range (0,1) and calculates other parameters,
  // including the rate in Hz and the max delay change
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
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
  
  Delay(double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100);
  ~Delay();
  // Processes a single sample in the unit generator
  double tick(double in);  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
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
  
  bool is_input(){ return false; }
  bool is_midi(){ return false; }
};


/*
A high or low pass filter
  param1 = cutoff frequency
  param2 = Q
*/
class Filter : public UnitGenerator {
public:
  // Makes a lowpass by default
  Filter(double p1 = 1000, double p2 = 1);
  ~Filter();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  // True for lowpass, False for highpass
  void set_lowpass(bool lowpass);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
private:
  DigitalFilter *f_;
  bool currently_lowpass_;
};


/*
A second order bandpass filter
  param1 = cutoff frequency
  param2 = Q
*/
class Bandpass : public UnitGenerator {
public:
  Bandpass(double p1 = 1000, double p2 = 1);
  ~Bandpass();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
private:
  DigitalBandpassFilter *f_;

};


/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per second
  param2 = number of beats
*/
class Looper : public UnitGenerator {
public:
  
  Looper(int sample_rate = 44100);
  ~Looper();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);
  
  // Starts counting down beats until recording starts 
  void start_countdown();

  bool is_input(){ return false; }
  bool is_midi(){ return false; }

private:
  // Cue Loop to start recording
  void start_recording();
  
  // Cue Loop to stop recording and start playing
  void stop_recording();
  
  // Cue for a single beat
  void pulse();

  bool params_set_;
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
A ring modulator. Multiplies the input by a sinusoid
  param1 = modulation frequency
  param2 = Not used
*/
class RingMod : public UnitGenerator {
public:
  static const double kMaxFreq = 5000.0;// Hz
  static const double kMinFreq = 100;// Hz

  RingMod(double p1 = .1, double p2 = 0, int sample_rate = 44100);
  ~RingMod();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
private:
  int sample_count_;
  int sample_rate_;
  double rate_hz_;
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

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
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
  
  Tremolo( double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100);
  ~Tremolo();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  //Updates the parameters
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_midi(){ return false; }
  
private:
  int sample_count_;
  int sample_rate_;
  double rate_hz_;

};

#endif