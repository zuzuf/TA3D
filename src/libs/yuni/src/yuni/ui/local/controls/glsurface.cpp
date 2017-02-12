
# include "glsurface.h"
# include <GL/gl.h>
# include <GL/glu.h>


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{


	bool GLSurface::initialize()
	{
		// Enable Smooth Shading
		glShadeModel(GL_SMOOTH);
		// Background Clear Color
		glClearColor(0.f, 0.f, 0.f, 0.0f);
		// Depth Buffer Setup
		glClearDepth(1.0f);
		// Enables Depth Testing
		glEnable(GL_DEPTH_TEST);
		// The type of depth testing to do
		glDepthFunc(GL_LEQUAL);
		// Really nice perspective calculations
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		// Flush
		glFlush();
		return true;
	}


	void GLSurface::resize(float width, float height)
	{
		// Prevent A Divide By Zero
		if (height < 1.f)
			height = 1.0f;

		if (width < 1.f)
			width = 1.f;

		// Reset The Current Viewport
		glViewport(0, 0, (GLsizei)width, (GLsizei)height);
		// Select the Projection Matrix
		glMatrixMode(GL_PROJECTION);
		// Reset the Projection Matrix
		glLoadIdentity();
		// Calculate the Aspect Ratio of the window
		gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		// Select the Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		// Reset the Modelview Matrix
		glLoadIdentity();
	}




} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni
