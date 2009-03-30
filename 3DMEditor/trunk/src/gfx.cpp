#include "gfx.h"
#include "misc/camera.h"
#include "misc/material.light.h"

using namespace TA3D;

Gfx *Gfx::pInstance = NULL;

Gfx::Gfx()
{
    Camera::inGame = new Camera();
    HWLight::inGame = new HWLight();
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

    glDisable(GL_TEXTURE_2D);

    glColor3ub(0xFF, 0, 0);
    drawArrow(Vec(0,0,0), Vec(10,0,0), 1);
    glColor3ub(0, 0xFF, 0);
    drawArrow(Vec(0,0,0), Vec(0,10,0), 1);
    glColor3ub(0, 0, 0xFF);
    drawArrow(Vec(0,0,0), Vec(0,0,10), 1);

    glColor3ub(0xFF, 0xFF, 0xFF);

    renderText(10.0, 0.0f, 0.0f, "x");
    renderText(0.0, 10.0f, 0.0f, "y");
    renderText(0.0, 0.0f, 10.0f, "z");

    glPopMatrix();
}

GLuint Gfx::loadTexture(const QString &filename)
{
    return bindTexture(filename);
}

void Gfx::destroyTexture(GLuint &gltex)
{
    if (gltex)
    {
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
        glVertex3f( r * dx, -0.5f * h, r * dy );
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
    Vec J = I * AB;
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
