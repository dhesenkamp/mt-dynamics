#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulationengine.h"

#include <iostream>
#include <vector>

using namespace std;

MainWindow::MainWindow (QWidget *parent)
  : QMainWindow (parent), ui (new Ui::MainWindow)
{
  ui->setupUi (this);
  engine = new SimulationEngine(this);
  scene = new QGraphicsScene(this);

  // set fixed boundaries for axon
  scene->setSceneRect(0, 0, engine->getAxonLength(), engine->getAxonWidth());

  // draw axon boundaries
  QGraphicsRectItem *rectItem = scene->addRect(0, 0, engine->getAxonLength(), engine->getAxonWidth());
  rectItem->setPen(QPen(Qt::red));

  ui->graphicsView->setScene(scene);

  assert(engine != nullptr);
  assert(scene != nullptr);

  connect(
    engine, &SimulationEngine::stepCompleted,
    this,   &MainWindow::onStepCompleted
  );
}

MainWindow::~MainWindow () { delete ui; }

void MainWindow::on_startButton_clicked()
{
  if (engine->getElapsedTime() == 0) {
    mtLines.clear();
    engine->initialiseAxon(10);

    vector<Microtubule> mts = engine->getMts();

    for (std::size_t i = 0; i < mts.size(); ++i) {
      auto& mt = mts[i];
      mtLines.push_back(
        scene->addLine(
          mt.getX(), mt.getY(),
          mt.getEndX(), mt.getY(),
          QPen(Qt::black, 2)
        )
      );
    }
  }

  engine->start();
}

void MainWindow::on_pauseButton_clicked() { engine->pause(); }

void MainWindow::on_resetButton_clicked() { engine->reset(); }

void MainWindow::onStepCompleted() {
  // update time step on canvas
  ui->stepCountLabel->setText(QString::number(engine->getElapsedTime()));

  // update drawings
  auto mts = engine->getMts();

  for (std::size_t i = 0; i < mts.size(); i++) {

      mtLines[i]->setLine(
      mts[i].getX(), mts[i].getY(),
      mts[i].getEndX(), mts[i].getY()
    );
  }
  ui->freeTubulinLabel->setText(QString::number(engine->getFreeTubulin()));
}
