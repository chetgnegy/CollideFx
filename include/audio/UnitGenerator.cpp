/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UnitGenerator.cpp
  This file the UnitGenerator class. This class is used as modules 
  for input and effects.

*/

#include "UnitGenerator.h"
#include "complex.h"
#include <iostream>//remove
#include <algorithm>

// Allows user to set the generic parameters, parameters are forced to be positive
void UnitGenerator::set_params(double p1, double p2){
  param1_ = p1>=0 ? p1 : 0;
  param2_ = p2>=0 ? p2 : 0;
}

double UnitGenerator::interpolate(double *array, int length, double index){
  int trunc_index = static_cast<int>(floor(index));
  double leftover = index-1.0*trunc_index;
  double sample_one = array[(trunc_index + length)%length];
  double sample_two = array[(trunc_index + length + 1)%length];
  double output = sample_one*(1-leftover) + sample_two*leftover;
  return output;
}

float UnitGenerator::interpolate(float *array, int length, double index){
  int trunc_index = floor(index);
  float leftover = index-trunc_index;
  float sample_one = array[(trunc_index + length)%length];
  float sample_two = array[(trunc_index + length + 1)%length];
  return sample_one*(1-leftover) + sample_two*leftover;
}





/*
The bitcrusher effect quantizes and downsamples the input 
  param1 = bits (casted to an int)
  param2 = downsampling factor
*/
BitCrusher::BitCrusher(int p1, int p2){
  set_params(p1, p2); 
  sample_ = 0;
  sample_count_ = 0;
}

BitCrusher::~BitCrusher(){}
// Processes a single sample in the unit generator
double BitCrusher::tick(double in){
  //Downsampling
  ++sample_count_;
  if (sample_count_ >= param2_){
    sample_count_ = 0;
    //Quantization
    sample_ = quantize(in);
  }
  return sample_;
}
// casts the parameters to ints and restricts them to a certain value
void BitCrusher::set_params(double p1, double p2){
  p1 = p1 > 16 ? 16 : floor(p1); // bounded by 1 and 16
  param1_ = p1 < 1 ? 1 : p1;  
  p2 = p2 > 16 ? 16 : floor(p2); // bounded by 1 and 16
  param2_ = p2 < 1? 1 : p2;
}

//Quantizes the double to the number of bits specified by param1
double BitCrusher::quantize(double in){
  //printf("Check this. We should know if this is exceeding the maximum...\n");
  if (param1_ == 1) return in;
  return round(in * pow(2.0, param1_)) / pow(2.0, param1_);
}







/*
The chorus effect delays the signal by a variable amount
  param1 = rate of the chorus LFO
  param2 = depth of the chorus effect
*/
Chorus::Chorus(int sample_rate, double p1, double p2){
  sample_rate_ = sample_rate;
  
  set_params(p1, p2);
  buffer_size_ = ceil((kMaxDelay + kDelayCenter)*sample_rate_);
  
  //Makes an empty buffer
  buffer_ = new double[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
  
  sample_count_ = 0;
  buf_write_ = 0;
}
Chorus::~Chorus(){
  delete[] buffer_;
}

// Processes a single sample in the unit generator
// Jon Dattorro - Part 2: Delay-Line Modulation and Chorus 
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart2.pdf
double Chorus::tick(double in){
  double blend =  0.7071;//1.0;
  double feedback = 0.7071;
  double feedforward = 1.0;
  //feedback
  double buf_fb = buf_write_ - sample_rate_ * kDelayCenter + buffer_size_;
  buf_fb = fmod(buf_fb, buffer_size_);
  
  //feedforward
  double buf_read = buf_write_ - sample_rate_ * ( kDelayCenter + 
        depth_* sin(rate_hz_ * sample_count_)) + buffer_size_;
  buf_read = fmod(buf_read,buffer_size_);
  
  buffer_[buf_write_] = in - feedback * interpolate(buffer_, buffer_size_, buf_fb);
  double output = feedforward * interpolate(buffer_, buffer_size_, buf_read) + blend * buffer_[buf_write_];
  
  //Wrap variables to prevent out-of-bounds/overflow
  ++sample_count_;
  if (rate_hz_ * sample_count_ > 6.2831853) {
    sample_count_ = fmod(sample_count_, 6.2831853/rate_hz_);
  }
  
  ++buf_write_;
  buf_write_ %= buffer_size_;
  
  return output;
}
// restricts parameters to range (0,1) and calculates other params
void Chorus::set_params(double p1, double p2){
  p1 = p1 > 1 ? 1 : p1;
  p1 = p1 < 0 ? 0 : p1;
  param1_ = p1;
  p1 = (kMaxFreq-kMinFreq) * pow( param1_, 4) + kMinFreq;
  //sets the rate of the chorusing
  rate_hz_ = 6.2831853 / (1.0 * sample_rate_) * p1;
  
  p2 = p2 > 1 ? 1 : p2;
  param2_ = p2 < 0? 0 : p2;
  depth_ = kMaxDelay * param2_;
  
}








/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
Delay::Delay(int sample_rate, double p1, double p2){
  sample_rate_ = sample_rate;
  
  param1_ = p1; 
  param2_ = p2;
  buffer_size_ = ceil(sample_rate_ * param1_);
  //Makes an empty buffer
  buffer_ = new float[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
  buf_write_ = 0;
}

Delay::~Delay(){
  delete[] buffer_;
}
// Processes a single sample in the unit generator
double Delay::tick(double in){
  double buf_read = fmod(buf_write_ - sample_rate_ * param1_+ buffer_size_, buffer_size_);
  double read_sample = interpolate(buffer_, buffer_size_, buf_read);
  buffer_[buf_write_] = in + param2_ * read_sample;
  double out =  in + read_sample;
  ++buf_write_;
  buf_write_ %= buffer_size_;
  return out;
}

void Delay::set_params(double p1, double p2){
  p1 = p1 > 2 ? 2 : p1;
  p1 = p1 < 0 ? 0 : p1;
  
  //Reallocates delay buffer
  if (p1!=param1_){
    param1_ = p1;
    //delay must be at least kShortestDelay samples
    int new_buffer_size = ceil(sample_rate_ * param1_);
    new_buffer_size = new_buffer_size < kShortestDelay ? kShortestDelay : new_buffer_size;
    //makes the new buffer
    float *new_buffer = new float[new_buffer_size];
    for (int i = 0; i < new_buffer_size; ++i) new_buffer[i] = 0;
    
    buf_write_ = 0; // starts back at the beginning
    double old_sample, fadeout = 1;
    int last = std::min(new_buffer_size, buffer_size_);
    
    for (int i = 0; i < last; ++i) {
      if (buffer_size_ - i < kShortestDelay){
        fadeout = (buffer_size_-i)/(1.0*kShortestDelay);
      } 
      old_sample = buffer_[(buf_write_ - i + buffer_size_) % buffer_size_]; 
      new_buffer[new_buffer_size-i-1] = fadeout * old_sample; 
    }
    
    float *trash_buffer = buffer_;
    //The order change should solve some concurrency issues (may not be necessary).
    //if (buffer_size_>new_buffer_size){
      buffer_size_ = new_buffer_size;
      buffer_ = new_buffer; 
    //}
    //else {
    //  buffer_ = new_buffer; 
    //  buffer_size_ = new_buffer_size;
    //}
    delete[] trash_buffer;
  }
  
  p2 = p2 > 1 ? 1 : p2;
  param2_ = p2 < 0 ? 0 : p2;
  
}








/*
The distortion effect clips the input to a speficied level
  param1 = pre clip gain
  param2 = clipping level
*/
Distortion::Distortion(double p1, double p2){
  set_params(p1, p2);
}
Distortion::~Distortion(){}
// Processes a single sample in the unit generator
double Distortion::tick(double in){
  return param2_ * atan(param1_*in);
}  










/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per second
  param2 = number of beats
*/
Looper::Looper(int sample_rate, double param1, double param2){
  //declare float buffer
  sample_rate_ = sample_rate;
  
  param1_= static_cast<int>(param1); 
  param2_ = static_cast<int>(param2);
  buffer_size_ = ceil(60* sample_rate_ * param2_ / param1_);
  //Makes two empty buffers
  buffer_ = new float[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) {
    buffer_[i] = 0;
  }
  
  buf_write_ =0;
  buf_read_ = 0;
  this_beat_ = 0;
  buf_write_ = 0;
  beat_count_ = 0;
  counting_down_ = false;
  is_recording_ = false;
  has_recording_ = false;
}
Looper::~Looper(){
  //destroy float buffer
  delete[] buffer_;
}
// Processes a single sample in the unit generator
double Looper::tick(double in){
  //Keeps track of beats
  ++beat_count_;
  if (beat_count_ > 60 * sample_rate_ / param1_) {
    pulse();
    beat_count_ = 0;
  }
  //Stores the current input
  if (is_recording_){
    buffer_[buf_write_] = in;
    ++buf_write_;
  }
  //Plays back the recording
  else if (has_recording_){
    double fadeout = 1;
    //Fades near the edges
    if (buf_read_ < 10){
      fadeout = buf_read_/10.0;
    }
    if (buffer_size_ - buf_read_ < 10){
      fadeout = (buffer_size_ - buf_read_)/10.0;
    }
    double out =  buffer_[buf_read_] * fadeout;
    ++buf_read_;
    buf_read_ %= buffer_size_;
    return out;
  }
  return 0;
}

void Looper::set_params(double a, double b){
  printf("Loop cannot change parameters. Create new instance.");
}

void Looper::pulse(){
  printf("Pulse! %d\n",this_beat_);
  if (counting_down_){
    this_beat_ = (this_beat_ - 1);
    if (this_beat_ < 0){
      this_beat_ = 0;
      start_recording();
    } 
  }
  else {
    this_beat_ = (this_beat_+1);//count either way
    //tells it when to stop
    if (this_beat_ == static_cast<int>(param2_)){
      this_beat_ = 0;
      if (is_recording_){
        stop_recording();
      }
    }
  }
}

// Starts counting down beats until recording starts 
void Looper::start_countdown(){
  printf("Counting Down! \n");
  this_beat_ = 4;
  beat_count_ = 0;
  is_recording_ = false;
  has_recording_ = false;
  counting_down_ = true;
}

// Cue Loop to start playing
void Looper::start_recording(){
  printf("Recording! \n");
  this_beat_ = 0;
  buf_write_ = 0;
  is_recording_ = true;
  has_recording_ = false;
  counting_down_ = false;
}

// Cue Loop to start playing
void Looper::stop_recording(){
  printf("Playing Back! \n");
  this_beat_ = 0;
  buf_read_ = 0;
  is_recording_ = false;
  has_recording_ = true;
  counting_down_ = false;
}










/*
The reverb effect convolves the signal with an impulse response
  param1 = room size
  param2 = damping
*/
const int Reverb::kCombDelays[] = {1116,1188,1356,1277,1422,1491,1617,1557};
const int Reverb::kAllPassDelays[] = {225, 556, 441, 341};

Reverb::Reverb(double p1, double p2){
  param1_ = p1;
  param2_ = p2;
  // Comb filtering
  fb_ = new FilterBank();
  for (int i = 0; i < 8; ++i){
    FilteredFeedbackCombFilter *k = new FilteredFeedbackCombFilter(kCombDelays[i], param1_, param2_);
    fb_->add_filter(k);  
  }
  // Allpass filtering  
  for (int i = 0; i < 4; ++i){
    aaf_.push_back(new AllpassApproximationFilter(kAllPassDelays[i], 0.5));
  }  
}

Reverb::~Reverb(){
  std::list<AllpassApproximationFilter *>::iterator it;
  it = aaf_.begin();
  //Deletes all filters
  while (aaf_.size() > 0 && it != aaf_.end()) {
    delete (*it);
    ++it;
  }
  delete fb_;
}
// Processes a single sample in the unit generator
double Reverb::tick(double in){
  complex sample = fb_->tick(.125*in);
  
  //Ticks each allpass
  std::list<AllpassApproximationFilter *>::iterator it;
  it = aaf_.begin();
  while (aaf_.size() > 0 && it != aaf_.end()) {
    sample = (*it)->tick(sample);
    ++it;
  }
  
  return sample.re();
}  

void Reverb::set_params(double p1, double p2){
  p1 = p1 > 1 ? 1 : p1;
  p1 = p1 < 0 ? 0 : p1;
  param1_ = p1;
  
  p2 = p2 > 1 ? 1 : p2;
  p2 = p2 < 0 ? 0 : p2;
  param2_ = p2;
  
  int i = 0;
  std::list<DigitalFilter *>::iterator it;
  it = fb_->filters_.begin();
  while (fb_->filters_.size() > 0 && it != fb_->filters_.end()) {
    FilteredFeedbackCombFilter *apf = static_cast<FilteredFeedbackCombFilter *>(*it);
    apf->change_parameters(kCombDelays[i++], param1_, param2_);
    apf->sp_->change_parameters(param2_, 1 - param2_, 1.0);
    ++it;
  }
}











/*
The Tremolo effect modulates the amplitude of the signal
  param1 = rate
  param2 = depth
*/
Tremolo::Tremolo(int sample_rate, double p1, double p2){
  sample_rate_ = sample_rate;
  set_params(p1, p2);
  sample_count_ = 0;
  mix_ = .5;
}
Tremolo::~Tremolo(){}

double Tremolo::tick(double in){
  double out = in * ((1-param2_) + (param2_)*sin(rate_hz_ * sample_count_));
  //Wrap variables to prevent out-of-bounds/overflow
  ++sample_count_;
  if (rate_hz_ * sample_count_ > 6.2831853) {
    sample_count_ = fmod(sample_count_, 6.2831853/rate_hz_);
  }
  return out;
}
// restricts parameters to range (0,1) and calculates other params
void Tremolo::set_params(double p1, double p2){
  p1 = p1 > 1 ? 1 : p1;
  p1 = p1 < 0 ? 0 : p1;
  param1_ = p1;
  p1 = (kMaxFreq-kMinFreq) * pow( param1_, 4) + kMinFreq;
  //sets the rate of the tremolo
  rate_hz_ = 6.2831853 / (1.0 * sample_rate_) * p1;
  
  p2 = p2 > 1 ? 1 : p2;
  param2_ = p2 < 0? 0 : p2;
  
}