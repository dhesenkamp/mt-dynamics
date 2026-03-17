#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "simulationengine.h"

#include <QMainWindow>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QMap>
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();

private slots:
  void on_startButton_clicked();
  void on_pauseButton_clicked();
  void on_resetButton_clicked();
  void onStepCompleted();
  void onResetCompleted();

private:
  Ui::MainWindow *ui;
  SimulationEngine *engine;
  QGraphicsScene *scene;

  void getParams();

  // Chart components for the tubulin pool chart
  QLineSeries *freeTubulinSeries;
  QLineSeries *boundTubulinSeries;
  QValueAxis *axisX;
  QValueAxis *axisY;

  // Chart components for the MT length distribution KDE
  QLineSeries *kdeSeries;
  QChart *lengthChart;
  QValueAxis *lengthAxisX;
  QValueAxis *lengthAxisY;
  void drawLengthKde();

  static constexpr QChart::ChartTheme CHART_THEME = QChart::ChartThemeBrownSand;
  static constexpr int X_AXIS_WINDOW_SIZE = 5.0; // Time window (in minutes) shown on the x-axis of the tubulin pool chart, controls how much historical data is visible
  static constexpr int    KDE_POINTS   = 200;  // Resolution of the KDE curve (higher = smoother but more computationally expensive)
  static constexpr double KDE_MAX_LEN  = 1000; // Should correspond to axon length

  // Keyed by the microtubule ID, deletions or insertions do not corrupt the indeces/other entries
  QMap<unsigned, QGraphicsLineItem*> mtLines;

};
#endif // MAINWINDOW_H
