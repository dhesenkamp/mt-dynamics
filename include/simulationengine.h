#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include "microtubule.h"

#include <vector>
#include <memory>
#include <random>
#include <QObject>
#include <QGraphicsScene>

class SimulationEngine : public QObject
{
  Q_OBJECT

public:
  explicit SimulationEngine (QObject *parent = nullptr);

  // controls
  void start();
  void pause();
  void reset();

  unsigned getAxonLength() const { return axonLength; }
  unsigned getAxonWidth() const { return axonWidth; }
  void initialiseAxon();
  std::vector<Microtubule>& getMts() { return mts; }

  unsigned getStepCount() { return stepCount; }
  void increaseStepCount() { stepCount++; }
  double getElapsedTime() { return time; }

  double getTotalTubulin() const { return totalTubulin; }
  void setTotalTubulin(int t) { totalTubulin = t; }
  double getFreeTubulin() const { return freeTubulin; }
  //double getCatastropeProb() const { return catastropheProb; }
  //double getRescueProb() const { return rescueProb; }
  void setInitialNrMts(int x) { initialNrMts = x; }
  void setVGrow(double x) { vGrow = x; }
  void setVShrink(double x) { vShrink = x; }
  void setBaseCatRate(double x) { baseCatastropheRate = x; }
  void setRescueRate(double x) { rescueRate = x; }
  void setAlpha(double x) { alpha = x; }
  void setNucRate(double x) { nucleationRate = x; }
  void setDeltaTime(double x) { dt = x; }

private:
  QTimer *timer;
  double dt = 1.0; // timestep, s/update
  double time = 0.0; // biological time

  unsigned stepCount = 0;
  unsigned axonLength = 1000;
  unsigned axonWidth = 150;
  unsigned initialNrMts = 50;

  std::vector<Microtubule> mts;

  // rng
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<int> randomIntX; //(1, getAxonLength() - (getAxonLength() / 5));
  std::uniform_int_distribution<int> randomIntY; //(1, getAxonWidth());
  std::lognormal_distribution<double> randomLength; //(2.3, 0.1); // relatively tight around 10.0
  std::uniform_real_distribution<double> stateProb; //(0.0, 1.0);


  // Biological parameters (default values, can be overwritten from GUI before running simulation)
  double vGrow = 0.6;  // microns per second
  double vShrink = 5.0;  // shrink velocity (positive value)
  double totalTubulin = 10000.0;
  double freeTubulin = 0.0;
  double baseCatastropheRate = 0.005;
  double rescueRate = 0.05;
  double alpha = 0.0005; // tubulin stabilization strength
  double nucleationRate = 0.1;

signals:
  void stepCompleted();
  void resetCompleted();

private slots:
  void step();

};

#endif // SIMULATIONENGINE_H
