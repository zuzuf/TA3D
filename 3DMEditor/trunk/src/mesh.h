#ifndef MESH_H
#define MESH_H

#include <QGLWidget>
#include <QString>
#include <QVector>
#include "misc/vector.h"

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void destroy();

    void load(const QString &filename);
    void save(const QString &filename);

    void load3DM(const QString &filename);
    void load3DO(const QString &filename);
    void loadASC(const QString &filename);
    void load3DS(const QString &filename);

private:
    QString         name;
    Vec             pos;
    Mesh            *child;
    Mesh            *next;
    QVector<Vec>    vertex;
    QVector<GLuint> index;
    QVector<GLuint> tex;

public:
    static Mesh instance;
};

#endif // MESH_H
