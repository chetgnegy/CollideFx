/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Physics.h
  A physics engine to move the discs, and particles around within the world.
  */

#include "Physics.h"

std::list<Physical *> Physics::all_;

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
  
  //Collision Detection
  bool too_close = check_reduce_timestep();
  
  double update = too_close ? update_time/10.0 : update_time;
  for (int i = 0; i < too_close * 10 + 1 ; ++i){
    
    if (all_.size() > 0) {
      std::list<Physical *>::iterator it;
      it = all_.begin();
      while (it != all_.end()) {
        velocity_verlet(update, *it); 
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
  double mu_k = 3.99;
      
  Vector3d v_half = obj->vel_ + obj->acc_ * 0.5 * timestep ;
  obj->pos_ += v_half * timestep;
  Vector3d acc_next_ = obj->external_forces();
  
  // Friction
  if (obj->uses_friction()){
      if ( obj->vel_.length() > timestep * mu_k * timestep){
        Vector3d v_norm = obj->vel_ / obj->vel_.length();
        acc_next_ += - v_norm * mu_k;
      }
      else {
        obj->vel_ = Vector3d(0,0,0);
      }
  }
  obj->vel_ += acc_next_ * 0.5 * timestep;   

}


// Uses vector projections to make sure things don't get too close to each other
void Physics::collision_prevention(){
    Vector3d between;
    if (all_.size() > 1) {
      std::list<Physical *>::iterator it_a;
      std::list<Physical *>::iterator it_b;
      it_a = all_.begin();
      while (it_a != all_.end()) {
        if ( (*it_a)->has_collisions() ){ // Only if A can collide
          it_b = it_a;
          while (it_b != all_.end()) {
            // Two different objects
            if ( it_b != it_a && (*it_b)->has_collisions()) { // Only if B can collide
              //  Check if radii intersect
              between = (*it_b)->pos_ - (*it_a)->pos_;
              if (between.length() < (*it_b)->intersection_distance() 
                  + (*it_a)->intersection_distance()){
                double d = between.lengthSq();
                //  Project vectors onto vector connecting radii
                Vector3d b_proj = between * between.dotProduct((*it_b)->vel_)/d;
                Vector3d a_proj = between * between.dotProduct((*it_a)->vel_)/d;
                (*it_b)->vel_ -= b_proj * 2;
                (*it_a)->vel_ -= a_proj * 2;
              }
            } ++it_b;
          } 
        } ++it_a;
      }

    }
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
        while (it_b != all_.end()) {
          // Two different objects
          if ( it_b != it_a && (*it_b)->has_collisions()) {
            //  Check if radii intersect
            Vector3d between = (*it_b)->pos_ - (*it_a)->pos_;
            if (between.length() < 1.005*((*it_b)->intersection_distance() 
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