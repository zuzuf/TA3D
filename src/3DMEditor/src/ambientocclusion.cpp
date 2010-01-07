#include "mesh.h"
#include "progressdialog.h"
#include "gfx.h"
#include "misc/grid.h"
#include <QGLFramebufferObject>

void Mesh::computeAmbientOcclusion(int w, int h, int precision)     // For the Mesh and each submesh, create a texture representing global occlusion generated by the mesh itself
{
	if (tcoord.isEmpty())
		return;

	if (precision < 1)  precision = 1;

	QVector<Vec> dirs;
	for(int i = 0 ; i < precision ; i++)
	{
		float alpha = (abs(qrand()) % 10000) * M_PI * 0.0002f;
		float beta = asinf((abs(qrand()) % 20000) * 0.0001f - 1.0f);
		Vec dir(cosf(beta) * cosf(alpha), cosf(beta) * sinf(alpha), sinf(beta));
		dirs.push_back( dir );
		dirs.push_back( -dir );
	}

	const int renderSize = 1024;

	Grid<int> img(w, h);
	Grid<int> tmp(w, h);
	Grid<unsigned short> texData(3 * renderSize, renderSize);
	img.clear();

	Gfx::instance()->makeCurrent();
	QGLFramebufferObject fbo(renderSize, renderSize, QGLFramebufferObject::Depth, GL_TEXTURE_2D, GL_RGB16);

	Mesh::instance()->computeSize();

	float msize = 0.0f;
	for( Mesh *cur = Mesh::instance() ; cur != NULL ; cur = cur->next )
		msize = qMax(msize, cur->size + cur->pos.norm());
	msize *= 2.0f;
	glEnable(GL_CULL_FACE);

	Vec relPos = Mesh::instance()->getRelativePosition(ID);

	fbo.bind();
	glViewport(0, 0, renderSize, renderSize);

	GLuint dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE);
	Mesh::instance()->drawOcclusion(getID());
	glEndList();

	int i = 0;
	float conv = 1.0f / 65535.0f;
	foreach(Vec dir, dirs)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Vector3D FP((msize + 10.0f) * dir);
		Vector3D side = dir ^ Vec(0.0f, 1.0f, 0.0f);
		side.unit();
		Vector3D up = dir ^ side;
		up.unit();
		gluLookAt(FP.x, FP.y, FP.z,
				  0.0f, 0.0f, 0.0f,
				  up.x, up.y, up.z);

		float minx = 0.0f, maxx = 0.0f;
		float miny = 0.0f, maxy = 0.0f;
		for(int e = 0 ; e < vertex.size() ; ++e)
		{
			float x = (vertex[e] + relPos) * side;
			float y = (vertex[e] + relPos) * up;
			if (e == 0 || minx > x)
				minx = x;
			if (e == 0 || maxx < x)
				maxx = x;
			if (e == 0 || miny > y)
				miny = y;
			if (e == 0 || maxy < y)
				maxy = y;
		}

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho(maxx, minx, maxy, miny, -1000.0f, 2.0f * msize + 1000.0f);
		glMatrixMode(GL_MODELVIEW);

		glCallList(dlist);

		glBindTexture(GL_TEXTURE_2D, fbo.texture());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT, texData.pointerToData());

		tmp.clear();
		for(int y = 0 ; y < renderSize ; ++y)
		{
			for(int x = 0 ; x < renderSize ; ++x)
			{
				if (texData(x * 3 + 2, y) == 0)
					continue;
				float fx = texData(x * 3, y) * conv;
				float fy = texData(x * 3 + 1, y) * conv;
				int ix = fx * tmp.getWidth();
				int iy = fy * tmp.getHeight();
				if (ix < 0 || ix >= tmp.getWidth() || iy < 0 || iy >= tmp.getHeight())
					continue;
				tmp(ix, iy) = 1;
			}
		}

		int *in = tmp.pointerToData();
		int *out = img.pointerToData();
		for(int *end = in + img.getWidth() * img.getHeight() ; in != end ; ++in, ++out)
			*out += *in;

		++i;
		ProgressDialog::setProgress(i * 100 / dirs.size());
	}
	fbo.release();
	glDeleteLists(dlist, 1);

	QImage mask(w, h, QImage::Format_RGB888);
	float coef = 255.0f / (dirs.size() / 2);
	for(int y = 0 ; y < h ; ++y)
	{
		for(int x = 0 ; x < w ; ++x)
		{
			int c = qMin(255, int(coef * img(x, h - 1 - y)));
			mask.setPixel(x, y, qRgb(c, c, c));
		}
	}

	tex.push_back(Gfx::instance()->bindTexture(mask));

	ProgressDialog::setProgress(100);
}

void Mesh::drawOcclusion(int id)
{
	glPushMatrix();

	glTranslatef(pos.x, pos.y, pos.z);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if (!vertex.isEmpty())
	{
		glVertexPointer(3, GL_FLOAT, 0, vertex.data());
		QVector<GLfloat> colors;
		if (!tcoord.isEmpty() && (id == -1 || id == ID))
		{
			for(int i = 0 ; i < vertex.size() ; ++i)
			{
				colors << tcoord[i * 2];
				colors << tcoord[i * 2 + 1];
				colors << 1.0f;
			}
		}
		else
		{
			for(int i = 0 ; i < vertex.size() ; ++i)
			{
				colors << 0.0f;
				colors << 0.0f;
				colors << 0.0f;
			}
		}
		glColorPointer(3, GL_FLOAT, 0, colors.data());

		glDisable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);

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
	}
	glDisableClientState(GL_COLOR_ARRAY);

	if (child)
		child->drawOcclusion(id);
	glPopMatrix();
	if (next)
		next->drawOcclusion(id);
}
