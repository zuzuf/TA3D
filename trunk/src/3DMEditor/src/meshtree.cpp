#include "config.h"
#include "meshtree.h"

MeshTree::MeshTree()
{
    mesh = NULL;
    child = NULL;
}

MeshTree::~MeshTree()
{
    if (child)
        delete[] child;
}

bool MeshTree::quickTest(const Vec &pos, const Vec &dir, int idx)
{
    if (idx < 0)
        return false;
    Vec &a = mesh->vertex[ mesh->index[idx] ];
    Vec &b = mesh->vertex[ mesh->index[idx + 1] ];
    Vec &c = mesh->vertex[ mesh->index[idx + 2] ];
    Vec Z;
    return mesh->hitTriangle(a, b, c, pos, dir, Z);
}

int MeshTree::collision(const Vec &pos, const Vec &dir)
{
    Vec OM = center - pos;
    float dist = OM.sq();
    if (dir * OM <= 0.0f && dist >= ray)        // We are out and looking away ==> even if we wanted we couldn't see the object
        return -1;
    float l = (dir ^ OM).sq();
    Vec Z;
    if (l + l * l / dist <= ray)      // Spherical collision ?
    {
        foreach(int f, faces)
        {
            Vec &a = mesh->vertex[ mesh->index[f] ];
            Vec &b = mesh->vertex[ mesh->index[f+1] ];
            Vec &c = mesh->vertex[ mesh->index[f+2] ];
            if (mesh->hitTriangle(a, b, c, pos, dir, Z))
                return f;
        }
        if (child)
        {
            int idx = child[0].collision(pos, dir);
            if (idx >= 0)
                return idx;
            return child[1].collision(pos, dir);
        }
    }
    return -1;
}

void MeshTree::build(Mesh *mesh)
{
    QVector<int> facesToProcess;
    switch(mesh->type)
    {
    case MESH_TRIANGLES:
        for(int i = 0 ; i < mesh->index.size(); i += 3)
            facesToProcess << i;
        break;
    case MESH_TRIANGLE_STRIP:
        for(int i = 2 ; i < mesh->index.size() ; i++)
        {
            facesToProcess << i - 2;
            facesToProcess << i - 1;
            facesToProcess << i;
        }
        break;
    };
    build(mesh, facesToProcess);
}

int closestPoint(const QVector<Vec> &points, const Vec &p)
{
    int i = 0;
    float m = (points[0] - p).sq();
    for(int e = 1 ; e < points.size() ; e++)
    {
        float d = (points[e] - p).sq();
        if (d < m)
        {
            i = e;
            m = d;
        }
    }
    return i;
}

void MeshTree::build(Mesh *mesh, QVector<int> &facesToProcess)
{
    this->mesh = mesh;
    center = Vec();
    foreach(int f, facesToProcess)
        center += mesh->vertex[ mesh->index[f] ] + mesh->vertex[ mesh->index[f+1] ] + mesh->vertex[ mesh->index[f+2] ];
    center = 1.0f / (3 * facesToProcess.size()) * center;
    ray = 0.0f;
    foreach(int f, facesToProcess)
    {
        ray = fmax( ray, (mesh->vertex[ mesh->index[f] ] - center).sq() );
        ray = fmax( ray, (mesh->vertex[ mesh->index[f+1] ] - center).sq() );
        ray = fmax( ray, (mesh->vertex[ mesh->index[f+2] ] - center).sq() );
    }

    if (facesToProcess.size() > 20)
    {
        QVector<Vec> centers = kmeans(2, facesToProcess);
        QVector<int> faceSet[2];
        foreach(int f, facesToProcess)
        {
            int res = 0;
            res |= 1 + closestPoint(centers, mesh->vertex[ mesh->index[f] ]);
            res |= 1 + closestPoint(centers, mesh->vertex[ mesh->index[f+1] ]);
            res |= 1 + closestPoint(centers, mesh->vertex[ mesh->index[f+2] ]);
            if (res & 1)
                faceSet[0] << f;
            if (res & 2)
                faceSet[1] << f;
        }
        if (faceSet[0].size() < facesToProcess.size() && faceSet[1].size() < facesToProcess.size())
        {
            child = new MeshTree[2];
            child[0].build(mesh, faceSet[0]);
            child[1].build(mesh, faceSet[1]);
            return;
        }
    }
    faces = facesToProcess;
}

QVector<Vec> MeshTree::kmeans(int n, const QVector<int> &faceIndex)
{
    QVector<Vec> points;
    for(int i = 0 ; i < n ; i++)
        points << mesh->vertex[ mesh->index[ faceIndex[i] ] ];

    for(int p = 0 ; p < 7 ; p++)
    {
        QVector<int> clusterSize;
        QVector<Vec> clusterCenter;
        clusterSize.resize(n);
        clusterCenter.resize(n);
        foreach(int f, faceIndex)
        {
            for(int e = 0 ; e < 3 ; e++)
            {
                Vec &P = mesh->vertex[ mesh->index[f+e] ];
                int i = closestPoint(points, P);
                clusterSize[i]++;
                clusterCenter[i] += P;
            }
        }
        for(int i = 0 ; i < n ; i++)
            points[i] = clusterSize[i] != 0 ? 1.0f / clusterSize[i] * clusterCenter[i] : Vec();
    }
    return points;
}
