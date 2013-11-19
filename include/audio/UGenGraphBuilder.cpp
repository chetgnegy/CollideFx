/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenGraphBuilder.cpp
  This class uses the positions of the discs and 
  the ugens to build a signal flow graph
*/

#include "UGenGraphBuilder.h"


UGenGraphBuilder::UGenGraphBuilder(){}

UGenGraphBuilder::~UGenGraphBuilder(){
  /*
  std::list<UnitGenerator *>::iterator list_iterator;
  list_iterator = chain_.begin();
  //Deletes all filters
  while (chain_.size() > 0 && list_iterator != chain_.end()) {
    delete (*list_iterator);
    ++list_iterator;
  }
  */
}

// Recomputes the graph based on the new positions of the discs
void UGenGraphBuilder::rebuild(){
  // Make a minimal spanning tree of the effects with each input


}

// Processes a single sample. Note that you must first handoff audio 
// and midi data to the graph by using the handoff_audio and 
// handoff midi functions
double UGenGraphBuilder::tick(){
  return 0;
}

// Prints all data about the graph, including the nodes,
  // their type and positions
void UGenGraphBuilder::print_all(){
  std::cout << "********************" << std::endl;
  std::cout << "Inputs:" << std::endl;
  for (int i = 0; i < inputs_.size(); ++i){
    std::cout << "\t"<< inputs_.at(i)->get_ugen()->name() << " ";
    std::cout << inputs_.at(i)->pos_ << std::endl;
  }

  std::cout << "Midi:" << std::endl;
  for (int i = 0; i < midi_modules_.size(); ++i){
    std::cout << "\t"<< midi_modules_.at(i)->get_ugen()->name() << " ";
    std::cout << midi_modules_.at(i)->pos_ << std::endl;
  }

  std::cout << "Effects:" << std::endl;
  for (int i = 0; i < fx_.size(); ++i){
    std::cout << "\t"<< fx_.at(i)->get_ugen()->name() << " ";
    std::cout << fx_.at(i)->pos_ << std::endl;
  }
}


// Passes any audio samples to the Input ugens. 
void UGenGraphBuilder::handoff_audio(double sample){
  int i = 0;
  //Process each effect in chain
  while (i < inputs_.size()) {
    static_cast<Input *>(inputs_.at(i)->get_ugen())->set_sample( sample );
    ++i;
  }
}

// Passes any midi notes the MidiUnitGenerators. Decides using the
// value of velocity whether the event is a note on or a note off
void UGenGraphBuilder::handoff_midi(int MIDI_pitch, int velocity){
  int i = 0;
  //Process each effect in chain
  while (i < midi_modules_.size()) {
    // Stops the note
    if (velocity == 0){
      static_cast<MidiUnitGenerator *>(midi_modules_.at(i)->get_ugen())
                                      ->stop_note(MIDI_pitch);
    }
    // Starts the note
    else{
      static_cast<MidiUnitGenerator *>(midi_modules_.at(i)->get_ugen())
                                      ->play_note(MIDI_pitch, velocity);
    }
    ++i;
  }
}



// Adds a unit generator to the signal chain
bool UGenGraphBuilder::add_effect(Disc *fx){
  if (!fx->get_ugen()->is_input() && !fx->get_ugen()->is_midi()){
    fx_.push_back(fx);
    return true;
  }
  return false;

}

// Adds midi unit generator to a list of objects that must be checked
// when new midi event is created
bool UGenGraphBuilder::add_input(Disc *input){
  if (input->get_ugen()->is_input()){
    inputs_.push_back(input);
    return true;
  }
  return false;
}

// Adds midi unit generator to a list of objects that must be checked
// when new midi event is created
bool UGenGraphBuilder::add_midi_ugen(Disc *mugen){
   if (mugen->get_ugen()->is_midi()){
    midi_modules_.push_back(mugen);
    return true;
  }
  return false;
}

bool UGenGraphBuilder::remove_disc(Disc *d){
  std::vector<Disc *> *vec;
  // Figures out which vector to look in
  if (d->get_ugen()->is_input()){
    if (d->get_ugen()->is_midi()) vec = &midi_modules_;
    else vec = &inputs_;
  }
  else vec = &fx_;

  std::vector<Disc *>::iterator it = vec->begin();
  while (it != vec->end()){
    if ((*it) == d){
      delete *it;
      vec->erase(it);
      return true;
    }  ++it;
  } 
  return false;
}


// The distance between two discs
double UGenGraphBuilder::get_edge_cost(Disc* a, Disc* b){
  return (a->pos_ - b->pos_).length();
}


GraphData::GraphData(){}
GraphData::~GraphData(){}