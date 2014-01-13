/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenChain.cpp
  This file contains the signal chain and the audio framework.
*/

#include "UGenChain.h"

bool UGenChain::audio_initialized_ = false;
bool UGenChain::midi_initialized_ = false;
// Check to see if the audio and midi has been set up properly
bool UGenChain::has_audio(){ return audio_initialized_; }
bool UGenChain::has_midi(){ return midi_initialized_; }



// #--------------- callback functions ---------------#

// This callback interprets the midi messages and prepares them for the UGenChain,
// which in turn hands them off to any midi modules that may have been created 
void midiCallback( double dt, std::vector< unsigned char > *message, void *data )
{
    UGenGraphBuilder *graph = (UGenGraphBuilder *) data;
    unsigned int nBytes = message->size();

    // if a NoteOn message is received, pluck the string
    if (nBytes > 0 && (int)message->at(0) == 144 )
    {
      int MIDI_pitch = static_cast<int>(message->at(1));
      int veloctiy = static_cast<int>(message->at(2));
      graph->handoff_midi(MIDI_pitch, veloctiy);
    }
}

double k = 0;
// This callback deals directly with the audio callback buffers. All interaction with 
// the soundcard happens in this function
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int num_frames, double streamTime, RtAudioStreamStatus status, void * data) {

  double *input_buffer = (double *) inputBuffer;
  double *output_buffer = (double *) outputBuffer;
  
  UGenGraphBuilder *graph = (UGenGraphBuilder *) data;
  int numChannels = UGenChain::kNumChannels;
  
  
  double newVal = 0, lastVal;

  graph->handoff_audio_buffer(input_buffer,num_frames);

  double *out_mono = new double[num_frames];
  graph->lock_thread(true);
  graph->load_buffer(out_mono, num_frames);
  graph->signal_new_buffer();
  graph->rebuild();
  graph->lock_thread(false);

  //Fills the other channels
  for (unsigned int i = 0; i < num_frames; ++i) {
    for (unsigned int j = 0; j < numChannels; ++j) {
      // Limiting
      //if (fabs(out_mono[i]) > 15) out_mono[i] = 0;

      output_buffer[i * numChannels + j] = 0.1 * out_mono[i];
    }
  }

  delete[] out_mono;

  return 0;

}



// #--------------------UGenChain--------------------#



//Creates all structures. You still need to call initialize()!
UGenChain::UGenChain(){
  graph_builder_ = new UGenGraphBuilder(kBufferFrames);
}

//Closes the buffer and cleans up objects
UGenChain::~UGenChain(){
  //Make sure we close things gracefully
  stop_audio();
  delete graph_builder_;
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
    adac_->openStream(&output_params, 
                      &input_params, format, 
                      sample_rate, 
                      &buffer_frames, 
                      &audioCallback, 
                      (void *) this->graph_builder_, 
                      &options_);
     // opens the audio buffer
    adac_->startStream();
  } catch (RtError& e) {
    std::cout << e.getMessage() << std::endl;
    return -1;
  }
  audio_initialized_ = true;
  return 0;
}
  
// Tests if a string is a number or not
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
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
  int priority_port = 0;

  if(nPorts>1){
    
    for (int i = 0; i < nPorts; ++i){
      if(nPorts>1)printf("%d: %s\n",i,midi_->getPortName(i).c_str()); 
    }

    //User must select a midi interface if there are several
    std::string s;
    do{ 
      printf("Select a MIDI interface: ");
      std::cin >> s;
      try{
        priority_port = atoi(s.c_str());
      }catch(int e){//try again, fool
        priority_port = -1;
      }
    }while(priority_port<0 || priority_port>nPorts || !is_number(s.c_str()));
  }
  
  midi_->openPort(priority_port);
  
  midi_->setCallback( &midiCallback, (void *) this->graph_builder_);
  printf("Using MIDI: %s\n",midi_->getPortName(priority_port).c_str());
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
  


// Returns the interconnects of unit generators
UGenGraphBuilder *UGenChain::get_signal_graph(){
  return graph_builder_;
}




