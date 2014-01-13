/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.h
  This file contains the signal chain and the audio framework.

*/
#ifndef _UGENCHAIN_H_
#define _UGENCHAIN_H_


#include <cmath>
#include <cstring>
#include "RtAudio.h"
#include "RtMidi.h"
#include "RtError.h"
#include "vmath.h"
#include "UGenGraphBuilder.h"


class UGenChain {

public:
  static const RtAudioFormat kFormat = RTAUDIO_FLOAT64;
  static const unsigned int kSampleRate = 44100;
  static const int kNumChannels = 2;
  static const double kTwoPi = 6.2831853072;
  static const double kMaxOutput = 2.5;

  UGenChain();
  // Closes the buffer and cleans up objects
  ~UGenChain();
  
  // Sets up the RtAudio framework and passes the callback 
  // function to send and receive audio data.
  int initialize_audio(); 
  int initialize_midi();

  // Stops the audio stream gracefully
  void stop_audio();

  // Returns the interconnects of unit generators
  UGenGraphBuilder *get_signal_graph();

  // Check to see if the audio and midi has been set up properly
  static bool has_audio();
  static bool has_midi();

  
private: 
  // Set once the audio stream has been initialized and opened
  static bool audio_initialized_, midi_initialized_;

  // The signal chain
  UGenGraphBuilder *graph_builder_;

  // The RtAudio object that interacts with the soundcard.
  RtAudio *adac_;
  // The RtMidi object listens for MIDI events
  RtMidiIn *midi_;

  // The size of all buffers
  unsigned int buffer_frames_;
};
 
#endif
