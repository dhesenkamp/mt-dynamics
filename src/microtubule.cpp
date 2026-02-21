#include "microtubule.h"

#include <random>

Microtubule::Microtubule(unsigned x, unsigned y, double l) {
  originX = x;
  originY = y;
  currentLength = l;
}

void Microtubule::grow(double delta) { currentLength += delta; }

void Microtubule::shrink(double delta) { currentLength -= delta; }
