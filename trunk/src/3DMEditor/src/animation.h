#ifndef ANIMATION_H
#define ANIMATION_H

#include <QDockWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QToolBox>

class Animation : public QDockWidget
{
    Q_OBJECT;
public:
    Animation();

    bool isInDefaultAnimationMode();

private:
    QCheckBox *cbRotation;
    QCheckBox *cbRPerdioc;
    QCheckBox *cbRSinus;
    QCheckBox *cbTranslation;
    QCheckBox *cbTPeriodic;
    QCheckBox *cbTSinus;

    QDoubleSpinBox *sbAngle0X;
    QDoubleSpinBox *sbAngle0Y;
    QDoubleSpinBox *sbAngle0Z;
    QDoubleSpinBox *sbAngle1X;
    QDoubleSpinBox *sbAngle1Y;
    QDoubleSpinBox *sbAngle1Z;

    QDoubleSpinBox *sbVec0X;
    QDoubleSpinBox *sbVec0Y;
    QDoubleSpinBox *sbVec0Z;
    QDoubleSpinBox *sbVec1X;
    QDoubleSpinBox *sbVec1Y;
    QDoubleSpinBox *sbVec1Z;

    QDoubleSpinBox *sbRW;
    QDoubleSpinBox *sbTW;
    bool updating;
    QToolBox *wnd;

public slots:
    void updateData();
    void updateGUI();

private:
    static Animation *pInstance;
public:
    static Animation *instance();
};

#endif // ANIMATION_H
