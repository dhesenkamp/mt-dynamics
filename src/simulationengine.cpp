#include "simulationengine.h"

#include <random>
#include <iostream>
#include <QTimer>


using namespace std;

static constexpr int TIMER_INTERVAL_MS = 16;

SimulationEngine::SimulationEngine (QObject *parent)
  : QObject{ parent },
  gen(rd()),

  // Random distributions for initial MT placement and length
  randomIntX(1, axonLength - (axonLength / 5)),
  randomIntY(1, axonWidth),
  randomLength(2.3, 0.1),
  stateProb(0.0, 1.0)
{
  timer = new QTimer(this);
  /* Connect the timer's timeout signal to the step function, which advances 
   * the simulation by one time step. This allows the simulation to run in real-time, 
   * with the timer controlling the pace of the simulation steps. */
  connect(
    timer,  &QTimer::timeout,
    this,   &SimulationEngine::step
  );
}

void SimulationEngine::initialiseAxon() {
  /* Initialise the axon with a set of microtubules placed at random positions 
   * with random initial lengths. The number of microtubules and the 
   * total tubulin concentration can be configured from the GUI before 
   * starting the simulation (or from the respective header file). */
  mts.clear();

  for (unsigned i = 0; i < initialNrMts; i++) {
    int x = randomIntX(gen);
    int y = randomIntY(gen);
    double l = randomLength(gen);

    Microtubule mt(x, y, l);
    mts.push_back(mt);
  }

  double boundTubulin = 0.0;
  for (auto& mt : mts) { boundTubulin += mt.getLength(); }
  freeTubulin = totalTubulin - boundTubulin;
}

// Set to 16 = approx 60fps
void SimulationEngine::start() { timer->start(TIMER_INTERVAL_MS); }

// Pausing the simulation simply stops the timer, which halts the calls to the step function
void SimulationEngine::pause() { timer->stop(); } 

void SimulationEngine::reset() {
  /* This function is called upon clicking the reset button.
   * It resets all metrics from the simulation engine to zero, clears the plots,
   * and emits a reset signal, which is connected to the GUI */
  timer->stop();

  time = 0.0;
  stepCount = 0;
  freeTubulin = 0.0;
  mts.clear();
  Microtubule::resetIdCounter();

  emit resetCompleted();
}

void SimulationEngine::step() {
  /* This function implements the main simulation logic.
   * It handles several biological mechanisms:
   *
   * 1. Growing & shrinking of available MTs
   * 2. Deletion of zero-length (fully destructed) MTs
   * 3. Nucleation of new MTs */

  time += dt;
  stepCount++;

  // Keep track of depolymerized mts (length <= 0.0)
  vector<size_t> toRemove;

  // Iterating over the microtubules - the main logic happens in this loop
  for (size_t i = 0; i < mts.size(); ++i) {

    auto& mt = mts[i];

    // RNG for state change
    double r = stateProb(gen);

    switch (mt.getState()) {

      case MtState::GROWING: {
        /* The state change from growing to shrinking (catastrophe) is probabiliostic
         * and dependent on free tubulin concentration. 
         * Effective catastrophe rate is a hyperbolic decay function of the 
         * free tubulin concentration, where alpha controls the strength of the
         * stabilizing effect of free tubulin. */
        double effectiveCatastropheRate = baseCatastropheRate / (1.0 + alpha * freeTubulin);
        /* This is than translated into a probability with the exponential CDF */
        double pCatastrophe = 1.0 - std::exp(-effectiveCatastropheRate * dt);
        if (r < pCatastrophe) { mt.setState(MtState::SHRINKING); }

        else {
          /* Growth is then deterministic, dependent on the growth velocity and 
           * capped by available tubulin */
          double delta = vGrow * dt;
          double actual = std::min(delta, freeTubulin);

          /* Boundary check: growth is capped at axon boundary
           * If the MT tip would reach the boundary, only the remaining tubulin 
           * is consumed up until there and catastrophe is triggered immediately. 
           * This is a well researched catastrophe trigger (mechanical catastrophe).
           * Else, the MT grows as regular. */
          double remainingSpace = static_cast<double>(axonLength) - mt.getEndX();
          if (actual >= remainingSpace) {
            actual = std::max(remainingSpace, 0.0);
            mt.grow(actual);
            freeTubulin -= actual;
            mt.setState(MtState::SHRINKING); // Immediately switch to shrinking when reaching the axon boundary
          }
          else {
            mt.grow(actual);
            freeTubulin -= actual;
          }
        }
        break;
      }

      case MtState::SHRINKING: {
        /* Rescue (=state change from shrinking to growing) is also probabilistic */
        double pRescue = 1.0 - std::exp(-rescueRate * dt);
        if (r < pRescue) { mt.setState(MtState::GROWING); }

        else {
          double delta = vShrink * dt;
          // The MT cannot shrink more than its actual length
          double actual = std::min(delta, mt.getLength());

          mt.shrink(actual);
          freeTubulin += actual;

          /* Once a MT reaches length zero, it is tracked as "removable", 
           * which happens at the end of the loop */
          if (mt.getLength() <= 0.0) {toRemove.push_back(i);}
        }
        break;
      }
    }
  } // --- MT loop ends here ---

  // Removal of zero-length MTs
  for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
    mts.erase(mts.begin() + *it);
  }
  toRemove.clear();

  // Nucleation rate of new MTs is modelled linearly dependent on free tubulin concentration
  double pNucleation = nucleationRate * (freeTubulin / totalTubulin) * dt;
  if (stateProb(gen) < pNucleation) {
      int x = randomIntX(gen);
      int y = randomIntY(gen);
      double l = randomLength(gen);

      Microtubule mt(x, y, l);
      mts.push_back(mt);

      freeTubulin -= l;
    }

  emit stepCompleted();
}


