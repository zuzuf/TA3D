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
    // The real processing function
    int collision(const Vec &pos, const Vec &dir);
    // Useful for caching
    bool quickTest(const Vec &pos, const Vec &dir, int idx);

private:
    QVector<Vec> kmeans(int n, const QVector<int> &faceIndex);
};

#endif // MESHTREE_H
