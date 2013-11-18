/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  ClassicWaveform.h
  This is a library for producing classic waveform 
  sounds (sine, square, triangle, and sawtooth waves)
*/

#ifndef _CLASSICWAVEFORM_H_
#define _CLASSICWAVEFORM_H_

#include <string>
#include <list>
#include <math.h>
#include <thread.h>

struct Note{
  bool flag_for_deletion;
  int velocity;
  int stage; // 1 - Attack, 2 - Sustain, 3 - Release
  double pitch;
  long samples;
  long release_sample;
  double sustain_end_env;

};

class ClassicWaveform{
public:
  ClassicWaveform(const char * type, int sample_rate = 44100);
  ~ClassicWaveform();

  // We can tick for the next sample for the entire instruemnt
  double tick();

  // Adds a single note to the instrument
  void play_note(int pitch, int velocity);

  // Searches for a note of the same pitch and stops it.
  bool stop_note(int pitch);

  // Computes the envelope of the signal using the current attack, decay,
  // and release rates. Also responsible for handing the note from one stage
  // in the envelope to the next
  double compute_envelope(Note *n);

  void set_attack(double seconds);

  void set_sustain(double seconds);

  void set_release(double seconds);

private:
  // Gets the next sample for a single note
  double next_sample(Note *n);

  std::list<Note *> notes_;
  // The function that contains the waveform
  double (*wave_func_)(double);
  // The number of samples for ADSR
  int attack_samples_;
  int sustain_samples_;
  int release_samples_;
  // The sample rate of the program
  int sample_rate_;

  Mutex mutex_;
};

#endif