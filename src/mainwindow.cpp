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

  // Fix scene boundaries to the size of the axon
  scene->setSceneRect(0, 0, engine->getAxonLength(), engine->getAxonWidth());
  QGraphicsRectItem *rectItem = scene->addRect(0, 0, engine->getAxonLength(), engine->getAxonWidth());
  rectItem->setPen(QPen(Qt::red));

  ui->graphicsView->setScene(scene);

  // Make sure engine and scene are initialised
  assert(engine != nullptr);
  assert(scene != nullptr);

  /* Chart setup
   * Both the length histogram and the tubulin pool line chart have been created 
   * with the extensive support of Claude.ai */

  // --- Chart 1: MT length distribution ---
  kdeSeries = new QLineSeries();
  kdeSeries->setName("Length density");
  QPen kdePen(Qt::cyan);
  kdePen.setWidth(2);
  kdeSeries->setPen(kdePen);

  lengthChart = new QChart();
  lengthChart->addSeries(kdeSeries);
  lengthChart->setTitle("MT Length Distribution");
  lengthChart->setAnimationOptions(QChart::NoAnimation);
  lengthChart->setTheme(CHART_THEME);
  lengthChart->legend()->hide();

  lengthAxisX = new QValueAxis();
  lengthAxisX->setTitleText("Length");
  lengthAxisX->setRange(0, KDE_MAX_LEN);
  lengthAxisX->setLabelFormat("%.0f");
  lengthChart->addAxis(lengthAxisX, Qt::AlignBottom);
  kdeSeries->attachAxis(lengthAxisX);

  lengthAxisY = new QValueAxis();
  //lengthAxisY->setTitleText("Density");
  //lengthAxisY->setRange(0, 1.0);
  //lengthAxisY->setLabelFormat("%.1f");
  lengthChart->addAxis(lengthAxisY, Qt::AlignLeft);
  kdeSeries->attachAxis(lengthAxisY);

  ui->mtLengthChart->setChart(lengthChart);
  ui->mtLengthChart->setRenderHint(QPainter::Antialiasing);

  // --- Chart 2: tubulin pool ---
  freeTubulinSeries = new QLineSeries();
  freeTubulinSeries->setName("Free Tubulin");
  QPen freeTubPen(Qt::blue);
  freeTubPen.setWidth(2);
  freeTubulinSeries->setPen(freeTubPen);

  boundTubulinSeries = new QLineSeries();
  boundTubulinSeries->setName("Bound Tubulin");
  QPen boundTubPen(Qt::green);
  boundTubPen.setWidth(2);
  boundTubulinSeries->setPen(boundTubPen);

  QChart *tubChart = new QChart();
  tubChart->addSeries(freeTubulinSeries);
  tubChart->addSeries(boundTubulinSeries);
  tubChart->setAnimationOptions(QChart::NoAnimation);
  tubChart->setTheme(CHART_THEME);

  axisX = new QValueAxis();
  axisX->setTitleText("Time (min)");
  axisX->setRange(0, X_AXIS_WINDOW_SIZE + 0.5);
  axisX->setLabelFormat("%.1f");
  tubChart->addAxis(axisX, Qt::AlignBottom);

  axisY = new QValueAxis();
  axisY->setTitleText("Tubulin Mass");
  axisY->setLabelFormat("%.0f");
  axisY->setRange(0, engine->getTotalTubulin());
  tubChart->addAxis(axisY, Qt::AlignLeft);

  freeTubulinSeries->attachAxis(axisX);
  freeTubulinSeries->attachAxis(axisY);
  boundTubulinSeries->attachAxis(axisX);
  boundTubulinSeries->attachAxis(axisY);

  ui->tubulinChart->setChart(tubChart);

  ui->totalTubDisplay->setText(
    QString::number(ui->totalTubSlider->value())
  );

  // Connect signals & slots
  connect(engine, &SimulationEngine::stepCompleted,   this, &MainWindow::onStepCompleted);
  connect(engine, &SimulationEngine::resetCompleted,  this, &MainWindow::onResetCompleted);
}

MainWindow::~MainWindow () { delete ui; }

void MainWindow::on_startButton_clicked() {
  /* This function serves as the entry point in the program.
   * It performs a check at the beginning, if at the start of the simulation (time = 0),
   * the axon gets seeded with a predetermined number of MTs.
   *
   * After hitting the pause button, it will continue where it left off 
   * (hence the zero-check in the beginning) */
  if (engine->getElapsedTime() == 0) {
    getParams();

    // Update relevant parts of the GUI
    ui->totalTubDisplay->setText(QString::number(engine->getTotalTubulin()));
    axisY->setRange(0, engine->getTotalTubulin());

    mtLines.clear();
    engine->initialiseAxon();

    // Get the MTs as const reference, not copy
    const auto& mts = engine->getMts();

    // Draw the MTs
    for (const auto& mt : mts) {
      mtLines[mt.getId()] = scene->addLine(
        mt.getX(), mt.getY(),
        mt.getEndX(), mt.getY(),
        QPen(Qt::white, 2)
      );
    }
  }

  engine->start();
}

// Simple pause functionality as the simulation otherwise runs without interruption
void MainWindow::on_pauseButton_clicked() { engine->pause(); }

void MainWindow::on_resetButton_clicked() {
  /* Resetting calls reset for the engine (which in turn resets metrics such as time elapsed)
   * and removes all drawn lines from the scene.
   */
  engine->reset();
}

void MainWindow::getParams() {
  /* Retrieve simulation parameters from the GUI */

  engine->setInitialNrMts(ui->nrMtsSpinBox->value());
  engine->setDeltaTime(ui->dtSpinBox->value());
  engine->setVGrow(ui->vGrowSpinBox->value());
  engine->setVShrink(ui->vShrinkSpinBox->value());
  engine->setBaseCatRate(ui->baseCatRateSpinBox->value());
  engine->setRescueRate(ui->rescueRateSpinBox->value());
  engine->setAlpha(ui->alphaSpinBox->value());
  engine->setNucRate(ui->nucRateSpinBox->value());
  engine->setTotalTubulin(ui->totalTubSlider->value());
}

void MainWindow::onStepCompleted() {
  /* This function updates visuals and tracking metrics after every step.
   * It is called after the engine emits the "step completed signal */

  const auto& mts = engine->getMts();
  double t = engine->getElapsedTime();
  double tMin = t / 60.0; // Convert time to minutes for better readability on the x-axis
  double free = engine->getFreeTubulin();
  double bound = engine->getTotalTubulin() - free;
  double boundPercentage = (bound / engine->getTotalTubulin()) * 100;

  // Update the series objects for data display
  freeTubulinSeries->append(tMin, free);
  boundTubulinSeries->append(tMin, bound);

  // Sliding window for the x-axis to keep relevant data in view
  const double windowSize = X_AXIS_WINDOW_SIZE;
  if (tMin > windowSize) { axisX->setRange(tMin - windowSize, tMin+0.5); }

  // Update text fields next to the graphs
  ui->nrMtsDisplay->setText(QString::number(mts.size()));
  ui->boundTubDisplay->setText(QString::number(boundPercentage, 'f', 1) + " %");

  // Get the unique IDs of all currently living MTs to correctly update visuals
  QSet<unsigned> currentIds;
  for (const auto& mt : mts) { currentIds.insert(mt.getId()); }

  // --- Case 1: removing "dead" MTs from the scene ---
  auto it = mtLines.begin();
  while (it != mtLines.end()) {
    if (!currentIds.contains(it.key())) {
      scene->removeItem(it.value());
      delete it.value(); // Also clean up memory when removing a line from the scene
      it = mtLines.erase(it);
    }
    else { ++it; }
  }

  // --- Case 2: Handle existing and new MTs ---
  for (const auto& mt : mts) {
    // Case 2.1: Update the length of existing MTs in the scene
    if (mtLines.contains(mt.getId())) {
      mtLines[mt.getId()]->setLine(
        mt.getX(), mt.getY(),
        mt.getEndX(), mt.getY()
      );
    }

    // Case 2.2: Draw newly nucleated MTs which are not yet part of the scene
    else {
      mtLines[mt.getId()] = scene->addLine(
        mt.getX(), mt.getY(),
        mt.getEndX(), mt.getY(),
        QPen(Qt::blue, 2)
      );
    }
  }

  // Update & draw MT length histogram
  drawLengthKde();
}

void MainWindow::onResetCompleted() {
  /* Once the simulation is reset, all drawn lines are removed from the scene 
   * and the plots with respective data storages are cleared */
  for (auto* line : mtLines) {
    scene->removeItem(line);
    delete line;
  }

  mtLines.clear();

  // Reset tubulin pool chart
  freeTubulinSeries->clear();
  boundTubulinSeries->clear();
  axisX->setRange(0, 10);

  // Reset length KDE
  kdeSeries->clear();
  lengthAxisY->setRange(0, 1000);
}

void MainWindow::drawLengthKde() {
  /* Update the kernel density estimate for MT lengths. This part is 
   * computationally intensive and may slow down the UI. If so, reduce
   * the resolution (number of points used for the KDE calculation) in
   * the header file (KDE_POINTS). 
   * This part has been written with Claude.ai */
  const auto& mts = engine->getMts();
  QVector<double> lengths;
  lengths.reserve(mts.size());
  for (const auto& mt : mts) { lengths.append(mt.getEndX() - mt.getX()); }

  kdeSeries->clear();

  if (lengths.size() >= 2) {
    // Compute mean and standard deviation
    double sum = 0.0;
    for (double l : lengths) { sum += l; }
    double mean = sum / lengths.size();

    double sq = 0.0;
    for (double l : lengths) { sq += (l - mean) * (l - mean); }
    double stddev = std::sqrt(sq / lengths.size());

    /* Silverman's rule for bandwidth. Can be manually adjusted
     * to decrease smoothing when expecting multi-modal distributions. 
     * (higher h smoothes more drastically). 1.0 is a fallback. */ 
    double h = (stddev > 0)
                   ? 1.06 * stddev * std::pow(lengths.size(), -0.2)
                   : 1.0;

    // Evaluate KDE at KDE_POINTS evenly spaced positions
    double step = KDE_MAX_LEN / (KDE_POINTS - 1);
    double maxDensity = 0.0;
    QVector<QPointF> points;
    points.reserve(KDE_POINTS);

    const double norm = 1.0 / (lengths.size() * h * std::sqrt(2.0 * M_PI));
    for (int i = 0; i < KDE_POINTS; ++i) {
      double x = i * step;
      double density = 0.0;
      for (double l : lengths) {
        double u = (x - l) / h;
        density += std::exp(-0.5 * u * u);
      }
      density *= norm;
      maxDensity = std::max(maxDensity, density);
      points.append({x, density});
    }

    kdeSeries->replace(points);
    lengthAxisY->setRange(0, maxDensity * 1.1);
  }

  else {
    // Not enough data — flat line at zero
    kdeSeries->append(0, 0);
    kdeSeries->append(KDE_MAX_LEN, 0);
    lengthAxisY->setRange(0, 1.0);
  }
}




