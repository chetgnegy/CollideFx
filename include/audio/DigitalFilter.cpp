#include "DigitalFilter.h"
#include <iostream>


//Creates a generic filter with no history or previous input
DigitalFilter::DigitalFilter(double a[3], double b[3]){
  gain_ = 1;
  Q_ = 1;
  corner_frequency_ = 1;
  force_coefficients(a,b);
};

DigitalFilter::DigitalFilter(double center, double Q, double gain) {
  gain_ = gain;
  Q_ = Q;
  corner_frequency_ = center;  //Convert to rads/sec
  x_past_[0] = 0;
  x_past_[1] = 0;
  x_past_[2] = 0;
  y_past_[0] = 0;
  y_past_[1] = 0;
  y_past_[2] = 0;
}

DigitalFilter::~DigitalFilter() {}




//Advances the filter by a single sample, in.
complex DigitalFilter::tick(complex in) {
  x_past_[2] = x_past_[1];
  x_past_[1] = x_past_[0];
  x_past_[0] = in;
  y_past_[2] = y_past_[1];
  y_past_[1] = y_past_[0];

  y_past_[0] = -a_[1] * y_past_[1] - a_[2] * y_past_[2]
      + (b_[0] * x_past_[0] + b_[1] * x_past_[1] + b_[2] * x_past_[2]);
  
  return most_recent_sample();
}

//Gets the current output of the filter.
complex DigitalFilter::most_recent_sample() {
  return y_past_[0] * gain_;
}

//Calculates the DC gain of the system. Good for normalizing with high Q or extreme corner frequency values.
double DigitalFilter::dc_gain() {
  return fabs((b_[0] + b_[1] + b_[2]) / (a_[0] + a_[1] + a_[2]));
}
double DigitalFilter::hf_gain() {
  return fabs(((b_[0] - b_[1] + b_[2]) / (a_[0] - a_[1] + a_[2])));
}

DigitalFilter* DigitalFilter::create_inverse(){
  double a[] = {b_[0], b_[1], b_[2]};
  double b[] = {a_[0], a_[1], a_[2]};
  DigitalFilter *f = new DigitalFilter(b,a);
  return f;
}

void DigitalFilter::change_parameters(double center, double Q, double gain){
  gain_ = gain;
  Q_ = Q;
  corner_frequency_ = center;  //Convert to rads/sec
  calculate_coefficients();
}

// Stores the most recent values in the filter so that the state of the 
// filter can be restored later
DigitalFilterState* DigitalFilter::get_state(){
  DigitalFilterState *d = new DigitalFilterState();
  d->x_[0] = x_past_[0]; d->x_[1] = x_past_[1]; d->x_[2] = x_past_[2];
  d->y_[0] = y_past_[0]; d->y_[1] = y_past_[1]; d->y_[2] = y_past_[2];
  return d;
}

// Recalls the most recent values in the filter
void DigitalFilter::set_state(DigitalFilterState *d){
  x_past_[0] = d->x_[0]; x_past_[1] = d->x_[1]; x_past_[2] =  d->x_[2];
  y_past_[0] = d->y_[0]; y_past_[1] = d->y_[1]; y_past_[2] =  d->y_[2];
  delete d;
}



void DigitalBandpassFilter::calculate_coefficients() {
  double bandwidth = TWOPI * corner_frequency_ / Q_;
  double lambda_1 = TWOPI * corner_frequency_ - bandwidth / 2.0;
  double lambda_2 = TWOPI * corner_frequency_ + bandwidth / 2.0;

  double gamma_0 = 4 * SAMPLE_RATE * SAMPLE_RATE;
  double gamma_1 = 2 * SAMPLE_RATE * (bandwidth);
  double gamma_2 = lambda_1 * lambda_2;

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 - alpha_1);
  b_[1] = 0;
  b_[2] = -b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}


void DigitalBandstopFilter::calculate_coefficients() {

  double bandwidth = TWOPI * corner_frequency_ / Q_;
  double lambda_1 = TWOPI * corner_frequency_ - bandwidth / 2.0;
  double lambda_2 = TWOPI * corner_frequency_ + bandwidth / 2.0;

  double gamma_0 = 4 * SAMPLE_RATE * SAMPLE_RATE;
  double gamma_1 = 2 * SAMPLE_RATE * (bandwidth);
  double gamma_2 = lambda_1 * lambda_2;

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;
  double beta = -2 * (4 * pow(SAMPLE_RATE, 2) - gamma_2)
      / (4 * pow(SAMPLE_RATE, 2) + gamma_2);
  b_[0] = .5 * (2 + alpha_1 - alpha_2);
  b_[1] = beta * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}


void DigitalLowpassFilter::calculate_coefficients() {

  double gamma_0 = 4;
  double gamma_1 = 2 / SAMPLE_RATE * (TWOPI * corner_frequency_) / Q_;
  double gamma_2 = pow(TWOPI * corner_frequency_ / SAMPLE_RATE, 2);

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 - alpha_1);
  b_[1] = 2 * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;

}



void DigitalHighpassFilter::calculate_coefficients() {
  
  double gamma_0 = 4;
  double gamma_1 = 2 / SAMPLE_RATE * (TWOPI * corner_frequency_) / Q_;
  double gamma_2 = pow(TWOPI * corner_frequency_ / SAMPLE_RATE, 2);

  double denominator = gamma_0 + gamma_1 + gamma_2;
  double alpha_1 = (gamma_0 - gamma_1 - gamma_2) / denominator;
  double alpha_2 = (gamma_0 + gamma_1 - gamma_2) / denominator;

  b_[0] = .5 * (alpha_2 + alpha_1);
  b_[1] = -2 * b_[0];
  b_[2] = b_[0];
  a_[0] = 1;
  a_[1] = -(alpha_1 + alpha_2);
  a_[2] = 1 + alpha_1 - alpha_2;
}




void SinglePoleFilter::calculate_coefficients() {
  b_[0] = Q_;
  b_[1] = 0;
  b_[2] = 0;
  a_[0] = 1;
  a_[1] = -(corner_frequency_);
  a_[2] = 0; 
  
}




// A lowpass filter with filtered feedback
//https://ccrma.stanford.edu/~jos/pasp/Lowpass_Feedback_Comb_Filter.html
FilteredFeedbackCombFilter::FilteredFeedbackCombFilter(int samples, double roomsize, double damping): DigitalFilter(0.0, 0.0, 1.0){
  samples_ = samples; 
  roomsize_ = roomsize;
  damping_ = damping;
  
  sp_ = new SinglePoleFilter(damping_, 1 - damping_, 1.0);
  buffer_ = new complex[samples_];
  for (int i = 0; i < samples_; ++i){
    buffer_[i] = 0;
  }
  buf_index_ = 0;
}

FilteredFeedbackCombFilter::~FilteredFeedbackCombFilter(){
  delete[] buffer_;
  delete sp_;
}

// Computes a new value and adds it to a sample from the filtered delay line
complex FilteredFeedbackCombFilter::tick(complex in){
  complex out = in + roomsize_ * sp_->tick(buffer_[buf_index_]);
  //std::cout << out.re() << std::endl;
  buffer_[buf_index_] = out;
  ++buf_index_;
  buf_index_ %= samples_;
  return out;    
}

void FilteredFeedbackCombFilter::change_parameters(int samples, double roomsize, double damping){
  samples_ = samples; 
  roomsize_ = roomsize;
  damping_ = damping;
}




//One zero, one pole approximation of an allpass filter
//https://ccrma.stanford.edu/~jos/pasp/Freeverb_Allpass_Approximation.html
AllpassApproximationFilter::AllpassApproximationFilter(int samples, double g): DigitalFilter(0.0, 0.0, 1.0){
  samples_ = samples; 
  g_ = g;
  input_buffer_ = new complex[samples];
  output_buffer_ = new complex[samples];
  for (int i = 0; i < samples_; ++i){
    input_buffer_[i] = 0;
    output_buffer_[i] = 0;
  }
  buf_index_ = 0;
}

AllpassApproximationFilter::~AllpassApproximationFilter(){
  delete[] output_buffer_;
  delete[] input_buffer_;
  
}

// Computes a new value and adds it to a sample from the filtered delay line
complex AllpassApproximationFilter::tick(complex in){
  complex out = g_ * output_buffer_[buf_index_] - in + (1+g_)*input_buffer_[buf_index_];
  input_buffer_[buf_index_] = in;
  output_buffer_[buf_index_] = out;
  ++buf_index_;
  buf_index_ %= samples_;
  return out;    
}



// Creates a bank that is capable of holding parallel filters.
// Comes with a built in Lowpass Filter of frequency 10Hz that
// useful for building a peak detector. Gain is normalized to 
// 1/DCGain.
FilterBank::FilterBank() {
  gain_control_ = new DigitalLowpassFilter(10.0, 1.0, 1.0);
  gain_control_->gain_ = 1 / gain_control_->dc_gain();
  num_filters_ = 0;
  current_sample_ = 0;

}

// Frees memory for all filters on the list 'filters_'
FilterBank::~FilterBank() {
  std::list<DigitalFilter *>::iterator list_iterator;
  list_iterator = filters_.begin();
  //Deletes all filters
  while (filters_.size() > 0 && list_iterator != filters_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }

}

// Adds a filter in parallel
void FilterBank::add_filter(DigitalFilter *f) {
  filters_.push_back(f);
  ++num_filters_;
  
}

// A new sample 'in' is added into the filter and a new output is
// calculated (and can be accssed using most_recent_sample()). The 
// output of each filter is summed together. The lowpass filter
// is given the output.
complex FilterBank::tick(complex in) {

  if (filters_.size() > 0) {
    std::list<DigitalFilter *>::iterator list_iterator;
    list_iterator = filters_.begin();

    complex output = 0;
    while (list_iterator != filters_.end()) {
      //The convolution with the resonant body
      output += (*list_iterator)->tick(in);
      ++list_iterator;

    }

    gain_control_->tick(output.normsq());
    current_sample_ = output;
  }
  return most_recent_sample();
}

// The output of the lowpass filter. This is useful for time-varying 
// gain control.
double FilterBank::get_current_gain() {
  return gain_control_->most_recent_sample().re();
}


//Returns the most recently calculated sample
complex FilterBank::most_recent_sample() {
  return current_sample_;
}
