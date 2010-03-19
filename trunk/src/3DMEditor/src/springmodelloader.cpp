#include "springmodelloader.h"
#include "gfx.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QQueue>

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
	QString path;
	if (filename.contains('/'))
		path = filename.left(filename.lastIndexOf('/') + 1);
	else if (filename.contains('\\'))
		path = filename.left(filename.lastIndexOf('\\') + 1);
	if (header.texture1 > 0)
    {
		GLuint tex = Gfx::instance()->loadTexture(path + (char*) &fileBuf[header.texture1]);
        if (tex)
            model->tex.push_back(tex);
    }
    if (header.texture2 > 0)
    {
		GLuint tex = Gfx::instance()->loadTexture(path + (char*) &fileBuf[header.texture2]);
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

	piece->flag = SURFACE_ADVANCED | SURFACE_ROOT_TEXTURE | SURFACE_TEXTURED | SURFACE_GOURAUD | SURFACE_LIGHTED;

	piece->pos.x = 0.5f * fp->xoffset;
	piece->pos.y = 0.5f * fp->yoffset;
	piece->pos.z = 0.5f * fp->zoffset;
    switch(fp->primitiveType)
    {
    case S3O_PRIMTYPE_QUADS:
    case S3O_PRIMTYPE_TRIANGLES:
        piece->type = MESH_TRIANGLES;
        break;
    case S3O_PRIMTYPE_TRIANGLE_STRIP:
        piece->type = MESH_TRIANGLE_STRIP;
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
		piece->vertex.push_back(0.5f * v->pos);
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

void Mesh::saveS3O(const QString &filename)
{
	QFile file(filename);
	file.open(QIODevice::WriteOnly);

	QString tex1 = filename.left(filename.size() - 4) + "0.png";
	QString tex2 = filename.left(filename.size() - 4) + "1.png";
	QString tname1 = tex1;
	if (tname1.contains('/'))
		tname1 = tname1.right(tname1.size() - tname1.lastIndexOf('/') - 1);
	if (tname1.contains('\\'))
		tname1 = tname1.right(tname1.size() - tname1.lastIndexOf('\\') - 1);

	S3OHeader header;
	memcpy(header.magic, "Spring unit\0", 12);
	header.version = 0;
	header.radius = size;
	header.height = 0.0f;
	header.midx = 0.0f;
	header.midy = 0.0f;
	header.midz = 0.0f;
	header.rootPiece = 0;
	header.collisionData = 0;
	header.texture1 = tex.size() > 0 ? sizeof(Piece) : 0;
	header.texture2 = tex.size() > 1 ? header.texture1 + tname1.size() + 1 : 0;

	QQueue<Mesh*> queue;
	QQueue<Vec> qpos;
	queue.enqueue(this);
	qpos.enqueue(pos);
	while(!queue.empty())
	{
		Mesh *cur = queue.dequeue();
		Vec pos = qpos.dequeue();

		if (cur->next)
		{
			queue.push_back(cur->next);
			qpos.push_back(pos);
		}
		if (cur->child)
		{
			queue.push_back(cur->child);
			qpos.push_back(pos + cur->pos);
		}

		foreach(Vec v, cur->vertex)
			header.height = qMax(header.height, pos.y + cur->pos.y + v.y);
	}

	file.write((char*)&header, sizeof(header));
	if (header.texture1)
	{
		QImage img = Gfx::instance()->textureToImage( tex[0] );
		img.save(tex1);

		file.write(tname1.toAscii());
		file.write("\0", 1);
	}
	if (header.texture2)
	{
		QImage img = Gfx::instance()->textureToImage( tex[1] );
		img.save(tex2);
		QString tname = tex2;
		if (tname.contains('/'))
			tname = tname.right(tname.size() - tname.lastIndexOf('/') - 1);
		if (tname.contains('\\'))
			tname = tname.right(tname.size() - tname.lastIndexOf('\\') - 1);

		file.write(tname.toAscii());
		file.write("\0", 1);
	}

	Mesh *root = this;
	if (root->next)		// Do we need a fake root object ?
	{
		root = new Mesh;
		root->name = "root";
		root->child = this;
	}

	queue.enqueue(root);
	QQueue<int> ppos;
	ppos.enqueue(36);
	while(!queue.empty())
	{
		Mesh *cur = queue.dequeue();
		int offpos = ppos.dequeue();
		int p = file.pos();
		file.seek(offpos);
		file.write((char*)&p, sizeof(int));
		file.seek(p);

		Piece piece;
		piece.name = p + sizeof(Piece);
		piece.numChilds = 0;
		piece.childs = piece.name + cur->name.size() + 1;
		for (Mesh *m = cur->child ; m ; m = m->next)
		{
			queue.push_back(m);
			ppos.push_back(piece.childs + sizeof(int) * piece.numChilds);
			++piece.numChilds;
		}
		piece.numVertices = cur->vertex.size();
		piece.vertices = piece.childs + sizeof(int) * piece.numChilds;
		piece.vertexType = 0;
		piece.primitiveType = type == MESH_TRIANGLES ? 0 : 1;
		piece.vertexTableSize = cur->index.size();
		piece.vertexTable = piece.vertices + sizeof(SS3OVertex) * piece.numVertices;
		piece.collisionData = 0;
		piece.xoffset = 2.0f * cur->pos.x;
		piece.yoffset = 2.0f * cur->pos.y;
		piece.zoffset = 2.0f * cur->pos.z;

		file.write((char*)&piece, sizeof(Piece));
		file.write(cur->name.toAscii());
		file.write("\0", 1);

		for(int i = 0 ; i < piece.numChilds ; ++i)
			file.write((char*)&i, sizeof(int));

		for(int i = 0 ; i < cur->vertex.size() ; ++i)
		{
			SS3OVertex v;
			v.pos = 2.0f * cur->vertex[i];
			v.normal = cur->normal[i];
			if (cur->tcoord.empty())
			{
				v.textureX = 0.0f;
				v.textureY = 0.0f;
			}
			else
			{
				v.textureX = cur->tcoord[i << 1];
				v.textureY = cur->tcoord[(i << 1) | 1];
			}
			file.write((char*)&v, sizeof(SS3OVertex));
		}

		for(int i = 0 ; i < cur->index.size() ; ++i)
		{
			int idx = cur->index[i];
			file.write((char*)&idx, sizeof(int));
		}
	}

	if (root != this)
	{
		root->child = NULL;
		delete root;
	}

	file.close();
}
