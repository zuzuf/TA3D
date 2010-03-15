#include "obj.h"
#include "mesh.h"
#include "gfx.h"
#include <QFile>
#include <QHash>
#include <QDebug>
#include <QQueue>

/*!
* \brief fill the Mesh with gathered data
*/
void Mesh::obj_finalize(QVector<int> &face, QVector<Vec> &vertex,
                      QVector<Vector2D> &tcoord, Material* mtl)
{
    int nb_vtx = face.size() >> 1;
    int nb_idx = face.size() >> 1;
    this->index.resize( nb_idx );
    this->vertex.resize( nb_vtx );
    this->tcoord.resize( 2 * nb_vtx );
    type = MESH_TRIANGLES;

    for (int i = 0 ; i < nb_idx ; i++)
    {
        this->vertex[i] = vertex[ face[i * 2] ];
        if (face[i * 2 + 1] == -1)
        {
            this->tcoord[i * 2] = 0.0f;
            this->tcoord[i * 2 + 1] = 0.0f;
        }
        else
        {
            this->tcoord[i * 2] = tcoord[ face[i * 2 + 1]].x;
            this->tcoord[i * 2 + 1] = 1.0f - tcoord[face[i * 2 + 1]].y;
        }
        this->index[i] = i;
    }

    computeNormals();

    flag = SURFACE_ADVANCED | SURFACE_LIGHTED | SURFACE_GOURAUD;
    if (mtl)
    {
        tex.push_back( Gfx::instance()->loadTexture( mtl->textureName ) );
        if (tex[0])
        {
            flag |= SURFACE_TEXTURED;
        }
    }
}


void Mesh::loadOBJ(const QString& filename)
{
    destroy();

    QFile src_obj( filename );
    if (!src_obj.exists())
        return;
    src_obj.open(QIODevice::ReadOnly);
    if (!src_obj.isOpen())
        return;

    QString filepath = filename;
    if (filepath.lastIndexOf("/") != -1)
        filepath = filepath.mid(0, filepath.lastIndexOf("/") + 1);
    else if (filepath.lastIndexOf("\\") != -1)
        filepath = filepath.mid(0, filepath.lastIndexOf("\\") + 1);

    Mesh *cur = this;
    bool firstObject = true;
    QVector<Vec>                lVertex;
    QVector<Vector2D>           lTcoord;
    QVector<int>                face;
    QHash<QString, Material>    mtllib;
    Material                    currentMtl;

    while (!src_obj.atEnd()) // Reads the whole file
    {
        QString line = src_obj.readLine(1024).trimmed();
        QVector<QString> args = QVector<QString>::fromList(line.split(" ", QString::SkipEmptyParts));
        if (!args.isEmpty())
        {
            if ( (args[0] == "o" || args[0] == "g") && args.size() > 1)      // Creates a new object
            {
                if (!face.isEmpty())
                {
                    if (firstObject && cur->name.isEmpty())
                        cur->name = "default";
                    cur->obj_finalize( face, lVertex, lTcoord, &currentMtl );
                    face.clear();
                    cur->child = new Mesh();
                    cur = cur->child;
                }
                firstObject = false;
                cur->name = args[1];
            }
            else if (args[0] == "mtllib" && args.size() > 1)        // Read given material libraries
            {
                foreach(QString s, args)
                {
                    QFile src_mtl(filepath + s);
                    if (!src_mtl.exists())
                        continue;
                    src_mtl.open(QIODevice::ReadOnly);
                    if (!src_mtl.isOpen())
                        continue;
                    Material mtl;
                    while (!src_mtl.atEnd())
                    {
                        QString line0 = src_mtl.readLine(1024).trimmed();
                        QVector<QString> args0 = QVector<QString>::fromList( line0.split(" ", QString::SkipEmptyParts) );
                        if (!args0.isEmpty())
                        {
                            if (args0[0] == "newmtl")
                                mtl.name = args0[1];
                            else
                            {
                                if (args0[0] == "map_Kd")
                                {
                                    mtl.textureName = filepath + args0[1];
                                    mtllib.insert(mtl.name, mtl);
                                }
                            }
                        }
                    }
                    src_mtl.close();;
                }
            }
            else
            {
                if (args[0] == "usemtl" && args.size() > 1)        // Change current material
                {
                    if (mtllib.find(args[1]) != mtllib.end())
                        currentMtl = *mtllib.find(args[1]);
                    else
                        currentMtl.textureName.clear();
                }
                else if (args[0] == "v" && args.size() > 3)  // Add a vertex to current object
                    lVertex.push_back( Vec(args[1].toFloat(), args[2].toFloat(), args[3].toFloat()));
                else if (args[0] == "vn" && args.size() > 3)  // Add a normal vector to current object
                {}
                else if (args[0] == "vt" && args.size() > 2)  // Add a texture coordinate vector to current object
                    lTcoord.push_back( Vector2D( args[1].toFloat(), args[2].toFloat()));
                else if (args[0] == "f" && args.size() > 1)  // Add a face to current object
                {
                    QVector<int>  vertex_idx;
                    QVector<int>  tcoord_idx;
                    bool first_string = true;
                    foreach(QString s, args)
                    {
                        // The first string is crap if we read it as vertex data !!
                        if (first_string)
                        {
                            first_string = false;
                            continue;
                        }

                        QVector<QString> data = QVector<QString>::fromList( s.trimmed().split("/", QString::SkipEmptyParts) );

                        if (!data.isEmpty())
                        {
                            vertex_idx.push_back( data[0].toInt() - 1);
                            if (vertex_idx.back() < 0)
                                qDebug() << line << " -> " << s << " -> " << vertex_idx.back();
							if (data.size() >= 2)
                            {
                                if (data[1].isEmpty())
                                    tcoord_idx.push_back(-1);
                                else
                                    tcoord_idx.push_back(data[1].toInt() - 1);
                            }
                            else
                            {
                                tcoord_idx.push_back(-1);
                            }
                        }
                    }

                    for (int i = 2; i < vertex_idx.size(); ++i) // Make triangles (FAN method)
                    {
                        face.push_back(vertex_idx[0]);
                        face.push_back(tcoord_idx[0]);

                        face.push_back(vertex_idx[i-1]);
                        face.push_back(tcoord_idx[i-1]);

                        face.push_back(vertex_idx[i]);
                        face.push_back(tcoord_idx[i]);
                    }
                }
            }
        }
    }

	cur->obj_finalize(face, lVertex, lTcoord, &currentMtl);

    src_obj.close();

    computeInfo();

    emit loaded();
}

void Mesh::saveOBJ(const QString &filename)
{
	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);

	QString mtllib = filename.left(filename.size() - 3) + "mtl";
	QFile mtlfile(mtllib);
	mtlfile.open(QIODevice::WriteOnly);
	QTextStream mtl(&mtlfile);

	if (mtllib.contains('/'))
		mtllib = mtllib.right(mtllib.size() - mtllib.lastIndexOf('/') - 1);
	if (mtllib.contains('\\'))
		mtllib = mtllib.right(mtllib.size() - mtllib.lastIndexOf('\\') - 1);

	out << "# File generated by 3DMEditor2" << endl
		<< "# www.ta3d.org" << endl
		<< "mtllib " << mtllib << endl;

	QQueue<Mesh*> queue;
	QQueue<Vec>	qpos;
	queue.push_back(this);
	qpos.push_back(Vec());

	out << fixed;

	GLuint offset = 1;
	GLuint offsetUV = 1;

	if (flag & SURFACE_ROOT_TEXTURE)
	{
		QString texname = filename.left(filename.size() - 3) + "png";
		mtl << "newmtl Material" << endl;
		if (!tex.empty())
		{
			QImage img = Gfx::instance()->textureToImage( tex[0] );
			img.save(texname);

			mtl << "map_Kd " << texname << endl;
		}
	}

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

		out << "o " << cur->name << endl;
		foreach(Vec v, cur->vertex)
		{
			v += pos + cur->pos;
			out << "v " << v.x << " " << v.y << " " << v.z << endl;
		}

		if (!cur->tcoord.empty())
			for(int i = 0 ; i < cur->tcoord.size() ; i += 2)
				out << "vt " << cur->tcoord[i] << " " << cur->tcoord[i + 1] << endl;

		if (flag & SURFACE_ROOT_TEXTURE)
			out << "usemtl Material" << endl;
		else
		{
			QString mtlname = cur->name.isEmpty() ? QString("mtl") + QString::number(qint64(cur), 16) : cur->name;
			QString texname = mtllib.left(mtllib.size() - 4) + "-" + mtlname + ".png";
			QString texname2 = filename.left(filename.size() - 4) + "-" + mtlname + ".png";
			mtl << "newmtl " << mtlname << endl;
			out << "usemtl " << mtlname << endl;
			if (!cur->tex.empty())
			{
				QImage img = Gfx::instance()->textureToImage( cur->tex[0] );
				img.save(texname2);

				mtl << "map_Kd " << texname << endl;
			}
		}
		if (cur->type == MESH_TRIANGLES)
		{
			for(int i = 0 ; i < cur->index.size() ; i += 3)
			{
				out << "f " << (cur->index[i] + offset) << "/" << (cur->index[i] + offsetUV) << " "
							<< (cur->index[i + 1] + offset) << "/" << (cur->index[i + 1] + offsetUV) << " "
							<< (cur->index[i + 2] + offset) << "/" << (cur->index[i + 2] + offsetUV) << endl;
			}
		}
		else if (cur->type == MESH_TRIANGLE_STRIP)
		{
			GLuint a = cur->index[0] + offset;
			GLuint b = cur->index[1] + offset;
			int shift = offsetUV - offset;
			for(int i = 2 ; i < cur->index.size() ; ++i)
			{
				out << "f " << a << "/" << (a + shift) << " "
							<< b << "/" << (b + shift) << " "
							<< (cur->index[i] + offset) << "/" << (cur->index[i] + offsetUV) << endl;
				a = b;
				b = cur->index[i] + offset;
			}
		}
		offset += cur->vertex.size();
		offsetUV += cur->tcoord.size();
	}

	file.close();
}
