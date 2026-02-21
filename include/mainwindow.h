#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "simulationengine.h"

#include <QMainWindow>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();

  void drawMt(QGraphicsScene* scene, Microtubule mt);

private slots:
  void on_startButton_clicked();
  void on_pauseButton_clicked();
  void on_resetButton_clicked();
  void onStepCompleted();

private:
  Ui::MainWindow *ui;
  SimulationEngine *engine;
  QGraphicsScene *scene;

  // track the lines to update them
  std::vector<QGraphicsLineItem*> mtLines;

};
#endif // MAINWINDOW_H
