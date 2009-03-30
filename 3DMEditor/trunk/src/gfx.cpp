#include "gfx.h"
#include "misc/camera.h"

using namespace TA3D;

Gfx *Gfx::pInstance = NULL;

Gfx::Gfx()
{
    Camera::inGame = new Camera();
}

Gfx::~Gfx()
{
    delete Camera::inGame;
    Camera::inGame = NULL;
}

Gfx *Gfx::instance()
{
    if (!pInstance)
        pInstance = new Gfx();
    return pInstance;
}

void Gfx::initializeGL()
{
}

void Gfx::paintGL()
{
    Camera::inGame->setWidthFactor(width(), height());
    glViewport(0,0,width(),height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera::inGame->setView();

    glPushMatrix();

    glDisable(GL_TEXTURE_2D);

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(10.0f, 0.0f, 0.0f);

    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 10.0f, 0.0f);

    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 10.0f);
    glEnd();

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
