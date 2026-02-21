#ifndef MICROTUBULE_H
#define MICROTUBULE_H

#include <cmath>
#include <QGraphicsLineItem>

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
  unsigned getEndX() const { return originX + currentLength; } // convenience for drawing

  MtState getState() { return state; }
  void setState(MtState s) { state = s; }

  void grow(double delta);
  void shrink(double delta);

private:
  double originX, originY;
  double currentLength;
  MtState state = MtState::GROWING;
};

#endif // MICROTUBULE_H
