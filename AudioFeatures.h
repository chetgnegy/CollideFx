#ifndef AUDIOFEATURES_H_
#define AUDIOFEATURES_H_

#include <iostream>
#include <list>
#include "complex.h"
#include "fft.h"
#include "DigitalFilter.h"

class AudioFeatures {
 public:

  // Ignores bins this close to the edge of the spectra
  static const int kStartingFrame = 6;
  // Number of peaks taken from each from of the FoFT
  
  static const int kNumberPeaks = 5;
  // A constructor for the AudioFeatures class. Empty buffers are
  // allocated for each type of feature vector.

  AudioFeatures(int buffer_size);
  // All pointers to signal buffers are deleted 
  ~AudioFeatures();

  //Filters w/Rectification to remove large ripples
  void smooth_spectra(complex* in, double length, double cutoff, complex* out);

  // Changes a sample in the current buffer. Automatically 
  // recomputes FFTs, FoFTs, and does pitch detection when buffer is full
  // The workhorse of AudioFeatures.cpp
  void add_next_sample(complex value);
  void add_next_sample(double value);

  // Returns the size of the time buffer
  int get_buffer_size(void);

  // Returns a sample from the time buffer
  complex get_sample(int index);

  // The power of the spectrum
  double spectral_power(void);
  double NFoFT_power(void);

  // Gets a from of the current FFT. Frames are stored with DC at frame zero
  // up to Fs/2 halfway through the array. The negative frequencies are stored
  // in the second half of the array
  complex get_FFT_frame(int bin);
  complex get_NFFT_frame(int bin);

  // Gets a from of the current FoFT. Frames are stored with DC at frame zero
  // up to Fs/4 halfway through the array. The negative frequencies are stored
  // in the second half of the array. Only the positive frequencies of the FFT 
  // are used.
  complex get_FoFT_frame(int bin);
  complex get_NFoFT_frame(int bin);

  
  //Looks for spectral peaks and stores 'num_peaks' results in the two arrays
  void find_FFT_peaks(int num_peaks , int* frequencies, double *amplitudes);
  void find_FoFT_peaks(int num_peaks , int* frequencies, double *amplitudes);

  
  // If there is a note that is ready to read, it is accessed here. The pitch class
  // of the note is returned
  // TODO: Make this not a hack
  int get_note_event(void);
  
  // Says whether the FFT bins corresponds to the signal in time_
  bool valid_FFT_;
  // Says whether the FoFT bins corresponds to the signal in time_
  bool valid_FoFT_;
  
  // How many past FoFTs we should look at
  int depth_;
  //The past FoFTs
  double * FoFT_history_;
  // The peaks of previous FoFT spectra
  int * past_peaks_;
  // The amplitudes of the previous FoFT peaks
  double * past_amplitudes_;
  
  
 private:
 
  // Calculates the fourier transform for the signal currently in the time buffer. 
  // Setting window to true uses a standard hamming window
  void calculate_FFT(bool window);

  // Calculates the fourier of fourier transform (Sylvain Marchand, An efficient pitch 
  // tracking algorithm using a combination of fourier transforms)
  void calculate_FoFT(void);

 
  // Scales the vector such that the largest value is equal to 1. Looks through 
  // indices 'start' to 'length'.
  double normalize(complex* in, int length, int start, complex* out);
  // In place normalize
  double normalize(complex* in_buffer, int vector_length, int start);
  
  // Normalizes the FFT and FoFT buffers
  double normalizeFFT();
  double normalizeFoFT();

  // Looks for peaks in the FoFT spectrum and tries to connect peaks in current
  // FoFT with past spectrums. It connects a path, finds the most likely fundemental
  // and returns the chroma for it.
  // TODO: Fix the scoring algorithm so that it weighs the lower frequencies more
  double get_fundamental(void);
  
  
  // Looks for peaks in the first 'length' elements of 'source' and stores 
  // 'num_peaks' results in the two double arrays
  void find_peaks(complex *source, int length, int num_peaks, 
                  int* frequencies, double *amplitudes);
  
  // Gets the pitch class of the current frequency
  int calculate_chroma(double frequency);
  
  // Tells any external code that there is a note ready to play, it can be accessed
  // using get_note_event
  // TODO: Make this not a hack
  void post_note_event(void);
  
  int buffer_size_;
  // The signal
  complex *time_;
  // The FFT of the signal
  complex *freq_;
  // The FFT of the positive frequencies of the signal
  complex *foft_;
  // The normalized FFT
  complex *norm_freq_;
  // The normalized FoFT
  complex *norm_foft_;
  

  // Length of the FFT vector
  int FFT_length_;
  // Length of the FoFT vector
  int FoFT_length_;
  // current index of time domain vector
  int my_index_;


  // The current pitch class 
  int chroma_;
  // Whether or not the chroma value can be trusted
  double pitch_confidence_;
  
  
};



#endif
