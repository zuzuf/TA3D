#ifndef GFX_H
#define GFX_H

#include "types.h"
#include "misc/matrix.h"

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

    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
private:
    void arcballMove(const Vec &pos, const float r, const QPoint &A, const QPoint &B);

private:
    QPoint              previousMousePos;
    Qt::MouseButtons    previousMouseState;
    Matrix              meshMatrix;

public:
    static Gfx *instance();

private:
    static Gfx *pInstance;
};

#endif // GFX_H
