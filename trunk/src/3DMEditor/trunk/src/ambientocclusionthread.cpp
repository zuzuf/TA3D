#include "config.h"
#include "ambientocclusionthread.h"
#include "progressdialog.h"

AmbientOcclusionThread::AmbientOcclusionThread()
{
    tree = NULL;
    progress = 0;
}

void AmbientOcclusionThread::setTree(MeshTree *tree)
{
    this->tree = tree;
}

void AmbientOcclusionThread::pushTodoIdx(uint32 idx)
{
    todo << idx;
}

void AmbientOcclusionThread::setImage(QImage *img)
{
    this->img = img;
}

// Let do some work
void AmbientOcclusionThread::run()
{
    int w = img->width();
    int h = img->height();

    bool updateProcess = false;
    int prev = -1;
    progress = 0;
    int total = 1;
    for(int e = 0 ; e < listSize ; e++)
        total += list[e].todo.size();

    QVector<int> collisionCache;
    collisionCache.resize(dirs.size());
    foreach(uint32 i, todo)
    {
        progress++;
        updateProcess |= (i == 0);
        if (updateProcess)
        {
            int cur = 0;
            for(int e = 0 ; e < listSize ; e++)
                cur += list[e].progress;
            cur = cur * 100 / total;
            if (cur != prev)
                ProgressDialog::setProgress(cur);
            prev = cur;
        }

        Vec2 a(mesh->tcoord[mesh->index[i] * 2] * (w - 1), mesh->tcoord[mesh->index[i] * 2 + 1] * (h - 1));
        Vec2 b(mesh->tcoord[mesh->index[i + 1] * 2] * (w - 1), mesh->tcoord[mesh->index[i + 1] * 2 + 1] * (h - 1));
        Vec2 c(mesh->tcoord[mesh->index[i + 2] * 2] * (w - 1), mesh->tcoord[mesh->index[i + 2] * 2 + 1] * (h - 1));
        Vec A(mesh->vertex[mesh->index[i]]);
        Vec B(mesh->vertex[mesh->index[i + 1]]);
        Vec C(mesh->vertex[mesh->index[i + 2]]);

        Vec I(B - A);
        Vec J(C - A);
        Vec N(I ^J);
        N.unit();
        I = N ^ J;  I.unit();
        J = N ^ I;

        if (a.y > b.y)
        {
            qSwap(a, b);
            qSwap(A, B);
        }
        if (b.y > c.y)
        {
            qSwap(c, b);
            qSwap(C, B);
        }
        if (a.y > b.y)
        {
            qSwap(a, b);
            qSwap(A, B);
        }

        for(int e = 0 ; e < dirs.size() ; e++)
            collisionCache[e] = -1;

        if (a.y != b.y)
            for(int y = a.y ; y < b.y ; y++)
            {
                float fb = (y - a.y) / (b.y - a.y);
                float fc = (y - a.y) / (c.y - a.y);
                float x0 = a.x + (b.x - a.x) * fb;
                float x1 = a.x + (c.x - a.x) * fc;
                Vec P0 = A + fb * (B - A);
                Vec P1 = A + fc * (C - A);
                if (x0 > x1)
                {
                    qSwap(x0, x1);
                    qSwap(P0, P1);
                }
                for(int x = x0 ; x <= x1 ; x++)
                {
                    Vec D = P0 + ((float)(x - x0)) / (x1 - x0) * (P1 - P0) + relativePosition;
                    D += 0.001f * N;        // We don't want to detect a collision with ourselves

                    int n = 0;      // Number of occluders detected
                    for(int j = 0 ; j < dirs.size() ; j++)
                    {
                        Vec Dir = dirs[j].x * I + dirs[j].y * J + dirs[j].z * N;
                        if (tree->quickTest(D, Dir, collisionCache[j]))
                            n++;
                        else
                        {
                            int idx = tree->collision(D, Dir);
                            if (idx >= 0)
                            {
                                collisionCache[j] = idx;
                                n++;
                            }
                        }
                    }
                    n = 255 - n * 255 / dirs.size();

                    img->setPixel(x, h - 1 - y, qRgb(n, n, n));
                }
            }
        if (b.y != c.y)
            for(int y = b.y ; y <= c.y ; y++)
            {
                float fa = (y - a.y) / (c.y - a.y);
                float fb = (y - b.y) / (c.y - b.y);
                float x0 = b.x + (c.x - b.x) * fb;
                float x1 = a.x + (c.x - a.x) * fa;
                Vec P0 = B + fb * (C - B);
                Vec P1 = A + fa * (C - A);
                if (x0 > x1)
                {
                    qSwap(x0, x1);
                    qSwap(P0, P1);
                }
                for(int x = x0 ; x <= x1 ; x++)
                {
                    Vec D = P0 + ((float)(x - x0)) / (x1 - x0) * (P1 - P0) + relativePosition;
                    D += 0.001f * N;        // We don't want to detect a collision with ourselves

                    int n = 0;      // Number of occluders detected
                    for(int j = 0 ; j < dirs.size() ; j++)
                    {
                        Vec Dir = dirs[j].x * I + dirs[j].y * J + dirs[j].z * N;
                        if (tree->quickTest(D, Dir, collisionCache[j]))
                            n++;
                        else
                        {
                            int idx = tree->collision(D, Dir);
                            if (idx >= 0)
                            {
                                collisionCache[j] = idx;
                                n++;
                            }
                        }
                    }
                    n = 255 - n * 255 / dirs.size();

                    img->setPixel(x, h - 1 - y, qRgb(n, n, n));
                }
            }
    }
}

void AmbientOcclusionThread::setVecs(QVector<Vec> dirs, Vec const &relativePosition)
{
    this->dirs = dirs;
    this->relativePosition = relativePosition;
}

void AmbientOcclusionThread::setMesh(Mesh *mesh)
{
    this->mesh = mesh;
}

void AmbientOcclusionThread::setList(AmbientOcclusionThread *list, int n)
{
    this->list = list;
    listSize = n;
}
