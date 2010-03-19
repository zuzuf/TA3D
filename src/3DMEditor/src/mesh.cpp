#include <QDebug>
#include <QQueue>
#include <QPainter>
#include <QTime>
#include <zlib.h>
#include "mesh.h"
#include "gfx.h"
#include "animation.h"
#include <math.h>

bool Mesh::whiteSurface = false;
Mesh *Mesh::pInstance = NULL;
bool Mesh::animated = false;

#define DEG2RAD (M_PI / 180.0f)

Mesh *Mesh::instance()
{
    if (!pInstance)
        pInstance = new Mesh;
    return pInstance;
}

Mesh::Mesh()
{
    child = NULL;
    next = NULL;
    name.clear();
    flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
    color = rColor = 0xFFFFFFFF;
    resetAnimData();
    resetScriptData();
}

Mesh::~Mesh()
{
    if (this != pInstance)
        destroy();
}

void ANIMATION::animate( double &t, Vector3D &R, Vector3D& T)
{
    if (type & ROTATION)
    {
        if (type & ROTATION_PERIODIC)
        {
            float coef;
            if( type & ROTATION_COSINE )
                coef = 0.5f + 0.5f * cos( t * angle_w );
            else {
                coef = t * angle_w;
                int i = (int) coef;
                coef = coef - i;
                coef = (i&1) ? (1.0f - coef) : coef;
            }
            R = coef * angle_0 + (1.0f - coef) * angle_1;
        }
        else
            R = t * angle_0;
    }
    if (type & TRANSLATION)
    {
        if (type & TRANSLATION_PERIODIC)
        {
            float coef;
            if (type & TRANSLATION_COSINE)
                coef = 0.5f + 0.5f * cos( t * translate_w );
            else
            {
                coef = t * translate_w;
                int i = (int) coef;
                coef = coef - i;
                coef = (i&1) ? (1.0f - coef) : coef;
            }
            T = coef * translate_0 + (1.0f - coef) * translate_1;
        }
        else
            T = t * translate_0;
    }
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
    flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
    color = rColor = 0xFFFFFFFF;
    name.clear();
    ID = 0;

    emit loaded();
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
    else if (filename.endsWith(".obj", Qt::CaseInsensitive))
        loadOBJ(filename);
    else if (filename.endsWith(".s3o", Qt::CaseInsensitive))
        load3SO(filename);
    else if (!filename.isEmpty())
        destroy();
}

void Mesh::load3DM(const QString &filename)
{
    destroy();
    QFile file(filename);
    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);
        if (file.isOpen())
        {
            load3DMrec(file);
            file.close();

            computeInfo();
            emit loaded();
        }
    }
}

void Mesh::load3DO(const QString &filename)
{
	destroy();
	QFile file(filename);
	if (file.exists())
	{
		file.open(QIODevice::ReadOnly);
		if (file.isOpen())
		{
			load3DOrec(file);
			file.close();

			computeInfo();
			emit loaded();
		}
	}
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
        cur->flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;

        color = rColor = 0xFFFFFFFF;

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

    computeInfo();

    emit loaded();
}

void Mesh::save(const QString &filename)
{
    // We only save 3DM files
    if (!filename.endsWith(".3dm", Qt::CaseInsensitive))
        return;

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    if (!file.exists() || !file.isOpen())
    {
        qDebug() << "could not save mesh as '" << filename << "'";
        return;
    }

    save3DMrec(file);

    file.close();
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
            Vec n = (vertex[ index[i + 1] ] - vertex[ index[i] ]) ^ (vertex[ index[i + 2] ] - vertex[ index[i] ]);
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
            Vec n = (vertex[ index[i + 1] ] - vertex[ index[i] ]) ^ (vertex[ index[i + 2] ] - vertex[ index[i] ]);
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

void Mesh::draw(int id, Mesh *root)
{
    if (root == NULL)
        root = this;
    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);
    bool hidden = false;

    if (animated && Animation::instance()->isInDefaultAnimationMode())
    {
        double t = QTime().msecsTo(QTime::currentTime()) * 0.001;
        Vector3D R;
        Vector3D T;
        defaultAnim.animate(t, R, T);
        glTranslatef(T.x, T.y, T.z);
        glRotatef(R.x, 1.0f, 0.0f, 0.0f);
        glRotatef(R.y, 0.0f, 1.0f, 0.0f);
        glRotatef(R.z, 0.0f, 0.0f, 1.0f);
    }
    else if (animated)      // Script animation
    {
        glTranslatef(axe[0].pos, axe[1].pos, axe[2].pos);
        glRotatef(axe[0].angle, 1.0f, 0.0f, 0.0f);
        glRotatef(axe[1].angle, 0.0f, 1.0f, 0.0f);
        glRotatef(axe[2].angle, 0.0f, 0.0f, 1.0f);
        hidden = anim_flag & FLAG_HIDE;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (!vertex.isEmpty() && !normal.isEmpty() && !hidden)
    {
        QVector<GLuint> *pTex = &tex;
        if (flag & SURFACE_ROOT_TEXTURE)
            pTex = &(root->tex);

        glVertexPointer(3, GL_FLOAT, 0, vertex.data());
        glNormalPointer(GL_FLOAT, 0, normal.data());

        if (!whiteSurface)
        {
            GLfloat colorf[4] = { (color >> 24) / 255.0f,
                                  ((color >> 16) & 0xFF) / 255.0f,
                                  ((color >> 8) & 0xFF) / 255.0f,
                                  (color & 0xFF) / 255.0f };
            glColor4fv(colorf);

            if (flag & SURFACE_GLSL)
                shader.on();

            if (flag & SURFACE_GOURAUD)
                glShadeModel(GL_SMOOTH);
            else
                glShadeModel(GL_FLAT);

            if (flag & SURFACE_LIGHTED)
                glEnable(GL_LIGHTING);
            else
                glDisable(GL_LIGHTING);

            if (pTex->isEmpty() || !(flag & SURFACE_TEXTURED))
            {
                for(int i = 7 ; i >= 0 ; i--)
                {
                    glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glDisable(GL_TEXTURE_2D);
                    Gfx::instance()->ReInitTexSys();
                }
            }
            else
            {
                for(int i = pTex->size() - 1 ; i >= 0 ; i--)
                {
                    glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, (*pTex)[i]);
                    glTexCoordPointer(2, GL_FLOAT, 0, tcoord.data());

                    if ((flag & SURFACE_REFLEC) && i == pTex->size() - 1)
                    {
                        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                        glEnable(GL_TEXTURE_GEN_S);
                        glEnable(GL_TEXTURE_GEN_T);
                        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
                        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE);

                        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_TEXTURE);
                        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
                        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_PREVIOUS_EXT);
                        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
                        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_CONSTANT_EXT);
                        glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);
                        float rColorf[4] = { (rColor >> 24) / 255.0f,
                                             ((rColor >> 16) & 0xFF) / 255.0f,
                                             ((rColor >> 8) & 0xFF) / 255.0f,
                                             (rColor & 0xFF) / 255.0f };
                        glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,rColorf);
                    }
                    else
                    {
                        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                        glDisable(GL_TEXTURE_GEN_S);
                        glDisable(GL_TEXTURE_GEN_T);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    }
                    if (shader.isOn())
                        shader.setvar1i(QString("tex%1").arg(i).toAscii().data(), i);
                }
            }
            if (flag & SURFACE_BLENDED)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GREATER, 0.1f);
            }
            else
            {
                glDisable(GL_BLEND);
                glDisable(GL_ALPHA_TEST);
            }
        }

        if (id == -1 || id == ID)
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

        if (!whiteSurface)
        {
            if (flag & SURFACE_GLSL)
                shader.off();

            if (flag & SURFACE_BLENDED)
            {
                glDisable(GL_BLEND);
                glDisable(GL_ALPHA_TEST);
            }
            if (!pTex->isEmpty() && (flag & SURFACE_TEXTURED))
                for(int i = pTex->size() - 1 ; i >= 0 ; i--)
                {
                    glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glDisable(GL_TEXTURE_2D);
                    Gfx::instance()->ReInitTexSys();
                }
        }
    }

    if (id == -1)
    {
        if (child)
            child->draw(-1, root);

        glPopMatrix();
        if (next)
            next->draw(-1, root);
    }
    else if (id != ID)
    {
        if (((next && next->ID > id) || next == NULL) && child)
        {
            child->draw(id, root);
            glPopMatrix();
        }
        else if (next)
        {
            glPopMatrix();
            next->draw(id, root);
        }
        else
            glPopMatrix();
    }
    else
        glPopMatrix();
}

void Mesh::load3DMrec(QFile &file)
{
    if (file.atEnd())
        return;

    type = MESH_TRIANGLES;

    uint8 len;
    file.getChar((char*)&len);
    char tmp[257];
    file.read(tmp, len);
    tmp[len] = 0;
    name = QString(tmp);

    file.read((char*)&pos, sizeof(pos));

    sint16 nb_vtx;
    file.read((char*)&nb_vtx, sizeof(nb_vtx));

    if (nb_vtx < 0)
    {
        name.clear();
        return;
    }
    vertex.resize(nb_vtx);
    if (nb_vtx > 0)
        file.read((char*)vertex.data(), sizeof(Vec) * nb_vtx);

    GLushort sel[4];
    file.read((char*)sel, sizeof(GLushort) * 4);

    sint16 nb_p_idx;
    file.read((char*)&nb_p_idx, sizeof(nb_p_idx)); // Read point data
    if (nb_p_idx < 0)
    {
        vertex.clear();
        name.clear();
        return;
    }
    if (nb_p_idx > 0)
        file.read(sizeof(GLushort) * nb_p_idx);

    sint16 nb_l_idx;
    file.read((char*)&nb_l_idx, sizeof(nb_l_idx));	// Read line data
    if (nb_l_idx < 0)
    {
        vertex.clear();
        name.clear();
        return;
    }
    if (nb_l_idx > 0)
        file.read(sizeof(GLushort) * nb_l_idx);

    sint16 nb_idx;
    file.read((char*)&nb_idx, sizeof(nb_idx)); // Read triangle data
    if (nb_idx < 0)
    {
        vertex.clear();
        name.clear();
        return;
    }
    index.resize(nb_idx);
    for(int i = 0 ; i < nb_idx ; i++)
    {
        GLushort id;
        file.read((char*)&id, sizeof(GLushort));
        index[i] = id;
    }

    tcoord.resize(nb_vtx << 1);
    file.read((char*)tcoord.data(), sizeof(float) * nb_vtx << 1);

    float colorf[4];
    float rColorf[4];

    file.read((char*)colorf, sizeof(float) * 4);	// Read surface data
    file.read((char*)rColorf, sizeof(float) * 4);
    color = ((uint32)(colorf[0] * 255) << 24)
            |((uint32)(colorf[1] * 255) << 16)
            |((uint32)(colorf[2] * 255) << 8)
            |((uint32)(colorf[3] * 255));
    rColor = ((uint32)(rColorf[0] * 255) << 24)
            |((uint32)(rColorf[1] * 255) << 16)
            |((uint32)(rColorf[2] * 255) << 8)
            |((uint32)(rColorf[3] * 255));
    file.read((char*)&flag, sizeof(flag));
    sint8 nb_tex = 0;
    file.read((char*)&nb_tex, sizeof(nb_tex));
    bool compressed = nb_tex < 0;
    nb_tex = abs(nb_tex);
    tex.resize( nb_tex );
    for (int i = 0 ; i < nb_tex ; ++i)
    {
        QImage img;
        if (!compressed)
        {
            int tex_w;
            int tex_h;
            file.read((char*)&tex_w, sizeof(tex_w));
            file.read((char*)&tex_h, sizeof(tex_h));

            img = QImage(tex_w, tex_h, QImage::Format_ARGB32);
            for (int y = 0 ; y < tex_h ; ++y)
                for (int x = 0 ; x < tex_w ; ++x)
                {
                    uint32 col;
                    file.read((char*)&col, 4);
                    col = ((col << 8) & 0xFF0000) | (col << 24) | ((col >> 8) & 0xFF00) | (col >> 24);     // BGRA -> ARGB
                    col = (col >> 8) | (col << 24);     // RGBA -> ARGB
                    img.setPixel(x, tex_h - 1 - y, col);
                }
        }
        else
        {
            int img_size = 0;
            uint8 bpp;
            int w, h;
            file.read((char*)&w, sizeof(w));
            file.read((char*)&h, sizeof(h));
            file.read((char*)&bpp, sizeof(bpp));
            file.read((char*)&img_size, sizeof(img_size));	// Read RGBA data
            byte *buffer = new byte[ img_size ];

            file.read( (char*)buffer, img_size );

            byte *unbuf = new byte[ bpp * w * h / 8 ];
            uLongf len = w * h * bpp / 8;
            uncompress ( (Bytef*) unbuf, &len, (Bytef*) buffer, img_size);

            img = QImage(w, h, QImage::Format_ARGB32);
            for (int y = 0 ; y < h ; ++y)
                for (int x = 0 ; x < w ; ++x)
                {
                    uint32 col(0);
                    switch(bpp)
                    {
                    case 8:
                        col = unbuf[y * w + x] * 0x01010100 | 0xFF;
                        break;
                    case 24:
                        col = unbuf[(y * w + x) * 3] * 0x01000000 + unbuf[(y * w + x) * 3 + 1] * 0x010000 + unbuf[(y * w + x) * 3 + 2] * 0x0100;
                        break;
                    case 32:
                        col = ((uint32*)unbuf)[y * w + x];
                        break;
                    };
                    col = ((col << 8) & 0xFF0000) | (col << 24) | ((col >> 8) & 0xFF00) | (col >> 24);     // BGRA -> ARGB
                    col = (col >> 8) | (col << 24);     // RGBA -> ARGB
                    img.setPixel(x, h - 1 - y, col);
                }

            delete[] unbuf;

            delete[] buffer;
        }
        tex[i] = Gfx::instance()->bindTexture(img);
    }

    if (flag & SURFACE_GLSL) // Fragment & Vertex shaders
    {
        uint32 shader_size;
        file.read((char*)&shader_size, 4);
        QByteArray buf;
        buf.resize(shader_size);
        file.read(buf.data(), shader_size);
        vertexProgram = buf;

        file.read((char*)&shader_size, 4);
        buf.resize(shader_size);
        file.read(buf.data(), shader_size);
        fragmentProgram = buf;
        shader.load_memory(vertexProgram, fragmentProgram);
    }

    computeNormals();

    byte link;
    file.read((char*)&link, 1);

    if (link == 2) // Load animation data if present
    {
        ANIMATION *animation_data = &defaultAnim;
        file.read( (char*)&(animation_data->type), 1 );
        file.read( (char*)&(animation_data->angle_0), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->angle_1), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->angle_w), sizeof(float)  );
        file.read( (char*)&(animation_data->translate_0), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->translate_1), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->translate_w), sizeof(float) );

        file.read( (char*)&link, 1 );
    }
    else
        defaultAnim = ANIMATION();

    if (link)
    {
        child = new Mesh;
        child->load3DMrec(file);
    }
    else
        child = NULL;

    file.read((char*)&link, 1);
    if (link)
    {
        next = new Mesh;
        next->load3DMrec(file);
    }
    else
        next = NULL;
}

void Mesh::computeSize()
{
    size = size2 = 0.0f;
    foreach(Vec v, vertex)
        size2 = qMax(size2, v.sq());
    size = sqrtf(size2);
    if (child)
    {
        child->computeSize();
        for( Mesh *cur = child ; cur != NULL ; cur = cur->next )
            size = qMax(size, cur->size + cur->pos.norm());
        size2 = size * size;
    }
    if (next)
        next->computeSize();
}

void Mesh::computeInfo()
{
    computeSize();
    computeID();
}

bool Mesh::isEmpty()
{
    return child == NULL && next == NULL && vertex.isEmpty();
}

int Mesh::hit(const Vec &pos, const Vec &dir, Vec &p)
{
    Vec rPos = pos - this->pos;
    int hitId = -1;
    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        // It's slow but it's only for mesh manipulation
        for(int i = 0 ; i < index.size() ; i++)
        {
            Vec &a = vertex[ index[i] ];
            Vec &b = vertex[ index[i+1] ];
            Vec &c = vertex[ index[i+2] ];
            Vec q;
            if (hitTriangle(a, b, c, rPos, dir, q))
            {
                if (hitId < 0 || dir * p > dir * q)
                    p = q;
                hitId = ID;
            }
        }
        break;
    case MESH_TRIANGLES:
    default:
        // It's slow but it's only for mesh manipulation
        for(int i = 0 ; i < index.size() ; i += 3)
        {
            Vec &a = vertex[ index[i] ];
            Vec &b = vertex[ index[i+1] ];
            Vec &c = vertex[ index[i+2] ];
            Vec q;
            if (hitTriangle(a, b, c, rPos, dir, q))
            {
                if (hitId < 0 || dir * p > dir * q)
                    p = q;
                hitId = ID;
            }
        }
        break;
    };

    if (child)
    {
        Vec q;
        int childId = -1;
        if ((childId = child->hit(rPos, dir, q)) >= 0)
        {
            if (hitId < 0 || dir * p > dir * q)
            {
                p = q;
                hitId = childId;
            }
        }
    }
    if (hitId >= 0)
        p += this->pos;
    if (next)
    {
        Vec q;
        int nextId = -1;
        if ((nextId = next->hit(pos, dir, q)) >= 0)
        {
            if (hitId < 0 || dir * p > dir * q)
            {
                p = q;
                hitId = nextId;
            }
        }
    }
    return hitId;
}

bool Mesh::hitTriangle(const Vec &a, const Vec &b, const Vec &c, const Vec &pos, const Vec &dir, Vec &p)
{
    Vec mid = (1.0f / 3.0f) * (a + b + c);
    float md = qMax((mid-a).sq(), qMax((mid-b).sq(), (mid-c).sq()));
    Vec OM = mid - pos;
    float l = (dir ^ OM).sq();
    float OMsq = OM.sq();
    if (l * (OMsq + l) <= md * OMsq)      // Spherical intersection ?
    {
        Vec n = (b-a) ^ (c-a);
        if ((dir * n) * ((a - pos) * n) > 0.0f)     // Are we aiming in the right direction ?
        {
            p = pos + (((a - pos) * n) / (dir * n)) * dir;      // Intersection with triangle space
            // WARNING: critical function, don't hesitate to test/improve
            if (((p-a) ^ (b-a)) * ((c-a) ^ (b-a)) < 0.0f)       // First half space ?
                return false;
            if (((p-b) ^ (c-b)) * ((a-b) ^ (c-b)) < 0.0f)       // Second half space ?
                return false;
            if (((p-c) ^ (a-c)) * ((b-c) ^ (a-c)) < 0.0f)       // Third ?
                return false;
            return true;
        }
        return false;
    }
    return false;
}

sint32 Mesh::computeID(sint32 id)
{
    ID = id++;
    if (child)
        id = child->computeID(id);
    if (next)
        id = next->computeID(id);
    nbSubObj = id - ID;
    return id;
}

Mesh *Mesh::getMesh(int id)
{
    if (id == ID)
        return this;
    if (((next && next->getID() > id) || next == NULL) && child)
        return child->getMesh(id);
    if (next)
        return next->getMesh(id);
    return NULL;
}

Mesh *Mesh::getMesh(const QString &name)
{
    if (QString::compare(this->name, name, Qt::CaseInsensitive) == 0)
        return this;
    Mesh *mesh = NULL;
    if (child)
        mesh = child->getMesh(name);
    if (!mesh && next)
        mesh = next->getMesh(name);
    return mesh;
}

Mesh *Mesh::getMeshByScriptID(int id)
{
    if (id == scriptID)
        return this;
    Mesh *mesh = NULL;
    if (child)
        mesh = child->getMeshByScriptID(id);
    if (!mesh && next)
        mesh = next->getMeshByScriptID(id);
    return mesh;
}

int Mesh::getDepth(int id)
{
    if (id == ID)
        return 0;
    if (((next && next->getID() > id) || next == NULL) && child)
        return 1 + child->getDepth(id);
    if (next)
        return next->getDepth(id);
    return -1;
}

int Mesh::getParent(int id)
{
    if (id == ID)
        return -1;
    if (((next && next->getID() > id) || next == NULL) && child)
    {
        int childParent = child->getParent(id);
        if (childParent == -1)
            return ID;
        return childParent;
    }
    if (next)
        return next->getParent(id);
    return -2;
}

Vec Mesh::getRelativePosition(int id)
{
    if (id == ID)
        return pos;
    if (((next && next->getID() > id) || next == NULL) && child)
        return child->getRelativePosition(id) + pos;
    if (next)
        return next->getRelativePosition(id);
    return Vec();
}

sint32 Mesh::nbSubObjects()
{
    return nbSubObj;
}

Mesh *Mesh::createCube(float size)
{
    size *= 0.5f;
    Mesh *mesh = new Mesh;
    mesh->vertex.reserve(8);
    mesh->index.reserve(24);
    mesh->color = 0xFFFFFFFF;
    mesh->type = MESH_TRIANGLES;
    mesh->name = QString("cube(%1)").arg(size * 2.0f);

    mesh->vertex.push_back(Vec(-size, size, size));
    mesh->vertex.push_back(Vec(size, size, size));
    mesh->vertex.push_back(Vec(size, size, -size));
    mesh->vertex.push_back(Vec(-size, size, -size));
    mesh->vertex.push_back(Vec(-size, -size, size));
    mesh->vertex.push_back(Vec(size, -size, size));
    mesh->vertex.push_back(Vec(size, -size, -size));
    mesh->vertex.push_back(Vec(-size, -size, -size));

    mesh->index.push_back(0);       // Top
    mesh->index.push_back(1);
    mesh->index.push_back(2);
    mesh->index.push_back(0);
    mesh->index.push_back(2);
    mesh->index.push_back(3);

    mesh->index.push_back(4);       // Bottom
    mesh->index.push_back(7);
    mesh->index.push_back(6);
    mesh->index.push_back(5);
    mesh->index.push_back(4);
    mesh->index.push_back(6);

    mesh->index.push_back(0);       // Back
    mesh->index.push_back(4);
    mesh->index.push_back(1);
    mesh->index.push_back(1);
    mesh->index.push_back(4);
    mesh->index.push_back(5);

    mesh->index.push_back(2);       // Front
    mesh->index.push_back(6);
    mesh->index.push_back(3);
    mesh->index.push_back(3);
    mesh->index.push_back(6);
    mesh->index.push_back(7);

    mesh->index.push_back(0);       // Left
    mesh->index.push_back(3);
    mesh->index.push_back(4);
    mesh->index.push_back(3);
    mesh->index.push_back(7);
    mesh->index.push_back(4);

    mesh->index.push_back(1);       // Right
    mesh->index.push_back(5);
    mesh->index.push_back(2);
    mesh->index.push_back(2);
    mesh->index.push_back(5);
    mesh->index.push_back(6);

    mesh->computeNormals();

    return mesh;
}

Mesh *Mesh::createSphere(float r, int dw, int dh)
{
    Mesh *mesh = new Mesh;
    mesh->color = 0xFFFFFFFF;
    mesh->type = MESH_TRIANGLES;
    mesh->name = QString("sphere(%1)").arg(r);

    for(int i = 0 ; i <= dh ; i++)
    {
        for(int e = 0 ; e <= dw ; e++)
        {
            Vec p(  cosf( M_PI * 2.0f * e / dw ) * cosf( M_PI * i / dh - 0.5f * M_PI),
                    sinf( M_PI * i / dh - 0.5f * M_PI),
                    sinf( M_PI * 2.0f * e / dw ) * cosf( M_PI * i / dh - 0.5f * M_PI));
            mesh->vertex.push_back(r * p);
            mesh->normal.push_back(p);
            mesh->tcoord.push_back(((float)e) / dw);
            mesh->tcoord.push_back(((float)i) / dh);
        }
    }

    for(int i = 0 ; i < dh ; i++)
    {
        for(int e = 0 ; e < dw ; e++)
        {
            mesh->index.push_back(i * (dw + 1) + e);
            mesh->index.push_back((i + 1) * (dw + 1) + e);
            mesh->index.push_back(i * (dw + 1) + e + 1);

            mesh->index.push_back((i + 1) * (dw + 1) + e);
            mesh->index.push_back((i + 1) * (dw + 1) + e + 1);
            mesh->index.push_back(i * (dw + 1) + e + 1);
        }
    }

    return mesh;
}

Mesh *Mesh::createCylinder(float r, float h, int d, bool capped)
{
    h *= 0.5f;
    Mesh *mesh = new Mesh;
    mesh->color = 0xFFFFFFFF;
    mesh->type = MESH_TRIANGLES;
    mesh->name = QString("cylinder(%1)").arg(r);

    for(int e = 0 ; e <= d ; e++)
    {
        Vec p(  cosf( M_PI * 2.0f * e / d ),
                h / r,
                sinf( M_PI * 2.0f * e / d ));
        mesh->vertex.push_back(r * p);
        p.y = 0.0f;
        mesh->normal.push_back(p);
        mesh->tcoord.push_back( ((float)e) / d );
        mesh->tcoord.push_back( 0.0f );
    }
    for(int e = 0 ; e <= d ; e++)
    {
        Vec p(  cosf( M_PI * 2.0f * e / d ),
                -h / r,
                sinf( M_PI * 2.0f * e / d ));
        mesh->vertex.push_back(r * p);
        p.y = 0.0f;
        mesh->normal.push_back(p);
        mesh->tcoord.push_back( ((float)e) / d );
        mesh->tcoord.push_back( 1.0f );
    }

    for(int i = 0 ; i < d ; i++)
    {
        mesh->index.push_back(i);
        mesh->index.push_back(i + 1);
        mesh->index.push_back(i + d + 1);

        mesh->index.push_back(i + d + 1);
        mesh->index.push_back(i + 1);
        mesh->index.push_back(i + 1 + d + 1);
    }

    if (capped)
    {
        mesh->vertex.push_back(Vec(0,h,0));
        mesh->normal.push_back(Vec(0,1,0));
        mesh->tcoord.push_back( 0.0f );
        mesh->tcoord.push_back( 0.0f );

        mesh->vertex.push_back(Vec(0,-h,0));
        mesh->normal.push_back(Vec(0,-1,0));
        mesh->tcoord.push_back( 0.0f );
        mesh->tcoord.push_back( 1.0f );

        for(int i = 0 ; i < d ; i++)
        {
            mesh->index.push_back(2 * (d + 1));
            mesh->index.push_back(i + 1);
            mesh->index.push_back(i);
        }
        for(int i = 0 ; i < d ; i++)
        {
            mesh->index.push_back(2 * (d + 1) + 1);
            mesh->index.push_back(i + d + 1);
            mesh->index.push_back(i + 1 + d + 1);
        }
    }

    return mesh;
}

QList<Mesh*> Mesh::getSubList()
{
    QList<Mesh*> lMesh;
    lMesh.push_back(this);
    if (child)
        lMesh.append( child->getSubList() );
    if (next)
        lMesh.append( next->getSubList() );
    return lMesh;
}

Mesh *Mesh::createCone(float r, float h, int d, bool capped)
{
    h *= 0.5f;
    Mesh *mesh = new Mesh;
    mesh->color = 0xFFFFFFFF;
    mesh->type = MESH_TRIANGLES;
    mesh->name = QString("cone(%1)").arg(r);

    mesh->vertex.push_back(Vec(0,h,0));
    mesh->normal.push_back(Vec(0,1,0));
    mesh->tcoord.push_back( 0.0f );
    mesh->tcoord.push_back( 0.0f );
    for(int e = 0 ; e <= d ; e++)
    {
        Vec p(  cosf( M_PI * 2.0f * e / d ),
                -h / r,
                sinf( M_PI * 2.0f * e / d ));
        mesh->vertex.push_back(r * p);
        p.y = 0.0f;
        mesh->normal.push_back(p);
        mesh->tcoord.push_back( ((float)e) / d );
        mesh->tcoord.push_back( 1.0f );
    }

    for(int i = 0 ; i < d ; i++)
    {
        mesh->index.push_back(0);
        mesh->index.push_back(i + 1 + 1);
        mesh->index.push_back(i + 1);
    }

    if (capped)
    {
        mesh->vertex.push_back(Vec(0,-h,0));
        mesh->normal.push_back(Vec(0,-1,0));
        mesh->tcoord.push_back( 0.0f );
        mesh->tcoord.push_back( 1.0f );

        for(int i = 0 ; i < d ; i++)
        {
            mesh->index.push_back(d + 1 + 1);
            mesh->index.push_back(i + 1);
            mesh->index.push_back(i + 1 + 1);
        }
    }

    return mesh;
}

Mesh *Mesh::createTorus(float R, float r, int D, int d)
{
    Mesh *mesh = new Mesh;
    mesh->color = 0xFFFFFFFF;
    mesh->type = MESH_TRIANGLES;
    mesh->name = QString("cone(%1)").arg(R);

    for(int i = 0 ; i <= d ; i++)
    {
        for(int e = 0 ; e <= D ; e++)
        {
            Vec p(  cosf( M_PI * 2.0f * e / D ) * (R + r * cosf( M_PI * 2.0f * i / d - M_PI)),
                    sinf( M_PI * 2.0f * i / d - M_PI) * r,
                    sinf( M_PI * 2.0f * e / D ) * (R + r * cosf( M_PI * 2.0f * i / d - M_PI)));
            mesh->vertex.push_back(p);
            Vec n(  cosf( M_PI * 2.0f * e / D ) * cosf( M_PI * 2.0f * i / d - M_PI),
                    sinf( M_PI * 2.0f * i / d - M_PI),
                    sinf( M_PI * 2.0f * e / D ) * cosf( M_PI * 2.0f * i / d - M_PI));
            mesh->normal.push_back(n);
            mesh->tcoord.push_back(((float)e) / D);
            mesh->tcoord.push_back(((float)i) / d);
        }
    }

    for(int i = 0 ; i < d ; i++)
    {
        for(int e = 0 ; e < D ; e++)
        {
            mesh->index.push_back(i * (D + 1) + e);
            mesh->index.push_back((i + 1) * (D + 1) + e);
            mesh->index.push_back(i * (D + 1) + e + 1);

            mesh->index.push_back((i + 1) * (D + 1) + e);
            mesh->index.push_back((i + 1) * (D + 1) + e + 1);
            mesh->index.push_back(i * (D + 1) + e + 1);
        }
    }

    return mesh;
}

Mesh *Mesh::createEmpty()
{
	Mesh *mesh = new Mesh;
	mesh->color = 0xFFFFFFFF;
	mesh->type = MESH_TRIANGLES;
	mesh->name = QString("empty");

	return mesh;
}

void Mesh::deleteMesh(int id)
{
    if (id == ID)       // delete the root mesh
    {
        if (child)
        {
            delete child;
            child = NULL;
        }
        if (next)
        {
            Mesh *mesh = next;
            copy(mesh);
            mesh->clear();
            delete mesh;
        }
        else
            destroy();
        return;
    }
    else
    {
        if (child && child->ID == id)
        {
            Mesh *mesh = child;
            child = child->next;
            mesh->next = NULL;
            delete mesh;
            return;
        }
        if (next && next->ID == id)
        {
            Mesh *mesh = next;
            next = next->next;
            mesh->next = NULL;
            delete mesh;
            return;
        }
    }
    if (child)
        child->deleteMesh(id);
    if (next)
        next->deleteMesh(id);
}

void Mesh::copy(const Mesh *src)
{
    name = src->name;
    pos = src->pos;
    child = src->child;
    next = src->next;
    vertex = src->vertex;
    normal = src->normal;
    index = src->index;
    tex = src->tex;
    tcoord = src->tcoord;
    type = src->type;
    flag = src->type;
    color = src->color;
    rColor = src->rColor;
    size = src->size;
    size2 = src->size2;
    shader = src->shader;
    fragmentProgram = src->fragmentProgram;
    vertexProgram = src->vertexProgram;
    ID = src->ID;
    nbSubObj = src->nbSubObj;
}

void Mesh::clear()
{
    next = child = NULL;

    vertex.clear();
    normal.clear();
    tcoord.clear();
    index.clear();
    tex.clear();
    flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
    color = rColor = 0xFFFFFFFF;
    name.clear();
    ID = 0;

    emit loaded();
}

void Mesh::save3DMrec(QFile &file)
{
    file.putChar(name.size());
    file.write(name.toAscii().data(), name.size());
    file.write((char*)&pos, sizeof(pos));

    sint16 nb_vtx = vertex.size();
    file.write((char*)&nb_vtx, sizeof(nb_vtx));
    if (nb_vtx > 0)
        file.write((char*)vertex.data(), sizeof(Vec) * nb_vtx);

    GLushort sel[4];
    file.write((char*)sel, sizeof(GLushort) * 4);

    sint16 nb_p_idx = 0;
    file.write((char*)&nb_p_idx, sizeof(nb_p_idx)); // Write point data
//    if (nb_p_idx > 0)
//        file.write(sizeof(GLushort) * nb_p_idx);

    sint16 nb_l_idx = 0;
    file.write((char*)&nb_l_idx, sizeof(nb_l_idx));	// Write line data
//    if (nb_l_idx > 0)
//        file.write(sizeof(GLushort) * nb_l_idx);

    sint16 nb_idx = index.size();
    file.write((char*)&nb_idx, sizeof(nb_idx)); // Write triangle data
    for(int i = 0 ; i < nb_idx ; i++)
    {
        GLushort id = index[i];
        file.write((char*)&id, sizeof(GLushort));
    }

    if (tcoord.size() < nb_vtx * 2)
        tcoord.resize(nb_vtx << 1);
    file.write((char*)tcoord.data(), sizeof(float) * nb_vtx << 1);

    float colorf[4] = { (color >> 24) / 255.0f,
                        ((color >> 16) & 0xFF) / 255.0f,
                        ((color >> 8) & 0xFF) / 255.0f,
                        (color & 0xFF) / 255.0f };
    float rColorf[4] = { (rColor >> 24) / 255.0f,
                         ((rColor >> 16) & 0xFF) / 255.0f,
                         ((rColor >> 8) & 0xFF) / 255.0f,
                         (rColor & 0xFF) / 255.0f };

    file.write((char*)colorf, sizeof(float) * 4);	// Read surface data
    file.write((char*)rColorf, sizeof(float) * 4);
    flag |= SURFACE_ADVANCED;
    file.write((char*)&flag, sizeof(flag));
    sint8 nb_tex = -tex.size();                 // We can safely compress texture data since it uses lossless compression (zlib)
    file.write((char*)&nb_tex, sizeof(nb_tex));
    for (int i = 0 ; i < tex.size() ; ++i)
    {
        QImage img = Gfx::instance()->textureToImage(tex[i]).rgbSwapped();
        uint8 bpp = 32;
        int w = img.width(), h = img.height();

        file.write((char*)&w, sizeof(w));
        file.write((char*)&h, sizeof(h));
        file.write((char*)&bpp, sizeof(bpp));

        int buf_size = w * h * 5;
        byte *buffer = new byte[buf_size];
        int img_size = buf_size;
        uLongf __size = img_size;
        compress2 ( buffer, &__size, (Bytef*) img.bits(), w * h * bpp / 8, 9);
        img_size = __size;

        file.write((char*)&img_size, sizeof(img_size)); // Save the result
        file.write((char*)buffer, img_size);
        delete[] buffer;
    }

    if (flag & SURFACE_GLSL) // Fragment & Vertex shaders
    {
        QByteArray buf = vertexProgram.toAscii();
        uint32 shader_size = buf.size();
        file.write((char*)&shader_size, 4);
        file.write(buf.data(), shader_size);

        buf = fragmentProgram.toAscii();
        shader_size = buf.size();
        file.write((char*)&shader_size, 4);
        file.write(buf.data(), shader_size);
    }

    if ((defaultAnim.type & (ROTATION | TRANSLATION)) != 0) // Save animation data if present (currently not supported)
    {
        file.putChar(2);

        ANIMATION *animation_data = &defaultAnim;
        file.write( (char*)&(animation_data->type), 1 );
        file.write( (char*)&(animation_data->angle_0), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->angle_1), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->angle_w), sizeof(float)  );
        file.write( (char*)&(animation_data->translate_0), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->translate_1), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->translate_w), sizeof(float) );
    }

    byte link = (child != NULL) ? 1 : 0;
    file.write((char*)&link, 1);
    if (link)
        child->save3DMrec(file);

    link = (next != NULL) ? 1 : 0;
    file.write((char*)&link, 1);
    if (link)
		next->save3DMrec(file);
}

void Mesh::invertOrientation()
{
    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        for(int i = 0 ; i + 1 < index.size() ; i += 2)
        {
            index[i] ^= index[i + 1];
            index[i + 1] ^= index[i];
            index[i] ^= index[i + 1];
        }
        break;
    case MESH_TRIANGLES:
    default:
        for(int i = 0 ; i < index.size() ; i += 3)
        {
            index[i + 1] ^= index[i + 2];
            index[i + 2] ^= index[i + 1];
            index[i + 1] ^= index[i + 2];
        }
        break;
    };
}

void Mesh::autoComputeUVcoordinates()
{
	// Convert current mesh structure to a simple triangle soup
    toTriangleSoup();
    mergeSimilarVertices();

	// Compute the face set and neighborhood relations between faces
	QHash< GLuint, QHash< GLuint, QList<int> > >	edges;
	QSet< int >	faces;

	for(int i = 0 ; i < index.size() ; i += 3)
	{
		GLuint a = index[i];
		GLuint b = index[i + 1];
		GLuint c = index[i + 2];
		edges[a][b].push_back(i);
		edges[b][c].push_back(i);
		edges[c][a].push_back(i);
		edges[b][a].push_back(i);
		edges[c][b].push_back(i);
		edges[a][c].push_back(i);
		faces << i;
	}

	// Greedy algorithm to compute triangle strippes
	QList< QList< GLuint > > stripes;
	while(!faces.isEmpty())
	{
		int cur = *faces.begin();
		GLuint a = index[cur];
		GLuint b = index[cur + 1];
		GLuint c = index[cur + 2];

		QList< GLuint > strip;
		strip << a;
		strip << b;
		bool found = false;
		do
		{
			faces.remove(cur);
			strip << c;

			found = false;
			QList<int> &nlist = edges[b][c];
			for(QList<int>::iterator i = nlist.begin() ; i != nlist.end() && !found ; ++i)
			{
				if (faces.contains(*i))
				{
					found = true;
					cur = *i;
					a = b;
					b = c;
					if ((index[cur] == a && index[cur + 1] == b)
						|| (index[cur] == b && index[cur + 1] == a))
						c = index[cur + 2];
					else if ((index[cur] == a && index[cur + 2] == b)
						|| (index[cur] == b && index[cur + 2] == a))
						c = index[cur + 1];
					else
						c = index[cur];
				}
			}

		} while(found);
		stripes << strip;
	}

	QVector<Vec> nvertex;
	index.clear();
	float n = 0.0f;
	float mx = 0.0f;
	tcoord.clear();

	// Compute maximum strip length
	for(QList< QList< GLuint > >::iterator strip = stripes.begin() ; strip != stripes.end() ; ++strip)
		mx = qMax(mx, float((strip->size() - 1) / 2));

	QList< QPair< int, int > > freePlaces;

	for(QList< QList< GLuint > >::iterator strip = stripes.begin() ; strip != stripes.end() ; ++strip)
	{
		int len = (strip->size() - 1) / 2;
		int start = 0;
		int curn = n;

		if (freePlaces.isEmpty())
		{
			start = 0;
			curn = n;
			++n;
		}
		else
		{
			bool found = false;
			for(int i = 0 ; i < freePlaces.size() && !found ; ++i)
			{
				QPair<int, int> &item = freePlaces[i];
				if (item.second + len <= mx)
				{
					start = item.second;
					curn = item.first;
					freePlaces.removeAt(i);
					found = true;
				}
			}

			if (!found)
				++n;
		}

		if (start + len < mx)
			freePlaces << QPair<int, int>(curn, start + len);

		nvertex << vertex[(*strip)[0]];
		nvertex << vertex[(*strip)[1]];
		tcoord << start;
		tcoord << curn + 0.9f;
		tcoord << start;
		tcoord << curn;
		for(int i = 2 ; i < strip->size() ; ++i)
		{
			bool reverse = (i % 2) == 1;
			if (reverse)
			{
				index << nvertex.size() - 1;
				index << nvertex.size() - 2;
			}
			else
			{
				index << nvertex.size() - 2;
				index << nvertex.size() - 1;
			}
			index << nvertex.size();
			tcoord << start + (i / 2);
			tcoord << (reverse ? curn : curn + 0.9f);
			nvertex << vertex[(*strip)[i]];
		}
	}

	n = 1.0f / n;
	mx = 1.0f / mx;
	for(int i = 0 ; i < tcoord.size() ; i += 2)
	{
		tcoord[i] *= mx;
		tcoord[i + 1] *= n;
	}

	normal.clear();
	vertex = nvertex;

    computeNormals();
}

void Mesh::sphericalMapping()
{
    // Transform the mesh into a triangle soup
    // then merge duplicate vertices
    toTriangleSoup();           // These functions also compute normals
    mergeSimilarVertices();     // so there is no need to do it again

    tcoord.resize(normal.size() * 2);
    Vec I(1,0,0), J(0,0,1), K(0,1,0);
    for(int i = 0 ; i < normal.size() ; i++)
    {
        Vec n = normal[i];
        Vec p = Vec(n.x, 0.0f, n.z);
        if (p.sq() == 0.0f)
            p.x = 1.0f;
        else
            p.unit();
        float v = fabsf(VAngle(n, p));
        if (n.y < 0.0f)
            v = -v;
        v = v / M_PI + 0.5f;
        float u = VAngle(p, I);
        if (p * J < 0.0f)
            u = -u;
        u = 0.5f * u / M_PI + 0.5f;
        tcoord[i * 2] = u;
        tcoord[i * 2 + 1] = v;
    }

    // Remember we have type == MESH_TRIANGLES here :)
    // so we can easily check for inconsistencies
    for(int i = 0 ; i < index.size() ; i += 3)
    {
        Vec A = normal[index[i]];
        Vec B = normal[index[i + 1]];
        Vec C = normal[index[i + 2]];
        A.y = B.y = C.y = 0.0f;
        A.unit();
        B.unit();
        C.unit();
        bool dB = ( (A ^ B) * K ) * (tcoord[index[i] * 2] - tcoord[index[i + 1] * 2]) < 0.0f;
        bool dC = ( (A ^ C) * K ) * (tcoord[index[i] * 2] - tcoord[index[i + 2] * 2]) < 0.0f;
        if (dB)
        {
            vertex.push_back( vertex[index[i+1]] );
            normal.push_back( normal[index[i+1]] );
            if (tcoord[index[i+1] * 2] < tcoord[index[i] * 2])
                tcoord.push_back( tcoord[index[i+1] * 2] + 1.0f );
            else
                tcoord.push_back( tcoord[index[i+1] * 2] - 1.0f );
            tcoord.push_back( tcoord[index[i+1] * 2 + 1] );
            index[i+1] = vertex.size() - 1;
        }
        if (dC)
        {
            vertex.push_back( vertex[index[i+2]] );
            normal.push_back( normal[index[i+2]] );
            if (tcoord[index[i+2] * 2] < tcoord[index[i] * 2])
                tcoord.push_back( tcoord[index[i+2] * 2] + 1.0f );
            else
                tcoord.push_back( tcoord[index[i+2] * 2] - 1.0f );
            tcoord.push_back( tcoord[index[i+2] * 2 + 1] );
            index[i+2] = vertex.size() - 1;
        }
        else
        {
            dC = ( (C ^ B) * K ) * (tcoord[index[i + 2] * 2] - tcoord[index[i + 1] * 2]) < 0.0f;
            if (dC)
            {
                vertex.push_back( vertex[index[i+2]] );
                normal.push_back( normal[index[i+2]] );
                if (tcoord[index[i+2] * 2] < tcoord[index[i+1] * 2])
                    tcoord.push_back( tcoord[index[i+2] * 2] + 1.0f );
                else
                    tcoord.push_back( tcoord[index[i+2] * 2] - 1.0f );
                tcoord.push_back( tcoord[index[i+2] * 2 + 1] );
                index[i+2] = vertex.size() - 1;
            }
        }
    }
    float umin = 2.0f;
    float umax = -1.0f;
    for(int i = 0 ; i < tcoord.size() ; i += 2)
    {
        umin = qMin(umin, tcoord[i]);
        umax = qMax(umax, tcoord[i]);
    }
    umax -= umin;
    if (umax == 0.0f)
        umax = 1.0f;
    for(int i = 0 ; i < tcoord.size() ; i += 2)
        tcoord[i] = (tcoord[i] - umin) / umax;
    computeNormals();
}

void Mesh::basicMapping()
{
    QVector<Vec> nVertex;
    QVector<GLuint> nIndex;
    tcoord.clear();

    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        for(int i = 0 ; i + 2 < index.size() ; i++)
        {
            nIndex.push_back(i);
            nVertex.push_back( vertex[ index[i] ] );
            nIndex.push_back(i + 1);
            nVertex.push_back( vertex[ index[i + 1] ] );
            nIndex.push_back(i + 2);
            nVertex.push_back( vertex[ index[i + 2] ] );
        }
        break;
    case MESH_TRIANGLES:
    default:
        for(int i = 0 ; i < index.size() ; i++)
        {
            nIndex.push_back(i);
            nVertex.push_back( vertex[ index[i] ] );
        }
    };
    int nb = ((nIndex.size() / 3) + 1) / 2;
    int w = (int)(sqrtf(nb));
    int h = nb / w;
    if (h * w < nb)
        h++;

    tcoord.resize(2 * nVertex.size());
    float delta = 0.02f;
    float delta2 = 2 * delta;
    for(int i = 0 ; i < tcoord.size() ; i += 12)
    {
        int e = i / 12;
        tcoord[i  ] = ((float)(e % w)) / w + delta2 / w;
        tcoord[i+1] = ((float)(e / w)) / h + delta / h;
        tcoord[i+2] = ((float)(e % w)+1) / w - delta / w;
        tcoord[i+3] = ((float)(e / w)) / h + delta / h;
        tcoord[i+4] = ((float)(e % w)+1) / w - delta / w;
        tcoord[i+5] = ((float)(e / w) + 1) / h - delta2 / h;

        if (i + 6 >= tcoord.size())
            break;

        tcoord[i+6] = ((float)(e % w)) / w + delta / w;
        tcoord[i+7] = ((float)(e / w)) / h + delta2 / h;
        tcoord[i+8] = ((float)(e % w)) / w + delta / w;
        tcoord[i+9] = ((float)(e / w) + 1) / h - delta / h;
        tcoord[i+10] = ((float)(e % w)+1) / w - delta2 / w;
        tcoord[i+11] = ((float)(e / w) + 1) / h - delta / h;
    }
    index = nIndex;
    vertex = nVertex;
    type = MESH_TRIANGLES;
    computeNormals();
}

void Mesh::mergeSimilarVertices()
{
    QVector<Vec> nVertex;
    QVector<GLfloat> nTcoord;
    for(int i = 0 ; i < index.size() ; i++)
    {
        Vec &p = vertex[index[i]];
        int e = -1;
        for(int j = 0 ; j < nVertex.size() ; j++)
        {
			if (nVertex[j] == p
				&& (tcoord.empty()
					|| (tcoord[index[i] * 2] == nTcoord[j * 2]
						&& tcoord[index[i] * 2 + 1] == nTcoord[j * 2 + 1])))
            {
                e = j;
                break;
            }
        }
        if (e >= 0)
            index[i] = e;
        else
        {
            if (!tcoord.isEmpty())
            {
                nTcoord.push_back(tcoord[index[i] * 2]);
                nTcoord.push_back(tcoord[index[i] * 2 + 1]);
            }
            index[i] = nVertex.size();
            nVertex.push_back(p);
        }
    }

    vertex = nVertex;
    normal.clear();
    tcoord = nTcoord;

    computeNormals();
}

void Mesh::toTriangleSoup()     // This will result in a flat shaded mesh (vertices will be duplicated)
{
    QVector<Vec> nVertex;
    QVector<GLuint> nIndex;
    QVector<GLfloat> nTcoord;

    switch(type)
    {
    case MESH_TRIANGLE_STRIP:
        for(int i = 0 ; i + 2 < index.size() ; i++)
        {
            nIndex.push_back(i);
            nVertex.push_back( vertex[ index[i] ] );
			if (!tcoord.isEmpty())
			{
				nTcoord.push_back( tcoord[ index[i] * 2 ] );
				nTcoord.push_back( tcoord[ index[i] * 2 + 1 ] );
			}

            nIndex.push_back(i + 1);
            nVertex.push_back( vertex[ index[i + 1] ] );
			if (!tcoord.isEmpty())
			{
				nTcoord.push_back( tcoord[ index[i + 1] * 2 ] );
				nTcoord.push_back( tcoord[ index[i + 1] * 2 + 1 ] );
			}

            nIndex.push_back(i + 2);
            nVertex.push_back( vertex[ index[i + 2] ] );
			if (!tcoord.isEmpty())
			{
				nTcoord.push_back( tcoord[ index[i + 2] * 2 ] );
				nTcoord.push_back( tcoord[ index[i + 2] * 2 + 1 ] );
			}
		}
        break;
    case MESH_TRIANGLES:
    default:
        for(int i = 0 ; i < index.size() ; i++)
        {
            nIndex.push_back(i);
            nVertex.push_back( vertex[ index[i] ] );
			if (!tcoord.isEmpty())
			{
				nTcoord.push_back( tcoord[ index[i] * 2 ] );
				nTcoord.push_back( tcoord[ index[i] * 2 + 1 ] );
			}
        }
    };
    index = nIndex;
    vertex = nVertex;
    tcoord = nTcoord;
    type = MESH_TRIANGLES;
    computeNormals();
}

void Mesh::splitGeometry()
{
    toTriangleSoup();
    mergeSimilarVertices();

    int nb_component = 0;
    QVector< QVector< int > > neighbors;
    neighbors.resize(vertex.size());
    for(int i = 0 ; i < index.size() ; i += 3)      // Build neighborhood tables
    {
        for(int e = 0 ; e < 3 ; e++)
        {
            if (!neighbors[index[i + e]].contains(index[i + ((1 + e) % 3)]))
                neighbors[index[i + e]].push_back(index[i + ((1 + e) % 3)]);
            if (!neighbors[index[i + e]].contains(index[i + ((2 + e) % 3)]))
                neighbors[index[i + e]].push_back(index[i + ((2 + e) % 3)]);
        }
    }
    for(int i = 0 ; i < neighbors.size() ; i++)
        qSort( neighbors[i] );

    QVector< QVector< int > > componentVertex;
    QVector< int > component;
    QVector< int > dist;
    for(int i = 0 ; i < vertex.size() ; i++)
    {
        component.push_back(-1);
        dist.push_back(vertex.size());
    }

    QQueue< int > qWork;
    for(int i = 0 ; i < vertex.size() ; i++)
        qWork.enqueue(i);
    for(int i = 0 ; i < vertex.size() ; i++)
        qSwap( qWork[i], qWork[ qrand() % qWork.size() ] );

    while(!qWork.isEmpty())         // Compute connex components
    {
        int A = qWork.dequeue();
        int B = -1, C = -1;
        for(int i = 0 ; i < neighbors[A].size() && C == -1 ; i++)
        {
            int n = neighbors[A][i];
            if (component[n] == -1)
            {
                B = n;
                for(int e = i + 1 ; e < neighbors[A].size() && C == -1 ; e++)
                {
                    n = neighbors[A][e];
                    if (component[n] == -1 && neighbors[B].contains(n))
                        C = n;
                }
            }
        }
        if (C == -1)                // No unattributed triangle OO! This is not possible, but for robustness ...
        {
            qDebug() << "error : no triangle found! ( " << __FILE__ << " l." << __LINE__ << ")";
            continue;
        }
        int cmp = nb_component++;
        component[A] = component[B] = component[C] = cmp;
        // B and C have been chosen so we must not use them again
        qWork.removeOne(B);
        qWork.removeOne(C);
        dist[A] = 0;                // Compute distance from component initializer
        dist[B] = dist[C] = 1;
        componentVertex.push_back(QVector<int>());
        componentVertex.last() << A << B << C;

        QQueue<int> qComponent;
        qComponent << neighbors[A].toList() << neighbors[B].toList() << neighbors[C].toList();
        while(!qComponent.isEmpty())
        {
            int i = qComponent.dequeue();
            if (component[i] == cmp)        // Already in
                continue;

            A = i;
            B = -1;
            C = -1;
            for(int j = 0 ; j < neighbors[A].size() && C == -1 ; j++)
            {
                int n = neighbors[A][j];
                if (component[n] == cmp)
                {
                    B = n;
                    for(int e = j + 1 ; e < neighbors[A].size() && C == -1 ; e++)
                    {
                        n = neighbors[A][e];
                        if (component[n] == cmp && neighbors[B].contains(n))
                            C = n;
                    }
                }
            }
            if (C == -1)            // It's not in
                continue;
            componentVertex.last() << A;

            if (component[i] != -1)         // Special case, the vertex must be duplicated
            {                               // (2 components joined by a vertex :/)
                int orig = component[i];
                for(int e = 0 ; e < index.size() ; e += 3)
                {
                    if (index[e] == (uint32)A || index[e+1] == (uint32)A || index[e+2] == (uint32)A)
                    {
                        if (component[index[e]] == orig && component[index[e+1]] == orig && component[index[e+2]] == orig)
                        {
                            if (index[e] == (uint32)A)
                                index[e] = vertex.size();
                            if (index[e+1] == (uint32)A)
                                index[e+1] = vertex.size();
                            if (index[e+2] == (uint32)A)
                                index[e+2] = vertex.size();
                        }
                    }
                }
                dist.push_back(dist[A]);
                dist[A] = vertex.size();
                component.push_back(orig);
                vertex.push_back(vertex[i]);
                neighbors.push_back(QVector<int>());
                for(int e = 0 ; e < neighbors[i].size() ; e++)
                    if (component[neighbors[i][e]] == orig)
                    {
                        neighbors.last().push_back(neighbors[i][e]);
                        neighbors[i].remove(e--);
                    }
            }
            for(int j = 0 ; j < neighbors[A].size() ; j++)
                dist[A] = qMin(dist[A], dist[ neighbors[A][j] ] + 1);
            component[i] = cmp;
            qComponent << neighbors[i].toList();
        }
    }

    QVector<int> convert;
    convert.resize(vertex.size());
    for(int i = 0 ; i < nb_component ; i++)
        for(int e = 0 ; e < componentVertex[i].size() ; e++)
            convert[componentVertex[i][e]] = e;

    for(int i = 1 ; i < nb_component ; i++)
    {
        Mesh *mesh = new Mesh;
        mesh->type = MESH_TRIANGLES;
        mesh->child = NULL;
        mesh->next = next;
        next = mesh;

        mesh->name = name + QString::number(i);
        mesh->vertex.resize( componentVertex[i].size() );
        mesh->normal.resize( componentVertex[i].size() );
        mesh->tcoord.resize( 2 * componentVertex[i].size() );
        for(int e = 0 ; e < componentVertex[i].size() ; e++)
        {
            mesh->vertex[e] = vertex[componentVertex[i][e]];
            mesh->normal[e] = normal[componentVertex[i][e]];
            mesh->tcoord[e * 2] = tcoord[2 * componentVertex[i][e]];
            mesh->tcoord[e * 2 + 1] = tcoord[2 * componentVertex[i][e] + 1];
        }
        for(int e = 0 ; e < index.size() ; e++)
            if (component[index[e]] == i)
                mesh->index.push_back(convert[index[e]]);
    }
    name += "0";
    QVector<Vec> nVertex;
    QVector<GLuint> nIndex;
    QVector<Vec> nNormal;
    QVector<GLfloat> nTcoord;
    nVertex.resize( componentVertex[0].size() );
    nNormal.resize( componentVertex[0].size() );
    nTcoord.resize( 2 * componentVertex[0].size() );
    for(int e = 0 ; e < componentVertex[0].size() ; e++)
    {
        nVertex[e] = vertex[componentVertex[0][e]];
        nNormal[e] = normal[componentVertex[0][e]];
        nTcoord[e * 2] = tcoord[2 * componentVertex[0][e]];
        nTcoord[e * 2 + 1] = tcoord[2 * componentVertex[0][e] + 1];
    }
    for(int e = 0 ; e < index.size() ; e++)
        if (component[index[e]] == 0)
            nIndex.push_back(convert[index[e]]);
    vertex = nVertex;
    index = nIndex;
    normal = nNormal;
    tcoord = nTcoord;
}

Mesh *Mesh::merge(const QList<Mesh*> &list)
{
    if (list.isEmpty())
        return NULL;

    QList<Mesh*> lMesh = list;

    Mesh *base = lMesh.front();
    lMesh.pop_front();
    base->toTriangleSoup();
	Vec refpos = base->pos;
    while(!lMesh.isEmpty())
    {
        Mesh *mesh = lMesh.back();      // Reverse order to make sure we won't change one of the pointer in the list
        lMesh.pop_back();
        mesh->toTriangleSoup();

        int bVtx = base->vertex.size();         // Copy mesh data
		base->tcoord.resize(bVtx * 2 + 2 * mesh->vertex.size());
		Vec shift = mesh->pos - refpos;
		foreach(Vec P, mesh->vertex)
			base->vertex.push_back(P + shift);

        memcpy(base->tcoord.data() + 2 * bVtx, mesh->tcoord.data(), mesh->tcoord.size() * sizeof(GLfloat));
        foreach(GLuint i, mesh->index)
            base->index.push_back(i + bVtx);

        if (base->child == NULL)                // Move childs
        {
            base->child = mesh->child;
			if (base->child)
				base->child->pos += shift;
            mesh->child = NULL;
        }
        else
        {
            Mesh *cur = base->child;
            while(mesh->child)
            {
                Mesh *tmp = cur->next;
                cur->next = mesh->child;
                cur = cur->next;
				cur->pos += shift;
				mesh->child = mesh->child->next;
				cur->next = tmp;
            }
        }
        if (mesh->next)                 // Destroy the Mesh object
        {
            Mesh *tmp = mesh->next;
            mesh->copy( tmp );
            tmp->child = NULL;
            tmp->next = NULL;
            delete tmp;
        }
        else
        {
            Mesh *tmp = base;
            while(tmp != NULL && tmp->next != mesh)
                tmp = tmp->next;
            if (tmp)
                tmp->next = NULL;
            delete mesh;
        }
    }
    base->computeNormals();
    return base;
}

Mesh *Mesh::toSingleMesh()
{
    Mesh *mesh = new Mesh;
    mesh->copy(this);
    mesh->child = NULL;
    mesh->next = NULL;
    mesh->toTriangleSoup();
    mesh->tcoord.clear();
    mesh->normal.clear();
    mesh->tex.clear();

    for(int i = 0 ; i < mesh->vertex.size() ; i++)
        mesh->vertex[i] += mesh->pos;

    QVector<Mesh*> meshList;

    if (next)
        meshList.push_back(next->toSingleMesh());
    if (child)
    {
        meshList.push_back(child->toSingleMesh());
        for(int i = 0 ; i < meshList.back()->vertex.size() ; i++)
            meshList.back()->vertex[i] += mesh->pos;
    }
    mesh->pos.reset();

    foreach(Mesh *m, meshList)
    {
        int base = mesh->vertex.size();
        for(int i = 0 ; i < m->vertex.size() ; i++)
            mesh->vertex.push_back(m->vertex[i]);

        for(int i = 0 ; i < m->index.size() ; i++)
            mesh->index.push_back(base + m->index[i]);
    }

    return mesh;
}

void Mesh::resetAnimData()
{
    axe[0].reset();
    axe[1].reset();
    axe[2].reset();
    explode = false;
    explode_time = 0.0f;
    explosion_flag = 0;
    anim_flag = 0;
    if (child)
        child->resetAnimData();
    if (next)
        next->resetAnimData();
}

void Mesh::resetScriptData()
{
    scriptID = -1;
    if (child)
        child->resetScriptData();
    if (next)
        next->resetScriptData();
}

void Mesh::move(const float dt)
{
    const float g = 9.8f;

    if (explode_time > 0.0f)
        explode_time -= dt;
    explode = explode_time > 0.0f;

    if (anim_flag & FLAG_EXPLODE)// && (explosion_flag[e]&EXPLODE_SDL_SurfaceONLY)!=EXPLODE_SDL_SurfaceONLY)		// This piece is exploding
    {
        for (int i = 0; i < 3; ++i)
        {
            if (i == 1 && (explosion_flag & EXPLODE_FALL))
                axe[i].move_speed -= g;
            axe[i].pos += axe[i].move_speed * dt;
            axe[i].angle += axe[i].rot_speed * dt;
            axe[i].is_moving = true;
        }
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            axe[i].is_moving = false;

            float a = axe[i].move_distance;
            if (a != 0.0f)
            {
                float c = axe[i].move_speed * dt;
                axe[i].move_distance -= c;
                axe[i].pos += c;
                axe[i].is_moving = c != 0.0f;
                if ((a > 0.0f && axe[i].move_distance < 0.0f) || (a < 0.0f && axe[i].move_distance > 0.0f))
                {
                    axe[i].pos += axe[i].move_distance;
                    axe[i].move_distance = 0.0f;
                }
            }

            while (axe[i].angle > 180.0f)
                axe[i].angle -= 360.0f;		// Maintient l'angle dans les limites
            while (axe[i].angle < -180.0f)
                axe[i].angle += 360.0f;

            a = axe[i].rot_angle;
            if ((axe[i].rot_speed != 0.0f || axe[i].rot_accel != 0.0f) && ((a != 0.0f && axe[i].rot_limit) || !axe[i].rot_limit))
            {
                float b = axe[i].rot_speed;
                if (b < -7200.0f)
                    b = axe[i].rot_speed = -7200.0f;
                else if (b > 7200.0f)
                    b = axe[i].rot_speed = 7200.0f;

                axe[i].rot_speed += axe[i].rot_accel * dt;
                axe[i].is_moving = axe[i].rot_accel != 0.0f;
                if (axe[i].rot_speed_limit)
                {
                    if ((b <= axe[i].rot_target_speed && axe[i].rot_speed >= axe[i].rot_target_speed)
                        || (b >= axe[i].rot_target_speed && axe[i].rot_speed <= axe[i].rot_target_speed))
                        {
                        axe[i].rot_accel = 0.0f;
                        axe[i].rot_speed = axe[i].rot_target_speed;
                        axe[i].rot_speed_limit = false;
                    }
                }
                float c = axe[i].rot_speed * dt;
                axe[i].angle += c;
                axe[i].is_moving = c != 0.0f;
                if (axe[i].rot_limit)
                {
                    axe[i].rot_angle -= c;
                    if ((a >= 0.0f && axe[i].rot_angle <= 0.0f) || (a <= 0.0f && axe[i].rot_angle >= 0.0f))
                    {
                        axe[i].angle += axe[i].rot_angle;
                        axe[i].rot_angle = 0.0f;
                        axe[i].rot_speed = 0.0f;
                        axe[i].rot_accel = 0.0f;
                    }
                }
            }
        }
    }
    if (child)
        child->move(dt);
    if (next)
        next->move(dt);
}

Vec Mesh::getRelativePositionAnim(int id)
{
    Matrix M = Scale(1.0f);
    M = RotateZYX(axe[2].angle * DEG2RAD, axe[1].angle * DEG2RAD, axe[0].angle * DEG2RAD);

    if (id == ID)
        return pos + Vec(axe[0].pos, axe[1].pos, axe[2].pos);
    if (((next && next->getID() > id) || next == NULL) && child)
        return child->getRelativePosition(id) * M + pos + Vec(axe[0].pos, axe[1].pos, axe[2].pos);
    if (next)
        return next->getRelativePosition(id);
    return Vec();
}
