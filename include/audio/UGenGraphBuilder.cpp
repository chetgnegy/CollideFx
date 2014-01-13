/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  UGenGraphBuilder.cpp
  This class uses the positions of the discs and 
  the ugens to build a signal flow graph
*/

#include "UGenGraphBuilder.h"


bool compare_wires(Wire i, Wire j);

UGenGraphBuilder::UGenGraphBuilder(){
  buffer_ready_ = false;
  anti_aliasing_ = new DigitalLowpassFilter(15000, 1, 1);
  low_pass_ = new DigitalHighpassFilter(10, 1, 1);
}

UGenGraphBuilder::~UGenGraphBuilder(){
  delete low_pass_;  
  delete anti_aliasing_;
  delete[] fft_visual_;
}

void UGenGraphBuilder::initialize(int length, int sample_rate){
  buffer_length_ = length;
  fft_visual_ = new complex[buffer_length_];
  UnitGenerator::set_audio_settings(length, sample_rate);
}


// Prints all data about the graph, including the nodes,
// their type and positions
void UGenGraphBuilder::print_all(){
  std::cout << "********************" << std::endl;
  if(inputs_.size()>0)std::cout << "Inputs:" << std::endl;
  for (int i = 0; i < inputs_.size(); ++i){
    std::cout << "\t"<< inputs_[i]->get_ugen()->name() << " ";
    std::cout << inputs_[i]->pos_ << std::endl;
  }

  if(midi_modules_.size()>0)std::cout << "Midi:" << std::endl;
  for (int i = 0; i < midi_modules_.size(); ++i){
    std::cout << "\t"<< midi_modules_[i]->get_ugen()->name() << " ";
    std::cout << midi_modules_[i]->pos_ << std::endl;
  }

  if(fx_.size()>0)std::cout << "Effects:" << std::endl;
  for (int i = 0; i < fx_.size(); ++i){
    std::cout << "\t"<< fx_[i]->get_ugen()->name() << " ";
    std::cout << fx_[i]->pos_ << std::endl;
  }

  if(to_delete_.size()>0)std::cout << "Deleted:" << std::endl;
  for (int i = 0; i < to_delete_.size(); ++i){
    std::cout << "\t"<< to_delete_[i]->get_ugen()->name() << " ";
    std::cout << to_delete_[i]->pos_ << std::endl;
  }
}



// Recomputes the graph based on the new positions of the discs
void UGenGraphBuilder::rebuild(){
  wires_.clear();
  old_sinks_.clear();
  old_sinks_ = sinks_;
  sinks_.clear();

  int num_inputs = inputs_.size() + midi_modules_.size();
  int num_nodes = num_inputs+ fx_.size();
  if (num_nodes == 0) return;

  bool marked[num_nodes];
  bool is_sink[num_nodes];

  past_signature_ = signature_; 

  for (int i = 0; i < num_nodes; ++i){
    if (data_.count(indexed(i))){
      data_[indexed(i)].past_inputs_ = data_[indexed(i)].inputs_;
      data_[indexed(i)].past_outputs_ = data_[indexed(i)].outputs_;
      data_[indexed(i)].inputs_.clear();
      data_[indexed(i)].outputs_.clear();
      
    }
    else {
      data_[indexed(i)] = GraphData();
    }
    marked[i] = false;
    is_sink[i] = false;
  }

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
            if (!indexed(i)->get_ugen()->is_input() ||
                !indexed(j)->get_ugen()->is_input() ){
                
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
  
  // Sorts the wires to ensure that the directionality is stable
  std::sort(wires_.begin(), wires_.end(), compare_wires);
  signature_ = compute_signature();
  

  // Make sure inputs are only transmitting
  bool finalized[wires_.size()];
  for (int i = 0; i < wires_.size(); ++i) finalized[i] = false;
  // First pass
  for (int i = 0; i < wires_.size(); ++i){
    if (wires_[i].second->get_ugen()->is_input()){
      switch_wire_direction(wires_[i]);
      finalized[i] = true;
    }
    else if (wires_[i].first->get_ugen()->is_input()){
      finalized[i] = true;
    }
    // Give some preference to the looper
    else if (wires_[i].second->get_ugen()->is_looper() &&
      wires_[i].second->get_ugen()->is_input()){
      switch_wire_direction(wires_[i]);
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

  for (int i = 0; i < wires_.size(); ++i){
    data_[wires_[i].second].inputs_.push_back(wires_[i].first);
    data_[wires_[i].first].outputs_.push_back(wires_[i].second);
  }

  for (int i = 0; i < num_nodes; ++i){
    data_[indexed(i)].computed = false;
    if (data_[indexed(i)].outputs_.size() == 0){
      sinks_.push_back(indexed(i));
    }
    if (data_[indexed(i)].outputs_.size() == 1
      && data_[indexed(i)].outputs_[0]->get_ugen()->is_looper()){
      sinks_.push_back(indexed(i));
    }
  }
}

std::string UGenGraphBuilder::compute_signature(){
  std::stringstream s ;
  for (int i = 0; i < wires_.size(); i++){
    s << wires_[i].first->getID() << ":" << wires_[i].second->getID() << "."; 
  }
  return s.str();
}


// Sorts the wires by the addresses of their endpoints. This sort isn't 
// especially meaningful, but it provides stability in the connections 
// from rebuild to rebuild. The order in which wires are created should 
// not effect their directionality.
bool compare_wires (Wire i, Wire j){
  return i.first < j.first || (i.first == j.first && i.second < j.second);
}


// Processes a whole buffer. Note that you must first handoff audio 
// and midi data to the graph by using the handoff_audio and 
// handoff midi functions (mono)
void UGenGraphBuilder::load_buffer(double *out, int frames){
  // Zero out old array
  for (int i = 0; i < frames; ++i) out[i] = 0;
      
  double *temp;
  find_mix_levels();
  // There has been a change in the graph
  if (signature_ != past_signature_){
    int num_nodes = inputs_.size() + midi_modules_.size() 
                   + fx_.size() + to_delete_.size();

    // Compute the save states of all discs, including recently deleted discs
    std::map<Disc *, UGenState *> saved_states;
    for (int i = 0; i < num_nodes; ++i){
      UGenState *k = indexed(i)->get_ugen()->save_state();
      saved_states[indexed(i)] = k;
    }

    // Compute for past graph, this stores a buffer in the data_ object
    // that is accessed by sending a 2 state to the pull_result_buffer 
    // method
    for (std::vector<Disc *>::iterator it = old_sinks_.begin(); 
      it != old_sinks_.end(); ++it) {
      pull_result_buffer(*it, frames, 1);
    }  

    // Recover saved states
    for (std::map<Disc *, UGenState *>::iterator it = saved_states.begin(); 
      it != saved_states.end(); ++it){
      it->first->get_ugen()->recall_state(it->second);      
      data_[it->first].computed = false;
    }

    // Compute for present graph (internally crossfading)
    for (std::vector<Disc *>::iterator it = sinks_.begin(); 
      it != sinks_.end(); ++it) {
      temp = pull_result_buffer(*it, frames, 2);
      for (int i = 0; i < frames; ++i) out[i] += temp[i];
    }

    // Cleans up any extra buffers that were created during crossfade
    for (std::map<Disc *, UGenState *>::iterator it = saved_states.begin(); 
      it != saved_states.end(); ++it){
      if (data_[it->first].need_crossfade_){
        delete[] data_[it->first].crossfade_wet_;
        delete[] data_[it->first].crossfade_dry_;
        data_[it->first].need_crossfade_ = false;
      }  
    }

    // Cleans up any discs that are on the to_delete list
    finalize_delete();
  }

  // The graph has not changed
  else {
    for (std::vector<Disc *>::iterator it = sinks_.begin(); 
      it != sinks_.end(); it++) {
      temp = pull_result_buffer(*it, frames, 0);
      // copy new branch into output buffer
      for (int i = 0; i < frames; ++i){
        out[i] += temp[i];
      }
    }
  }

  for (int i = 0; i < frames; ++i){
    //Filters the signal to remove HF and DC components
    low_pass_->tick(out[i]);
    anti_aliasing_->tick(low_pass_->most_recent_sample());
    out[i] = anti_aliasing_->most_recent_sample().re();
  }

}


// Reverses the push architechture of "out = tick(in)" to recursively pull
// samples to the output sinks from the inputs. Works on an entire buffer
// The buffer out is cleared of any previous contents
// State - 0: Normal, 1: Prepare, 2: Recall
double *UGenGraphBuilder::pull_result_buffer(Disc *k, int length, int state){
  if (data_[k].computed) return k->get_ugen()->current_buffer();

  double *wet = new double[length];
  double *dry = new double[length];
  for (int i = 0; i < length; ++i){ 
    wet[i] = 0; dry[i] = 0;
  }

  double *temp, wet_level, scale;
  // Current Inputs
  std::vector<Disc *> input_list;
  if (state == 1){
    input_list = data_[k].past_inputs_;
  } else {
    input_list = data_[k].inputs_;
  }

  for (std::vector< Disc* >::iterator it = input_list.begin(); 
    it != input_list.end(); ++it) {
    // Finds mix level
    wet_level = get_mix_level(k, *it);
    
    // Scales down based on fan out
    if (state == 1) {
      scale = scale_factor( data_[*it].past_outputs_.size() );
    }
    else{
      scale = scale_factor( data_[*it].outputs_.size() );
    } 
    // Computes buffer coming from previous ugen
    temp = pull_result_buffer(*it, length, state);
    
    // Computes wet dry mix
    for (int i = 0; i < length; ++i){
      wet[i] += wet_level * scale * temp[i];
      dry[i] += (1-wet_level) * scale * temp[i];
    }  
  }

  // We need to store the input buffers so that they may be 
  // used in the recall state
  if (state == 1){
    data_[k].crossfade_wet_ = new double[length];
    data_[k].crossfade_dry_ = new double[length];
    for (int i = 0; i < length; ++i){
      data_[k].crossfade_wet_[i] = wet[i]; 
      data_[k].crossfade_dry_[i] = dry[i]; 
    }
    data_[k].need_crossfade_ = true;
  }

  // We recall the state and crossfade previous inputs and past inputs 
  // before they go into the current disc
  if (state == 2){
    if (data_[k].need_crossfade_) {
      double frac = 0;
      for (int i = 0; i < length; ++i){
        frac = i / (length * 1.0);
        wet[i] = frac * data_[k].crossfade_wet_[i] + (1-frac) * wet[i]; 
        dry[i] = frac * data_[k].crossfade_dry_[i] + (1-frac) * dry[i]; 
      }
    }
  }

  double *out_buffer = k->get_ugen()->process_buffer(wet, length);
  
  data_[k].computed = true;
  // Merges wet and dry
  if (!k->get_ugen()->is_input()){
    for (int i = 0; i < length; ++i){
      out_buffer[i] += dry[i];
    }
  }
  delete[] wet;
  delete[] dry;
  return out_buffer;
}

void UGenGraphBuilder::find_mix_levels(){
  wet_levels_.clear();
  int num_nodes = inputs_.size() + midi_modules_.size() + fx_.size() + to_delete_.size();
  for ( int i = 0; i < num_nodes; ++i ){
    for ( int j = i + 1; j < num_nodes; ++j ){
      if (indexed(i) < indexed(j)){
        wet_levels_[indexed(i)][indexed(j)] = compute_mix_level(indexed(i), indexed(j));
      }
      else{
        wet_levels_[indexed(j)][indexed(i)] = compute_mix_level(indexed(j), indexed(i));
      }
    }
  }
}

double UGenGraphBuilder::get_mix_level(Disc *a, Disc *b){
  if (a < b)return wet_levels_[a][b];
  return wet_levels_[b][a];
}


double UGenGraphBuilder::compute_mix_level(Disc *a, Disc *b){
  double both_radii = a->get_radius() + b->get_radius();
  double separation = (a->pos_ - b->pos_).length() - both_radii;
  double mix = 1 - separation/(kMaxDist- both_radii);
  mix = fmax(0, fmin(1, mix));
  return mix;
}



// Changes the mutex. Should be called when using anything related to the 
// audio path
void UGenGraphBuilder::lock_thread(bool lock){
  if (lock) audio_lock_.lock();
  else audio_lock_.unlock();
}




// Passes any audio samples to the Input ugens. 
void UGenGraphBuilder::handoff_audio(double sample){
  int i = 0;
  // Process each effect in chain
  while (i < inputs_.size()) {
    static_cast<Input *>(inputs_.at(i)->get_ugen())->set_sample( sample );
    ++i;
  }
}

// Passes any audio samples to the Input ugens. 
void UGenGraphBuilder::handoff_audio_buffer(double buffer[], int length){
  int i = 0;
  // Process each effect in chain
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


// Lets the other thread know that the depenencies are ready to compute
void UGenGraphBuilder::signal_new_buffer(){ buffer_ready_ = true; }

// Recalculates the FFT and moves the orbs around. This is called in between
// audio buffers. It is called from the graphics thread
void UGenGraphBuilder::update_graphics_dependencies(){
  calculate_fft();
  
  int num_nodes = inputs_.size() + midi_modules_.size() + fx_.size();
  
  int inputs, outputs;
  for (int i = 0; i < num_nodes; ++i){
    indexed(i)->excite(indexed(i)->get_ugen()->buffer_energy());

    // Shuffles Orbs around
    if (rand()%6 == 0){
      outputs = data_[indexed(i)].outputs_.size();
      if (outputs > 0){
        bool try_handoff = true;
        do{
          // Don't lose orbs while they are in transit!
          try_handoff = indexed(i)->orb_handoff(
                        data_[indexed(i)].outputs_[rand() % outputs]);
        }while(indexed(i)->above_capacity() && try_handoff);
      }
      else{
        // it's a sink
        inputs = data_[indexed(i)].inputs_.size();
        if (inputs > 0){
          indexed(i)->orb_limit();
        }
        else if (!indexed(i)->get_ugen()->is_input()
                && !indexed(i)->get_ugen()->is_looper()){
          // Lone discs shed orbs
          if (rand()%7 == 0) indexed(i)->orb_abandon();
        }

      }
      indexed(i)->orb_repopulate();
    }
  }
}

// #------------ Modify Graph -------------#


// Adds a unit generator to the signal chain
bool UGenGraphBuilder::add_effect(Disc *fx){
  if (!fx->get_ugen()->is_input() && !fx->get_ugen()->is_midi() 
                       || fx->get_ugen()->is_looper()){
    fx_.push_back(fx);
    return true;
  }
  return false;

}

// Adds midi unit generator to a list of objects that must be checked
// when new midi event is created
bool UGenGraphBuilder::add_input(Disc *input){
  if (input->get_ugen()->is_input() && !input->get_ugen()->is_looper()){
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
  if (d->get_ugen()->is_input() && !d->get_ugen()->is_looper()){
    if (d->get_ugen()->is_midi()) vec = &midi_modules_;
    else vec = &inputs_;
  }
  else vec = &fx_;

  // Puts disc in to_delete vector
  std::vector< Disc* >::iterator it = vec->begin();
  while (it != vec->end()){
    if ((*it) == d){
      to_delete_.push_back(*it);
      it = vec->erase(it);
      return true;
    } 
    else ++it;
  } 
  return false;
}

bool UGenGraphBuilder::finalize_delete(){
  if (to_delete_.size() == 0) return false;

  //Deletes discs from to_delete_ vector
  std::vector< Disc* >::iterator it = to_delete_.begin();
  while (it != to_delete_.end()){
    // Deletes data associated with disc
    if (data_.count(*it)){
      std::map < Disc *, GraphData >::iterator itr = data_.begin();
      while (itr != data_.end()) {
        if (itr->first == *it) {
          data_.erase(itr);
          break;
        }
        else ++itr;
      }
    }
    
    delete *it;
    it = to_delete_.erase(it);
  } 
  return true;
  
}

// #--------------- FFT ----------------#


// The graphics thread can grab this and display it. This keeps an average of the
// most recent spectra
void UGenGraphBuilder::calculate_fft(){
  int num_fft_stored = 16;
  if (Disc::spotlight_disc_ != NULL){
    complex *fft_ = new complex[buffer_length_];
    Disc::spotlight_disc_->get_ugen()->buffer_fft(buffer_length_, fft_);
    fft_list_.push_back(fft_);
    if (fft_list_.size() > num_fft_stored){
      delete[] *fft_list_.begin();
      fft_list_.pop_front();
    }
  }
  
  for (int i = 0; i < buffer_length_; ++i){
    fft_visual_[i] = 0;
  }
  double weight, weight_total = 0;
  int k = 0;
  for (std::list<complex *>::iterator it = fft_list_.begin(); 
      it!=fft_list_.end(); ++it){
    weight = 1/(++k*1.0+7);
    for (int i = 0; i < buffer_length_; ++i){
      fft_visual_[i]+= (*it)[i] * weight;
    }
    weight_total += weight;
  }
  for (int i = 0; i < buffer_length_; ++i){
    fft_visual_[i] /= weight_total;
  }
}

complex *UGenGraphBuilder::get_fft(){ return fft_visual_; }


// #--------------- UI ----------------#

// This is called when the up button is pressed in the menu
void UGenGraphBuilder::handle_up_press(){
  if (Disc::spotlight_disc_ != NULL){
    if (strcmp("Filter", Disc::spotlight_disc_->get_ugen()->name())==0){
      Filter *f = static_cast<Filter *>(Disc::spotlight_disc_->get_ugen());
      f->set_lowpass(!f->is_lowpass()); // Flips filter type
    }
    else if (strcmp("Looper", Disc::spotlight_disc_->get_ugen()->name())==0){
      Looper *l = static_cast<Looper *>(Disc::spotlight_disc_->get_ugen());
      l->set_start_counter((l->get_start_counter()) % 7 + 1);
    }
  }
}

// This is called when the down button is pressed in the menu
void UGenGraphBuilder::handle_down_press(){
  if (Disc::spotlight_disc_ != NULL){
    if (strcmp("Filter", Disc::spotlight_disc_->get_ugen()->name())==0){
      Filter *f = static_cast<Filter *>(Disc::spotlight_disc_->get_ugen());
      f->set_lowpass(!f->is_lowpass()); // Flips filter type
    }
    else if (strcmp("Looper", Disc::spotlight_disc_->get_ugen()->name())==0){
      Looper *l = static_cast<Looper *>(Disc::spotlight_disc_->get_ugen());
      l->set_start_counter((l->get_start_counter() + 5) % 7 + 1);
    }
  }
}

// This lets the menu know whether or not the arrows will do anything
bool UGenGraphBuilder::selector_activated(){
  if (Disc::spotlight_disc_ != NULL){
    if (strcmp("Filter", Disc::spotlight_disc_->get_ugen()->name())==0){
      return true;
    }
    else if (strcmp("Looper", Disc::spotlight_disc_->get_ugen()->name())==0){
      return true;
    }
  }
  return false;
}

// The text that appears in the little box next to the arrows on the menu
const char *UGenGraphBuilder::text_box_content(){
  if (Disc::spotlight_disc_ != NULL){
    if (strcmp("Filter", Disc::spotlight_disc_->get_ugen()->name())==0){
      Filter *f = static_cast<Filter *>(Disc::spotlight_disc_->get_ugen());
      if(f->is_lowpass())
        return "LPF";
      return "HPF";
    }
    if (strcmp("Looper", Disc::spotlight_disc_->get_ugen()->name())==0){
      Looper *l = static_cast<Looper *>(Disc::spotlight_disc_->get_ugen());
      sprintf(l->param3_str_, " %d",l->get_start_counter());
      return l->param3_str_;

    }
  }
  return "";
}

// The label that appears below the little box next to the arrows on the menu
const char *UGenGraphBuilder::text_box_label(){
  if (Disc::spotlight_disc_ != NULL){

    if (strcmp("Looper", Disc::spotlight_disc_->get_ugen()->name())==0){
      return "Count In";
    }
  }
  return "";
}

// #------------- Private --------------#



// The distance between two discs
double UGenGraphBuilder::get_edge_cost(Disc* a, Disc* b){
  return (a->pos_ - b->pos_).length();
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
  if (i<inputs_.size() + midi_modules_.size() + fx_.size() + to_delete_.size()){
    return to_delete_[i - inputs_.size() - midi_modules_.size() - fx_.size()];
  }
  return NULL;
}



// Scales down due to fan out
double UGenGraphBuilder::scale_factor(int factor){
  double scale = 1;
  if (factor>0){
    scale = 1 / pow(factor, 0.5);
  }
  return scale;
}


// Reverses the "to" and "from" ends of a wire
void UGenGraphBuilder::switch_wire_direction(Wire &w){
  Disc *temp = w.first;
  w.first = w.second;
  w.second = temp;

}

