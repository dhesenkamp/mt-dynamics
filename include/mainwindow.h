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

  QLineSeries *freeTubulinSeries;
  QLineSeries *boundTubulinSeries;
  QValueAxis *axisX;
  QValueAxis *axisY;

  QLineSeries *kdeSeries;
  QChart      *lengthChart;
  QValueAxis  *lengthAxisX;
  QValueAxis  *lengthAxisY;
  void drawLengthKde();

  static constexpr QChart::ChartTheme CHART_THEME = QChart::ChartThemeDark;
  static constexpr int X_AXIS_WINDOW_SIZE = 5.0;
  static constexpr int    KDE_POINTS   = 200;  // resolution of the curve
  static constexpr double KDE_MAX_LEN  = 1000; // x-axis upper bound in µm

  // Keyed by the microtubule ID, deletions or insertions do not corrupt the indeces/other entries
  QMap<unsigned, QGraphicsLineItem*> mtLines;

};
#endif // MAINWINDOW_H
