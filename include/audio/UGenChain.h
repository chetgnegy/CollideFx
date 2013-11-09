/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.h
  This file contains the signal chain and the audio framework.

*/
#ifndef _UGENCHAIN_H_
#define _UGENCHAIN_H_

#include <list>
#include <cmath>
#include "DigitalFilter.h" 
#include "UnitGenerator.h" 
#include "RtAudio.h"
#include "RtError.h"

 
class UGenChain {

public:
  static const RtAudioFormat kFormat = RTAUDIO_FLOAT64;
  static const unsigned int kSampleRate = 44100;
  static const int kNumChannels = 2;
  static const double kTwoPi = 6.2831853072;
  static const unsigned int kBufferFrames = 512;
  static const double kMaxOutput = 2.5;

  //Creates all structures. You still need to call initialize()!
  UGenChain();
  //Closes the buffer and cleans up objects
  ~UGenChain();
  
  // Sets up the RtAudio framework and passes the callback 
  // function to send and receive audio data.
  int initialize(); 
  
  // Stops the audio stream gracefully
  void stopAudio();

  // Process the next sample in the UGenChain
  double tick(double in);
 
 // Adds an effect to the signal chain
 double add_effect(UnitGenerator *fx);

private: 
  //Set once the audio stream has been initialized and opened
  bool initialized_;

  //The chain of effects that is processed before being sent to the output
  std::list<UnitGenerator *> chain_;
  
  //The RtAudio object that interacts with the soundcard.
  RtAudio *adac_;
  
  //Filters to process the output. Just for quality's sake...
  DigitalLowpassFilter *anti_aliasing_;
  DigitalHighpassFilter *low_pass_;

};
 
#endif