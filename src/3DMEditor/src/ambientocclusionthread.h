#ifndef AMBIENTOCCLUSIONTHREAD_H
#define AMBIENTOCCLUSIONTHREAD_H

#include <QThread>
#include "meshtree.h"

class AmbientOcclusionThread : public QThread
{
private:
    MeshTree            *tree;
    Mesh                *mesh;
    QVector<uint32>     todo;
    QVector<Vec>        dirs;
    QImage              *img;
    Vec                 relativePosition;
    AmbientOcclusionThread *list;
    int                 listSize;
    int                 progress;

public:
    AmbientOcclusionThread();

    void setList(AmbientOcclusionThread *list, int n);
    void pushTodoIdx(uint32 idx);
    void setTree(MeshTree *tree);
    void setMesh(Mesh *mesh);
    void setImage(QImage *img);
    void setVecs(QVector<Vec> dirs, Vec const &relativePosition);

    virtual void run();
};

#endif // AMBIENTOCCLUSIONTHREAD_H
