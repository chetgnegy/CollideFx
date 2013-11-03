#include "AudioFeatures.h"

#ifndef TWOPI
#define TWOPI 6.2831853072
#endif

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100.0
#endif

#define DEBUG 0

// A constructor for the AudioFeatures class. Empty buffers are
// allocated for each type of feature vector.
AudioFeatures::AudioFeatures(int buffer_size) {

  //We would like large buffers! If buffer size is small, 
  //keep a couple of them and compute less often.
  //We assume buffer is a power of two. We're gonna call
  //4096 samples the optimal buffer size, but we will also accept larger.

  int scaling_factor = 1;
  if (buffer_size < 4096) {
    scaling_factor = 4096 / buffer_size;  //int division is ok here (see assumption)
  }
  //Creates some buffers
  time_ = new complex[buffer_size * scaling_factor];
  freq_ = new complex[buffer_size * scaling_factor];
  foft_ = new complex[(buffer_size * scaling_factor) / 2];
  norm_freq_ = new complex[buffer_size * scaling_factor];
  norm_foft_ = new complex[(buffer_size * scaling_factor) / 2];

  buffer_size_ = buffer_size * scaling_factor;
  FFT_length_ = buffer_size * scaling_factor;
  FoFT_length_ = (buffer_size * scaling_factor) / 2;
  valid_FFT_ = 0;
  valid_FoFT_ = 0;
  my_index_ = 0;

  depth_ = 10;
  FoFT_history_ = new double[depth_ * FoFT_length_];
  past_peaks_ = new int[depth_ * kNumberPeaks];
  past_amplitudes_ = new double[depth_ * kNumberPeaks];
  for (int i = 0; i < depth_ * FoFT_length_; ++i) {
    FoFT_history_[i] = 0;
  }
  for (int i = 0; i < depth_ * kNumberPeaks; ++i) {
    past_peaks_[i] = 0;
    past_amplitudes_[i] = 0;
  }

  chroma_ = 0;
  pitch_confidence_ = 0;

}
;

// All pointers to signal buffers are deleted
AudioFeatures::~AudioFeatures() {
  delete[] past_amplitudes_;
  delete[] past_peaks_;
  delete[] FoFT_history_;
  delete[] foft_;
  delete[] freq_;
  delete[] time_;

}
;

// Calculates the fourier transform for the signal currently in the time buffer. 
// Setting window to true uses a standard hamming window
void AudioFeatures::calculate_FFT(bool window) {
  complex *temp = new complex[buffer_size_];
  if (window) {
    complex *windowed = new complex[buffer_size_];
    for (int i = 0; i < buffer_size_; ++i) {
      windowed[i] = time_[i] * .5
          * (1 - cos(TWOPI * i / (1.0 * buffer_size_ - 1.0)));
    }
    valid_FFT_ = CFFT::Forward(windowed, temp, buffer_size_);
    delete windowed;

  } else {
    valid_FFT_ = CFFT::Forward(time_, temp, buffer_size_);
  }
  //Copy into our current array
  for (int i = 0; i < buffer_size_; ++i) {
    freq_[i] = temp[i];
  }
  delete temp;
  normalizeFFT();
}

// Calculates the fourier of fourier transform (Sylvain Marchand, An efficient pitch 
// tracking algorithm using a combination of fourier transforms)
void AudioFeatures::calculate_FoFT(void) {
  complex *temp = new complex[FoFT_length_];
  for (int i = 0; i < FoFT_length_; ++i) {
    temp[i] = freq_[i].normsq();
  }
  valid_FoFT_ = CFFT::Forward(temp, FoFT_length_);
  //Copy into our current array
  for (int i = 0; i < FoFT_length_; ++i) {
    foft_[i] = temp[i];
  }
  delete temp;
  normalizeFoFT();

  for (int i = 0; i < depth_ * FoFT_length_ / 2; ++i) {
    //updates recent list
    if (i < (depth_ - 1) * FoFT_length_ / 2) {
      FoFT_history_[i] = FoFT_history_[i + FoFT_length_ / 2];
    } else {
      FoFT_history_[i] =
          norm_foft_[i - (depth_ - 1) * FoFT_length_ / 2].normsq();
    }
  }

}

//Filters w/Rectification to remove large ripples
void AudioFeatures::smooth_spectra(complex* in, double length, double cutoff,
                                   complex* out) {
  DigitalLowpassFilter dlpf(cutoff, 1, 1);
  dlpf.calculate_coefficients();
  for (int i = 0; i < length; ++i) {
    dlpf.tick(in[i].normsq());
    out[i] = dlpf.most_recent_sample();
  }
}

// Changes a sample in the current buffer. 
void AudioFeatures::add_next_sample(complex value) {
  time_[my_index_] = value;
  ++my_index_;
  if (my_index_ == buffer_size_) {
    calculate_FFT(true);
    calculate_FoFT();
    chroma_ = get_fundamental();
    post_note_event();
    my_index_ = 0;
  }

}
void AudioFeatures::add_next_sample(double value) {
  add_next_sample(complex(value));

}

// Returns the size of the time buffer
int AudioFeatures::get_buffer_size() {
  return buffer_size_;
}

// Returns a sample from the time buffer
complex AudioFeatures::get_sample(int index) {
  if (index < buffer_size_) {
    return time_[index];
  } else {
    if (DEBUG)
      printf("Cannot return signal[i] -- index out of bounds!!\n");
  }
  return complex(0);
}

// Gets a from of the current FFT. Frames are stored with DC at frame zero
// up to Fs/2 halfway through the array. The negative frequencies are stored
// in the second half of the array
complex AudioFeatures::get_FFT_frame(int index) {
  if (index < FFT_length_) {
    return freq_[index];
  } else {
    if (DEBUG)
      printf("Cannot return FFT[i] -- index out of bounds!!\n");
  }
  return complex(0);
}
complex AudioFeatures::get_NFFT_frame(int index) {
  if (index < FFT_length_) {
    return norm_freq_[index];
  } else {
    if (DEBUG)
      printf("Cannot return NFFT[i] -- index out of bounds!!\n");
  }
  return complex(0);
}
// Gets a from of the current FoFT. Frames are stored with DC at frame zero
// up to Fs/4 halfway through the array. The negative frequencies are stored
// in the second half of the array. Only the positive frequencies of the FFT 
// are used.
complex AudioFeatures::get_FoFT_frame(int index) {
  if (index < FoFT_length_) {
    return foft_[index];
  } else {
    if (DEBUG)
      printf("Cannot return FoFT[i] -- index out of bounds!!\n");
  }
  return complex(0);
}
complex AudioFeatures::get_NFoFT_frame(int index) {
  if (index < FoFT_length_) {
    return norm_foft_[index];
  } else {
    if (DEBUG)
      printf("Cannot return normalized FoFT[i] -- index out of bounds!!\n");
  }
  return complex(0);
}

// The power of the FFT spectrum
double AudioFeatures::spectral_power(void) {
  double sum = 0;
  for (int i = 0; i < FFT_length_; ++i) {
    sum += get_FFT_frame(i).norm();
  }
  return sum;
}
// The power of the normalized FoFT spectrum 
double AudioFeatures::NFoFT_power(void) {
  double sum = 0;
  for (int i = 7; i < FoFT_length_; ++i) {
    sum += get_NFoFT_frame(i).norm();
  }
  return sum;
}

//Scales the vector such that the largest value is equal to 1. Looks through indices 'start' to 'length'.
double AudioFeatures::normalize(complex* in, int length, int start,
                                complex* out) {
  double max_val = 0.000001;  //prevents divide by zero
  double min_val = 9999999.;
  for (int i = start; i < length; ++i) {
    max_val = fmax(in[i].norm(), max_val);
    min_val = fmin(in[i].norm(), min_val);
  }
  for (int i = 0; i < length; ++i) {
    if (i < start)
      out[i] = 0;
    else
      out[i] = (in[i] - min_val) / (max_val - min_val);

  }
  //std::cout << "After "<< max_val << " " << min_val << std::endl;
  return max_val;
}

// In place normalize
double AudioFeatures::normalize(complex* in, int length, int start) {
  return normalize(in, length, start, in);

}

// Normalizes the FFT and FoFT buffers
double AudioFeatures::normalizeFFT(void) {
  return normalize(freq_, FFT_length_, 0, norm_freq_);
}

// Normalizes the FFT and FoFT buffers
double AudioFeatures::normalizeFoFT(void) {
  return normalize(foft_, FoFT_length_, 0, norm_foft_);
}

//Looks for spectral peaks and stores 'num_peaks' results in the two arrays
void AudioFeatures::find_FFT_peaks(int num_peaks, int* frequencies,
                                   double *amplitudes) {
  find_peaks(norm_freq_, FFT_length_ / 2, num_peaks, frequencies, amplitudes);
}

//Looks for spectral peaks and stores 'num_peaks' results in the two arrays
void AudioFeatures::find_FoFT_peaks(int num_peaks, int* frequencies,
                                    double *amplitudes) {
  find_peaks(norm_foft_, FoFT_length_ / 2, num_peaks, frequencies, amplitudes);
}


// Looks for peaks in the first 'length' elements of 'source' and stores 
// 'num_peaks' results in the two double arrays
  
void AudioFeatures::find_peaks(complex *source, int length, int num_peaks,
                               int* frequencies, double *amplitudes) {

  int current_peak_count = -1;
  double current_frame;

  for (int i = AudioFeatures::kStartingFrame;
      i < length - AudioFeatures::kStartingFrame; ++i) {
    current_frame = source[i].norm();
    if (current_frame >= source[i - 1].norm()
        && current_frame >= source[i + 1].norm()) {
      ++current_peak_count;  // the peak index we are trying to add.

      //new peak!! - keeps the list in descending order
      for (int k = current_peak_count; k >= 0; --k) {
        if (k > 0 && current_frame > amplitudes[k - 1]) {
          //bump that element down
          amplitudes[k] = amplitudes[k - 1];
          frequencies[k] = frequencies[k - 1];
        } else {
          //Adds new element
          amplitudes[k] = current_frame;
          frequencies[k] = i;
          break;
        }

      }
      //That's enough!
      if (current_peak_count == num_peaks - 1)
        break;
    }

  }
}

// Converts a frequency to it's pitch class
int AudioFeatures::calculate_chroma(double f) {
  int k = (int) round((12 * log(f / 440.0) / log(2.0)) + 69);
  return (k + 12) % 12;
}

// Looks for peaks in the FoFT spectrum and tries to connect peaks in current
// FoFT with past spectrums. It connects a path, finds the most likely fundemental
// and returns the chroma for it.
// TODO: Fix the scoring algorithm so that it weighs the lower frequencies more
double AudioFeatures::get_fundamental() {
  //Looks for peaks
  double foft_peaks[kNumberPeaks] = { 0. };
  int foft_peak_freqs[kNumberPeaks] = { 0. };

  find_FoFT_peaks(kNumberPeaks, foft_peak_freqs, foft_peaks);

  for (int i = 0; i < depth_ * kNumberPeaks; ++i) {

    //updates recent list of peaks
    if (i < (depth_ - 1) * kNumberPeaks) {
      past_peaks_[i] = past_peaks_[i + kNumberPeaks];
      past_amplitudes_[i] = past_amplitudes_[i + kNumberPeaks];
    } else {
      past_peaks_[i] = foft_peak_freqs[i - (depth_ - 1) * kNumberPeaks];
      past_amplitudes_[i] = foft_peaks[i - (depth_ - 1) * kNumberPeaks];
    }
  }

  std::list<int> *paths = new std::list<int>[5];  //This is a hack because kNumberPeaks is 5.
  
  //Connects the peaks of the FoFT's throughout time
  for (int i = 0; i < kNumberPeaks; i++) {
    //Starts the list of paths
    paths[i].push_back(i + (depth_ - 1) * kNumberPeaks);

    int k = 1;
    while (k < depth_ - 1) {
      int closest_distance = 99999;
      int closest_index = 99999;
      //Finds the closest peak from the last set of peaks.
      for (int j = 0; j < kNumberPeaks; j++) {
        if (abs(
            past_peaks_[paths[i].back()]
                - past_peaks_[j + (depth_ - 1 - k) * kNumberPeaks])
            < closest_distance) {
          closest_distance = abs(
              past_peaks_[paths[i].back()]
                  - past_peaks_[j + (depth_ - 1 - k) * kNumberPeaks]);
          closest_index = j + (depth_ - 1 - k) * kNumberPeaks;
        }
        //This path is better than the previous path because it has a higher amplitude.
        else if (abs(
            past_peaks_[paths[i].back()]
                - past_peaks_[j + (depth_ - 1 - k) * kNumberPeaks])
            == closest_distance) {
          if (past_amplitudes_[j + (depth_ - 1 - k) * kNumberPeaks]
              > past_amplitudes_[closest_index]) {
            closest_distance = abs(
                past_peaks_[paths[i].back()]
                    - past_peaks_[j + (depth_ - 1 - k) * kNumberPeaks]);
            closest_index = j + (depth_ - 1 - k) * kNumberPeaks;
          }
        }
      }
      ++k;
      paths[i].push_back(closest_index);
    }
  }
  
  double tempFreq = 0;
  double tempAmp = 0;
  double *scores = new double[5];  //This is a hack because kNumberPeaks is 5.
  std::list<int>::iterator index;
  for (int i = 0; i < kNumberPeaks; i++) {
    scores[i] = 0;
    index = paths[i].begin();

    //Pops bubbles that are close together
    while (index != paths[i].end()) {
      scores[i] += tempAmp / (1 + pow(abs(tempFreq - past_peaks_[*index]), 2));
      tempAmp = past_peaks_[*index];
      tempFreq = past_amplitudes_[*index];
      //std::cout << past_peaks_[*index] << " ";
      ++index;

    }

    //std::string allChroma[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    //std::cout << l << std::endl;
    //std::cout << allChroma[(l+12)%12] << std::endl;
    //std::cout << " SCORE: " << scores[i] << " FREQ: "<< SAMPLE_RATE/2.0/past_peaks_[paths[i].front()] << " "<< k << std::endl;
  }

  double bestScore = 0;
  double bestFreq = 0;
  for (int i = 0; i < kNumberPeaks; i++) {
    if (scores[i] > bestScore) {
      bestScore = scores[i];
      bestFreq = SAMPLE_RATE / 2.0 / past_peaks_[paths[i].front()];
    }

  }

  //std::cout << "POW " <<NFoFT_power() << std::endl;
  //std::cout << "SPECPOW " <<spectral_power() << std::endl;
  delete[] scores;
  delete[] paths;

  return calculate_chroma(bestFreq);
}

// Tells any external code that there is a note ready to play, it can be accessed
// using get_note_event
// TODO: Make this not a hack
void AudioFeatures::post_note_event(void) {
  if (rand() / (1.0 * RAND_MAX) < .3) {
    pitch_confidence_ = 1;
  } else
    pitch_confidence_ = 0;
}

// If there is a note that is ready to read, it is accessed here. The pitch class
// of the note is returned
// TODO: Make this not a hack
int AudioFeatures::get_note_event(void) {
  if (spectral_power() > 1200 && pitch_confidence_ == 1) {
    pitch_confidence_ = 0;
    return chroma_;
  }
  return -1;
}

