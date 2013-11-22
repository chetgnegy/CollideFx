/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenGraphBuilder.cpp
  This class uses the positions of the discs and 
  the ugens to build a signal flow graph
*/

#include "UGenGraphBuilder.h"


bool compare_wires (Wire i, Wire j);

UGenGraphBuilder::UGenGraphBuilder(int length){
  buffer_length_ = length;
}

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
  wires_.clear();
  data_.clear();
  sinks_.clear();

  int num_inputs = inputs_.size() + midi_modules_.size();
  int num_nodes = num_inputs+ fx_.size();
  if (num_nodes == 0) return;



  bool marked[num_nodes];
  bool is_sink[num_nodes];


  for (int i = 0; i < num_nodes; ++i){
    data_[indexed(i)] = GraphData();
    marked[i] = false;
    is_sink[i];
  }
/*
  // Get all of the edges -- I don't actually need this!
  double dist;
  for (int i = 0; i < num_nodes; ++i){
    for (int j = i+1; j < num_nodes; ++j){
      dist = get_edge_cost(indexed(i),indexed(j));
      data_[indexed(i)].edges_.push_back(Edge( indexed(j), dist ));
      data_[indexed(j)].edges_.push_back(Edge( indexed(i), dist ));
    }
  }
  std::cout << "Listing Edges" << std::endl;
  for (int i = 0; i < num_nodes; ++i){
    std::cout << indexed(i)->get_ugen()->name() << std::endl;
    data_[indexed(i)].list_edges();
  }
  */


  // Modified Prim's Algorithm -- May not result in spanning tree 
  // if distances are too far to make a wire! Also, there is the constraint 
  // that two inpput nodes cannot connect with each other

  double this_dist, min_dist;
  int next_i = 0, next_j =0;

  marked[0] = true;
  bool nothing_happened = false;
  while (!nothing_happened){
    min_dist = 10e12;
    nothing_happened = true;
    for (int i = 0; i < num_nodes; ++i){
      if (marked[i]){
        for (int j = 0; j < num_nodes; ++j){
          //Connecting an attached node to an unattached node
          if (!marked[j]){
            // Not connecting two inputs
            if (i >= num_inputs || j >= num_inputs){
                
              this_dist = get_edge_cost(indexed(i), indexed(j));
              if (this_dist < min_dist && this_dist < kMaxDist){
                next_j = j;
                next_i = i;
                min_dist = this_dist;
                nothing_happened = false;
              }
            }
          }
        }
      }
    }

    if (nothing_happened){
      for (int i = 0; i < num_nodes; ++i){
        if (!marked[i]){
          marked[i] = true; 
          nothing_happened = false;
          break;
        }
      }
    }
    else{
      marked[next_i] = true;
      marked[next_j] = true;
      wires_.push_back(Wire(indexed(next_i), indexed(next_j)));
    }
  }
  
  // Sorts the wires so that the directionality is stable
  //std::sort(wires_.begin(), wires_.end(), compare_wires);

  // Make sure inputs are only transmitting
  bool finalized[wires_.size()];
  for (int i = 0; i < wires_.size(); ++i)finalized[i] = false;
  // First pass
  for (int i = 0; i < wires_.size(); ++i){
    if (wires_[i].second->get_ugen()->is_input()){
      switch_wire_direction(wires_[i]);
      finalized[i] = true;
    }
    else if (wires_[i].first->get_ugen()->is_input()){
      finalized[i] = true;
    }

  }
  //Iterate until convergence
  while (!nothing_happened){
    nothing_happened = true;
    // Make propagate transmission
    for (int i = 0; i < wires_.size(); ++i){
      if (finalized[i]){
        for (int j = 0; j < wires_.size(); ++j){
          if (!finalized[j]){
            if (wires_[i].second == wires_[j].second){
              switch_wire_direction(wires_[j]);
              nothing_happened = false;
              finalized[j] = true;
            }
            else if (wires_[i].second == wires_[j].first){
              finalized[j] = true;
            }
          }
        }
      }
    }
  }

  std::cout << "Listing Wires" << std::endl;
  for (int i = 0; i < wires_.size(); ++i){

    std::cout << "\t" << wires_[i].first->get_ugen()->name() << " ";
    std::cout << "\t" << wires_[i].second->get_ugen()->name() << std::endl;
    data_[wires_[i].second].inputs_.push_back(wires_[i].first);
    data_[wires_[i].first].outputs_.push_back(wires_[i].second);
  }

  for (int i = 0; i < num_nodes; ++i){
    if (data_[indexed(i)].outputs_.size() == 0){
      sinks_.push_back(indexed(i));
    }
  }
  
}


void UGenGraphBuilder::lock_thread(bool lock){
  if (lock) audio_lock_.lock();
  else audio_lock_.unlock();
}

// Sorts the wires by the addresses of their endpoints. This sort isn't 
// especially meaningful, but it provides stability in the connections 
// from rebuild to rebuild. The order in which wires are created should 
// not effect their directionality.
bool compare_wires (Wire i, Wire j){
  return i.first < j.first || (i.first == j.first && i.second < j.second);
}


void UGenGraphBuilder::switch_wire_direction(Wire &w){
  Disc *temp = w.first;
  w.first = w.second;
  w.second = temp;

}



// Reverses the push architechture of "out = tick(in)" to recursively pull
// samples to the output sinks from the inputs
double UGenGraphBuilder::pull_result(UnitGenerator *k, std::vector<Disc *> inputs){
  double sum = 0;
  if (inputs.size() > 0) {
    std::vector<Disc *>::iterator it;
    it = inputs.begin();
      
    while (it != inputs.end()) {
      double scale = 1;//pow(data_[*it].inputs_.size(), 0.5);
      sum += scale * pull_result((*it)->get_ugen(), data_[*it].inputs_);
      ++it;
    }
  }
  return k->tick(sum);
}


// Reverses the push architechture of "out = tick(in)" to recursively pull
// samples to the output sinks from the inputs. Works on an entire buffer
// The buffer out is cleared of any previous contents
double *UGenGraphBuilder::pull_result_buffer(UnitGenerator *k, std::vector<Disc *> inputs, 
                                  int length){
  double *out = new double[length];
  // Zero out old array
  for (int i = 0; i < length; ++i) out[i] = 0;
      
  if (inputs.size() > 0) {
    std::vector<Disc *>::iterator it;
    it = inputs.begin();
    double *temp;
    while (it != inputs.end()) {
      double scale = 1;//pow(data_[*it].inputs_.size(), 0.5);
      temp = pull_result_buffer((*it)->get_ugen(), data_[*it].inputs_, length);
      // copy new branch into output buffer
      for (int i = 0; i < length; ++i){
        out[i] += temp[i];
      }
      ++it;
    }
  }
  
  return k->process_buffer(out, length);

}


// Processes a single sample. Note that you must first handoff audio 
// and midi data to the graph by using the handoff_audio and 
// handoff midi functions
double UGenGraphBuilder::tick(){
  double sum = 0;
  std::vector<Disc *>::iterator it;
    it = sinks_.begin();
    while (it != sinks_.end()) {
      sum += pull_result((*it)->get_ugen(), data_[*it].inputs_);
      ++it;
    }
  return sum;
}

// Processes a whole buffer. Note that you must first handoff audio 
// and midi data to the graph by using the handoff_audio and 
// handoff midi functions (mono)
void UGenGraphBuilder::load_buffer(double *out, int frames){
  // Zero out old array
  for (int i = 0; i < frames; ++i) out[i] = 0;
      
  std::vector<Disc *>::iterator it;
  it = sinks_.begin();
  double *temp;
  while (it != sinks_.end()) {
    temp = pull_result_buffer((*it)->get_ugen(), data_[*it].inputs_, frames);
    // copy new branch into output buffer
    for (int i = 0; i < frames; ++i){
      out[i] += temp[i];
    }
    ++it;
  }
}


// Prints all data about the graph, including the nodes,
  // their type and positions
void UGenGraphBuilder::print_all(){
  std::cout << "********************" << std::endl;
  std::cout << "Inputs:" << std::endl;
  for (int i = 0; i < inputs_.size(); ++i){
    std::cout << "\t"<< inputs_[i]->get_ugen()->name() << " ";
    std::cout << inputs_[i]->pos_ << std::endl;
  }

  std::cout << "Midi:" << std::endl;
  for (int i = 0; i < midi_modules_.size(); ++i){
    std::cout << "\t"<< midi_modules_[i]->get_ugen()->name() << " ";
    std::cout << midi_modules_[i]->pos_ << std::endl;
  }

  std::cout << "Effects:" << std::endl;
  for (int i = 0; i < fx_.size(); ++i){
    std::cout << "\t"<< fx_[i]->get_ugen()->name() << " ";
    std::cout << fx_[i]->pos_ << std::endl;
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

// Passes any audio samples to the Input ugens. 
void UGenGraphBuilder::handoff_audio_buffer(double buffer[], int length){
  int i = 0;
  //Process each effect in chain
  while (i < inputs_.size()) {
    static_cast<Input *>(inputs_.at(i)->get_ugen())->set_buffer( buffer, length );
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

// Allows all ugens to be called uniformly
Disc *UGenGraphBuilder::indexed(int i){
  if (i<inputs_.size()) return inputs_[i];
  if (i<inputs_.size() + midi_modules_.size()){
    return midi_modules_[i-inputs_.size()];
  }
  if (i<inputs_.size() + midi_modules_.size() + fx_.size()){
    return fx_[i - inputs_.size() - midi_modules_.size()];
  }
  return NULL;
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

void GraphData::list_edges(){
  for (int i = 0; i < edges_.size(); ++i){
    std::cout << "\t[" << edges_[i].first->get_ugen()->name() << 
                   " " << edges_[i].second << "]" << std::endl;
  }

}


