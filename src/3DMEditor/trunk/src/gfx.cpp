#include <QMouseEvent>
#include <QWheelEvent>
#include <QtDebug>
#include <QPixmap>
#include <QFile>
#include "gfx.h"
#include "mesh.h"
#include "geometrygraph.h"
#include "misc/camera.h"
#include "misc/material.light.h"

using namespace TA3D;

Gfx *Gfx::pInstance = NULL;

Gfx::Gfx()
{
    QGLFormat format;
    format.setDoubleBuffer(true);
    setFormat(format);
    Camera::inGame = new Camera();
    HWLight::inGame = new HWLight();
    previousMousePos = QPoint();
    previousMouseState = Qt::NoButton;
    meshMatrix = Scale(1.0f);
    selectedID = -1;
    drawSelection = true;

    makeCurrent();
    glewInit();

    editMode = VIEW;
}

Gfx::~Gfx()
{
    delete Camera::inGame;
    Camera::inGame = NULL;
    delete HWLight::inGame;
    HWLight::inGame = NULL;
}

Gfx *Gfx::instance()
{
    if (!pInstance)
        pInstance = new Gfx();
    return pInstance;
}

void Gfx::initializeGL()
{
    SetDefState();
}

void Gfx::paintGL()
{
    Camera::inGame->setWidthFactor(width(), height());
    glViewport(0,0,width(),height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera::inGame->setView();
    HWLight::inGame->Set(*Camera::inGame);
    HWLight::inGame->Enable();

    glPushMatrix();

    glMultMatrixf( meshMatrix.data() );

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
    Mesh::instance()->draw();

    if (selectedID >= 0 && drawSelection)
    {
        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

        glDepthFunc(GL_EQUAL);

        Mesh::whiteSurface = true;
        Mesh::instance()->draw(selectedID);
        Mesh::whiteSurface = false;

        glDepthFunc(GL_LESS);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);

        glColor4ub(0xFF, 0x0, 0x0, 0xFF);

        glDepthFunc(GL_ALWAYS);
        glDisable(GL_TEXTURE_2D);

        glPushMatrix();
        Vec pos = Mesh::instance()->getRelativePosition(selectedID);
        glTranslatef(pos.x, pos.y, pos.z);
        drawCube(1.0f);
        glDepthFunc(GL_LESS);
        drawCube(1.0f);
        glPopMatrix();

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
    }

    glDisable(GL_TEXTURE_2D);

    glColor3ub(0xFF, 0, 0);
    drawArrow(Vec(0,0,0), Vec(10,0,0), 0.3f);
    glColor3ub(0, 0xFF, 0);
    drawArrow(Vec(0,0,0), Vec(0,10,0), 0.3f);
    glColor3ub(0, 0, 0xFF);
    drawArrow(Vec(0,0,0), Vec(0,0,10), 0.3f);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glColor4ub(0x0, 0x0, 0x0, 0xFF);
    renderText(10.0, 0.0f, 0.0f, "x");
    renderText(0.0, 10.0f, 0.0f, "y");
    renderText(0.0, 0.0f, 10.0f, "z");

    glColor3ub(0xFF, 0xFF, 0xFF);
    renderText(10.0, 0.0f, 0.0f, "x");
    renderText(0.0, 10.0f, 0.0f, "y");
    renderText(0.0, 0.0f, 10.0f, "z");

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glPopMatrix();

    Camera::setScreenCoordinates(width(), height());
    switch(editMode)
    {
    case EDIT:
        renderText(0.0f, height() - 5, 0.0f, tr("Editing mode"));
        break;
    case VIEW:
        renderText(0.0f, height() - 5, 0.0f, tr("Viewing mode"));
        break;
    case ANIMATE:
        renderText(0.0f, height() - 5, 0.0f, tr("Animation mode"));
        break;
    };
}

GLuint Gfx::loadTexture(const QString &filename)
{
    if (!QFile(filename).exists())
    {
        qDebug() << "file not found: " << filename;
        return 0;
    }
    makeCurrent();
    GLuint tex = filename.endsWith(".tga", Qt::CaseInsensitive)
                 ? bindTexture( loadTGA( filename) )
                 : bindTexture( QPixmap( filename ) );
    if (tex == 0)
        qDebug() << "could not load file " << filename;
    else
    {
        glBindTexture(GL_TEXTURE_2D, tex);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    }
    return tex;
}

void Gfx::destroyTexture(GLuint &gltex)
{
    if (gltex)
    {
        makeCurrent();
        glDeleteTextures(1, &gltex);
        gltex = 0;
    }
}

void Gfx::SetDefState()
{
    glClearColor (0, 0, 0, 0);
    glShadeModel (GL_SMOOTH);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glDepthFunc( GL_LESS );
    glEnable (GL_DEPTH_TEST);
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glHint(GL_FOG_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    ReInitTexSys();
}

void Gfx::ReInitTexSys(bool matrix_reset)
{
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    if (matrix_reset)
    {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
    }
}

void Gfx::drawCylinder(float r, float h, int d)
{
    glBegin(GL_QUAD_STRIP);
    for(int i = 0 ; i <= d ; i++)
    {
        float dx = cosf( M_PI * 2.0f * i / d );
        float dy = sinf( M_PI * 2.0f * i / d );
        glNormal3f( dx, 0.0f, dy );
        glVertex3f( r * dx, -0.5f * h, r * dy );
        glVertex3f( r * dx, 0.5f * h, r * dy );
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, -0.5f * h, 0.0f);
    for(int i = 0 ; i <= d ; i++)
        glVertex3f( r * cosf( M_PI * 2.0f * i / d ), -0.5f * h, r * sinf( M_PI * 2.0f * i / d ) );
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.5f * h, 0.0f);
    for(int i = 0 ; i <= d ; i++)
        glVertex3f( r * cosf( M_PI * 2.0f * i / d ), 0.5f * h, r * sinf( M_PI * 2.0f * i / d ) );
    glEnd();
}

void Gfx::drawCone(float r, float h, int d)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f( 0.0f, 0.5f * h, 0.0f );
    for(int i = 0 ; i <= d ; i++)
    {
        float dx = cosf( M_PI * 2.0f * i / d );
        float dy = sinf( M_PI * 2.0f * i / d );
        Vec n = h * Vec(dx, 0.0f, dy) + r * Vec(0.0f, 1.0f, 0.0f);
        n.unit();
        glNormal3f( n.x, n.y, n.z );
        glVertex3f( r * dx, -0.5f * h, -r * dy );
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f( 0.0f, -0.5f * h, 0.0f );
    for(int i = 0 ; i <= d ; i++)
        glVertex3f( r * cosf( M_PI * 2.0f * i / d ), -0.5f * h, r * sinf( M_PI * 2.0f * i / d ) );
    glEnd();
}

void Gfx::drawArrow(const Vec &a, const Vec &b, float r)
{
    Vec AB = b - a;
    Vec I(0.0f, 1.0f, 0.0f);
    Vec J = I ^ AB;
    J.unit();
    float angle = VAngle( AB, I ) * 180.0f / M_PI;

    glPushMatrix();
    glRotatef(angle, J.x, J.y, J.z);
    float h = AB.norm() - r;
    glTranslatef(0.0f, h * 0.5f, 0.0f);
    drawCylinder(0.5f * r, h, 10);
    glTranslatef(0.0f, h * 0.5f + r * 0.5f, 0.0f);
    drawCone(r, r, 10);
    glPopMatrix();
}

void Gfx::drawSphere(float r, int d)
{
    glBegin(GL_TRIANGLE_STRIP);
    int h = d / 2;
    for(int e = 0 ; e < h ; e++)
    {
        for(int i = 0 ; i <= d ; i++)
        {
            for(int f = e ; f <= e + 1 ; f++)
            {
                float dx = cosf( M_PI * 2.0f * i / d ) * cosf( M_PI * f / h - 0.5f * M_PI);
                float dy = sinf( M_PI * f / h - 0.5f * M_PI);
                float dz = sinf( M_PI * 2.0f * i / d ) * cosf( M_PI * f / h - 0.5f * M_PI);
                glNormal3f( dx, dy, dz );
                glVertex3f( r * dx, r * dy, r * dz );
            }
        }
    }
    glEnd();
}

void Gfx::drawCube(float size)
{
    size *= 0.5f;
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);                // Up
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    glVertex3f(-size, size, -size);

    glNormal3f(0, -1, 0);               // Down
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);

    glNormal3f(1, 0, 0);                // Right
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);

    glNormal3f(-1, 0, 0);               // Left
    glVertex3f(-size, size, -size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);

    glNormal3f(0, 0, 1);                // Back
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    glNormal3f(0, 0, -1);               // Front
    glVertex3f(size, -size, -size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glEnd();
}

void Gfx::mouseMoveEvent(QMouseEvent *event)
{
    switch(editMode)
    {
    case VIEW:
        if (previousMouseState == event->buttons())
        {
            if (event->buttons() == Qt::LeftButton)         // Rotate the camera
            {
                float r = 20.0f;
                Vec center;
                if (!Mesh::instance()->isEmpty())
                {
                    Vec p;
                    Matrix inv = Invert( Transpose(meshMatrix) );
                    Vec pos = Camera::inGame->rpos * inv;
                    Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height()) * inv;
                    if (Mesh::instance()->hit(pos, dir, p) >= 0)
                    {
                        r = p.norm();
                    }
                    else
                        r = Mesh::instance()->getSize();
                }
                arcballMove(center, r, previousMousePos, event->pos());
            }
            else if (event->buttons() == Qt::RightButton)     // Move camera // to viewing plane
            {
                Vec p;
                Matrix inv = Invert( Transpose(meshMatrix) );
                Vec pos = Camera::inGame->rpos * inv;
                Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height()) * inv;
                if (Mesh::instance()->hit(pos, dir, p) >= 0)
                {
                    p = p * Transpose(meshMatrix);
                    Vec dir2 = Camera::inGame->getScreenVector( ((float)event->pos().x()) / width(), ((float)event->pos().y()) / height());
                    float dist = (p - Camera::inGame->rpos) * Camera::inGame->dir;
                    Vec np = Camera::inGame->rpos + dist / (dir2 * Camera::inGame->dir) * dir2;
                    Camera::inGame->rpos += p - np;
                }
                else
                {
                    dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height());
                    float dist = (Vec() - Camera::inGame->rpos) * Camera::inGame->dir;
                    p = Camera::inGame->rpos + dist / (dir * Camera::inGame->dir) * dir;
                    Vec dir2 = Camera::inGame->getScreenVector( ((float)event->pos().x()) / width(), ((float)event->pos().y()) / height());
                    Vec np = Camera::inGame->rpos + dist / (dir2 * Camera::inGame->dir) * dir2;
                    Camera::inGame->rpos += p - np;
                }
                updateGL();
            }
        }
        break;
    case EDIT:
        if (previousMouseState == event->buttons() && selectedID >= 0)
        {
            if (event->buttons() == Qt::LeftButton)         // Move attach point
            {
                Matrix inv = Invert( Transpose(meshMatrix) );
                Mesh *mesh = Mesh::instance()->getMesh(selectedID);
                Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height());
                float dist = (Vec() - Camera::inGame->rpos) * Camera::inGame->dir;
                Vec p = Camera::inGame->rpos + dist / (dir * Camera::inGame->dir) * dir;
                Vec dir2 = Camera::inGame->getScreenVector( ((float)event->pos().x()) / width(), ((float)event->pos().y()) / height());
                Vec np = Camera::inGame->rpos + dist / (dir2 * Camera::inGame->dir) * dir2;
                Vec move = (np - p) * inv;
                mesh->pos += move;
                for(int i = 0 ; i < mesh->vertex.size() ; i++)
                    mesh->vertex[i] -= move;
                for( Mesh *cur = mesh->child ; cur != NULL ; cur = cur->next )
                    cur->pos -= move;
                updateGL();
            }
            else if (event->buttons() == Qt::RightButton)   // Move
            {
                Vec p;
                Matrix inv = Invert( Transpose(meshMatrix) );
                Mesh *mesh = Mesh::instance()->getMesh(selectedID);
                Vec pos = Camera::inGame->rpos * inv - Mesh::instance()->getRelativePosition(selectedID) + mesh->pos;
                Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height()) * inv;
                if (mesh->hit(pos, dir, p) >= 0)
                {
                    p = (p + Mesh::instance()->getRelativePosition(selectedID) - mesh->pos) * Transpose(meshMatrix);
                    Vec dir2 = Camera::inGame->getScreenVector( ((float)event->pos().x()) / width(), ((float)event->pos().y()) / height());
                    float dist = (p - Camera::inGame->rpos) * Camera::inGame->dir;
                    Vec np = Camera::inGame->rpos + dist / (dir2 * Camera::inGame->dir) * dir2;
                    mesh->pos += (np - p) * inv;
                }
                else
                {
                    dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height());
                    float dist = (Vec() - Camera::inGame->rpos) * Camera::inGame->dir;
                    p = Camera::inGame->rpos + dist / (dir * Camera::inGame->dir) * dir;
                    Vec dir2 = Camera::inGame->getScreenVector( ((float)event->pos().x()) / width(), ((float)event->pos().y()) / height());
                    Vec np = Camera::inGame->rpos + dist / (dir2 * Camera::inGame->dir) * dir2;
                    mesh->pos += (np - p) * inv;
                }
                updateGL();
            }
        }
        break;
    };

    previousMousePos = event->pos();
    previousMouseState = event->buttons();
}

void Gfx::wheelEvent(QWheelEvent *event)
{
    Camera::inGame->rpos += 0.01f * event->delta() * Camera::inGame->dir;
    updateGL();
}

void Gfx::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
    {
        Vec p;
        Matrix inv = Invert( Transpose(meshMatrix), 150 );
        Vec pos = Camera::inGame->rpos * inv;
        Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height()) * inv;
        updateSelection( Mesh::instance()->hit(pos, dir, p) );
    }

    previousMousePos = event->pos();
    previousMouseState = event->buttons();
}

void Gfx::arcballMove(const Vec &pos, const float r, const QPoint &A, const QPoint &B)
{
    float r2 = r * r;
    // On screen directions
    Vec va = Camera::inGame->getScreenVector( ((float)A.x()) / width(), ((float)A.y()) / height() );
    Vec vb = Camera::inGame->getScreenVector( ((float)B.x()) / width(), ((float)B.y()) / height() );
    va.unit();
    vb.unit();

    // The vector from aiming point to object center
    Vec dir = pos - Camera::inGame->rpos;

    // The projection of dir on va and vb
    Vec ha = Camera::inGame->rpos + (dir * va) * va;
    Vec hb = Camera::inGame->rpos + (dir * vb) * vb;

    // The first intersection point with the sphere (relative to its center)
    float da = r2 - (ha - pos).sq();
    float db = r2 - (hb - pos).sq();
    if (da < 0.0f || db < 0.0f)          // If we are not hitting the sphere abort now
        return;
    Vec sa = ha - sqrtf(da) * va - pos;
    Vec sb = hb - sqrtf(db) * vb - pos;
    sa.unit();
    sb.unit();

    Vec axis = sa ^ sb;
    axis.unit();

    float angle = VAngle(sa, sb) * 180.0f / M_PI;

    makeCurrent();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(angle, axis.x, axis.y, axis.z);
    glMultMatrixf(meshMatrix.data());
    glGetFloatv(GL_MODELVIEW_MATRIX, meshMatrix.data());
    glPopMatrix();

    updateGL();
}

void Gfx::updateSelection(int ID)
{
    if (ID >= 0 && Mesh::instance()->getMesh(ID) == Mesh::instance() && Mesh::instance()->isEmpty())
        ID = -1;
    if (selectedID != ID)           // Avoid infinite loops
    {
        selectedID = ID;
        updateGL();
        GeometryGraph::instance()->updateSelectionID(ID);
        emit selectionChange(ID);
    }
}

void Gfx::showSelection()
{
    if (!drawSelection)
    {
        drawSelection = true;
        updateGL();
    }
}

void Gfx::hideSelection()
{
    if (drawSelection)
    {
        drawSelection = false;
        updateGL();
    }
}

QImage Gfx::loadTGA(const QString &filename)                // Load a TGA file into a QImage object
{
    byte        TGAheader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
    byte        TGAcompare[12];								// Used To Compare TGA Header
    byte        header[6];									// First 6 Useful Bytes From The Header
    int         bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
    int         imageSize;									// Used To Store The Image Size When Setting Aside Ram

    QFile file(filename);						// Open The TGA File
    file.open(QIODevice::ReadOnly);
    if (!file.exists() || !file.isOpen())
        return QImage(1, 1, QImage::Format_ARGB32);

    if (file.read((char*)TGAcompare, sizeof(TGAcompare)) != sizeof(TGAcompare))
    {
        qDebug() << "could not read TGA header";
        file.close();
        return QImage(1, 1, QImage::Format_ARGB32);
    }
    if (memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0)
    {
        qDebug() << "TGA header doesn't match";
        file.close();
        return QImage(1, 1, QImage::Format_ARGB32);
    }
    if (file.read((char*)header, sizeof(header)) != sizeof(header))
    {
        qDebug() << "could not read TGA header data";
        file.close();
        return QImage(1, 1, QImage::Format_ARGB32);
    }

    int w = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
    int h = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
    int bpp = header[4];

    if(bpp != 24 && bpp != 32)					// Is The TGA 24 or 32 Bit?
    {
        qDebug() << "This is not a 24/32 bpp TGA file";
        file.close();
        return QImage(1, 1, QImage::Format_ARGB32);
    }

    bytesPerPixel	= bpp / 8;
    imageSize		= w * h * bytesPerPixel;	// Calculate The Memory Required For The TGA Data

    QByteArray imageData(imageSize, 0);
    if (file.read((char*)imageData.data(), imageSize) != imageSize)
    {
        qDebug() << "could not read TGA image data";
        file.close();
        return QImage(1, 1, QImage::Format_ARGB32);
    }

    QImage img(w, h, bpp == 32 ? QImage::Format_ARGB32 : QImage::Format_RGB888);

    for(int y = 0 ; y < h ; y++)
    {
        for(int x = 0 ; x < w ; x++)
        {
            uint32 color = 0;
            switch(bpp)
            {
            case 24:
                color = imageData[ (y * w + x) * bytesPerPixel ]
                        | (imageData[ (y * w + x) * bytesPerPixel + 1 ] << 8)
                        | (imageData[ (y * w + x) * bytesPerPixel + 2 ] << 16);
                break;
            case 32:
                color = imageData[ (y * w + x) * bytesPerPixel ]
                        | (imageData[ (y * w + x) * bytesPerPixel + 1 ] << 8)
                        | (imageData[ (y * w + x) * bytesPerPixel + 2 ] << 16)
                        | (imageData[ (y * w + x) * bytesPerPixel + 3 ] << 24);
                break;
            };
            img.setPixel(x, y, color);
        }
    }

    file.close();

    return img;											// Texture Building Went Ok, Return True
}

int Gfx::getSelectionID()
{
    return selectedID;
}

int Gfx::getTextureWidth(GLuint tex)
{
    makeCurrent();
    GLint w;
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w );
    return w;
}

int Gfx::getTextureHeight(GLuint tex)
{
    makeCurrent();
    int h;
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h );
    return h;
}

void Gfx::setEditMode()
{
    editMode = EDIT;
    updateGL();
}

void Gfx::setViewMode()
{
    editMode = VIEW;
    updateGL();
}

void Gfx::setAnimateMode()
{
    editMode = ANIMATE;
    updateGL();
}

QImage Gfx::textureToImage(GLuint tex)
{
    int w = getTextureWidth(tex);               // From here our context is the current one
    int h = getTextureHeight(tex);              // so following code is context safe
    QImage img(w, h, QImage::Format_ARGB32);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA,GL_UNSIGNED_BYTE, img.bits());
    return img;
}
