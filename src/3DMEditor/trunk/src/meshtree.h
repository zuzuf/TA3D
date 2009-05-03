#ifndef MESHTREE_H
#define MESHTREE_H

#include "types.h"
#include "mesh.h"

class MeshTree
{
private:
    QVector<int>    faces;
    MeshTree        *child;
    Vec             center;
    float           ray;
    Mesh            *mesh;
public:
    MeshTree();
    ~MeshTree();

    void build(Mesh *mesh);
    void build(Mesh *mesh, QVector<int> &facesToProcess);
    bool collision(const Vec &pos, const Vec &dir);

private:
    QVector<Vec> kmeans(int n, const QVector<int> &faceIndex);
};

#endif // MESHTREE_H
