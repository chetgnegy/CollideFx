/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Physics.h
  A physics engine to move the discs, and particles around within the world.
  */

#include "Physics.h"

typedef std::pair< Physical *, Physical *> Collision;
std::list<Physical *> Physics::all_;
std::list<Collision *> Physics::recent_collisions_;

// Removes physics from an object
bool Physics::take_physics(Physical *object){
  if (all_.size() > 0) {
    std::list<Physical *>::iterator it;
    it = all_.begin();
    while (it != all_.end()) {
      if (*it == object){
        all_.erase(it);
        return true;
      }
      ++it;
    }
  }
  return false;
}

// Adds an object for which physics will be computed
void Physics::give_physics(Physical *object){
  all_.push_back(object); 
}

// Updates the positions of all objects 
void Physics::update(double update_time){
  double max_iterations = 500.0;
  double standard_iterations = 10.0;
  //Collision Detection
  bool too_close = check_reduce_timestep();
  //We use a smaller timestep when things get close together.
  double update = too_close ? update_time/max_iterations : update_time/standard_iterations;
  double iterations = too_close ? max_iterations : standard_iterations;
  

  for (int i = 0; i < iterations; ++i){
    
    if (all_.size() > 0) {
      std::list<Physical *>::iterator it;
      it = all_.begin();
      while (it != all_.end()) {
        velocity_verlet(update, *it); 
        angular_verlet(update, *it); 
        ++it;
      }
    }

    //only need to prevent collisions if things got close
    if (too_close) collision_prevention();
    
  
    
  }
}

// Uses Velocity Verlet integration to compute the next positions of the object
void Physics::velocity_verlet(double timestep, Physical* obj){
  //kinetic friction
  double mu_k = 0.99;
      
  Vector3d v_half = obj->vel_ + obj->acc_ * 0.5 * timestep ;
  obj->pos_ += v_half * timestep;
  Vector3d acc_next = obj->external_forces();
  
  // Friction
  if (obj->uses_friction()){
      if ( obj->vel_.length() > timestep * mu_k * timestep){
        Vector3d v_norm = obj->vel_ / obj->vel_.length();
        acc_next += - v_norm * mu_k;
      }
      else {
        obj->vel_ = Vector3d(0,0,0);
      }
  }
  obj->vel_ += acc_next * 0.5 * timestep;   

}

// Velocity Verlet algorithm applied to the angular motion
void Physics::angular_verlet(double timestep, Physical* obj){
  //kinetic friction
  double mu_k = 3.99;
      
  Vector3d w_half = obj->ang_vel_ + obj->ang_acc_ * 0.5 * timestep ;
  obj->ang_pos_ += w_half * timestep;
  Vector3d ang_acc_next = obj->external_torques();
  /*
  // Friction
  if (obj->uses_friction()){
      if ( obj->ang_vel_.length() > timestep * mu_k * timestep){
        Vector3d w_norm = obj->ang_vel_ / obj->ang_vel_.length();
        ang_acc_next += - w_norm * mu_k;
      }
      else {
        obj->ang_vel_ = Vector3d(0,0,0);
      }
  }*/
  obj->ang_vel_ += ang_acc_next * 0.5 * timestep;   

}


// Uses vector projections to make sure things don't get too close to each other
void Physics::collision_prevention(){
    int to_remove = recent_collisions_.size();
    
    if (all_.size() > 1) {
      std::list<Physical *>::iterator it_a;
      std::list<Physical *>::iterator it_b;
      it_a = all_.begin();
      while (it_a != all_.end()) {
        if ( (*it_a)->has_collisions() ){ // Only if A can collide
          it_b = it_a;
          ++it_b;
          while (it_b != all_.end()) {
            if ((*it_b)->has_collisions()) { // Only if B can collide
              collide(*it_a, *it_b);
              
            } ++it_b;
          } 
        } ++it_a;
      }

    }

  std::list<Collision *>::iterator it;
  int count = 0;
  it = recent_collisions_.begin();
  while (count < to_remove && it != recent_collisions_.end()){
    delete *it;
    it = recent_collisions_.erase(it);
    ++it;
    ++count;
  }
}

// Handles a collision using conservation of linear momentum;
void Physics::collide(Physical* a, Physical* b){
  Vector3d between = b->pos_ - a->pos_;
              
  if (between.length() < b->intersection_distance() 
    + a->intersection_distance()){

    // Checks to see if it has collided recently
    if (!on_collision_list(a,b)){
      double d = between.lengthSq();
      // Project vectors onto vector connecting radii
      Vector3d b_proj = between * between.dotProduct(b->vel_)/d;
      Vector3d a_proj = between * between.dotProduct(a->vel_)/d;
      //Perfectly inelastic collison. Momentum is conserved.
      double term1_1 = (a->m_-b->m_) / (a->m_ + b->m_);
      double term1_2 = (2*b->m_) / (a->m_ + b->m_);
      double term2_1 = (2*a->m_) / (a->m_ + b->m_);
      double term2_2 = (b->m_-a->m_) / (a->m_ + b->m_);
      a->vel_ = a->vel_ - a_proj + a_proj * term1_1 + b_proj * term1_2;
      b->vel_ = b->vel_ - b_proj + a_proj * term2_1 + b_proj * term2_2;
      recent_collisions_.push_back( new Collision(a, b) );
    }
  }
}

// Checks to see if a pair has collided recently
bool Physics::on_collision_list(Physical* a, Physical* b){
  std::list<Collision *>::iterator it;
  it = recent_collisions_.begin();
  while (it != recent_collisions_.end()){
    if (a == (*it)->first && b == (*it)->second){
      return true;
    }
    ++it;
  }
  return false;
}

// Check to see if two objects are so close that we should reduce the timestep
bool Physics::check_reduce_timestep(){
  if (all_.size() > 1) {
    std::list<Physical *>::iterator it_a;
    std::list<Physical *>::iterator it_b;
    it_a = all_.begin();
    while (it_a != all_.end()) {
      if ( (*it_a)->has_collisions()){
        it_b = it_a;
        ++it_b;
        while (it_b != all_.end()) {
          if ((*it_b)->has_collisions()) {
            //  Check if radii intersect
            Vector3d between = (*it_b)->pos_ - (*it_a)->pos_;
            if (between.length() < 1.05*((*it_b)->intersection_distance() 
              + (*it_a)->intersection_distance())){
              
             return true;
            }
            
          } ++it_b;
        }
      } ++it_a;
    }
  }
  return false;
}