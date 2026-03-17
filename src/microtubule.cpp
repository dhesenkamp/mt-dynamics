#include "microtubule.h"

#include <random>

unsigned Microtubule::nextId = 0;

Microtubule::Microtubule(unsigned x, unsigned y, double l) {
  id = nextId++;
  originX = x;
  originY = y;
  currentLength = l;
}