#include "simulationengine.h"
#include "mainwindow.h"

#include <random>
#include <iostream>

using namespace std;

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

void SimulationEngine::initialiseAxon(unsigned numMts) {
  mts.clear();

  for (unsigned i = 0; i < numMts; i++) {
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

void SimulationEngine::start() { timer->start(16); } // 16ms = approx 60fps

void SimulationEngine::pause() { timer->stop(); }

void SimulationEngine::reset() { timer->stop(); } // TODO: clear storage and visuals, reset timer to 0

void SimulationEngine::step() {

  time += dt;
  stepCount++;

  // keep track of depolymerized mts (length <= 0.0)
  vector<size_t> toRemove;

  for (size_t i = 0; i < mts.size(); ++i) {

    auto& mt = mts[i];
    double r = stateProb(gen);

    switch (mt.getState()) {

      case MtState::GROWING: {
        // effective cat rate is dependent on the free tub concentration: higher conc -> lower cat rate
        double effectiveCatastropheRate = baseCatastropheRate / (1.0 + alpha * freeTubulin);
        double pCatastrophe = 1.0 - std::exp(-effectiveCatastropheRate * dt);

        if (r < pCatastrophe) { mt.setState(MtState::SHRINKING); }
        else {
          double delta = vGrow * dt;
          double actual = std::min(delta, freeTubulin);

          mt.grow(actual);
          freeTubulin -= actual;
        }
        break;
      }

      case MtState::SHRINKING: {
        double pRescue = 1.0 - std::exp(-rescueRate * dt);

        if (r < pRescue) { mt.setState(MtState::GROWING); }
        else {
          double delta = vShrink * dt;
          double actual = std::min(delta, mt.getLength());

          mt.shrink(actual);
          freeTubulin += actual;

          if (mt.getLength() <= 0.0) {toRemove.push_back(i);}
        }
        break;
      }
    }

    // TODO: remove fully depolymerized MTs
    // TODO: also remove visuals
    // for (auto it = toRemove.rbegin();
    //      it != toRemove.rend(); ++it)
    //   {
    //     mts.erase(mts.begin() + *it);
    //   }
  }

  emit stepCompleted();
}


