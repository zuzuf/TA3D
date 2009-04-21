#include <QDebug>
#include <QQueue>
#include <zlib.h>
#include "mesh.h"
#include "gfx.h"

bool Mesh::whiteSurface = false;
Mesh *Mesh::pInstance = NULL;

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
}

Mesh::~Mesh()
{
    if (this != pInstance)
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
    computeInfo();
    emit loaded();
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

void Mesh::draw(int id)
{
    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (!vertex.isEmpty() && !normal.isEmpty())
    {
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

            if (tex.isEmpty() || !(flag & SURFACE_TEXTURED))
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
                for(int i = tex.size() - 1 ; i >= 0 ; i--)
                {
                    glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glActiveTextureARB(GL_TEXTURE0_ARB + i);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, tex[i]);
                    glTexCoordPointer(2, GL_FLOAT, 0, tcoord.data());

                    if ((flag & SURFACE_REFLEC) && i == tex.size() - 1)
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
            if (!tex.isEmpty() && (flag & SURFACE_TEXTURED))
                for(int i = tex.size() - 1 ; i >= 0 ; i--)
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
            child->draw();

        glPopMatrix();
        if (next)
            next->draw();
    }
    else if (id != ID)
    {
        if (((next && next->ID > id) || next == NULL) && child)
        {
            child->draw(id);
            glPopMatrix();
        }
        else if (next)
        {
            glPopMatrix();
            next->draw(id);
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
                    uint32 col;
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
        ANIMATION *animation_data = new ANIMATION;
        file.read( (char*)&(animation_data->type), 1 );
        file.read( (char*)&(animation_data->angle_0), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->angle_1), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->angle_w), sizeof(float)  );
        file.read( (char*)&(animation_data->translate_0), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->translate_1), sizeof(Vector3D) );
        file.read( (char*)&(animation_data->translate_w), sizeof(float) );

        file.read( (char*)&link, 1 );

        delete animation_data;
    }

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
    if (l + l * l / OM.sq() <= md)      // Spherical intersection ?
    {
        Vec n = (b-a) ^ (c-a);
        if ((dir * n) * ((a - pos) * n) > 0.0f)     // Are we aiming in the right direction ?
        {
            p = pos + (((a - pos) * n) / (dir * n)) * dir;      // Intersection with triangle space
            // WARNING: fonction critique a tester abusivement
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

    byte link = (child != NULL) ? 1 : 0;
    file.write((char*)&link, 1);

    if (link == 2) // Save animation data if present (currently not supported)
    {
        ANIMATION *animation_data = new ANIMATION;
        file.write( (char*)&(animation_data->type), 1 );
        file.write( (char*)&(animation_data->angle_0), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->angle_1), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->angle_w), sizeof(float)  );
        file.write( (char*)&(animation_data->translate_0), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->translate_1), sizeof(Vector3D) );
        file.write( (char*)&(animation_data->translate_w), sizeof(float) );

        file.write( (char*)&link, 1 );

        delete animation_data;
    }

    if (link)
        child->save3DMrec(file);

    link = (next != NULL) ? 1 : 0;
    file.write((char*)&link, 1);
    if (link)
        next->load3DMrec(file);
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
                    if (index[e] == A || index[e+1] == A || index[e+2] == A)
                    {
                        if (component[index[e]] == orig && component[index[e+1]] == orig && component[index[e+2]] == orig)
                        {
                            if (index[e] == A)
                                index[e] = vertex.size();
                            if (index[e+1] == A)
                                index[e+1] = vertex.size();
                            if (index[e+2] == A)
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

    tcoord.resize(2 * vertex.size());

    // Compute texture coordinates per component (physics like stuffs)
    for(int c = 0 ; c < nb_component ; c++)
    {
        // Open the mesh at its extremities (distance extremas - but not 0 - which have more than 2 neighbors)
        // in order to have a mesh with a topology that can be mapped :)
        for(int i = 0 ; i < componentVertex[c].size() ; i++)
        {
            int n = componentVertex[c][i];
            int d = dist[n];
            if (neighbors[n].size() > 2)
            {
                bool extremum = true;
                for(int e = 0 ; e < neighbors[n].size() && extremum ; e++)
                    extremum = dist[neighbors[n][e]] <= d;
                if (extremum)       // Duplicate this vertex
                {
                    QVector<int> vParent;
                    for(int j = 0 ; j < index.size() ; j += 3)
                        if (index[j] == n || index[j+1] == n || index[j+2] == n)
                            vParent.push_back(j);
                    for(int j = 1 ; j < vParent.size() ; j++)
                    {
                        int k = vParent[j];
                        if (index[k] == n)  index[k] = vertex.size();
                        if (index[k+1] == n)  index[k+1] = vertex.size();
                        if (index[k+2] == n)  index[k+2] = vertex.size();
                        neighbors.push_back(QVector<int>());
                        for(int e = 0 ; e < neighbors[n].size() ; e++)
                        {
                            neighbors.last().push_back(neighbors[n][e]);
                            neighbors[n].remove(e--);
                        }
                        dist.push_back(dist[n]);
                        component.push_back(c);
                        componentVertex[c].push_back(vertex.size());
                        vertex.push_back(vertex[n]);
                        tcoord.push_back(0.0f);
                        tcoord.push_back(0.0f);
                    }
                }
            }
        }

        QVector<int> nbDist;
        QVector<int> nbLeft;
        nbDist.resize( componentVertex[c].size() );
        for(int i = 0 ; i < componentVertex[c].size() ; i++)
            nbDist[ dist[ componentVertex[c][i] ] ]++;
        nbLeft = nbDist;
        for(int i = 0 ; i < componentVertex[c].size() ; i++)       // Initialize texture coordinates
        {
            int n = componentVertex[c][i];
            int d = dist[n];
            if (d == 0)
            {
                tcoord[n * 2] = 0.0f;
                tcoord[n * 2 + 1] = 0.0f;
            }
            else
            {
                tcoord[n * 2] = cosf(nbLeft[d] * M_PI * 2.0f / nbDist[d]) * d;
                tcoord[n * 2 + 1] = sinf(nbLeft[d] * M_PI * 2.0f / nbDist[d]) * d;
                nbLeft[d]--;
            }
        }

        // First simulation steps (tries to make the mesh occupies the texture space)
        for(int k = 0 ; k < 200 ; k++)
        {
            for(int i = 0 ; i < componentVertex[c].size() ; i++)
            {
                Vec move;
                int n = componentVertex[c][i];
                Vec N(tcoord[n * 2], tcoord[n * 2 + 1], 0);
                for(int e = 0 ; e < componentVertex[c].size() ; e++)
                {
                    if (e != i)
                    {
                        int m = componentVertex[c][e];
                        Vec M(tcoord[m * 2], tcoord[m * 2 + 1], 0);
                        if (neighbors[n].contains(m))
                        {
                            Vec D(N - M);
                            float l = D.norm();
                            D = 1.0f / l * D;
                            move += (1.0f - l) * D;
                        }
                        else
                        {
                            move += 1.0f / (N - M).sq() * (N - M);
                        }
                    }
                }
                if (move.sq() > 1.0f)
                    move.unit();
                tcoord[n * 2] += move.x;
                tcoord[n * 2 + 1] += move.y;
            }
        }
        // First simulation steps (tries to make the texture spaces reflect 3D triangle spaces)
        for(int k = 0 ; k < 100 ; k++)
        {
            float f = 0.1f / (k + 1);
            for(int i = 0 ; i < componentVertex[c].size() ; i++)
            {
                Vec move;
                float constraint = 0.0f;
                int n = componentVertex[c][i];
                Vec N(tcoord[n * 2], tcoord[n * 2 + 1], 0);
                for(int e = 0 ; e < componentVertex[c].size() ; e++)
                {
                    if (e != i)
                    {
                        int m = componentVertex[c][e];
                        Vec M(tcoord[m * 2], tcoord[m * 2 + 1], 0);
                        if (neighbors[n].contains(m))
                        {
                            Vec D(N - M);
                            float l = D.norm();
                            D = 1.0f / l * D;
                            float value = ((vertex[n] - vertex[m]).norm() - l);
                            move += value * D;
                            constraint += fabsf(value);
                        }
                        else
                        {
                            float value = 10.0f / (f * (N - M).sq());
                            move += 10.0f / (f * (N - M).sq()) * (N - M);
                            constraint += fabsf(value);
                        }
                    }
                }
                // FIXME: links that supports too much distortion should be broken, currently it doesn't work
                if (!isnan(constraint) && false && constraint > 300.0f)        // This needs to be duplicated
                {
                    QVector<int> vParent;
                    for(int j = 0 ; j < index.size() ; j += 3)
                        if (index[j] == n || index[j+1] == n || index[j+2] == n)
                            vParent.push_back(j);
                    for(int j = 1 ; j < vParent.size() ; j++)
                    {
                        int t = vParent[j];
                        if (index[t] == n)  index[t] = vertex.size();
                        if (index[t+1] == n)  index[t+1] = vertex.size();
                        if (index[t+2] == n)  index[t+2] = vertex.size();
                        neighbors.push_back(QVector<int>());
                        for(int e = 0 ; e < neighbors[n].size() ; e++)
                        {
                            neighbors.last().push_back(neighbors[n][e]);
                            neighbors[n].remove(e--);
                        }
                        component.push_back(c);
                        componentVertex[c].push_back(vertex.size());
                        vertex.push_back(vertex[n]);
                        tcoord.push_back(tcoord[n*2]);
                        tcoord.push_back(tcoord[n*2+1]);
                    }
                }
                qDebug() << constraint;
                if (move.sq() > 1.0f)
                    move.unit();
                tcoord[n * 2] += f * move.x;
                tcoord[n * 2 + 1] += f * move.y;
            }
        }
    }
    float umin, umax, vmin, vmax;
    for(int i = 0 ; i < tcoord.size() ; i+=2)
    {
        if (i == 0)
        {
            umin = umax = tcoord[i];
            vmin = vmax = tcoord[i+2];
        }
        else
        {
            umin = qMin(umin, tcoord[i]);
            vmin = qMin(vmin, tcoord[i+1]);
            umax = qMax(umax, tcoord[i]);
            vmax = qMax(vmax, tcoord[i+1]);
        }
    }
    umax -= umin;
    vmax -= vmin;
    for(int i = 0 ; i < tcoord.size() ; i+=2)
    {
        tcoord[i] = (tcoord[i] - umin) / umax;
        tcoord[i+1] = (tcoord[i+1] - vmin) / vmax;
    }
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
    for(int i = 0 ; i < tcoord.size() ; i += 12)
    {
        int e = i / 12;
        tcoord[i  ] = ((float)(e % w)) / w + 0.02f / w;
        tcoord[i+1] = ((float)(e / w)) / h + 0.01f / h;
        tcoord[i+2] = ((float)(e % w)+1) / w - 0.01f / w;
        tcoord[i+3] = ((float)(e / w)) / h + 0.01f / h;
        tcoord[i+4] = ((float)(e % w)+1) / w - 0.01f / w;
        tcoord[i+5] = ((float)(e / w) + 1) / h - 0.02f / h;

        tcoord[i+6] = ((float)(e % w)) / w + 0.01f / w;
        tcoord[i+7] = ((float)(e / w)) / h + 0.02f / h;
        tcoord[i+8] = ((float)(e % w)) / w + 0.01f / w;
        tcoord[i+9] = ((float)(e / w) + 1) / h - 0.01f / h;
        tcoord[i+10] = ((float)(e % w)+1) / w - 0.02f / w;
        tcoord[i+11] = ((float)(e / w) + 1) / h - 0.01f / h;
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
            if (nVertex[j] == p)
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
            nTcoord.push_back( tcoord[ index[i] * 2 ] );
            nTcoord.push_back( tcoord[ index[i] * 2 + 1 ] );

            nIndex.push_back(i + 1);
            nVertex.push_back( vertex[ index[i + 1] ] );
            nTcoord.push_back( tcoord[ index[i + 1] * 2 ] );
            nTcoord.push_back( tcoord[ index[i + 1] * 2 + 1 ] );

            nIndex.push_back(i + 2);
            nVertex.push_back( vertex[ index[i + 2] ] );
            nTcoord.push_back( tcoord[ index[i + 2] * 2 ] );
            nTcoord.push_back( tcoord[ index[i + 2] * 2 + 1 ] );
        }
        break;
    case MESH_TRIANGLES:
    default:
        for(int i = 0 ; i < index.size() ; i++)
        {
            nIndex.push_back(i);
            nVertex.push_back( vertex[ index[i] ] );
            nTcoord.push_back( tcoord[ index[i] * 2 ] );
            nTcoord.push_back( tcoord[ index[i] * 2 + 1 ] );
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
                    if (index[e] == A || index[e+1] == A || index[e+2] == A)
                    {
                        if (component[index[e]] == orig && component[index[e+1]] == orig && component[index[e+2]] == orig)
                        {
                            if (index[e] == A)
                                index[e] = vertex.size();
                            if (index[e+1] == A)
                                index[e+1] = vertex.size();
                            if (index[e+2] == A)
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
    qSort(lMesh);

    Mesh *base = lMesh.front();
    lMesh.pop_front();
    base->toTriangleSoup();
    while(!lMesh.isEmpty())
    {
        Mesh *mesh = lMesh.back();      // Reverse order to make sure we won't change one of the pointer in the list
        lMesh.pop_back();
        mesh->toTriangleSoup();

        int bVtx = base->vertex.size();         // Copy mesh data
        int bIdx = base->index.size();
        base->tcoord.resize(bVtx * 2 + mesh->tcoord.size());
        base->vertex.resize(bVtx + mesh->vertex.size());
        memcpy(base->vertex.data() + bVtx, mesh->vertex.data(), mesh->vertex.size() * sizeof(Vec));
        memcpy(base->tcoord.data() + 2 * bVtx, mesh->tcoord.data(), mesh->tcoord.size() * sizeof(GLfloat));
        foreach(GLuint i, mesh->index)
            base->index.push_back(i + bVtx);

        if (base->child == NULL)                // Move childs
        {
            base->child = mesh->child;
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
                cur->next = tmp;
                mesh->child = mesh->child->next;
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
