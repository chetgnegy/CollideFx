/*
 * dfilt.h
 *
 *  Created on: Oct 14, 2013
 *      Author: chetgnegy
 */

#ifndef DFILT_H_
#define DFILT_H_

#include <list>
#include "complex.h"
#include <iostream>



#ifndef TWOPI
#define TWOPI 6.2831853072
#endif

/*
       All numerical stuff from here:

 Tunable and Variable Passive Digital Filters
      for Multimedia Signal Processing
                  H. K. Kwan
*/

class DigitalFilterState{
public:
  DigitalFilterState(){}
  complex x_[3];
  complex y_[3];
};


class DigitalFilter {
 public:
  //Creates a generic filter with no history or previous input
  DigitalFilter(double a[3], double b[3]);//Pick coefficents directly
  DigitalFilter(double center_frequency, double Q, double gain);
  virtual ~DigitalFilter();
  
  static void set_sample_rate(int sr);

  // Must be overridden by subclass
  virtual void calculate_coefficients(){};

  // Advances the filter by a single sample, in. The new value is returned.
  virtual complex tick(complex in);

  // Gets the current output of the filter.
  complex most_recent_sample(void);
  
  // Calculates the gain of the system at frequency zero. Good for 
  // normalizing with high Q or extreme corner frequency values.
  double dc_gain(void);
  double hf_gain(void);
  double gain_;

  // Creates a new instance of a filter that is the inverse of the current
  // filter
  DigitalFilter* create_inverse();

  // Changes the parameters. For some filters, this still might do 
  // nothing (because it may be overridden), depends on filter implementation
  virtual void change_parameters(double center_frequency, double Q, double gain);
 
  virtual DigitalFilterState* get_state();
  virtual void set_state(DigitalFilterState *d);

 private:
  void force_coefficients(double a[3], double b[3]){
    a_[0] = a[0]; b_[0] = b[0];
    a_[1] = a[1]; b_[1] = b[1];
    a_[2] = a[2]; b_[2] = b[2];
  }

 protected:

  static float sample_rate;
  
  //The coefficients for the numerator
  double a_[3];  
  //The coefficients for the denominator
  double b_[3];  
  //The previous unfiltered values
  complex x_past_[3];  
  //The previous outputs
  complex y_past_[3];  
  //Entered in Hz
  double corner_frequency_;  
  //Quality Factor of the filter
  double Q_;

};

// Bandpass Filter is a subclass of DigitalFilter
class DigitalBandpassFilter : public DigitalFilter {
 public:
  DigitalBandpassFilter(double center_frequency, double Q, double gain)
      : DigitalFilter(center_frequency, Q, gain) {
      this->calculate_coefficients();
  };
  //Calculates the bandpass filter coefficients
  void calculate_coefficients();

  
};

// Bandstop Filter is a subclass of DigitalFilter
class DigitalBandstopFilter : public DigitalFilter {
 public:
  DigitalBandstopFilter(double center_frequency, double Q, double gain)
      : DigitalFilter(center_frequency, Q, gain) {
      this->calculate_coefficients();
  };
  // Calculates the bandstop filter coefficients
  void calculate_coefficients();
  
};

// Lowpass Filter is a subclass of DigitalFilter
class DigitalLowpassFilter : public DigitalFilter {
 public:
  DigitalLowpassFilter(double center_frequency, double Q, double gain)
      : DigitalFilter(center_frequency, Q, gain) {
      this->calculate_coefficients();
  };
  // Calculates the lowpass filter coefficients
  void calculate_coefficients();
  
};

// Highpass Filter is a subclass of DigitalFilter
class DigitalHighpassFilter : public DigitalFilter {
 public:
  DigitalHighpassFilter(double center_frequency, double Q, double gain)
      : DigitalFilter(center_frequency, Q, gain) {
      this->calculate_coefficients();
  };
  //Calculates the highpass filter coefficients
  void calculate_coefficients();
  
};

class SinglePoleFilter : public DigitalFilter {
 public:
  SinglePoleFilter(double pole, double damping, double gain)
      : DigitalFilter(pole, damping, gain) {
      this->calculate_coefficients();
  };
  //Calculates the single pole filter's coefficients
  void calculate_coefficients();
  
};


// This is mainly used for reverberation (Freeverb)
class FilteredFeedbackCombFilter : public DigitalFilter {
 public:
  FilteredFeedbackCombFilter(int samples, double roomsize, double damping);
  ~FilteredFeedbackCombFilter();

  // Computes a new value and adds it to a sample from the filtered delay line
  complex tick(complex in);
  
  //The internal filter
  double roomsize_, damping_;
  SinglePoleFilter *sp_;
  
  //This filter is a bit different
  void change_parameters(int samples, double roomsize, double damping);

private:
  // With this type of filter, we don't need to compute anything
  void calculate_coefficients(){}
  int samples_;
  complex *buffer_;
  int buf_index_;
};


class AllpassApproximationFilter : public DigitalFilter {
 public:
  AllpassApproximationFilter(int samples, double g);
  ~AllpassApproximationFilter();

  // Computes a new value and adds it to a sample from the filtered delay line
  complex tick(complex in);

  DigitalFilterState* get_state();
  void set_state(DigitalFilterState *d);
  void patch_buffer(double *buffer, int length);

private:
  // With this type of filter, we don't need to compute anything
  void calculate_coefficients(){}
  int samples_;
  double g_;
  complex *output_buffer_;
  complex *input_buffer_;
  int buf_index_;
};

class AllpassApproximationFilterState: public DigitalFilterState{
public:
  AllpassApproximationFilterState(){}
  complex *output_buffer_;
  complex *input_buffer_;
  int buf_index_;
};

//A bunch of filters can be added to this. They are all used in parallel.
class FilterBank {
 public:
  // Creates a bank that is capable of holding parallel filters.
  // Comes with a built in Lowpass Filter of frequency 10Hz that
  // useful for building a peak detector. Gain is normalized to 
  // 1/DCGain.
  FilterBank();
  
  // Frees memory for all filters on the list 'filters_'
  ~FilterBank();
  
  // A new sample 'in' is added into the filter and a new output is
  // calculated (and can be accssed using most_recent_sample()). The 
  // output of each filter is summed together. The lowpass filter
  // is given the output. The new value is returned.
  complex tick(complex);
  
  
  // Adds a filter in parallel
  void add_filter(DigitalFilter *);

  // The output of the lowpass filter. This is useful for time-varying 
  // gain control.
  double get_current_gain(void);
  
  //Returns the most recently calculated sample
  complex most_recent_sample(void);
  
  //The filters that are inside the filter bank
  std::list<DigitalFilter *> filters_;
  
  //The peak detecting low pass filter
  DigitalLowpassFilter *gain_control_;


 private:
  // The number of filters that are inside the filter bank
  int num_filters_;
  // The current output of the filter
  complex current_sample_;
};


#endif /* DFILT_H_ */
