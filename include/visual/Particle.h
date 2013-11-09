/*
  Author: Chet Gnegy
  chetgnegy@gmail.com

  Particle.h
  Particles are containers for small drawable objects that glow!
*/

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

typedef struct {
    bool active;
    float life;
    float fade;
    float r, g, b;
    float x, y, z;
    float dx, dy, dz;
} Particle;

#endif