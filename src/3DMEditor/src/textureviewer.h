#ifndef TEXTUREVIEWER_H
#define TEXTUREVIEWER_H

#include "types.h"

class TextureViewer : public QGLWidget
{
    Q_OBJECT;
public:
    TextureViewer();

    void initializeGL();
    void paintGL();

public slots:
    void updateSelection(int ID);

private:
    int selectedID;

public:
    static TextureViewer *instance();

private:
    static TextureViewer *pInstance;
};

#endif // TEXTUREVIEWER_H
