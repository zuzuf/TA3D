#ifndef GFX_H
#define GFX_H

#include "types.h"
#include "misc/matrix.h"

class Gfx : public QGLWidget
{
    Q_OBJECT;

    friend class MainWindow;

    enum EditMode { EDIT, VIEW, ANIMATE };

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
    void drawCube(float size);
    void drawSphere(float r, int d);

    int getSelectionID();

    int getTextureWidth(GLuint tex);
    int getTextureHeight(GLuint tex);
    QImage textureToImage(GLuint tex);

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
private:
    void arcballMove(const Vec &pos, const float r, const QPoint &A, const QPoint &B);

public slots:
    void updateSelection(int ID);
    void showSelection();
    void hideSelection();
    void setViewMode();
    void setEditMode();
    void setAnimateMode();

signals:
    void selectionChange(int ID);

private:
    QPoint              previousMousePos;
    Qt::MouseButtons    previousMouseState;
    Matrix              meshMatrix;
    int                 selectedID;
    bool                drawSelection;
    EditMode            editMode;

public:
    static Gfx *instance();
    static QImage loadTGA(const QString &filename);

private:
    static Gfx *pInstance;
};

#endif // GFX_H
