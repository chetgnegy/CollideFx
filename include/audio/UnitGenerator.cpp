/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UnitGenerator.cpp
  This file the UnitGenerator class. This class is used as modules 
  for input and effects.

*/

#include "UnitGenerator.h"

// Default values
int UnitGenerator::sample_rate = 44100;
int UnitGenerator::buffer_length = 512;

float interpolate(float *array, int length, double index);
double interpolate(double *array, int length, double index);


// #--------------Unit Generator Base Classes ----------------#

void UnitGenerator::set_audio_settings(int bl, int sr){
  UnitGenerator::buffer_length = bl;
  UnitGenerator::sample_rate = sr;
  DigitalFilter::set_sample_rate(sr);
}

// Allows user to set the generic parameters, bounds must already be set
void UnitGenerator::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
}

// Allows entire buffers to be processed at once
double *UnitGenerator::process_buffer(double *buffer, int length){
  if (length != ugen_buffer_size_) printf("Buffer size mismatch! Input: %d  internal: %d\n", length, ugen_buffer_size_);
    for (int i = 0; i < length; ++i){
      ugen_buffer_[i] = tick(buffer[i]);  
  }
  return ugen_buffer_;
}

// The absolute average of the samples in the buffer. Used to 
// calculate brightness
double UnitGenerator::buffer_energy(){
  double sum = 0;
  for (int i = 0; i < ugen_buffer_size_; ++i){
    sum += fabs(ugen_buffer_[i]);
  }
  return sum / (1.0 * ugen_buffer_size_);
}

// Gets the FFT of the unit generator's current buffer
void UnitGenerator::buffer_fft(int full_length, complex *out){
  if (full_length != ugen_buffer_size_) printf("FFT Buffer size mismatch! Input: %d  internal: %d\n", full_length, ugen_buffer_size_);
  complex *complex_arr_ = new complex[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; ++i){
    complex_arr_[i] = ugen_buffer_[i];
  }
  CFFT::Forward(complex_arr_, out, full_length);
    for (int i = 0; i < ugen_buffer_size_; ++i){
    out[i] = out[i] * 0.5 * (1 + cos(6.2831853 * i/(ugen_buffer_size_-1)));
  }
  delete[] complex_arr_;
}


// Scales the input to the range 0 - 1, requires maximum 
// and minimum parameters to be set
double UnitGenerator::get_normalized_param(int param){
if (param == 1){
    return (param1_-min_param1_)/(max_param1_ - min_param1_);
  }
  return (param2_-min_param2_)/(max_param2_ - min_param2_);
}

// Sets the input using the range 0 - 1, requires maximum 
// and minimum parameters to be set
void UnitGenerator::set_normalized_param(double param1, double param2){
  set_params(
    (1-param1)*min_param1_+(param1)*max_param1_,
    (1-param2)*min_param2_+(param2)*max_param2_);
}

// Sets the bounds on the parameters of the ugen
void UnitGenerator::set_limits(double min1, double max1, double min2, double max2){
  min_param1_ = min1;
  min_param2_ = min2;
  max_param1_ = max1;
  max_param2_ = max2;
}

// Uses the specified minimum and maximum bounds to restrict parameter to
// valid range
double UnitGenerator::clamp(double param_in, int which){
  double min_param, max_param;
  if (which == 1){max_param = max_param1_;min_param = min_param1_;}
  else{max_param = max_param2_;min_param = min_param2_;}
  param_in = param_in > max_param ? max_param : param_in;
  return param_in < min_param ? min_param : param_in;
}

const char *UnitGenerator::report_param(int which){
  std::stringstream s;
  if (which == 1 & report_param1_ != NULL) {
    if ((int)*report_param1_ == *report_param1_)
      sprintf(param1_str_, "%d",(int)(*report_param1_));
    else
      sprintf(param1_str_, "%.3f",*report_param1_);
    return param1_str_;
    
  }
  else if (report_param2_ != NULL) {
    if ((int)(*report_param2_) == *report_param2_)
      sprintf(param2_str_, "%d",(int)(*report_param2_));
    else
      sprintf(param2_str_, "%.3f",*report_param2_);
    return param2_str_;
    
  }
  return "";
}

void UnitGenerator::define_printouts(double *report_param1, const char *p1_units, 
                        double *report_param2, const char *p2_units){
  report_param1_ = report_param1;
  report_param2_ = report_param2;
  param1_units_ = p1_units;
  param2_units_ = p2_units;
}
// #------------ Midi Unit Generator Classes --------------#



// Adds a single note to the instruent's play list
void MidiUnitGenerator::play_note(int MIDI_Pitch, int velocity){
  myCW_->play_note(MIDI_Pitch, velocity);
}

// Searches for a note of the same pitch and stops it.
void MidiUnitGenerator::stop_note(int MIDI_Pitch){
  myCW_->stop_note(MIDI_Pitch);
}

// Gets the next sample from the instrument
double MidiUnitGenerator::tick(){
  return myCW_->tick();
}

UGenState *MidiUnitGenerator::save_state(){
  MidiInputState *s = new MidiInputState();
  s->buffer_length_ = ugen_buffer_size_;
  s->buffer_ = new double[s->buffer_length_];
  for (int i = 0; i < s->buffer_length_; ++i){
    s->buffer_[i] = ugen_buffer_[i];
  }
  return s;
}
void MidiUnitGenerator::recall_state(UGenState *state){
  MidiInputState *s = static_cast<MidiInputState *>(state);
  if (s->buffer_length_ == ugen_buffer_size_){
    for (int i = 0; i < s->buffer_length_; ++i){
    ugen_buffer_[i] = s->buffer_[i];
    }
  }
  else printf("Mismatched buffer size (MidiUnitGenerator::Recall_State)\n");
  delete state;
}

MidiInputState::~MidiInputState(){delete[] buffer_;}

// #------------Unit Generator Inherited Classes --------------#

Input::Input(){
    name_ = "Input";
    param1_name_ = "Volume";
    param2_name_ = "Not Used";
    set_limits(0, 10, 0, 1);
    set_params(1, 0);
    define_printouts(&param1_, "", NULL, "");
    ugen_buffer_size_ = UnitGenerator::buffer_length;
    ugen_buffer_ = new double[ugen_buffer_size_];
    for (int i = 0; i < ugen_buffer_size_; i++){
      ugen_buffer_[i] = 0;
    }

    current_index_ = 0;
    current_value_ = 0;
  }
Input::~Input(){}

// Pulls the current sample out of the buffer and advances the read index
double Input::tick(double in){ 
  double out = ugen_buffer_[current_index_];
  current_index_ = (current_index_ + 1) % ugen_buffer_size_;
  return param1_*out; 
}  

// Sets the sample at the current index in the buffer
void Input::set_sample(double val){ 
  ugen_buffer_[current_index_] = val; 
}

// Sets the entire buffer
void Input::set_buffer(double buffer[], int length){ 
  if (length != ugen_buffer_size_) {
    printf("Resizing input buffer\n");
    ugen_buffer_size_ = length;
    delete[] ugen_buffer_;
    ugen_buffer_ = new double[ugen_buffer_size_];
  }
  for (int i = 0; i < length; ++i){
    ugen_buffer_[i] = buffer[i];
  }
  // Reset read buffer 
  current_index_ = 0;
}

UGenState* Input::save_state(){
  InputState *s = new InputState();
  s->buffer_length_ = ugen_buffer_size_;
  s->buffer_ = new double[s->buffer_length_];
  for (int i = 0; i < s->buffer_length_; ++i){
    s->buffer_[i] = ugen_buffer_[i];
  }
  return s;
}

void Input::recall_state(UGenState *state){
  InputState *s = static_cast<InputState *>(state);
  if (s->buffer_length_ == ugen_buffer_size_){
    for (int i = 0; i < s->buffer_length_; ++i){
      ugen_buffer_[i] = s->buffer_[i];
    }
  }
  else printf("Mismatched buffer size (Input::Recall_State)\n");
  delete state;
}

InputState::~InputState(){delete[] buffer_;}



/*
The sine wave listens to the midi controller
param1 = attack (seconds)
param2 = sustain (seconds)
*/
Sine::Sine(double p1, double p2){
  name_ = "Sine";
  param1_name_ = "Attack";
  param2_name_ = "Sustain";
  myCW_ = new ClassicWaveform("sine", UnitGenerator::sample_rate);
  set_limits(0.05, 10, 0.05, 10);
  define_printouts(&param1_, "s", &param2_, "s");
  set_params(p1, p2);
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}

Sine::~Sine(){
  delete myCW_;
}

// casts the parameters to ints and restricts them to a certain value
void Sine::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  myCW_->set_attack(param1_);
  myCW_->set_sustain(param2_);
}








/*
The square wave listens to the midi controller
param1 = attack
param2 = sustain
*/
Square::Square(double p1, double p2){
  name_ = "Square";
  param1_name_ = "Attack";
  param2_name_ = "Sustain";
  myCW_ = new ClassicWaveform("square", UnitGenerator::sample_rate);
  set_limits(0.05, 10, 0.05, 10);
  set_params(p1, p2);
  define_printouts(&param1_, "s", &param2_, "s");
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}
Square::~Square(){
  delete myCW_;
}
// Processes a single sample in the unit generator
// casts the parameters to ints and restricts them to a certain value
void Square::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  myCW_->set_attack(param1_);
  myCW_->set_sustain(param2_);
}








/*
The tri wave listens to the midi controller
param1 = attack
param2 = sustain
*/
Tri::Tri(double p1, double p2){
  name_ = "Tri";
  param1_name_ = "Attack";
  param2_name_ = "Sustain";
  myCW_ = new ClassicWaveform("tri", UnitGenerator::sample_rate);
  set_limits(0.05, 10, 0.05, 10);
  set_params(p1, p2);
  define_printouts(&param1_, "s", &param2_, "s");
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}
Tri::~Tri(){
  delete myCW_;
}
// casts the parameters to ints and restricts them to a certain value
void Tri::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  myCW_->set_attack(param1_);
  myCW_->set_sustain(param2_);
}







/*
The saw wave listens to the midi controller
param1 = attack
param2 = sustain
*/
Saw::Saw(double p1, double p2){
  name_ = "Saw";
  param1_name_ = "Attack";
  param2_name_ = "Sustain";
  myCW_ = new ClassicWaveform("saw", UnitGenerator::sample_rate);
  set_limits(0.05, 10, 0.05, 10);
  set_params(p1, p2);
  define_printouts(&param1_, "s", &param2_, "s");
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}
Saw::~Saw(){
  delete myCW_;
}
// casts the parameters to ints and restricts them to a certain value
void Saw::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  myCW_->set_attack(param1_);
  myCW_->set_sustain(param2_);
}








/*
The bitcrusher effect quantizes and downsamples the input 
  param1 = bits (casted to an int)
  param2 = downsampling factor
*/
BitCrusher::BitCrusher(int p1, int p2){
  name_ = "BitCrusher";
  param1_name_ = "Bits";
  param2_name_ = "Downsampling Factor";
  set_limits(2, 16, 1, 16);
  set_params(p1, p2); 
  define_printouts(&param1_, "", &param2_, "x");
  sample_ = 0;
  sample_count_ = 0;
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
  
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
  param1_ = clamp(floor(p1), 1);
  param2_ = clamp(floor(p2), 2);
}

//Quantizes the double to the number of bits specified by param1
double BitCrusher::quantize(double in){
  if (param1_ == 1) return in;
  return round(in * pow(2.0, param1_)) / pow(2.0, param1_);
}


UGenState* BitCrusher::save_state(){
  BitCrusherState *s = new BitCrusherState();
  s->sample_count_ = sample_count_;
  s->sample_ = sample_;
  return s;
}
void BitCrusher::recall_state(UGenState *state){
  BitCrusherState *s = static_cast<BitCrusherState *>(state);
  sample_count_ = s->sample_count_;
  sample_ = s->sample_;
  delete state;
}





/*
The chorus effect delays the signal by a variable amount
  param1 = rate of the chorus LFO
  param2 = depth of the chorus effect
*/
Chorus::Chorus(double p1, double p2){
  name_ = "Chorus";
  param1_name_ = "Rate";
  param2_name_ = "Depth";
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(0, 1, 0, 1);
  set_params(p1, p2);
  define_printouts(&report_hz_, "Hz", &param2_, "");
  buffer_size_ = ceil((kMaxDelay + kDelayCenter)*sample_rate_);
  
  //Makes an empty buffer
  buffer_ = new double[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
  
  sample_count_ = 0;
  buf_write_ = 0;

  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
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
// restricts parameters to range (0,1) and calculates other parameters,
// including the rate in Hz and the max delay change
void Chorus::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  p1 = (kMaxFreq-kMinFreq) * pow( param1_, 4) + kMinFreq;
  //sets the rate of the chorusing
  report_hz_ = p1;
  rate_hz_ = 6.2831853 / (1.0 * sample_rate_) * p1;
   
  param2_ = clamp(p2, 2);
  depth_ = kMaxDelay * param2_;
  
}

UGenState* Chorus::save_state(){
  ChorusState *s = new ChorusState();
  s->buf_write_ = buf_write_;
  s->buffer_size_ = buffer_size_;
  s->sample_count_ = sample_count_;
  s->buffer_ = new double[s->buffer_size_];
  for (int i = 0; i < s->buffer_size_; ++i){
    s->buffer_[i] = buffer_[i];
  }
  return s;
}
void Chorus::recall_state(UGenState *state){
  ChorusState *s = static_cast<ChorusState *>(state);
  if (buffer_size_ == s->buffer_size_){
    buf_write_ = s->buf_write_;
    buffer_size_ = s->buffer_size_;
    sample_count_ = s->sample_count_;
    for (int i = 0; i < s->buffer_size_; ++i){
      buffer_[i] = s->buffer_[i];
    }
  } else { printf("Mismatched buffer size (Chorus::Recall_State)\n"); }
  delete state;
}





/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
Delay::Delay(double p1, double p2){
  name_ = "Delay";
  param1_name_ = "Time";
  param2_name_ = "Feedback";
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(0.01, 2, 0, 1);
  define_printouts(&param1_, "s", &param2_, "");
  
  param1_ = p1; 
  param2_ = p2;
  max_buffer_size_= ceil(sample_rate_ * max_param1_);
  buffer_size_ = sample_rate_ * param1_;
  //Makes an empty buffer
  buffer_ = new float[max_buffer_size_];
  for (int i = 0; i < max_buffer_size_; ++i) buffer_[i] = 0;
  buf_write_ = 0;

  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }

}

Delay::~Delay(){
  delete[] buffer_;
}
// Processes a single sample in the unit generator
double Delay::tick(double in){
  float buf_read = fmod(buf_write_ - buffer_size_ + max_buffer_size_, max_buffer_size_);
  float read_sample = interpolate(buffer_, max_buffer_size_, buf_read);
  buffer_[buf_write_] = in + param2_ * read_sample;
  double out =  in + read_sample;
  ++buf_write_;
  buf_write_ %= max_buffer_size_;
  return out;
}

void Delay::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  //Reallocates delay buffer
  buffer_size_ = ceil(sample_rate_ * param1_);
  param2_ = clamp(p2, 2);
  
}


UGenState* Delay::save_state(){
  DelayState *s = new DelayState();
  s->buf_write_ = buf_write_;
  s->buffer_size_ = buffer_size_;
  s->buffer_ = new float[s->buffer_size_];
  for (int i = 0; i < s->buffer_size_; ++i){
    s->buffer_[i] = buffer_[i];
  }
  return s;
}
void Delay::recall_state(UGenState *state){
  DelayState *s = static_cast<DelayState *>(state);
  if (buffer_size_ == s->buffer_size_){
    buf_write_ = s->buf_write_;
    buffer_size_ = s->buffer_size_;
    for (int i = 0; i < s->buffer_size_; ++i){
      buffer_[i] = s->buffer_[i];
    }
  } else { printf("Mismatched buffer size (Delay::Recall_State)\n"); }
  delete state;
}









/*
The distortion effect clips the input to a specified level
  param1 = pre clip gain
  param2 = clipping level
*/
Distortion::Distortion(double p1, double p2){
  name_ = "Distortion";
  param1_name_ = "Pre-gain";
  param2_name_ = "Post-gain";
  set_limits(1, 100, 0, 5);
  set_params(p1, p2);
  define_printouts(&param1_, "", &param2_, "");


  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
  f_ = new DigitalLowpassFilter(1000,1,1); 
  f_->calculate_coefficients();
  inv_ = f_->create_inverse();
}
Distortion::~Distortion(){
  delete f_;
  delete inv_;
}
// Processes a single sample in the unit generator
double Distortion::tick(double in){
  double gain_adj = f_->dc_gain();
  double offset = 0.00;//4;
  //in = f_->tick(in).re() / gain_adj;
  in = param1_ * in + offset;
  double out = 0;
  // Cubic transfer function
  if (in > 1) out = 2/3.0;
  else if (in < -1) out = -2/3.0;
  else out = in - pow(in, 3)/3.0;
  out -= offset - pow(offset, 3)/3.0;
  //out = inv_->tick(out).re() * gain_adj;
  return param2_ * out;
}  

UGenState* Distortion::save_state(){
  DistortionState *s = new DistortionState();
  s->f_state_ = f_->get_state();
  s->inv_state_ = inv_->get_state();
  return s;
}
void Distortion::recall_state(UGenState *state){
  DistortionState *s = static_cast<DistortionState *>(state);
  f_->set_state(s->f_state_);
  inv_->set_state(s->inv_state_);
  delete state;
}







/*
A second order high or low pass filter
  param1 = cutoff frequency
  param2 = Q
*/

Filter::Filter(double p1, double p2){
  name_ = "Filter";
  param1_name_ = "Cutoff Frequency";
  param2_name_ = "Q";
  set_limits(100, 10000, 1, 10);
  define_printouts(&param1_, "Hz", &param2_, "");
  
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  f_ = new DigitalLowpassFilter(param1_, param2_, 1);
  f_->calculate_coefficients();
  f2_ = new DigitalLowpassFilter(param1_, param2_, 1);
  f2_->calculate_coefficients();
  currently_lowpass_ = true;

  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}


Filter::~Filter(){
  delete f_;
  delete f2_;
}

// Processes a single sample in the unit generator
double Filter::tick(double in){
  double factor = 1;
  if (currently_lowpass_)
    factor = 1/f_->dc_gain();
  else
    factor = 1/f_->hf_gain();
  return factor * f2_->tick(factor * f_->tick(in)).re();
}

// Tells the filter to change parameters
void Filter::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  f_->change_parameters(param1_, param2_, 1);
  f2_->change_parameters(param1_, param2_, 1);
}

// The filter can be either high or low pass.
// True for lowpass, False for highpass
void Filter::set_lowpass(bool lowpass){
  if (lowpass && !currently_lowpass_){
    delete f_;
    delete f2_;
    f_ = new DigitalLowpassFilter(param1_, param2_, 1);
    f2_ = new DigitalLowpassFilter(param1_, param2_, 1);
    f_->calculate_coefficients();
    f2_->calculate_coefficients();
    currently_lowpass_ = !currently_lowpass_;
  }
  else if (!lowpass && currently_lowpass_){
    delete f_;
    delete f2_;
    f_ = new DigitalHighpassFilter(param1_, param2_, 1);
    f2_ = new DigitalHighpassFilter(param1_, param2_, 1);
    f_->calculate_coefficients();
    f2_->calculate_coefficients();
    currently_lowpass_ = !currently_lowpass_;
  }

}

UGenState* Filter::save_state(){
  FilterState *s = new FilterState();
  s->f_state_ = f_->get_state();
  s->currently_lowpass_ = currently_lowpass_;
  return s;
}
void Filter::recall_state(UGenState *state){
  FilterState *s = static_cast<FilterState *>(state);
  f_->set_state(s->f_state_);
  currently_lowpass_ = s->currently_lowpass_;
  delete state;
}






/*
A second order bandpass filter
param1 = cutoff frequency
param2 = Q
*/

Bandpass::Bandpass(double p1, double p2){
  name_ = "Bandpass";
  param1_name_ = "Cutoff Frequency";
  param2_name_ = "Q";
  set_limits(100, 10000, 1, 10);
  define_printouts(&param1_, "Hz", &param2_, "");
  
  param1_ = p1;
  param2_ = p2;

  f_ = new DigitalBandpassFilter(param1_, param2_, 1);
  f_->calculate_coefficients();
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}

Bandpass::~Bandpass(){}
// Processes a single sample in the unit generator
double Bandpass::tick(double in){
  return f_->tick(in).re();
}
void Bandpass::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  f_->change_parameters(param1_, param2_, 1);
}

UGenState* Bandpass::save_state(){
  BandpassState *s = new BandpassState();
  s->f_state_ = f_->get_state();
  return s;
}
void Bandpass::recall_state(UGenState *state){
  BandpassState *s = static_cast<BandpassState *>(state);
  f_->set_state(s->f_state_);
  delete state;
}



/*
The delay effect plays the signal back some time later
  param1 = time in seconds until delay repeats
  param2 = amount of feedback in delay buffer
*/
Granular::Granular(double p1, double p2){
  name_ = "Granular";
  param1_name_ = "Granule Length";
  param2_name_ = "Density";
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(30, 10000, 0.01, 1);
  define_printouts(&param1_, "samples", &param2_, "");
  
  param1_ = p1; 
  param2_ = p2;
  buffer_size_= ceil(sample_rate_);
  //Makes an empty buffer
  buffer_ = new double[buffer_size_];
  for (int i = 0; i < buffer_size_; ++i) buffer_[i] = 0;
  buf_write_ = 0;
  
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }

}

Granular::~Granular(){
  delete[] buffer_;
}

// Processes a single sample in the unit generator
double Granular::tick(double in){
  buffer_[buf_write_] = in;
  if (rand()%static_cast<int>(1000*(1.01-param2_)) == 0 && granules_.size() < 10){
    Granule g;
    g.win_length = static_cast<int>(param1_);
    g.start = buf_write_ - (rand() % (buffer_size_ - g.win_length) - g.win_length);
    g.start = (g.start + buffer_size_)%buffer_size_;
    g.end = (g.start + g.win_length +
                    buffer_size_) % buffer_size_;
    g.at = 0;
    //std::cout << g.start << " " << g.end << " " << buf_write_ << std::endl;
    granules_.push_back(g);
  }
    
  double sum = 0;
  double window,window_length;
  int this_sample;
  std::vector<Granule>::iterator it = granules_.begin();
  while (it != granules_.end()){
    this_sample = (it->start + it->at)%buffer_size_;
    if (it->end == (it->start + it->at)%buffer_size_ ){
      it = granules_.erase(it);
    }
    else{
      window = 0.5 * (1 - cos(6.2831853 * (++(it->at))/it->win_length));
      sum += buffer_[this_sample] * window;
      ++it;
      
     
    }
  }
  ++buf_write_;
  buf_write_ %= buffer_size_;
  
  return sum;
}

void Granular::set_params(double p1, double p2){
  param1_ = clamp(round(p1), 1);
  param2_ = clamp(p2, 2);
  
}

UGenState* Granular::save_state(){
  GranularState *s = new GranularState();
  s->buf_write_ = buf_write_;
  s->buffer_size_ = buffer_size_;
  s->granules_ = granules_;
  s->buffer_ = new double[s->buffer_size_];
  for (int i = 0; i < s->buffer_size_; ++i){
    s->buffer_[i] = buffer_[i];
  }
  return s;
}
void Granular::recall_state(UGenState *state){
  GranularState *s = static_cast<GranularState *>(state);
  if (buffer_size_ == s->buffer_size_){
    buf_write_ = s->buf_write_;
    buffer_size_ = s->buffer_size_;
    granules_ = s->granules_;
    for (int i = 0; i < s->buffer_size_; ++i){
      buffer_[i] = s->buffer_[i];
    }
  } else { printf("Mismatched buffer size (Granular::Recall_State)\n"); }
  delete state;
}






/*
The looper effect keeps a section of the input in a buffer and loops it back
  param1 = beats per minute
  param2 = number of beats
*/
Looper::Looper(){
  name_ = "Looper";
  param1_name_ = "BPM";
  param2_name_ = "Number of Beats";
  //declare float buffer
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(60, 250, 1, 60);
  define_printouts(&param1_, "BPM", &param2_, "Beats");
  
  param1_ = 120;
  param2_ = 16;
  params_set_ = false;
  
  buf_write_ = 0; buf_read_ = 0;
  this_beat_ = 0; beat_count_ = 0;
  start_counter_ = 4;
  // Sets initial state of module
  counting_down_ = false;
  is_recording_ = false;
  has_recording_ = false;
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }

  buffer_ = NULL;

  click_data.first = 0;
  click_data.second = 0;
}
Looper::~Looper(){
  //destroy float buffer
  if (params_set_) delete[] buffer_;
}
// Processes a single sample in the unit generator
double Looper::tick(double in){
  if (!params_set_) return 0;
  //Keeps track of beats
  ++beat_count_;
  if (beat_count_ > 60 * sample_rate_ / param1_) {
    pulse();
    beat_count_ = 0;
    if (counting_down_ || is_recording_)return 1;
    
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
  if (is_recording_ || counting_down_ || has_recording_) return;
  param1_= round(a); 
  param2_ = round(b);
  
}

void Looper::pulse(){
  //printf("Pulse! %d\n",this_beat_);
  if (counting_down_){
    if (pulsefnc!=NULL) pulsefnc(data, this_beat_);
    this_beat_ = (this_beat_ - 1);
    if (this_beat_ < 0){
      this_beat_ = 0;
      start_recording();
    } 
  }
  else {
    this_beat_ = (this_beat_+1);//count either way
    //tells it when to stop
    if ((is_recording_ || has_recording_) 
                        && pulsefnc!=NULL){ 
      pulsefnc(data, -100);
    }

    if (this_beat_ == round(param2_)){
      this_beat_ = 0;
      if (is_recording_){
        
        stop_recording();
        if (pulsefnc!=NULL) pulsefnc(data, -4);
      }
    }
  }
}

// Starts counting down beats until recording starts 
void Looper::start_countdown(){
  if (!params_set_){
    buffer_size_ = ceil(60* sample_rate_ * param2_ / param1_);
    //Makes empty buffer
    buffer_ = new float[buffer_size_];
  }
  for (int i = 0; i < buffer_size_; ++i) {
    buffer_[i] = 0;
  }
  params_set_ = true;

  this_beat_ = start_counter_;
  beat_count_ = 0;
  is_recording_ = false;
  has_recording_ = false;
  counting_down_ = true;
}

// Sets the number of count in beats
void Looper::set_start_counter(int num){
  if (counting_down_) return;
  start_counter_ = num;
}

// Gets the number of count in beats
int Looper::get_start_counter(){
  return start_counter_;
}

// Cue Loop to start playing
void Looper::start_recording(){

  this_beat_ = 0;
  buf_write_ = 0;
  is_recording_ = true;
  has_recording_ = false;
  counting_down_ = false;
}

// Cue Loop to start playing
void Looper::stop_recording(){
  //printf("Playing Back! \n");
  this_beat_ = 0;
  buf_read_ = 0;
  is_recording_ = false;
  has_recording_ = true;
  counting_down_ = false;
}


UGenState* Looper::save_state(){
  LooperState *s = new LooperState();
  s->buf_write_ = buf_write_;
  s->buf_read_ = buf_read_;
  s->buffer_size_ = buffer_size_;
  s->this_beat_ = this_beat_;
  s->beat_count_ = beat_count_;
  s->start_counter_ = start_counter_;
  if (buffer_ != NULL){
    s->buffer_ = new float[s->buffer_size_];
    for (int i = 0; i < s->buffer_size_; ++i){
      s->buffer_[i] = buffer_[i];
    }
  }
  else s->buffer_ = NULL;
  return s;
}



void Looper::recall_state(UGenState *state){
  LooperState *s = static_cast<LooperState *>(state);
  if (buffer_size_ == s->buffer_size_){
    buf_write_ = s->buf_write_;
    buf_read_ = s->buf_read_;
    buffer_size_ = s->buffer_size_;
    this_beat_ = s->this_beat_;
    beat_count_ = s->beat_count_;
    start_counter_ = s->start_counter_;
    if (s->buffer_ != NULL){
      for (int i = 0; i < s->buffer_size_; ++i){
        buffer_[i] = s->buffer_[i];
      }
    }
  } else { printf("Mismatched buffer size (Looper::Recall_State)\n"); }
  
  delete state;
}








/*
A ring modulator. Multiplies the input by a sinusoid
  param1 = frequency
  param2 = Not Used
*/
RingMod::RingMod(double p1, double p2){
  name_ = "RingMod";
  param1_name_ = "Frequency";
  param2_name_ = "Not Used";
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(0, 1, 0, 1);
  set_params(p1, p2);
  define_printouts(&report_hz_, "Hz", NULL, "");
  
  sample_count_ = 0;
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}

RingMod::~RingMod(){}
// Processes a single sample in the unit generator
double RingMod::tick(double in){
  double out = in * sin(rate_hz_ * sample_count_);
  //Wrap variables to prevent out-of-bounds/overflow
  ++sample_count_;
  if (rate_hz_ * sample_count_ > 6.2831853) {
    sample_count_ = fmod(sample_count_, 6.2831853/rate_hz_);
  }
  return out;
}

void RingMod::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  // Non linear scaling
  p1 = (kMaxFreq - kMinFreq) * pow( param1_, 4) + kMinFreq;
  report_hz_ = p1;
  rate_hz_ = 6.2831853 / (1.0 * sample_rate_) * p1;
}

UGenState* RingMod::save_state(){
  RingModState *s = new RingModState();
  s->sample_count_ = sample_count_;
  return s;
}
void RingMod::recall_state(UGenState *state){
  RingModState *s = static_cast<RingModState *>(state);
  sample_count_ = s->sample_count_;
  delete state;

}







/*
The reverb effect convolves the signal with an impulse response
  param1 = room size
  param2 = damping
*/
const int Reverb::kCombDelays[] = {1116,1188,1356,1277,1422,1491,1617,1557};
const int Reverb::kAllPassDelays[] = {225, 556, 441, 341};

Reverb::Reverb(double p1, double p2){
  name_ = "Reverb";
  param1_name_ = "Room Size";
  param2_name_ = "Damping";
  set_limits(0, 1, 0, 1);
  define_printouts(&param1_, "", &param2_, "");
  

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
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }
}

Reverb::~Reverb(){
  std::list<AllpassApproximationFilter *>::iterator it;
  it = aaf_.begin();
  // Deletes all filters
  while (aaf_.size() > 0 && it != aaf_.end()) {
    delete (*it);
    ++it;
  }
  delete fb_;
}

// Processes a single sample in the unit generator
double Reverb::tick(double in){
  // This should probably use 1/sqrt(8), but it's too loud as it is...
  complex sample = fb_->tick(.125*in);
  
  // Ticks each allpass
  std::list<AllpassApproximationFilter *>::iterator it;
  it = aaf_.begin();
  while (aaf_.size() > 0 && it != aaf_.end()) {
    sample = (*it)->tick(sample);
    ++it;
  }
  return sample.re();
}  

void Reverb::set_params(double p1, double p2){
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  
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

UGenState* Reverb::save_state(){
  ReverbState *s = new ReverbState();
  std::list<DigitalFilter *>::iterator it = fb_->filters_.begin();
  int i = 0;
  s->comb_state_ = new DigitalFilterState*[8];
  while (fb_->filters_.size() > 0 && it != fb_->filters_.end()) {  
    s->comb_state_[i++] = (*it)->get_state();
    ++it;
  }

  s->ap_state_ = new DigitalFilterState*[4];
  std::list<AllpassApproximationFilter *>::iterator it2 = aaf_.begin();
  i = 0;
  while (aaf_.size() > 0 && it2 != aaf_.end()) {  
    s->ap_state_[i++] = (*it2)->get_state();
    ++it2;
  }
  return s;

}



void Reverb::recall_state(UGenState *state){
  ReverbState *s = static_cast<ReverbState *>(state);
  std::list<DigitalFilter *>::iterator it = fb_->filters_.begin();
  int i = 0;
  while (fb_->filters_.size() > 0 && it != fb_->filters_.end()) {  
    (*it)->set_state(s->comb_state_[i++]);
    ++it;
  }

  std::list<AllpassApproximationFilter *>::iterator it2 = aaf_.begin();
  i = 0;
  while (aaf_.size() > 0 && it2 != aaf_.end()) {  
    (*it2)->set_state(s->ap_state_[i++]);
    ++it2;
  }

  delete state;
}







/*
The Tremolo effect modulates the amplitude of the signal
  param1 = rate
  param2 = depth
*/
Tremolo::Tremolo(double p1, double p2){
  name_ = "Tremolo";
  param1_name_ = "Rate";
  param2_name_ = "Depth";
  sample_rate_ = UnitGenerator::sample_rate;
  set_limits(0, 1, 0, 1);
  set_params(p1, p2);
  define_printouts(&report_hz_, "Hz", &param2_, "");
  
  sample_count_ = 0;
  ugen_buffer_size_ = UnitGenerator::buffer_length;
  ugen_buffer_ = new double[ugen_buffer_size_];
  for (int i = 0; i < ugen_buffer_size_; i++){
    ugen_buffer_[i] = 0;
  }

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
  param1_ = clamp(p1, 1);
  param2_ = clamp(p2, 2);
  // Non linear scaling
  p1 = (kMaxFreq - kMinFreq) * pow( param1_, 4) + kMinFreq;
  report_hz_ = p1;
  
  //sets the rate of the tremolo
  rate_hz_ = 6.2831853 / (1.0 * sample_rate_) * p1;
}

UGenState* Tremolo::save_state(){
  TremoloState *s = new TremoloState();
  s->sample_count_ = sample_count_;
  return s;
}
void Tremolo::recall_state(UGenState *state){
  TremoloState *s = static_cast<TremoloState *>(state);
  sample_count_ = s->sample_count_;
  delete state;
}

// #-------Useful functions for sound buffer operations --------#




// Gets the interpolated value between two samples of array
double interpolate(double *array, int length, double index){
  int trunc_index = static_cast<int>(floor(index));
  double leftover = index-1.0*trunc_index;
  double sample_one = array[(trunc_index + length)%length];
  double sample_two = array[(trunc_index + length + 1)%length];
  double output = sample_one*(1-leftover) + sample_two*leftover;
  return output;
}

// Gets the interpolated value between two samples of array
float interpolate(float *array, int length, double index){
  int trunc_index = floor(index);
  float leftover = index-trunc_index;
  float sample_one = array[(trunc_index + length)%length];
  float sample_two = array[(trunc_index + length + 1)%length];
  return sample_one*(1-leftover) + sample_two*leftover;
}

