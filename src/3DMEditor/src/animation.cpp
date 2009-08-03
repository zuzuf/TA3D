#include "animation.h"
#include "mainwindow.h"
#include "mesh.h"
#include "gfx.h"
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

    updating = false;

    connect(cbRotation, SIGNAL(toggled(bool)), this, SLOT(updateData()));
    connect(cbRPerdioc, SIGNAL(toggled(bool)), this, SLOT(updateData()));
    connect(cbRSinus, SIGNAL(toggled(bool)), this, SLOT(updateData()));
    connect(cbTranslation, SIGNAL(toggled(bool)), this, SLOT(updateData()));
    connect(cbTPeriodic, SIGNAL(toggled(bool)), this, SLOT(updateData()));
    connect(cbTSinus, SIGNAL(toggled(bool)), this, SLOT(updateData()));

    connect(sbAngle0X, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbAngle0Y, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbAngle0Z, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbAngle1X, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbAngle1Y, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbAngle1Z, SIGNAL(valueChanged(double)), this, SLOT(updateData()));

    connect(sbVec0X, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbVec0Y, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbVec0Z, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbVec1X, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbVec1Y, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbVec1Z, SIGNAL(valueChanged(double)), this, SLOT(updateData()));

    connect(sbRW, SIGNAL(valueChanged(double)), this, SLOT(updateData()));
    connect(sbTW, SIGNAL(valueChanged(double)), this, SLOT(updateData()));

    updateGUI();
}

void Animation::updateData()
{
    if (updating)   return;

    Mesh *cur = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    if (!cur)
    {
        updateGUI();
        return;
    }
    cur->defaultAnim.type = 0;
    if (cbRotation->isChecked())
        cur->defaultAnim.type |= ROTATION;
    if (cbRPerdioc->isChecked())
        cur->defaultAnim.type |= ROTATION_PERIODIC;
    if (cbRSinus->isChecked())
        cur->defaultAnim.type |= ROTATION_COSINE;
    if (cbTranslation->isChecked())
        cur->defaultAnim.type |= TRANSLATION;
    if (cbTPeriodic->isChecked())
        cur->defaultAnim.type |= TRANSLATION_PERIODIC;
    if (cbTSinus->isChecked())
        cur->defaultAnim.type |= TRANSLATION_COSINE;

    cur->defaultAnim.angle_0.x = sbAngle0X->value();
    cur->defaultAnim.angle_0.y = sbAngle0Y->value();
    cur->defaultAnim.angle_0.z = sbAngle0Z->value();
    cur->defaultAnim.angle_1.x = sbAngle1X->value();
    cur->defaultAnim.angle_1.y = sbAngle1Y->value();
    cur->defaultAnim.angle_1.z = sbAngle1Z->value();

    cur->defaultAnim.translate_0.x = sbVec0X->value();
    cur->defaultAnim.translate_0.y = sbVec0Y->value();
    cur->defaultAnim.translate_0.z = sbVec0Z->value();
    cur->defaultAnim.translate_1.x = sbVec1X->value();
    cur->defaultAnim.translate_1.y = sbVec1Y->value();
    cur->defaultAnim.translate_1.z = sbVec1Z->value();

    cur->defaultAnim.angle_w = sbRW->value();
    cur->defaultAnim.translate_w = sbTW->value();
}

void Animation::updateGUI()
{
    if (updating)   return;
    updating = true;
    Mesh *cur = Mesh::instance()->getMesh(Gfx::instance()->getSelectionID());
    ANIMATION *pAnim = NULL;
    if (!cur)
        pAnim = new ANIMATION();
    else
        pAnim = &(cur->defaultAnim);

    cbRotation->setChecked(pAnim->type & ROTATION);
    cbRPerdioc->setChecked(pAnim->type & ROTATION_PERIODIC);
    cbRSinus->setChecked(pAnim->type & ROTATION_COSINE);
    cbTranslation->setChecked(pAnim->type & TRANSLATION);
    cbTPeriodic->setChecked(pAnim->type & TRANSLATION_PERIODIC);
    cbTSinus->setChecked(pAnim->type & TRANSLATION_COSINE);

    sbAngle0X->setValue(pAnim->angle_0.x);
    sbAngle0Y->setValue(pAnim->angle_0.y);
    sbAngle0Z->setValue(pAnim->angle_0.z);
    sbAngle1X->setValue(pAnim->angle_1.x);
    sbAngle1Y->setValue(pAnim->angle_1.y);
    sbAngle1Z->setValue(pAnim->angle_1.z);

    sbVec0X->setValue(pAnim->translate_0.x);
    sbVec0Y->setValue(pAnim->translate_0.y);
    sbVec0Z->setValue(pAnim->translate_0.z);
    sbVec1X->setValue(pAnim->translate_1.x);
    sbVec1Y->setValue(pAnim->translate_1.y);
    sbVec1Z->setValue(pAnim->translate_1.z);

    sbRW->setValue(pAnim->angle_w);
    sbTW->setValue(pAnim->translate_w);

    if (!cur)
        delete pAnim;

    updating = false;
}
