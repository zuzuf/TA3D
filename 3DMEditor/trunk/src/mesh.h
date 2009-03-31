#ifndef MESH_H
#define MESH_H

#include <QGLWidget>
#include <QString>
#include <QVector>
#include "misc/vector.h"

enum MeshType { MESH_TRIANGLES, MESH_TRIANGLE_STRIP };

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void destroy();

    void draw();

    void load(const QString &filename);
    void save(const QString &filename);

    void load3DM(const QString &filename);
    void load3DO(const QString &filename);
    void loadASC(const QString &filename, float size = 10.0f);
    void load3DS(const QString &filename);

    void computeNormals();

private:
    QString             name;
    Vec                 pos;
    Mesh                *child;
    Mesh                *next;
    QVector<Vec>        vertex;
    QVector<Vec>        normal;
    QVector<GLuint>     index;
    QVector<GLuint>     tex;
    QVector<GLfloat>    tcoord;
    int                 type;

public:
    static Mesh instance;
};

#endif // MESH_H
