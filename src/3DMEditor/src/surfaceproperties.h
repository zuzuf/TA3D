#ifndef SURFACEPROPERTIES_H
#define SURFACEPROPERTIES_H

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include "imagelistview.h"

class SurfaceProperties : public QWidget
{
    Q_OBJECT;
public:
    SurfaceProperties();

private:
    QSpinBox        *red, *green, *blue, *alpha;
    QSpinBox        *redR, *greenR, *blueR, *alphaR;

    QCheckBox       *flagReflec;
    QCheckBox       *flagLighted;
    QCheckBox       *flagTextured;
    QCheckBox       *flagGouraud;
    QCheckBox       *flagBlended;
    QCheckBox       *flagPlayerColor;
    QCheckBox       *flagGLSL;

    ImageListView   *imageListView;
    bool            updating;

public slots:
    void refreshGUI();
    void readData();
    void updateWindowTitle();
    void loadTexture();
    void saveTexture();
    void newTexture();
    void deleteTexture();
    void moveTextureLeft();
    void moveTextureRight();
    void basicUV();
    void sphericalUV();
    void autoUV();
    void mergeVertices();
    void computeAmbientOcclusion();

signals:
    void surfaceChanged();

public:
    static SurfaceProperties *instance();

private:
    static SurfaceProperties *pInstance;
};

#endif // SURFACEPROPERTIES_H
