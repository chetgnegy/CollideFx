/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UnitGenerator.cpp
  This file the UnitGenerator class. This class is used as modules 
  for input and effects.

*/

#include "UnitGenerator.h"
#include <iostream>//remove
// Allows user to set the generic parameters, parameters are forced to be positive
void UnitGenerator::set_params(double p1, double p2){
  param1_ = p1>=0 ? p1 : 0;
  param2_ = p2>=0 ? p2 : 0;
}

double UnitGenerator::interpolate(double *array, int length, double index){
  int trunc_index = floor(index);
  double leftover = index-trunc_index;
  double sample_one = array[(trunc_index + length)%length];
  double sample_two = array[(trunc_index + length + 1)%length];
  return sample_one*(1-leftover) + sample_two*leftover;
}


/*
The bitcrusher effect quantizes and downsamples the input 
  param1 = bits (casted to an int)
  param2 = downsampling factor
*/
BitCrusher::BitCrusher(){
  set_params(1, 1);
  
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
  p1 = floor(p1); // bounded by 1 and 16
  p1 = p1 > 16 ? 16 : p1;
  param1_ = p1 < 1 ? 1 : p1;
  
  p2 = floor(p2); // bounded by 1 and 16
  p2 = p2 > 16 ? 16 : p2;
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
Chorus::Chorus(int sample_rate){
  set_params(0,0);
  
  sample_rate_ = sample_rate;
  buffer_size_ = ceil((kMaxDelay + kDelayCenter)*sample_rate_);
  
  //Makes an empty buffer
  buffer_ = new double[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
  
  sample_count_ = 0;
  buf_write_ = 0;
  mix_ = .5;
}
Chorus::~Chorus(){
  delete[] buffer_;
}
// Processes a single sample in the unit generator
// Jon Dattorro - Part 2: Delay-Line Modulation and Chorus 
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart2.pdf
double Chorus::tick(double in){
  double blend = 1.0;//1.0;
  double feedback = 0.7071;
  double feedforward = 0.7071;
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
  
  rate_hz_ = 6.2831853 / sample_rate_ * p1;
  
  p2 = p2 > 1 ? 1 : p2;
  param2_ = p2 < 0? 0 : p2;
  depth_ = kMaxDelay * param2_;
  
}

/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
Delay::Delay(int sample_rate){
  sample_rate_ = sample_rate;
  buffer_size_ = ceil(sample_rate_ * param1_);
  //Makes an empty buffer
  buffer_ = new float[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
}

Delay::~Delay(){
  delete[] buffer_;
}
// Processes a single sample in the unit generator
double Delay::tick(double in){
  return in;
}

void Delay::set_params(double p1, double p2){
  p1 = p1 > 2 ? 2 : p1;
  p1 = p1 < 0 ? 0 : p1;
  if (p1!=param1_){
    param1_ = p1;
    int new_buffer_size = ceil(sample_rate_ * param1_);
  
    float *new_buffer;
    new_buffer = new float[new_buffer_size];
    for (int i = 0; i < new_buffer_size; ++i) new_buffer[new_buffer_size-i-1] = buffer_[(buf_write_-i+buffer_size_)%buffer_size_]; 
    delete[] buffer_;
    buffer_ = new_buffer;
    buffer_size_ = new_buffer_size;
  }
  
  p2 = p2 > 1 ? 1 : p2;
  param2_ = p2 < 0? 0 : p2;
  
}





/*
The distortion effect clips the input to a speficied level
  param1 = pre clip gain
  param2 = clipping level
*/
Distortion::Distortion(){}
Distortion::~Distortion(){}
// Processes a single sample in the unit generator
double Distortion::tick(double in){
  return in;
}  


/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per second
  param2 = number of beats
*/
Looper::Looper(){
  //declare float buffer
}
Looper::~Looper(){
  //destroy float buffer
}
// Processes a single sample in the unit generator
double Looper::tick(double in){
  return in;
}

/*
The reverb effect convolves the signal with an impulse response
  param1 = 
  param2 = 
*/
Reverb::Reverb(){}
Reverb::~Reverb(){}
// Processes a single sample in the unit generator
double Reverb::tick(double in){
  return in;
}  

