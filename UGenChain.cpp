/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.cpp
  This file contains the signal chain and the audio framework.
*/

#include "UGenChain.h"
#include <iostream>//remove

double globaltime = 0;
// This callback deals directly with the audio callback buffers. All interaction with 
// the soundcard happens in this function
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int num_frames, double streamTime, RtAudioStreamStatus status, void * data) {

  double *input_buffer = (double *) inputBuffer;
  double *output_buffer = (double *) outputBuffer;
  
  UGenChain *chain = (UGenChain *) data;
  int numChannels = UGenChain::kNumChannels;
  
   
  double newVal = 0;
  for (unsigned int i = 0; i < num_frames; ++i) {
    newVal = chain->tick(input_buffer[i] / UGenChain::kMaxOutput);
    //double out = .3 * sin(2*3.141*400/44100*(++globaltime));
    //newVal = chain->tick(out);
    //std::cout << newVal << std::endl;
    
    //Output limiting
    if (fabs(newVal) > 1.001){
      printf("Clipping has occured!\n");
      newVal = 0;
    }
    
    output_buffer[i * numChannels] =  newVal;
  }
  
  //Fills the other channels
  for (unsigned int i = 0; i < num_frames; ++i) {
    for (unsigned int j = 1; j < numChannels; ++j) {
      output_buffer[i * numChannels + j] = output_buffer[i * numChannels ];
    }
  }
  
  //char input;
  //std::cin.get(input); //Pause after a single frame
  return 0;
}


//Creates all structures. You still need to call initialize()!
UGenChain::UGenChain(){
  initialized_ = false;
  anti_aliasing_ = new DigitalLowpassFilter(10000, 1, 1);
  low_pass_ = new DigitalHighpassFilter(10, 1, 1);

}

//Closes the buffer and cleans up objects
UGenChain::~UGenChain(){
  //Make sure we close things gracefully
  stopAudio();
  std::list<UnitGenerator *>::iterator list_iterator;
  list_iterator = chain_.begin();
  //Deletes all filters
  while (chain_.size() > 0 && list_iterator != chain_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }
  delete anti_aliasing_;
  delete low_pass_;  
}

// Sets up the RtAudio framework and passes the callback 
// function to send and receive audio data.
int UGenChain::initialize(){
  if (initialized_) return -1;
  adac_ = new RtAudio();
  RtAudio::StreamParameters input_params, output_params;
  RtAudio::StreamOptions options_;
  
  // check for audio devices
  if (adac_->getDeviceCount() < 1) {
    printf("Audio devices not found!\n");
    exit(-1);
  }
  // let RtAudio print messages to stderr.
  adac_->showWarnings(true);
  // set input and output parameters
  input_params.deviceId = adac_->getDefaultInputDevice();
  input_params.nChannels = 1;
  input_params.firstChannel = 0;
  output_params.deviceId = adac_->getDefaultOutputDevice();
  output_params.nChannels = kNumChannels;
  output_params.firstChannel = 0; 
  
  try { 
    // Yeah, I hate this too, it's just to make the openStream function happy...
    unsigned int sample_rate = kSampleRate;
    unsigned int buffer_frames = kBufferFrames;
    RtAudioFormat format = kFormat;
    // Tells the audio stream how to open and what callback to use
    adac_->openStream(&output_params, &input_params, format, sample_rate, &buffer_frames, &audioCallback, (void *) this, &options_);
     // opens the audio buffer
    adac_->startStream();
  } catch (RtError& e) {
    std::cout << e.getMessage() << std::endl;
    return -1;
  }
  initialized_ = true;
  return 0;
}
  
// Stops the audio stream gracefully
void UGenChain::stopAudio(){  
  adac_->stopStream();
  // close if open
  if (adac_->isStreamOpen())
    adac_->closeStream();
}
  
  
// Process the next sample in the UGenChain
double UGenChain::tick(double in){
  if (chain_.size() > 0) {
    std::list<UnitGenerator *>::iterator it;
    it = chain_.begin();
    //Process each effect in chain
    while (it != chain_.end()) {
      in = (*it)->tick(in);
      ++it;
    }
  }
  //Filters the signal to remove HF and DC components
  low_pass_->tick(in);
  anti_aliasing_->tick(low_pass_->most_recent_sample());
  return anti_aliasing_->most_recent_sample().re();
}

// Adds an effect to the signal chain
double UGenChain::add_effect(UnitGenerator *fx){
    chain_.push_back(fx);
}
