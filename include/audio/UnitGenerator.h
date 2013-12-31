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
#include <list>
#include <vector>
#include <algorithm>
#include <sstream>
#include "ClassicWaveform.h"
#include "DigitalFilter.h"
#include "complex.h"
#include "fft.h"

class UGenState;


class UnitGenerator{
public: 
  virtual ~UnitGenerator(){ 
    delete[] ugen_buffer_; 
  };

  // Processes a single sample in the unit generator
  virtual double tick(double in) = 0;

  // Allows user to set the generic parameters, bounds must already be set
  virtual void set_params(double p1, double p2);

  // Allows outside world to distinguish between types of UnitGenerators
  virtual bool is_input() = 0;
  virtual bool is_midi() = 0;
  virtual bool is_looper() = 0;
  virtual bool needs_buffer_patch(){return false;}

  virtual UGenState *save_state() = 0;
  virtual void recall_state(UGenState *state) = 0;
  virtual void patch_buffer(double *buffer, int length){}

  // Allows entire buffers to be processed at once
  double *process_buffer(double *buffer, int length);
  double *current_buffer(){return ugen_buffer_;}

  // The absolute average of the samples in the buffer. Used to 
  // calculate brightness
  double buffer_energy();

  // Get the fft of the buffer's current contents
  void buffer_fft(int full_length, complex *out);

  // Scales the input to the range 0 - 1,requires maximum 
  // and minimum parameters to be set
  double get_normalized_param(int param);

  // Sets the input using the range 0 - 1, requires maximum 
  // and minimum parameters to be set
  void set_normalized_param(double param1, double param2);

  const char * name() { return name_;}
  const char * p_name(int i) {return i==1 ? param1_name_ : param2_name_;}
  const char *report_param(int which);
protected:
  // Sets the bounds on the parameters of the ugen
  void set_limits(double min1, double max1, double min2, double max2);

  // Uses the specified minimum and maximum bounds to restrict parameter to
  // valid range
  double clamp(double param_in, int which);

  // Used for block processing of buffer
  int ugen_buffer_size_;
  double *ugen_buffer_;

  void define_printouts(double *report_param1, const char *p1_units, 
                        double *report_param2, const char *p2_units);
  // generic parameters for the unit generator. It is up 
  // to the subclass to define these and make them meaningful
  double param1_, max_param1_, min_param1_, *report_param1_;
  double param2_, max_param2_, min_param2_, *report_param2_;
  const char *name_, *param1_name_, *param2_name_, *param1_units_, *param2_units_;
};

class UGenState{
public:
  virtual ~UGenState(){};
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

  // Allows outside world to distinguish between types of UnitGenerators
  bool is_input(){ return true; }
  bool is_looper(){ return false; }
  bool is_midi(){ return true; }

  UGenState *save_state();
  void recall_state(UGenState *state);

protected:
  ClassicWaveform *myCW_;
};

class MidiInputState : public UGenState {
public:
  MidiInputState(){}
  ~MidiInputState();
  int buffer_length_;
  double *buffer_;
};


/*
The input unit gen just returns what it was given
  parameters are ignored
*/
class Input : public UnitGenerator {
public:
  Input(int length = 512);
  ~Input();
  // Pulls the current sample out of the buffer and advances the read index
  double tick(double in);
  bool is_input(){ return true; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }
  // Sets the sample at the current index in the buffer
  void set_sample(double val);
  // Sets the entire buffer
  void set_buffer(double buffer[], int length);
  
  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  double current_value_;
  int current_index_; 
};

class InputState : public UGenState {
public:
  InputState(){}
  ~InputState();
  int buffer_length_;
  double *buffer_;
};

/*
The sine wave listens to the midi controller
  param1 = attack
  param2 = sustain
*/
class Sine : public MidiUnitGenerator {
public:
  Sine(double p1 = 0.001, double p2 = .2, int sample_rate = 44100, int length = 512);
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
  Square(double p1 = 0.25, double p2 = 2, int sample_rate = 44100, int length = 512);
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
  Tri(double p1 = 0.25, double p2 = 2, int sample_rate = 44100, int length = 512);
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
  Saw(double p1 = 0.25, double p2 = 2, int sample_rate = 44100, int length = 512);
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
  BitCrusher(int p1 = 8, int p2 = 2, int length = 512);
  ~BitCrusher();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  // Quantizes the double to the number of bits specified by param1
  double quantize(double in);
  int sample_count_;
  double sample_;
};

class BitCrusherState : public UGenState {
public:
  BitCrusherState(){}
  int sample_count_;
  double sample_;
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
  
  Chorus(double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100, int length = 512);
  ~Chorus();
  // Processes a single sample in the unit generator
  double tick(double in);  

  // restricts parameters to range (0,1) and calculates other parameters,
  // including the rate in Hz and the max delay change
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }
  bool needs_buffer_patch(){ return true; }

  UGenState *save_state();
  void recall_state(UGenState *state);
  void patch_buffer(double *buffer, int length);

private:
  int buf_write_;
  int buffer_size_;
  int sample_rate_;
  double rate_hz_, depth_, report_hz_;
  double sample_count_;
  double *buffer_;
};

class ChorusState : public UGenState {
public:
  ChorusState(){}
  ~ChorusState(){
    delete buffer_;
  }
  int buf_write_;
  int buffer_size_;
  double sample_count_;
  double *buffer_;
};



/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
class Delay : public UnitGenerator {
public:
  static const int kShortestDelay = 50;
  
  Delay(double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100, int length = 512);
  ~Delay();
  // Processes a single sample in the unit generator
  double tick(double in);  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }
  bool needs_buffer_patch(){ return true; }

  UGenState *save_state();
  void recall_state(UGenState *state);
  void patch_buffer(double *buffer, int length);

private:
  int buf_write_;
  int max_buffer_size_;
  int sample_rate_;
  float *buffer_;
  double buffer_size_;   
};

class DelayState : public UGenState {
public:
  DelayState(){}
  ~DelayState(){ delete buffer_;}
  int buf_write_;
  int buffer_size_; 
  float *buffer_;
};



/*
The distortion effect clips the input to a speficied level
  param1 = pre clip gain
  param2 = post clip gain
*/
class Distortion : public UnitGenerator {
public:
  
  Distortion(double p1 = 5.0, double p2 = 0.2, int length = 512);
  ~Distortion();

  // Processes a single sample in the unit generator
  double tick(double in);  
  
  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  DigitalLowpassFilter *f_;
  DigitalFilter *inv_;

};

class DistortionState : public UGenState {
public:
  DistortionState(){}
  ~DistortionState(){}
  
  DigitalFilterState *f_state_, *inv_state_;
};



/*
A high or low pass filter
  param1 = cutoff frequency
  param2 = Q
*/
class Filter : public UnitGenerator {
public:
  // Makes a lowpass by default
  Filter(double p1 = 1000, double p2 = 1, int length = 512);
  ~Filter();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  // True for lowpass, False for highpass
  void set_lowpass(bool lowpass);
  bool is_lowpass(){return currently_lowpass_;};

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  DigitalFilter *f_, *f2_;
  bool currently_lowpass_;
};

class FilterState : public UGenState {
public:
  FilterState(){}
  ~FilterState(){ 
    delete f_state_;
  }
  DigitalFilterState *f_state_;
  bool currently_lowpass_;
};


/*
A second order bandpass filter
  param1 = cutoff frequency
  param2 = Q
*/
class Bandpass : public UnitGenerator {
public:
  Bandpass(double p1 = 1000, double p2 = 1, int length = 512);
  ~Bandpass();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  DigitalBandpassFilter *f_;

};

class BandpassState : public UGenState {
public:
  BandpassState(){}
  ~BandpassState(){ 
    delete f_state_;
  }
  DigitalFilterState *f_state_;
};


/*
The granular effect plays the signal back some time later
  param1 = granule length
  param2 = 
*/

// First int is start sample, second is end sample in buffer, third is
// granule current position
struct Granule{
  int start; int end; int at; int win_length;
};

class Granular : public UnitGenerator {
public:
  Granular(double p1 = 600, double p2 = .5, int sample_rate = 44100, int length = 512);
  ~Granular();
  // Processes a single sample in the unit generator
  double tick(double in);  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }
  bool needs_buffer_patch(){ return true; }

  UGenState *save_state();
  void recall_state(UGenState *state);
  void patch_buffer(double *buffer, int length);

private:
  int buf_write_;
  int sample_rate_;
  int buffer_size_; 
  double *buffer_;
  std::vector<Granule> granules_;
};

class GranularState : public UGenState {
public:
  GranularState(){}
  ~GranularState(){
    delete buffer_;
  }
  int buf_write_;
  int buffer_size_; 
  double *buffer_;
  std::vector<Granule> granules_;
};


/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per second
  param2 = number of beats
*/
class Looper : public UnitGenerator {
public:
  
  Looper(int sample_rate = 44100, int length = 512);
  ~Looper();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  // reallocates the buffer if the delay length changes
  void set_params(double p1, double p2);
  
  // Starts counting down beats until recording starts 
  void start_countdown();
  // Getters and setters for the number of count in beats
  void set_start_counter(int num);
  int get_start_counter();

  bool is_input(){ return has_recording_; }
  bool is_looper(){ return true; }
  bool is_midi(){ return false; }
  bool needs_buffer_patch(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);
  void patch_buffer(double *buffer, int length);
  
  // Used in the disc. Stored here so that we don't allocate
  // this memory for every disc. Used only for state transitions
  struct timeval timer; 
  std::pair<long, long> click_data;
  void (*pulsefnc)(void *,int);
  void *data;

  
private:
  // Cue Loop to start recording
  void start_recording();
  
  // Cue Loop to stop recording and start playing
  void stop_recording();
  
  // Cue for a single beat
  void pulse();

  int start_counter_;
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

class LooperState : public UGenState {
public:
  LooperState(){};
  ~LooperState(){
    delete buffer_;
  }
  bool params_set_;
  bool counting_down_;
  bool is_recording_, has_recording_;
  int start_counter_;
  int buf_write_, buf_read_;
  int buffer_size_;
  int sample_rate_;
  int this_beat_;
  int beat_count_;
  float *buffer_;
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

  RingMod(double p1 = .1, double p2 = 0, int sample_rate = 44100, int length = 512);
  ~RingMod();
  // Processes a single sample in the unit generator
  double tick(double in);
  // casts the parameters to ints and restricts them to a certain value
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  long sample_count_;
  int sample_rate_;
  double rate_hz_, report_hz_;
};

class RingModState : public UGenState {
public:
  RingModState(){}
  long sample_count_;
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
  
  Reverb(double p1 = 0.8, double p2 = 0.2, int length = 512);
  ~Reverb();
  // Processes a single sample in the unit generator
  double tick(double in);  
  //Updates the filter variables
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);

private:
  FilterBank *fb_;
  std::list<AllpassApproximationFilter *> aaf_;
};

class ReverbState : public UGenState {
public:
  ReverbState(){}
  ~ReverbState(){
    delete[] comb_state_;
    delete[] ap_state_;
  }
  DigitalFilterState **comb_state_;
  DigitalFilterState **ap_state_;
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
  
  Tremolo( double p1 = 0.5, double p2 = 0.5, int sample_rate = 44100, int length = 512);
  ~Tremolo();
  
  // Processes a single sample in the unit generator
  double tick(double in);  
  
  //Updates the parameters
  void set_params(double p1, double p2);

  bool is_input(){ return false; }
  bool is_looper(){ return false; }
  bool is_midi(){ return false; }

  UGenState *save_state();
  void recall_state(UGenState *state);
  
private:
  int sample_rate_;
  long sample_count_;
  double rate_hz_, report_hz_;
};

class TremoloState : public UGenState {
public:
  TremoloState(){};
  long sample_count_;
  //double rate_hz_, report_hz_;
};

#endif
