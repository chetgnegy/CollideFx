/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Physics.h
  A physics engine to move the discs, and particles around within the world.
  */

#include "Physics.h"

typedef std::pair< Physical *, Physical *> Collision;
std::list<Physical *> Physics::all_;
double Physics::x_min_ = -10e10, Physics::x_max_ = 10e10;
double Physics::y_min_ = -10e10, Physics::y_max_ = -10e10;

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
  double max_iterations = 50.0;
  double standard_iterations = 50.0;
  //Collision Detection
  bool too_close = check_reduce_timestep();
  //We use a smaller timestep when things get close together.
  double update = too_close ? update_time/max_iterations : update_time/standard_iterations;
  double iterations = too_close ? max_iterations : standard_iterations;
  
  
  //Numerical Integration
  for (int i = 0; i < iterations; ++i){

    if (all_.size() > 0) {
      std::list<Physical *>::iterator it;
      it = all_.begin();
      while (it != all_.end()) {
        (*it)->acc_ = (*it)->external_forces()/(*it)->m_;
        (*it)->ang_acc_ = (*it)->external_torques()/(*it)->I_;
        ++it;
      }
    }

    //only need to prevent collisions if things got close
    if (too_close) collision_prevention(update_time);
    check_in_bounds(update_time);


    if (all_.size() > 0) {
      std::list<Physical *>::iterator it;
      it = all_.begin();
      while (it != all_.end()) {
        velocity_verlet(update, *it); 
        if (!(*it)->rotates()) angular_verlet(update, *it); 
        ++it;
      }
    }


    
  }
}

// Uses Velocity Verlet integration to compute the next positions of the object
void Physics::velocity_verlet(double timestep, Physical* obj){
  //kinetic friction
  double mu_loss = 0.99;
   
  // Friction
  if (obj->uses_friction()){
    if (obj->vel_.length() > 0){
      obj->acc_ += - obj->vel_ / obj->vel_.length() * mu_loss;
    }
  }

  obj->vel_ += obj->acc_ * timestep;   
  obj->pos_ += obj->vel_ * timestep;
}

// Velocity Verlet algorithm applied to the angular motion
void Physics::angular_verlet(double timestep, Physical* obj){
  //kinetic friction
  double mu_loss = 0.7;
     
  // Friction
  if (obj->uses_friction()){
    if (obj->ang_vel_.length() > 0){
      obj->ang_acc_ += - obj->ang_vel_ / obj->ang_vel_.length() * mu_loss;
    }
  }

  obj->ang_vel_ += obj->ang_acc_ * timestep;   
  obj->ang_pos_ += obj->ang_vel_ * timestep; 
}


// Uses vector projections to make sure things don't get too close to each other
void Physics::collision_prevention(double update_time){
    //int to_remove = recent_collisions_.size();
    
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
              collide(*it_a, *it_b, update_time);
              
            } ++it_b;
          } 
        } ++it_a;
      }

    }
}

// Handles a collision using conservation of linear momentum;
void Physics::collide(Physical* a, Physical* b, double update_time){
  Vector3d between = b->pos_ - a->pos_;
              
  if (between.length() < b->intersection_distance() 
    + a->intersection_distance() && (between.dotProduct(a->vel_)>0 || between.dotProduct(b->vel_)<0)){

      double d = between.lengthSq();
      // Project vectors onto vector connecting radii
      Vector3d b_proj = between * between.dotProduct(b->vel_)/d;
      Vector3d a_proj = between * between.dotProduct(a->vel_)/d;
      //Perfectly inelastic collison. Momentum is conserved.
      double term1_1 = (a->m_-b->m_) / (a->m_ + b->m_);
      double term1_2 = (2*b->m_) / (a->m_ + b->m_);
      double term2_1 = (2*a->m_) / (a->m_ + b->m_);
      double term2_2 = (b->m_-a->m_) / (a->m_ + b->m_);
      Vector3d a_tang = a->vel_ - a_proj;
      Vector3d b_tang = b->vel_ - b_proj;
      a->vel_ = a->vel_ - a_proj + a_proj * term1_1 + b_proj * term1_2;
      b->vel_ = b->vel_ - b_proj + a_proj * term2_1 + b_proj * term2_2;
        
      double mu = 3;
      double impulse   = 2 * a->m_ * a->intersection_distance() / update_time / a->I_;
      //a->ang_acc_.z += a_proj.length() / a->vel_.length() * impulse   * mu * a_proj.length();
      double impulse_b = 2 * b->m_ * b->intersection_distance() / update_time / b->I_;
      //b->ang_acc_.z += b_proj.length() / b->vel_.length() * impulse_b * mu * b_proj.length();
      
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

void Physics::check_in_bounds(double update_time){
  if (all_.size() > 0) {
      double mu = 3;
      std::list<Physical *>::iterator it;
      it = all_.begin();
      while (it != all_.end()) {
        if ((*it)->has_collisions()){
          double impulse = 2 * (*it)->m_ * (*it)->intersection_distance() / update_time / (*it)->I_;
            
          //Left Wall
          if ((*it)->pos_.x - (*it)->intersection_distance() < x_min_ && (*it)->vel_.x < 0){
            (*it)->pos_.x =  x_min_ + (*it)->intersection_distance();
            (*it)->vel_.x = -(*it)->vel_.x;
            if((*it)->acc_.x < 0) (*it)->acc_.x = 0;
            (*it)->ang_acc_.z +=  (*it)->vel_.x * (*it)->vel_.y / (*it)->vel_.length() * impulse * mu;
          }

          //Right Wall
          else if ((*it)->pos_.x + (*it)->intersection_distance() > x_max_ && (*it)->vel_.x > 0){
            (*it)->pos_.x =  x_max_ - (*it)->intersection_distance();
            (*it)->vel_.x = -(*it)->vel_.x;
            if((*it)->acc_.x > 0) (*it)->acc_.x = 0;
            (*it)->ang_acc_.z +=  (*it)->vel_.x * (*it)->vel_.y / (*it)->vel_.length() * impulse * mu;
          }

          //Bottom Wall
          if ((*it)->pos_.y - (*it)->intersection_distance() < y_min_ && (*it)->vel_.y < 0){
            (*it)->pos_.y =  y_min_ + (*it)->intersection_distance();
            (*it)->vel_.y = -(*it)->vel_.y;
            if((*it)->acc_.y < 0) (*it)->acc_.y = 0;
            (*it)->ang_acc_.z +=  (*it)->vel_.x * (*it)->vel_.y / (*it)->vel_.length() * impulse * mu;
          }

          //Top Wall
          else if ((*it)->pos_.y + (*it)->intersection_distance() > y_max_ && (*it)->vel_.y > 0){
            (*it)->pos_.y =  y_max_ - (*it)->intersection_distance();
            (*it)->vel_.y = -(*it)->vel_.y;
            if((*it)->acc_.y > 0) (*it)->acc_.y = 0;
            (*it)->ang_acc_.z +=  (*it)->vel_.x * (*it)->vel_.y / (*it)->vel_.length() * impulse * mu;
          }
          
        } 
        ++it;
      }
    }
}

void Physics::set_bounds(double size_x, double size_y, double x, double y){
  x_min_ = -size_x/2.0 + x;
  x_max_ = size_x/2.0 + x;
  y_min_ = -size_y/2.0 + y;
  y_max_ = size_y/2.0 + y;

}


bool Physics::is_clear_area(double x, double y, double r){
  if (x-r<x_min_ || x+r>x_max_ || y+r>y_max_ || y-r<y_min_){
    return false;
  }
  if (all_.size() > 1) {
    std::list<Physical *>::iterator it_a;
    it_a = all_.begin();
    while (it_a != all_.end()) {
      if ( (*it_a)->has_collisions()){
        Vector3d between = Vector3d(x,y,0) - (*it_a)->pos_;
        if (between.length() < 1.05*(r + (*it_a)->intersection_distance())){
          return false;  
        }
      } ++it_a;
    }
  }
  return true;

}