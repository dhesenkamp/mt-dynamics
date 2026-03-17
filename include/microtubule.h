#ifndef MICROTUBULE_H
#define MICROTUBULE_H

#include <cmath>

enum class MtState {
  GROWING,
  SHRINKING,
};

class Microtubule {
public:
  Microtubule(unsigned x, unsigned y, double l);

  double getX() const { return originX; }
  double getY() const { return originY; }
  double getLength() const { return currentLength; }
  double getEndX() const { return originX + currentLength; } // convenience for drawing

  unsigned getId() const { return id; }

  MtState getState() const { return state; }
  void setState(MtState s) { state = s; }

  void grow(double delta) { currentLength += delta; }
  void shrink(double delta) { currentLength -= delta; }

  static void resetIdCounter() { nextId = 0; }

private:
  double originX, originY;
  double currentLength;

  // Unique identifier for each microtubule, used for tracking in the GUI
  static unsigned nextId;
  unsigned id;

  MtState state = MtState::GROWING;
};

#endif // MICROTUBULE_H
