#include <QFile>
#include <QDebug>
#include "mesh.h"
#include "gfx.h"

Mesh Mesh::instance;

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
    normal.clear();
    tcoord.clear();
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

void Mesh::loadASC(const QString &filename, float size)
{
    QFile file(filename);
    if (!file.exists())
        return;

    destroy();
    QVector<Vec> coor;
    QVector<int> face;

    int i;
    float x, y, z;
    int offset = 0;
    float xmin = 0xFFFFFF, ymin = 0xFFFFFF, zmin = 0xFFFFFF,
          xmax = -0xFFFFFF, ymax = -0xFFFFFF, zmax = -0xFFFFFF;

    QVector<int> structD;     // Data for structure reconstruction
    QVector<QString> structName;

    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return;

    while (!file.atEnd())
    {
        // Read the file
        QString line = file.readLine(1024).toUpper();
        for(i = 0 ; i < line.size() ; i++)
            if (line[i] == QChar(':'))
                line[i] = ' ';
        if (line.startsWith("VERTEX"))
        {
            if (!line.startsWith("VERTEX LIST"))
            {
                // Read vertex coordinates
                i = 0;
                while (!line.section(" ", i, i).startsWith("X") && i < 100)    i++;
                i++;
                while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                x = line.section(" ", i, i).toFloat();

                while (!line.section(" ", i, i).startsWith("Y") && i < 100)    i++;
                i++;
                while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                y = line.section(" ", i, i).toFloat();

                while (!line.section(" ", i, i).startsWith("Z") && i < 100)    i++;
                i++;
                while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                z = line.section(" ", i, i).toFloat();

                coor.push_back( Vec(x,y,z) );

                if (x < xmin) xmin = x;
                if (x > xmax) xmax = x;
                if (y < ymin) ymin = y;
                if (y > ymax) ymax = y;
                if (z < zmin) zmin = z;
                if (z > zmax) zmax = z;
            }
        }
        else
        {
            if (line.startsWith("FACE"))
            {
                if (!line.startsWith("FACE LIST"))
                {
                    // Read a polygon
                    i = 0;
                    while (!line.section(" ", i, i).startsWith("A") && i < 100)    i++;
                    i++;
                    while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                    face.push_back( line.section(" ", i, i).toInt() + offset );

                    while (!line.section(" ", i, i).startsWith("B") && i < 100)    i++;
                    i++;
                    while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                    face.push_back( line.section(" ", i, i).toInt() + offset );

                    while (!line.section(" ", i, i).startsWith("C") && i < 100)    i++;
                    i++;
                    while (line.section(" ", i, i).isEmpty() && i < 100)    i++;
                    face.push_back( line.section(" ", i, i).toInt() + offset );
                }
            }
            else
            {
                if (line.startsWith("NAMED OBJECT"))
                {
                    structName.push_back( line.mid(13, line.size() - 13).trimmed() );
                    offset = vertex.size();
                    structD.push_back( face.size() );
                }
            }
        }
    }

    file.close();

    structD.push_back( face.size() );
    Mesh *cur = this;

    xmin = xmax - xmin;
    ymin = ymax - ymin;
    zmin = zmax - zmin;
    xmax = sqrtf(xmin * xmin + ymin * ymin + zmin * zmin);
    size = size / xmax;

    for (i = 0; i < structName.size() ; ++i)    // Generate mesh parts
    {
        if (i > 0)
        {
            cur->next = new Mesh;
            cur = cur->next;
        }
        cur->name = structName[i];
        int nb_vtx = structD[i + 1] - structD[i];
        cur->vertex.reserve( nb_vtx );
        cur->index.reserve( nb_vtx );
        cur->tcoord.reserve( nb_vtx * 2 );
        cur->type = MESH_TRIANGLES;
        cur->pos = Vec();

//        cur->surface.Flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
//        for (int k = 0; k < 4; ++k)
//            cur->surface.Color[k] = cur->surface.RColor[k] = 1.0f;

        int nbp = 0;

        for (int k = structD[i] ; k < structD[i+1] ; k += 3) // Compte et organise les points
        {
            cur->index.push_back( nbp++ );
            cur->index.push_back( nbp++ );
            cur->index.push_back( nbp++ );

            cur->vertex.push_back( coor[face[k]] );
            cur->vertex.push_back( coor[face[k+1]] );
            cur->vertex.push_back( coor[face[k+2]] );
        }
    }

    cur = this;
    while (cur)
    {
        for (i = 0 ; i < cur->vertex.size() ; ++i)
        {
            cur->vertex[i].x *= size;
            cur->vertex[i].y *= size;
            cur->vertex[i].z *= size;
        }

        cur->computeNormals();      // Compute normals
        cur = cur->next;
    }
}

void Mesh::save(const QString &filename)
{
    // We only save 3DM files
    if (!filename.endsWith(".3dm", Qt::CaseInsensitive))
        return;
}

void Mesh::computeNormals()
{
    normal.resize(vertex.size());
    for(int i = 0 ; i < normal.size() ; i++)
        normal[i].reset();
    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        for(int i = 0 ; i < index.size() ; i++)
        {
            Vec n = (vertex[ index[i + 1] ] - vertex[ index[i] ]) * (vertex[ index[i + 2] ] - vertex[ index[i] ]);
            n.unit();
            normal[ index[i] ] += n;
            normal[ index[i + 1] ] += n;
            normal[ index[i + 2] ] += n;
        }
        break;
    case MESH_TRIANGLES:
    default:
        for(int i = 0 ; i < index.size() ; i += 3)
        {
            Vec n = (vertex[ index[i + 1] ] - vertex[ index[i] ]) * (vertex[ index[i + 2] ] - vertex[ index[i] ]);
            n.unit();
            normal[ index[i] ] += n;
            normal[ index[i + 1] ] += n;
            normal[ index[i + 2] ] += n;
        }
        break;
    };
    for(int i = 0 ; i < normal.size() ; i++)
        normal[i].unit();
}

void Mesh::draw()
{
    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertex.data());
    glNormalPointer(GL_FLOAT, 0, normal.data());

    if (tex.isEmpty())
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        glDrawElements(GL_TRIANGLE_STRIP, index.size(), GL_UNSIGNED_INT, index.data());
        break;
    case MESH_TRIANGLES:
    default:
        glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, index.data());
        break;
    };

    if (child)
        child->draw();

    glPopMatrix();
    if (next)
        next->draw();
}
