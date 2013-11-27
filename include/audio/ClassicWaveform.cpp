/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  ClassicWaveform.cpp
  This is a library for producing classic waveform 
  sounds (square, triangle, and sawtooth waves)
*/

#include "ClassicWaveform.h"
#include <iostream>//remove

double square(double phase);
double saw(double phase);
double tri(double phase);

ClassicWaveform::ClassicWaveform(const char * type, int sample_rate){
  if (strcmp(type, "sine")==0)  wave_func_ = sin;
  else if (strcmp(type, "square")==0) wave_func_ = square;
  else if (strcmp(type, "saw")==0) wave_func_ = saw;
  else if (strcmp(type, "tri")==0) wave_func_ = tri;
  else printf("Error: Wavetype not found!\n");
  sample_rate_ = sample_rate;

  attack_samples_ = 5000;
  sustain_samples_ = 9000;
  release_samples_ = 5000;
}

ClassicWaveform::~ClassicWaveform(){

}

// We can tick for the next sample for the entire instrument  
double ClassicWaveform::tick(){
  std::list<Note *>::iterator it;
  it = notes_.begin();  
  
  double output = 0;
  mutex_.lock();
    
  while (it != notes_.end()){
    output += next_sample(*it);

    if ((*it)->flag_for_deletion){
      delete *it;
      it = notes_.erase(it);
    }
    else{
      ++it;
    }
  }

  mutex_.unlock();
  return output;
}

// Adds a single note to the instrument
void ClassicWaveform::play_note(int MIDI_pitch, int velocity){
  mutex_.lock();
    
  if (notes_.size() > 16) {  
    std::list<Note *>::iterator it;
    it = notes_.begin();

    while (it != notes_.end()) { // Forces a note to end
      if ((*it)->stage < 3){
        (*it)->stage = 3;
        break;
      }
      ++it;
    } 
  }

  Note *n = new Note();
  // Converts to a frequency
  double freq = pow(2, (MIDI_pitch - 69) / 12.0) * 440.0;
  n->pitch = freq;
  n->velocity = velocity;
  n->stage = 1;
  n->samples = 0;
  n->release_sample = 0;
  n->sustain_end_env = 1;
  n->flag_for_deletion = false;
  notes_.push_back(n);
  mutex_.unlock();
  
}


// Searches for a note of the same pitch and stops it.
bool ClassicWaveform::stop_note(int MIDI_pitch){
  // Converts to a frequency
  double freq = pow(2, (MIDI_pitch - 69) / 12.0) * 440.0;
  
  std::list<Note *>::iterator it;
  it = notes_.begin();
  mutex_.lock();
  
  while ( it != notes_.end() ){
    if ((*it)->pitch == freq && (*it)->stage < 3){
      double env = compute_envelope(*it);
      (*it)->stage = 3;
      
      (*it)->sustain_end_env = env;
      (*it)->release_sample = (*it)->samples;

      mutex_.unlock();
      return true;
    }
    ++it;
    //keep going until you find a note or run out
  } 
  mutex_.unlock();
  
  return false;
}


// Gets the next sample for a single note
double ClassicWaveform::next_sample(Note *n){
  ++(n->samples);
  double envelope = n->velocity / 127.0 * compute_envelope(n);
  double digital_freq = 6.2831853 * n->pitch  / sample_rate_;
  return envelope * wave_func_(digital_freq * n->samples);
}

// Computes the envelope of the signal using the current attack, decay,
// and release rates. Also responsible for handing the note from one stage
// in the envelope to the next
double ClassicWaveform::compute_envelope(Note *n){
  double envelope = 1;
  // Attack Envelope
  if (n->stage == 1){
    // Moves into sustain stage
    if (n->samples >= attack_samples_) {
      n->stage = 2;
    }
    else envelope = n->samples/(1.0 * attack_samples_);  
  }

  // Sustain Envelope
  if (n->stage == 2){
    double sus_env = (n->samples - attack_samples_)/(1.0 * sustain_samples_);
    if (n->samples - attack_samples_ >= sustain_samples_){
      // Moves into release stage
      n->stage = 3;
      n->sustain_end_env = 1-sus_env;
      n->release_sample = n->samples;
    }
    else envelope = 1-sus_env;
  }
  // Release Envelope (samples is reset at beginning of release)
  if (n->stage == 3){
    double past = (n->samples - n->release_sample)/(1.0 * release_samples_);
    envelope = n->sustain_end_env * (1 - past);
    envelope  = fmax(0, envelope);
    if (envelope == 0) n->flag_for_deletion = true;
  }  
  return envelope;
}

void ClassicWaveform::set_attack(double seconds){
  attack_samples_ = sample_rate_ * seconds;
}

void ClassicWaveform::set_sustain(double seconds){
  sustain_samples_ = sample_rate_ * seconds;
}

void ClassicWaveform::set_release(double seconds){
  release_samples_ = sample_rate_ * seconds;
}

// #------------------Waveform Functions-----------------#  

// A rectangular pulse is generated with high value of 1 and low value of -1. 
double square(double phase) {
  //Additive synthesis square wave
  double sum = 0;
  double n = 1;
  double nPi, coeff;

  while (n < 15) {
    nPi = n * 3.1415926535;
    coeff = sin(nPi / 2) / nPi;
    sum += coeff * cos(n * phase);
    n += 1;
  }
  return 4 * (0.25 + sum) - 1;
  
}


// A triangle wave is generated with peak values of 1 and -1. 
// Additive synthesis is used to prevent aliasing.
double tri(double phase) {
  //Additive synthesis triangle wave
  double sum = 0, sum_n = 0;
  double n = 1;
  double nPi, coeff;
  double width = 0.5;
  //Precalculating things that happen a lot
  double w_inv = 1 / width;
  double wneg_inv = 1 / (width - 1);
  double ww = 2 * width;
  double ww_neg = 2 * (width - 1);

  while (n < 15) {
    nPi = n * 3.1415926535;
    sum_n = 0;

    //the even terms
    coeff = wneg_inv * (cos(ww_neg * nPi) - 1) - w_inv * (cos(ww * nPi) - 1);
    sum_n += coeff * cos(n * phase);

    //The odd terms
    coeff = wneg_inv * sin(ww_neg * nPi) - w_inv * sin(ww * nPi);
    sum_n += coeff * sin(n * phase);

    sum += sum_n / (n * n);

    n += 1;
  }
  return sum / pow(3.1415926535, 2);

}

// A sawtooth wave is generated with peak values of 1 and -1. 
// Additive synthesis is used to prevent aliasing.
double saw(double phase) {
  //Additive synthesis triangle wave
  double sum = 0, sum_n = 0;
  double n = 1;
  double nPi, coeff;
  double width = 0.01;

  //Precalculating things that happen a lot
  double w_inv = 1 / width;
  double wneg_inv = 1 / (width - 1);
  double ww = 2 * width;
  double ww_neg = 2 * (width - 1);

  while (n < 15) {
    nPi = n * 3.1415926535;
    sum_n = 0;

    //the even terms
    coeff = wneg_inv * (cos(ww_neg * nPi) - 1) - w_inv * (cos(ww * nPi) - 1);
    sum_n += coeff * cos(n * phase);

    //The odd terms
    coeff = wneg_inv * sin(ww_neg * nPi) - w_inv * sin(ww * nPi);
    sum_n += coeff * sin(n * phase);

    sum += sum_n / (n * n);

    n += 1;
  }
  return sum / pow(3.1415926535, 2);

}
