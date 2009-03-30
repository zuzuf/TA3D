#ifndef GFX_H
#define GFX_H

#include <QGLWidget>
#include "misc/vector.h"

class Gfx : public QGLWidget
{
    Q_OBJECT;
public:
    Gfx();
    ~Gfx();

    void initializeGL();
    void paintGL();
    GLuint loadTexture(const QString &filename);
    void destroyTexture(GLuint &gltex);
    void SetDefState();
    void ReInitTexSys(bool matrix_reset = true);

    void drawCylinder(float r, float h, int d);
    void drawCone(float r, float h, int d);
    void drawArrow(const Vec &a, const Vec &b, float r);

public:
    static Gfx *instance();

private:
    static Gfx *pInstance;
};

#endif // GFX_H
