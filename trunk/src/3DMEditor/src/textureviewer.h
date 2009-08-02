#ifndef TEXTUREVIEWER_H
#define TEXTUREVIEWER_H

#include "types.h"
#include <QDockWidget>

class TextureViewer : public QGLWidget
{
    Q_OBJECT;
public:
    TextureViewer();

    void initializeGL();
    void paintGL();
    void resize(int w, int h);

public slots:
    void updateSelection(int ID);
    void show();

private:
    int selectedID;
    QDockWidget *dock;

public:
    static TextureViewer *instance();

private:
    static TextureViewer *pInstance;
};

#endif // TEXTUREVIEWER_H
