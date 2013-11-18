/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.h
  This file contains the signal chain and the audio framework.

*/
#ifndef _UGENCHAIN_H_
#define _UGENCHAIN_H_

#include <list>
#include <vector>
#include <cmath>
#include "DigitalFilter.h" 
#include "UnitGenerator.h" 
#include "RtAudio.h"
#include "RtMidi.h"
#include "RtError.h"
#include "vmath.h"

typedef std::pair<Input *, Vector3d *> InputUGenNode;
typedef std::pair<MidiUnitGenerator *, Vector3d *> MidiUGenNode;
typedef std::pair<UnitGenerator *, Vector3d *> FxUGenNode;
 
class UGenChain {

public:
  static const RtAudioFormat kFormat = RTAUDIO_FLOAT64;
  static const unsigned int kSampleRate = 44100;
  static const int kNumChannels = 2;
  static const double kTwoPi = 6.2831853072;
  static const unsigned int kBufferFrames = 512;
  static const double kMaxOutput = 2.5;

  // Creates all structures. You still need to call initialize()!
  UGenChain();
  // Closes the buffer and cleans up objects
  ~UGenChain();
  
  // Sets up the RtAudio framework and passes the callback 
  // function to send and receive audio data.
  int initialize_audio(); 
  int initialize_midi();

  // Stops the audio stream gracefully
  void stop_audio();

  // Passes any midi notes the MidiUnitGenerators. Decides using the
  // value of velocity whether the event is a note on or a note off
  void handoff_midi(int MIDI_pitch, int velocity);

  // Passes any audio samples to the Input ugens. 
  void handoff_audio(double samples);

  // Process the next sample in the UGenChain. For anything interesting
  // to happen, be sure to handoff a sample or a midi event to the UGenChain
  double tick();
 
  // Adds a unit generator to the signal chain
  void add_effect(FxUGenNode *ugen);
  
  // Adds midi unit generator to a list of objects that must be checked
  // when new midi event is created
  void add_input(InputUGenNode *input);

  // Adds midi unit generator to a list of objects that must be checked
  // when new midi event is created
  void add_midi_ugen(MidiUGenNode *mugen);

  // Check to see if the audio and midi has been set up properly
  static bool has_audio();
  static bool has_midi();

private: 
  // Set once the audio stream has been initialized and opened
  static bool audio_initialized_, midi_initialized_;

  // The chain of effects that is processed before being sent to the output
  std::list<FxUGenNode *> chain_;
  
  // The list of audio inputs that receive samples
  std::vector<InputUGenNode *> inputs_;
  
  // The list of unit generators that require midi events to work
  std::vector<MidiUGenNode *> midi_modules_;
  
  // The RtAudio object that interacts with the soundcard.
  RtAudio *adac_;
  // The RtMidi object listens for MIDI events
  RtMidiIn *midi_;
  
  //Filters to process the output. Just for quality's sake...
  DigitalLowpassFilter *anti_aliasing_;
  DigitalHighpassFilter *low_pass_;

};
 
#endif