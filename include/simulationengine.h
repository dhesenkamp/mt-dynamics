#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include "microtubule.h"

#include <vector>
#include <memory>
#include <random>
#include <QObject>
#include <QTimer>

class SimulationEngine : public QObject
{
  Q_OBJECT

public:
  explicit SimulationEngine (QObject *parent = nullptr);

  // Control functions
  void start();
  void pause();
  void reset();

  // Used during setup phase to configure the simulation
  unsigned getAxonLength() const { return axonLength; }
  unsigned getAxonWidth() const { return axonWidth; }
  void initialiseAxon();
  std::vector<Microtubule>& getMts() { return mts; }

  // Getters and setters for parameters
  unsigned getStepCount() { return stepCount; }
  double getElapsedTime() { return time; }
  double getTotalTubulin() const { return totalTubulin; }
  void setTotalTubulin(int t) { totalTubulin = t; }
  double getFreeTubulin() const { return freeTubulin; }
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

  /* Time step in seconds. This is a key parameter that controls the resolution 
   * of the simulation and can be adjusted for performance vs accuracy trade-offs.
   * Smaller = more accurate simulation but longer runtime
   * Larger = faster simulation but less accurate and may miss important events */
  double dt = 1.0; 
  double time = 0.0; // Actual biological time elapsed in seconds

  unsigned stepCount = 0;
  unsigned axonLength = 1000;
  unsigned axonWidth = 150;
  unsigned initialNrMts = 50;

  std::vector<Microtubule> mts;

  // Random number generators for various aspects of the simulation
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<int> randomIntX; //(1, getAxonLength() - (getAxonLength() / 5));
  std::uniform_int_distribution<int> randomIntY; //(1, getAxonWidth());
  std::lognormal_distribution<double> randomLength; //(2.3, 0.1); // relatively tight around 10.0
  std::uniform_real_distribution<double> stateProb; //(0.0, 1.0);


  // Biological parameters (default values, can be overwritten from GUI before running simulation)
  double vGrow = 0.6;
  double vShrink = 5.0; // Shrink velocity (keep positive value!)
  double totalTubulin = 10000.0;
  double freeTubulin = 0.0;
  double baseCatastropheRate = 0.005;
  double rescueRate = 0.05;
  double alpha = 0.0005; // Tubulin stabilization parameter
  double nucleationRate = 0.1;

  void increaseStepCount() { stepCount++; }

signals:
  void stepCompleted();
  void resetCompleted();

private slots:
  void step();

};

#endif // SIMULATIONENGINE_H
