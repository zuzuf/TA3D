#include "textureviewer.h"
#include "gfx.h"
#include "mesh.h"

TextureViewer *TextureViewer::pInstance = NULL;

TextureViewer *TextureViewer::instance()
{
    if (pInstance == NULL)
        pInstance = new TextureViewer;
    return pInstance;
}

TextureViewer::TextureViewer()
{
    setWindowTitle(tr("Texture viewer"));
    ((QGLContext*)context())->create( Gfx::instance()->context() );
    selectedID = -1;
}

void TextureViewer::updateSelection(int ID)
{
    if (ID != selectedID)
    {
        selectedID = ID;
        Mesh *mesh = Mesh::instance.getMesh(ID);
        if (mesh && !mesh->tex.isEmpty())
        {
           int w = Gfx::instance()->getTextureWidth(mesh->tex[0]);
           int h = Gfx::instance()->getTextureHeight(mesh->tex[0]);
           resize(w, h);
        }
        updateGL();
    }
}

void TextureViewer::initializeGL()
{
}

void TextureViewer::paintGL()
{
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Mesh *mesh = Mesh::instance.getMesh(selectedID);

    if (mesh)
    {
        if (!mesh->tex.isEmpty())
        {
            glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
            glEnable(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, mesh->tex[0]);

            glBegin(GL_QUADS);
            glTexCoord2f( 0.0f, 0.0f);  glVertex2f( 0.0f, 0.0f );
            glTexCoord2f( 1.0f, 0.0f);  glVertex2f( 1.0f, 0.0f );
            glTexCoord2f( 1.0f, 1.0f);  glVertex2f( 1.0f, 1.0f );
            glTexCoord2f( 0.0f, 1.0f);  glVertex2f( 0.0f, 1.0f );
            glEnd();

            glDisable(GL_TEXTURE_2D);
        }
        if (!mesh->tcoord.isEmpty())
        {
            glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

            glEnableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);

            glVertexPointer(2, GL_FLOAT, 0, mesh->tcoord.data());

            switch(mesh->type)
            {
            case MESH_TRIANGLE_STRIP:
                glDrawElements(GL_TRIANGLE_STRIP, mesh->index.size(), GL_UNSIGNED_INT, mesh->index.data());
                break;
            case MESH_TRIANGLES:
            default:
                glDrawElements(GL_TRIANGLES, mesh->index.size(), GL_UNSIGNED_INT, mesh->index.data());
                break;
            };
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_BLEND);
        }
    }
}
