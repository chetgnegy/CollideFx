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
#include <queue>
#include <vector>
#include <algorithm>
#include "UnitGenerator.h"
#include "Disc.h"

class GraphData;


typedef std::pair<Disc *, double > Edge; 
typedef std::pair<Disc *, Disc * > Wire; 


class UGenGraphBuilder {
public:

  static const double kMaxDist = 7.0;
  UGenGraphBuilder();
  ~UGenGraphBuilder();

  // Recomputes the graph based on the new positions of the discs
  void rebuild();

  // Processes a single sample. Note that you must first handoff audio 
  // and midi data to the graph by using the handoff_audio and 
  // handoff midi functions
  double tick();

  // Prints all data about the graph, including the nodes,
  // their type and positions
  void print_all();

  // Passes any audio samples to the Input ugens. 
  void handoff_audio(double samples);

  // Passes any midi notes the MidiUnitGenerators. Decides using the
  // value of velocity whether the event is a note on or a note off
  void handoff_midi(int MIDI_pitch, int velocity);

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

  void switch_wire_direction(Wire &w);

  std::vector<Wire> wires_;

private:
  // The distance between two discs
  double get_edge_cost(Disc* a, Disc* b);

  void find_edges();

  // Allows all ugens to be called uniformly
  Disc *indexed(int i);

  // The chain of effects that is processed before being sent to the output
  std::vector<Disc *> fx_;
  
  // The list of audio inputs that receive samples
  std::vector<Disc *> inputs_;
  
  // The list of unit generators that require midi events to work
  std::vector<Disc *> midi_modules_;
  
  
  std::map < Disc *, GraphData > data_;
};


class GraphData{
public:
  GraphData();
  ~GraphData();
  std::vector< Edge > edges_;
  bool marked_;
  void list_edges();
};


#endif
