/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenGraphBuilder.h
  This class uses the positions of the discs and 
  the ugens to build a signal flow graph
*/


#ifndef _UGENGRAPHBUILDER_H_
#define _UGENGRAPHBUILDER_H_

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "UnitGenerator.h"
#include "DigitalFilter.h" 
#include "Disc.h"
#include "Thread.h"

struct GraphData;

typedef std::pair<Disc *, double > Edge; 
typedef std::pair<Disc *, Disc * > Wire; 


class UGenGraphBuilder {
public:

  static const double kMaxDist = 7.0;

  UGenGraphBuilder(int buffer_length);
  ~UGenGraphBuilder();

  // Prints all data about the graph, including the nodes,
  // their type and positions
  void print_all();

  // Recomputes the graph based on the new positions of the discs
  void rebuild();


  // Changes the mutex. Should be called when using anything related to the 
  // audio path
  void lock_thread(bool lock);

  // Processes a single buffer. Note that you must first handoff audio 
  // and midi data to the graph by using the handoff_audio and 
  // handoff midi functions
  void load_buffer(double *out, int length);


  // Passes any audio samples to the Input ugens. 
  void handoff_audio(double samples);
  void handoff_audio_buffer(double *buffer, int samples);

  // Passes any midi notes the MidiUnitGenerators. Decides using the
  // value of velocity whether the event is a note on or a note off
  void handoff_midi(int MIDI_pitch, int velocity);

  // Recalculates the FFT and moves the orbs around. This is called in between
  // audio buffers. It is called from the graphics thread
  void update_graphics_dependencies();

  // Lets the other thread know that the FFT is ready to compute
  void signal_new_buffer();
  bool is_new_buffer(){return buffer_ready_;}

  // #------------ Modify Graph -------------#

  // Adds a unit generator to the signal chain. Returns false for invalid
  // ugen type
  bool add_effect(Disc *fx);
  
  // Adds midi unit generator to a list of objects that must be checked
  // when new midi event is created. Returns false for invalid
  // ugen type
  bool add_input(Disc *input);

  // Adds midi unit generator to a list of objects that must be checked
  // when new midi event is created. Returns false for invalid
  // ugen type
  bool add_midi_ugen(Disc *mugen);

  // Removes a disc from the graph and deletes the disc
  bool remove_disc(Disc *ugen);


  
  // #--------------- FFT ----------------#

  
  // The graphics thread can grab this and display it. This keeps an average of the
  // most recent spectra
  void calculate_fft();

  complex *get_fft();
  int get_fft_length(){return buffer_length_/2;}
  
  // #--------------- UI ----------------#

  // This is called when the up button is pressed in the menu
  void handle_up_press();

  // This is called when the down button is pressed in the menu
  void handle_down_press();

  // This lets the menu know whether or not the arrows will do anything
  bool selector_activated();

  // The text that appears in the little box next to the arrows on the menu
  const char *text_box_content();

  // The label that appears below the little box next to the arrows on the menu
  const char *text_box_label();

  std::vector<Disc *> sinks_;
  std::vector<Disc *> old_sinks_;

private:

  // Finds the signature for the current graph
  std::string compute_signature();
  // All discs scheduled for deletion are removed (called after buffer is generated)
  bool finalize_delete();

  // The distance between two discs
  double get_edge_cost(Disc* a, Disc* b);

  // Reverses the push architecture of "out = tick(in)" to recursively pull
  // samples to the output sinks from the inputs. past_data allows us to use
  // the previously stored graph. This is used for crossfading between graph 
  // changes
  // State - 0: Normal, 1: Prepare, 2: Recall
  double *pull_result_buffer(Disc *k, int length, int state = 0);

  // Reverses the "to" and "from" ends of a wire
  void switch_wire_direction(Wire &w);

  // Finds the mix level for two discs based on their proximity
  double compute_mix_level(Disc *a, Disc *b);
  // Computes all mix levels pairwise
  void find_mix_levels();
  //Returns the calculated mix level
  double get_mix_level(Disc *a, Disc *b);

  // Returns 1/sqrt(factor)
  double scale_factor(int factor);

  // Allows all ugens to be called uniformly
  Disc *indexed(int i);

  int buffer_length_;
  std::list<complex *> fft_list_;
  complex *fft_visual_;
  bool buffer_ready_;

  
  // The lists of inputs and effects that are processed for samples
  std::vector<Disc *> inputs_;
  std::vector<Disc *> midi_modules_;
  std::vector<Disc *> fx_;
  std::vector<Disc *> to_delete_;
  
  std::vector<Wire> wires_;
  
  // Data containing the current connections
  std::map < Disc *, GraphData > data_;
  std::map < Disc *, std::map <Disc *, double> > wet_levels_;
  // Protects the audio and graphics thread from
  // concurrency issues
  Mutex audio_lock_;


  //Filters to process the output. Just for quality's sake...
  DigitalLowpassFilter *anti_aliasing_;
  DigitalHighpassFilter *low_pass_;

  // Comparable description of graph
  std::string signature_;
  std::string past_signature_;
  
};

// Stores previous state of the graph from the discs' perspective
// UGen states are stored at UGen level.
struct GraphData{
  // Current Connections
  std::vector< Disc* > inputs_;
  std::vector< Disc* > outputs_;
  // Has the buffer been used in this computation yet?
  bool computed;
  // Info for previous buffer's graph
  std::vector< Disc* > past_inputs_;
  std::vector< Disc* > past_outputs_;
  bool past_computed;

  bool need_crossfade_;
  double *crossfade_dry_;
  double *crossfade_wet_;
  
};


#endif
