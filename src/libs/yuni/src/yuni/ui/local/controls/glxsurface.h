#ifndef __YUNI_UI_LOCAL_CONTROLS_GLXSURFACE_H__
# define __YUNI_UI_LOCAL_CONTROLS_GLXSURFACE_H__

# include "../../../yuni.h"
# include "glsurface.h"
# include "../../../core/system/x11.hdr.h"


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{

	/*!
	** \brief Surface for OpenGL Rendering under X11 (with GLX)
	*/
    class GLXSurface: public GLSurface
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		GLXSurface(::Display* display);
		//! Destructor
		virtual ~GLXSurface();
		//@}

		//! Initialize the surface
		virtual bool initialize();

		//! Destroy the surface
		virtual void destroy();

		//! Set whether the surface must be shown in full screen
		virtual void fullScreen(bool isFullScreen);

	protected:
		//! Make this GL context the current one
		void makeCurrent();

	protected:
		//! X11 Display
		::Display* pDisplay;
		//! Screen ID
		int pScreen;
		//! Window
		::Window pWindow;
		//! Context
		GLXContext pContext;
		XSetWindowAttributes pAttr;
		XWindowAttributes pWndAttr;
		XEvent pXEvent;

	}; // class WGLSurface




} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_CONTROLS_WGLSURFACE_H__
