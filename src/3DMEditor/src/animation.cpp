#include "animation.h"
#include "mainwindow.h"
#include <QtGui>

Animation *Animation::pInstance = NULL;

Animation *Animation::instance()
{
    if (!pInstance)
        pInstance = new Animation;
    return pInstance;
}

Animation::Animation() : QDockWidget(MainWindow::instance())
{
    // Docking options
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    MainWindow::instance()->addDockWidget(Qt::RightDockWidgetArea, this);
    QWidget *wnd = new QWidget;
    setWidget(wnd);

    // Normal initialization
    setWindowTitle(tr("Animation"));

    QHBoxLayout *mainLayout = new QHBoxLayout(wnd);
    QWidget *left = new QWidget;
    QWidget *right = new QWidget;
    QFormLayout *layoutL = new QFormLayout(left);
    QFormLayout *layoutR = new QFormLayout(right);
    mainLayout->addWidget(left);
    mainLayout->addWidget(right);
    layoutL->addRow(tr("rotation :"), cbRotation = new QCheckBox);
    layoutL->addRow(tr("periodic :"), cbRPerdioc = new QCheckBox);
    layoutL->addRow(tr("sinus :"), cbRSinus = new QCheckBox);
    layoutR->addRow(tr("translation :"), cbTranslation = new QCheckBox);
    layoutR->addRow(tr("periodic :"), cbTPeriodic = new QCheckBox);
    layoutR->addRow(tr("sinus :"), cbTSinus = new QCheckBox);

    layoutL->addRow(tr("angle0 X :"), sbAngle0X = new QDoubleSpinBox);
    layoutL->addRow(tr("angle0 Y :"), sbAngle0Y = new QDoubleSpinBox);
    layoutL->addRow(tr("angle0 Z :"), sbAngle0Z = new QDoubleSpinBox);
    layoutL->addRow(tr("angle1 X :"), sbAngle1X = new QDoubleSpinBox);
    layoutL->addRow(tr("angle1 Y :"), sbAngle1Y = new QDoubleSpinBox);
    layoutL->addRow(tr("angle1 Z :"), sbAngle1Z = new QDoubleSpinBox);

    layoutR->addRow(tr("position0 X :"), sbVec0X = new QDoubleSpinBox);
    layoutR->addRow(tr("position0 Y :"), sbVec0Y = new QDoubleSpinBox);
    layoutR->addRow(tr("position0 Z :"), sbVec0Z = new QDoubleSpinBox);
    layoutR->addRow(tr("position1 X :"), sbVec1X = new QDoubleSpinBox);
    layoutR->addRow(tr("position1 Y :"), sbVec1Y = new QDoubleSpinBox);
    layoutR->addRow(tr("position1 Z :"), sbVec1Z = new QDoubleSpinBox);

    layoutL->addRow(tr("w :"), sbRW = new QDoubleSpinBox);
    layoutR->addRow(tr("w :"), sbTW = new QDoubleSpinBox);

#define INFINITE(x)  x->setRange(-0x7FFFFFFF, 0x7FFFFFFF); \
    x->setDecimals(4);

    INFINITE(sbAngle0X);
    INFINITE(sbAngle0Y);
    INFINITE(sbAngle0Z);
    INFINITE(sbAngle1X);
    INFINITE(sbAngle1Y);
    INFINITE(sbAngle1Z);

    INFINITE(sbVec0X);
    INFINITE(sbVec0Y);
    INFINITE(sbVec0Z);
    INFINITE(sbVec1X);
    INFINITE(sbVec1Y);
    INFINITE(sbVec1Z);

    INFINITE(sbRW);
    INFINITE(sbTW);
}
