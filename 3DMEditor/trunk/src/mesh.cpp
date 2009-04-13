#include <QDebug>
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
