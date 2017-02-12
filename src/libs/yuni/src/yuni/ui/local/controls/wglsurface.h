#ifndef __YUNI_UI_LOCAL_CONTROLS_WGLSURFACE_H__
# define __YUNI_UI_LOCAL_CONTROLS_WGLSURFACE_H__

# include "../../../yuni.h"
# include "glsurface.h"
# include "../../../core/system/windows.hdr.h"


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{

	/*!
	** \brief Surface for OpenGL Rendering under Windows (with WGL)
	*/
    class WGLSurface: public GLSurface
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		WGLSurface(HWND handle);
		//! Destructor
		virtual ~WGLSurface();
		//@}

		//! Initialize the surface
		virtual bool initialize();

		//! Destroy the surface
		virtual void destroy();

		//! Refresh the surface
		virtual void refresh();

	protected:
		//! Make this GL context the current one
		void makeCurrent();

	protected:
		//! Window handle
		HWND pHWnd;

		//! Device context
		HDC pDC;

		//! Rendering context
		HGLRC pRC;

	}; // class WGLSurface




} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_CONTROLS_WGLSURFACE_H__
