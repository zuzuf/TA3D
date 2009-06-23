#include "springmodelloader.h"
#include "gfx.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>

void Mesh::load3SO(const QString &filename)
{
    destroy();

    QFile file(filename);
    if (!file.exists())
    {
        QMessageBox::critical(NULL, tr("Spring Model Loader error"), tr("File doesn't exist"));
        return;
    }
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
    {
        QMessageBox::critical(NULL, tr("Spring Model Loader error"), tr("File could not be open for reading"));
        return;
    }

    byte* fileBuf = new byte[file.size()];
    file.read((char*)fileBuf, file.size());

    S3OHeader header;
    memcpy(&header, fileBuf, sizeof(header));
    if (memcmp(header.magic, "Spring unit\0", 12))      // File corrupt or wrong format
    {
        delete[] fileBuf;
        QMessageBox::critical(NULL, tr("Spring Model Loader error"), tr("File is corrupt or in wrong format"));
        return;
    }

    Mesh* model = this;
    model->type = MESH_TRIANGLES;
    model->name = filename;
    if (header.texture1 > 0)
    {
        GLuint tex = Gfx::instance()->loadTexture((char*) &fileBuf[header.texture1]);
        if (tex)
            model->tex.push_back(tex);
    }
    if (header.texture2 > 0)
    {
        GLuint tex = Gfx::instance()->loadTexture((char*) &fileBuf[header.texture2]);
        if (tex)
            model->tex.push_back(tex);
    }

    SpringModelLoader::LoadPiece(fileBuf, header.rootPiece, model);

    delete[] fileBuf;

    computeInfo();
    emit loaded();
}

Mesh* SpringModelLoader::LoadPiece(byte* buf, int offset, Mesh* model)
{
    Mesh* piece = model ? model : new Mesh;
    piece->type = MESH_TRIANGLES;

    Piece* fp = (Piece*)&buf[offset];

    piece->pos.x = fp->xoffset;
    piece->pos.y = fp->yoffset;
    piece->pos.z = fp->zoffset;
    switch(fp->primitiveType)
    {
    case S3O_PRIMTYPE_QUADS:
    case S3O_PRIMTYPE_TRIANGLES:
        piece->type = MESH_TRIANGLES;
        break;
    case S3O_PRIMTYPE_TRIANGLE_STRIP:
        break;
    };
    piece->name = (char*) &buf[fp->name];
    qDebug() << "SpringModelLoader loading piece '" << piece->name << "'";

    // retrieve each vertex
    int vertexOffset = fp->vertices;
    qDebug() << "piece has " << fp->numVertices << " vertices";
    switch(fp->primitiveType)
    {
    case S3O_PRIMTYPE_QUADS:                qDebug() << "piece is of type QUADS";   break;
    case S3O_PRIMTYPE_TRIANGLES:            qDebug() << "piece is of type TRIANGLES";   break;
    case S3O_PRIMTYPE_TRIANGLE_STRIP:       qDebug() << "piece is of type TRIANGLE_STRIP";   break;
    };

    for (int a = 0; a < fp->numVertices; ++a)
    {
        SS3OVertex* v = (SS3OVertex*) &buf[vertexOffset];
        piece->vertex.push_back(v->pos);
        piece->normal.push_back(v->normal);
        piece->tcoord.push_back(v->textureX);
        piece->tcoord.push_back(v->textureY);

        vertexOffset += sizeof(SS3OVertex);
    }


    // retrieve the draw order for the vertices
    int vertexTableOffset = fp->vertexTable;
    qDebug() << "piece has " << fp->vertexTableSize << " indices";

    for (int a = 0; a < fp->vertexTableSize; ++a)
    {
        int vertexDrawIdx = *(int*) &buf[vertexTableOffset];

        piece->index.push_back(vertexDrawIdx);
        if (fp->primitiveType == S3O_PRIMTYPE_QUADS && (a % 4) == 2)        // QUADS need to be split into triangles (this would be done internally by OpenGL anyway since quads are rendered as 2 triangles)
        {
            piece->index.push_back(piece->index[piece->index.size() - 3]);
            piece->index.push_back(vertexDrawIdx);
        }
        vertexTableOffset += sizeof(int);

        // -1 == 0xFFFFFFFF (U)
        if (vertexDrawIdx == -1 && a != fp->vertexTableSize - 1)
        {
            // for triangle strips
            piece->index.push_back(vertexDrawIdx);

            vertexDrawIdx = *(int*) &buf[vertexTableOffset];
            piece->index.push_back(vertexDrawIdx);
        }
    }

    int childTableOffset = fp->childs;

    qDebug() << "piece has " << fp->numChilds << " childs";
    for (int a = 0; a < fp->numChilds; ++a)
    {
        int childOffset = *(int*) &buf[childTableOffset];

        Mesh* childPiece = LoadPiece(buf, childOffset);
        if (piece->child)
        {
            childPiece->next = piece->child;
            piece->child = childPiece;
        }
        else
            piece->child = childPiece;

        childTableOffset += sizeof(int);
    }

    return piece;
}
