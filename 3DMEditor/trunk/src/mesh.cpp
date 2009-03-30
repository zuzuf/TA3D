#include "mesh.h"
#include "gfx.h"

Mesh::Mesh()
{
    child = NULL;
    next = NULL;
    name.clear();
}

Mesh::~Mesh()
{
    destroy();
}

void Mesh::destroy()
{
    if (child)
        delete child;
    if (next)
        delete next;
    next = child = NULL;

    vertex.clear();
    index.clear();
    foreach(GLuint gltex, tex)
        Gfx::instance()->destroyTexture(gltex);
    tex.clear();
}

void Mesh::load(const QString &filename)
{
    if (filename.endsWith(".3dm", Qt::CaseInsensitive))
        load3DM(filename);
    else if (filename.endsWith(".3do", Qt::CaseInsensitive))
        load3DO(filename);
    else if (filename.endsWith(".3ds", Qt::CaseInsensitive))
        load3DS(filename);
    else if (filename.endsWith(".asc", Qt::CaseInsensitive))
        loadASC(filename);
}

void Mesh::load3DM(const QString &filename)
{
}

void Mesh::load3DO(const QString &filename)
{
}

void Mesh::load3DS(const QString &filename)
{
}

void Mesh::loadASC(const QString &filename)
{
}
