#ifndef GFX_H
#define GFX_H

#include <QGLWidget>

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

public:
    static Gfx *instance();

private:
    static Gfx *pInstance;
};

#endif // GFX_H
