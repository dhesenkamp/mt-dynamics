#include "microtubule.h"

#include <random>

unsigned Microtubule::nextId = 0;

Microtubule::Microtubule(unsigned x, unsigned y, double l) {
  /* The constructor initializes a microtubule with a given origin (x, y) and initial length l. 
   * Each microtubule is assigned a unique ID using a static counter (nextId) 
   * that increments with each new instance. */

  id = nextId++;
  originX = x;
  originY = y;
  currentLength = l;
}