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

    makeCurrent();
    glewInit();
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

    glDisable(GL_TEXTURE_2D);

    glColor3ub(0xFF, 0, 0);
    drawArrow(Vec(0,0,0), Vec(10,0,0), 0.3f);
    glColor3ub(0, 0xFF, 0);
    drawArrow(Vec(0,0,0), Vec(0,10,0), 0.3f);
    glColor3ub(0, 0, 0xFF);
    drawArrow(Vec(0,0,0), Vec(0,0,10), 0.3f);

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
    Mesh::instance.draw();

    if (selectedID >= 0)
    {
        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

        glDepthFunc(GL_EQUAL);

        Mesh::whiteSurface = true;
        Mesh::instance.draw(selectedID);
        Mesh::whiteSurface = false;

        glDepthFunc(GL_LESS);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
    }

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
}

GLuint Gfx::loadTexture(const QString &filename)
{
    if (!QFile(filename).exists())
    {
        qDebug() << "file not found: " << filename;
        return 0;
    }
    makeCurrent();
    GLuint tex = bindTexture( QPixmap( filename ) );
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

void Gfx::mouseMoveEvent(QMouseEvent *event)
{
    if (previousMouseState == event->buttons())
    {
        if (event->buttons() == Qt::LeftButton)
        {
            float r = 20.0f;
            Vec center;
            if (!Mesh::instance.isEmpty())
            {
                Vec p;
                Matrix inv = Invert( Transpose(meshMatrix) );
                Vec pos = Camera::inGame->rpos * inv;
                Vec dir = Camera::inGame->getScreenVector( ((float)previousMousePos.x()) / width(), ((float)previousMousePos.y()) / height()) * inv;
                if (Mesh::instance.hit(pos, dir, p))
                {
                    r = p.norm();
                }
                else
                    r = Mesh::instance.getSize();
            }
            arcballMove(center, r, previousMousePos, event->pos());
        }
        else if (event->buttons() == Qt::RightButton)
        {
            QPoint move = event->pos() - previousMousePos;
            Camera::inGame->rpos += 0.01f * (-move.x() * Camera::inGame->side + move.y() * Camera::inGame->up);
            updateGL();
        }
    }

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
    if (selectedID != ID)           // Avoid infinite loops
    {
        selectedID = ID;
        updateGL();
        GeometryGraph::instance()->updateSelectionID(ID);
    }
}
