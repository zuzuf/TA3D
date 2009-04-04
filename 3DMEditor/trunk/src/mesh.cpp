#include <QDebug>
#include <zlib.h>
#include "mesh.h"
#include "gfx.h"

Mesh Mesh::instance;

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
    if (this != &instance)
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

void Mesh::draw()
{
    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertex.data());
    glNormalPointer(GL_FLOAT, 0, normal.data());

    if (flag & SURFACE_GLSL)
        shader.on();

    if (tex.isEmpty() || !(flag & SURFACE_TEXTURED))
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
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

    if (flag & SURFACE_GLSL)
        shader.off();

    if (flag & SURFACE_BLENDED)
    {
        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
    }
    if (!tex.isEmpty() && (flag & SURFACE_TEXTURED))
        for(int i = tex.size() - 1 ; i >= 0 ; i--)
            Gfx::instance()->ReInitTexSys();

    if (child)
        child->draw();

    glPopMatrix();
    if (next)
        next->draw();
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
}

bool Mesh::isEmpty()
{
    return child == NULL && next == NULL && vertex.isEmpty();
}

bool Mesh::hit(const Vec &pos, const Vec &dir, Vec &p)
{
    Vec rPos = pos - this->pos;
    bool bHit = false;
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
                if (!bHit || dir * p > dir * q)
                    p = q;
                bHit = true;
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
                if (!bHit || dir * p > dir * q)
                    p = q;
                bHit = true;
            }
        }
        break;
    };

    if (child)
    {
        Vec q;
        if (child->hit(rPos, dir, q))
        {
            if (!bHit || dir * p > dir * q)
                p = q;
            bHit = true;
        }
    }
    if (bHit)
        p += this->pos;
    if (next)
    {
        Vec q;
        if (next->hit(pos, dir, q))
        {
            if (!bHit || dir * p > dir * q)
                p = q;
            bHit = true;
        }
    }
    return bHit;
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
