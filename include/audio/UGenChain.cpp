/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.cpp
  This file contains the signal chain and the audio framework.
*/

#include "UGenChain.h"
#include <iostream>//remove

double globaltime = 0; //remove
bool UGenChain::audio_initialized_ = false;
bool UGenChain::midi_initialized_ = false;


// #--------------- callback functions ---------------#

// This callback interprets the midi messages and prepares them for the UGenChain,
// which in turn hands them off to any midi modules that may have been created 
void midiCallback( double dt, std::vector< unsigned char > *message, void *data )
{
    UGenChain *chain = (UGenChain *) data;
    unsigned int nBytes = message->size();

    // if a NoteOn message is received, pluck the string
    if (nBytes > 0 && (int)message->at(0) == 144 )
    {
      int MIDI_pitch = static_cast<int>(message->at(1));
      int veloctiy = static_cast<int>(message->at(2));
      chain->handoff_midi(MIDI_pitch, veloctiy);
    }
}


// This callback deals directly with the audio callback buffers. All interaction with 
// the soundcard happens in this function
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int num_frames, double streamTime, RtAudioStreamStatus status, void * data) {

  double *input_buffer = (double *) inputBuffer;
  double *output_buffer = (double *) outputBuffer;
  
  UGenChain *chain = (UGenChain *) data;
  int numChannels = UGenChain::kNumChannels;
  

  double newVal = 0, lastVal;
  for (unsigned int i = 0; i < num_frames; ++i) {
    newVal = chain->tick(input_buffer[i] / UGenChain::kMaxOutput);
    //double out = .3 * sin(2*3.141*400/44100*(++globaltime));
    //newVal = chain->tick(out);
    //std::cout << newVal << std::endl;
    
    
    //Output limiting
    /*
    if (fabs(newVal) > 1.001){
      newVal = lastVal;
      printf("Clipping has occured!\n");
    }
    else{
      lastVal = newVal;
    }
    */
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



// #--------------------UGenChain--------------------#



//Creates all structures. You still need to call initialize()!
UGenChain::UGenChain(){
  anti_aliasing_ = new DigitalLowpassFilter(10000, 1, 1);
  low_pass_ = new DigitalHighpassFilter(10, 1, 1);
}

//Closes the buffer and cleans up objects
UGenChain::~UGenChain(){
  //Make sure we close things gracefully
  stop_audio();
  std::list<UnitGenerator *>::iterator list_iterator;
  list_iterator = chain_.begin();
  //Deletes all filters
  while (chain_.size() > 0 && list_iterator != chain_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }
  delete anti_aliasing_;
  delete low_pass_;  
  delete midi_;
}

// Sets up the RtAudio framework and passes the callback 
// function to send and receive audio data.
int UGenChain::initialize_audio(){
  if (audio_initialized_) return -1;
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
  audio_initialized_ = true;
  return 0;
}
  

// Checks to see if any midi ports are available
int UGenChain::initialize_midi(){
  if (midi_initialized_) return -1;
  midi_ = new RtMidiIn();
  // Check available ports.
  unsigned int nPorts = midi_->getPortCount();
  if (nPorts == 0) {
    printf("No ports available!\n");
    delete midi_;
    midi_initialized_ = false;
    return 0;
  }

  midi_->openPort(0);
  midi_->setCallback( &midiCallback, (void *)this);
  // Don't ignore sysex, timing, or active sensing messages.
  midi_->ignoreTypes(false, false, false);
  midi_initialized_ = true;
  return 1;
}



// Stops the audio stream gracefully
void UGenChain::stop_audio(){  
  adac_->stopStream();
  // close if open
  if (adac_->isStreamOpen())
    adac_->closeStream();
}
  

// Passes any midi notes the MidiUnitGenerators. Decides using the
// value of velocity whether the event is a note on or a note off
void UGenChain::handoff_midi(int MIDI_pitch, int velocity){
  int i = 0;
  //Process each effect in chain
  while (i < midi_modules_.size()) {
    // Stops the note
    if (velocity == 0){
      midi_modules_.at(i)->stop_note(MIDI_pitch);
    }
    // Starts the note
    else{
      midi_modules_.at(i)->play_note(MIDI_pitch, velocity);
    }
    ++i;
  }
}

// HANDLE AUDIO IN PROPERLY

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


// Adds a unit generator to the signal chain
void UGenChain::add_ugen(UnitGenerator *ugen){
    chain_.push_back(ugen);
}


// Adds midi unit generator to a list of objects that must be checked
// when new midi event is created
void UGenChain::add_midi_ugen(MidiUnitGenerator *mugen){
    chain_.push_back(mugen);
    midi_modules_.push_back(mugen);
}

// Check to see if the audio and midi has been set up properly
bool UGenChain::has_audio(){ return audio_initialized_; }
bool UGenChain::has_midi(){ return midi_initialized_; }