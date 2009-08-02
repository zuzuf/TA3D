#ifndef SURFACEPROPERTIES_H
#define SURFACEPROPERTIES_H

#include <QDockWidget>
#include <QCheckBox>
#include <QSpinBox>
#include "imagelistview.h"
#include "mesh.h"

class SurfaceProperties : public QDockWidget
{
    Q_OBJECT;
public:
    SurfaceProperties();

private:
    Mesh *getMeshWithTextures();
    void computeTexturePartition();

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
    QCheckBox       *flagRootTexture;

    ImageListView   *imageListView;
    bool            updating;

public slots:
    void refreshGUI();
    void readData();
    void updateWindowTitle();
    void loadTexture();
    void saveTexture();
    void newTexture();
    void flipTexture();
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
