#include "simulationengine.h"

#include <random>
#include <iostream>
#include <QTimer>


using namespace std;

static constexpr int TIMER_INTERVAL_MS = 16;

SimulationEngine::SimulationEngine (QObject *parent)
  : QObject{ parent },
  gen(rd()),
  randomIntX(1, axonLength - (axonLength / 5)),
  randomIntY(1, axonWidth),
  randomLength(2.3, 0.1),
  stateProb(0.0, 1.0)
{
  timer = new QTimer(this);
  connect(
    timer,  &QTimer::timeout,
    this,   &SimulationEngine::step
  );
}

void SimulationEngine::initialiseAxon() {
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

void SimulationEngine::start() { timer->start(TIMER_INTERVAL_MS); } // 16ms = approx 60fps

void SimulationEngine::pause() { timer->stop(); }

void SimulationEngine::reset() {
  /* This function is called upon clicking the reset button.
   * It resets all metrics from the simulation engine to zero and emits a reset signal,
   * which ... TODO complete
   */
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
   * 3. Nucleation of new MTs
   */

  time += dt;
  stepCount++;

  // keep track of depolymerized mts (length <= 0.0)
  vector<size_t> toRemove;

  // Iterating over the microtubules - the biology happens here
  for (size_t i = 0; i < mts.size(); ++i) {

    auto& mt = mts[i];

    // RNG for state change
    double r = stateProb(gen);

    switch (mt.getState()) {

      case MtState::GROWING: {
        /* Effective catastrophe rate is dependent on
         * 1. base catastrophe rate (fixed) and
         * 2. the free tubulin concentration:
         * higher concentration -> lower catastrophe rate (stabilisation) */
        double effectiveCatastropheRate = baseCatastropheRate / (1.0 + alpha * freeTubulin);
        double pCatastrophe = 1.0 - std::exp(-effectiveCatastropheRate * dt);

        // State change from growing to shrinking is probabilistic
        if (r < pCatastrophe) { mt.setState(MtState::SHRINKING); }

        else {
          double delta = vGrow * dt;
          double actual = std::min(delta, freeTubulin);

          /* Boundary check: growth is capped at axon boundary
           * If the MT tip wpuld reach the boundary, only the remaining tubulin is consumed up until there
           * and catastrophe is triggered immediately. This is a well researched catastrophe trigger (mechanical catastrophe).
           * Else, the MT grows as regular.
           */
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
        // Rescue (=state change from shrinking to growing) is also probabilistic
        double pRescue = 1.0 - std::exp(-rescueRate * dt);
        if (r < pRescue) { mt.setState(MtState::GROWING); }

        else {
          double delta = vShrink * dt;
          // The MT cannot shrink more than its actual length
          double actual = std::min(delta, mt.getLength());

          mt.shrink(actual);
          freeTubulin += actual;

          // Once a MT reaches length zero, it is marked as "removable"
          if (mt.getLength() <= 0.0) {toRemove.push_back(i);}
        }
        break;
      }
    }
  } // --- MT loop ends here ---

  // Removal of zero-lengthMTs
  for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
    mts.erase(mts.begin() + *it);
  }
  toRemove.clear();

  // Nucleation of new MTs is log-probabilistic and dependent on the free tubulin concentration
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


