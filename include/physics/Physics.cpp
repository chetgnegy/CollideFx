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
  int standard_iterations = 20.0;
  double update = update_time/(1.0*standard_iterations);
  
  //Numerical Integration
  for (int i = 0; i < standard_iterations; ++i){

    if (all_.size() > 0) {
      std::list<Physical *>::iterator it = all_.begin();
      while (it != all_.end()) {
        (*it)->acc_ = (*it)->external_forces()/(*it)->m_;
        (*it)->ang_acc_ = (*it)->external_torques()/(*it)->I_;
        ++it;
      }
    }

    collision_prevention();
    check_in_bounds();

    if (all_.size() > 0) {
      std::list<Physical *>::iterator it = all_.begin();
      while (it != all_.end()) {
        integrate_translational(update, *it); 
        if (!(*it)->rotates()) integrate_rotational(update, *it); 
        ++it;
      }
    }
  }
}

// Uses Velocity Verlet integration to compute the next positions of the object
void Physics::integrate_translational(double timestep, Physical* obj){
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
void Physics::integrate_rotational(double timestep, Physical* obj){
  //kinetic friction
  double mu_loss = 0.99;
     
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
void Physics::collision_prevention(){
    //int to_remove = recent_collisions_.size();
    
    if (all_.size() > 1) {
      std::list<Physical *>::iterator it_a = all_.begin();
      std::list<Physical *>::iterator it_b = all_.begin();
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
}

// Handles a collision using conservation of linear momentum;
void Physics::collide(Physical* a, Physical* b){
  Vector3d between = b->pos_ - a->pos_;
  double impulse, ra, rb, j, mu = .2;
  Vector3d tang_v, fric_dir, f_fric_max, dw_fric_max, dv_fric_max;
  
  ra = a->intersection_distance();
  rb = b->intersection_distance();

  // They are intersecting and one is moving towards the other
  if (between.length() < rb + ra && 
     (between.dotProduct(a->vel_)>0 || between.dotProduct(b->vel_)<0)){

      // Separate the discs from each other
      double move_dist = between.length() - rb - ra;
      a->pos_ += between * move_dist/2.0;
      b->pos_ += -between * move_dist/2.0;
      
      between.normalize();
      
      // Project vectors onto vector connecting radii
      Vector3d vb_norm = between * between.dotProduct(b->vel_);
      Vector3d va_norm = between * between.dotProduct(a->vel_);
      // Perfectly inelastic collison. Momentum is conserved.
      double term1_1 = (a->m_-b->m_) / (a->m_ + b->m_);
      double term1_2 = (2*b->m_) / (a->m_ + b->m_);
      double term2_1 = (2*a->m_) / (a->m_ + b->m_);
      double term2_2 = (b->m_-a->m_) / (a->m_ + b->m_);
 
      // Handles motion normal to collision
      a->vel_ = a->vel_ - va_norm + va_norm * term1_1 + vb_norm * term1_2;
      b->vel_ = b->vel_ - vb_norm + va_norm * term2_1 + vb_norm * term2_2;
      
      
      Vector3d na = -between, nb = between;
      
      j = 2 / ( 1/a->m_ + 1/b->m_ + ra*ra/a->I_ + rb*rb/b->I_ );

      // The impulse in the direction normal to the wall
      impulse = a->vel_.dotProduct(na) * j * a->m_ ;
      // The direction of the friction
      tang_v = -(a->vel_ + na.crossProduct(a->ang_vel_)*ra - b->vel_ - nb.crossProduct(b->ang_vel_)*rb);
      
      fric_dir = tang_v - na.projectOnto(tang_v);
      
      // Will cause divide by zero. This is a head on collision
      if (fric_dir.length()==0) return;
      
      fric_dir.normalize();
      // Amount of rotation and velocity change to add
      f_fric_max = fric_dir * impulse * mu;
      dw_fric_max = -na.crossProduct(f_fric_max) * ra / a->I_;
      dv_fric_max = fric_dir * dw_fric_max.length() * ra;
      // Limit friction              
      dv_fric_max = fric_dir * fmin(fabs(dv_fric_max.length()), fabs(a->vel_.dotProduct(fric_dir)));
      
      a->ang_vel_ +=  dw_fric_max;
      a->vel_ += dv_fric_max; 
    

      // The impulse in the direction normal to the wall
      impulse = b->vel_.dotProduct(nb) * j * b->m_ ;
      // The direction of the friction
      fric_dir = -fric_dir;
      // Amount of rotation and velocity change to add
      f_fric_max = fric_dir * impulse * mu;
      dw_fric_max = -nb.crossProduct(f_fric_max) * rb / b->I_;
      dv_fric_max = fric_dir * dw_fric_max.length() * rb;
      // Limit friction              
      dv_fric_max = fric_dir * fmin(fabs(dv_fric_max.length()), fabs(b->vel_.dotProduct(fric_dir)));
      
      b->ang_vel_ +=  dw_fric_max;
      b->vel_ += dv_fric_max; 
      
  }
}



void Physics::check_in_bounds(){
  if (all_.size() > 0) {
      double mu = .2;
      std::list<Physical *>::iterator it = all_.begin();
      double impulse, r;
      Vector3d tang_v, fric_dir, f_fric_max, dw_fric_max, dv_fric_max;
      
      while (it != all_.end()) {
        Physical *p = (*it);
        r = p->intersection_distance();
        if (p->has_collisions()){
          // Wall Normal Vector    
          Vector3d n;
          bool collides = false;
          //Left Wall
          if (p->pos_.x - p->intersection_distance() < x_min_ && p->vel_.x < 0){
            p->pos_.x =  x_min_ + p->intersection_distance();
            p->vel_.x = -p->vel_.x;
            if(p->acc_.x < 0) p->acc_.x = 0;

            n = Vector3d(1, 0, 0); // Normal vector for wall
            collides = true;
          }

          //Right Wall
          else if (p->pos_.x + p->intersection_distance() > x_max_ && p->vel_.x > 0){
            p->pos_.x =  x_max_ - p->intersection_distance();
            p->vel_.x = -p->vel_.x;
            if(p->acc_.x > 0) p->acc_.x = 0;

            n = Vector3d(-1, 0, 0); // Normal vector for wall
            collides = true;
            
          }

          //Bottom Wall
          if (p->pos_.y - p->intersection_distance() < y_min_ && p->vel_.y < 0){
            p->pos_.y =  y_min_ + p->intersection_distance();
            p->vel_.y = -p->vel_.y;
            if(p->acc_.y < 0) p->acc_.y = 0;
            n = Vector3d(0, 1, 0); // Normal vector for wall
            collides = true;
          }

          //Top Wall
          else if (p->pos_.y + p->intersection_distance() > y_max_ && p->vel_.y > 0){
            p->pos_.y =  y_max_ - p->intersection_distance();
            p->vel_.y = -p->vel_.y;
            if(p->acc_.y > 0) p->acc_.y = 0;
            n = Vector3d(0, -1, 0); // Normal vector for wall
            collides = true;
          }
          
          if (collides && p->vel_.dotProduct(n) != p->vel_.length()){// Handles divide by zero cases
            // The impulse in the direction normal to the wall
            impulse = p->vel_.dotProduct(n) * 2 * p->m_ ;
            // The direction of the friction
            tang_v = -(p->vel_ + n.crossProduct(p->ang_vel_) * r);
            fric_dir = tang_v - n.projectOnto(tang_v);
            fric_dir.normalize();
            // Amount of rotation and velocity change to add
            f_fric_max = fric_dir * impulse * mu;
            dw_fric_max = -n.crossProduct(f_fric_max) * r / p->I_;
            dv_fric_max = fric_dir * dw_fric_max.length() * r;
            // Limit friction              
            dv_fric_max = fric_dir * fmin(fabs(dv_fric_max.length()), fabs(p->vel_.dotProduct(fric_dir)));
  
            p->ang_vel_ +=  dw_fric_max;
            p->vel_ += dv_fric_max; 
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
    std::list<Physical *>::iterator it_a = all_.begin();
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
