
#include <iostream>
#include "../../../core/string.h"
#include "glxsurface.h"

namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{


	GLXSurface::GLXSurface(::Display* display):
		pDisplay(display)
	{}


	GLXSurface::~GLXSurface()
	{}


	bool GLXSurface::initialize()
	{
		// Connect to the X server to open the local display
		if (!(pDisplay = XOpenDisplay(NULL)))
			return false;

		// Attributes
		int attributes[] =
			{
				GLX_RGBA,
				GLX_DEPTH_SIZE, 24,
				GLX_STENCIL_SIZE, 8,
				GLX_ALPHA_SIZE, 8,
				GLX_DOUBLEBUFFER,
				YUNI_X11LIB_NONE // end of list
			};
		XVisualInfo* vinfo = glXChooseVisual(pDisplay, DefaultScreen(pDisplay), attributes);
		if (NULL == vinfo)
		{
			std::cerr << "ERROR: Cannot open window" << std::endl;
			return false;
		}
		::Window root = DefaultRootWindow(pDisplay);

		pAttr.colormap = XCreateColormap(pDisplay, root, vinfo->visual, AllocNone);
		pAttr.background_pixel = BlackPixel(pDisplay, vinfo->screen);
		pAttr.border_pixel = BlackPixel(pDisplay, vinfo->screen);
		pAttr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;

		pWindow = XCreateWindow(pDisplay, root, 30, 30,
			(unsigned int)pRect.width(), (unsigned int)pRect.height(), 0,
			vinfo->depth, CopyFromParent, vinfo->visual,
			CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &pAttr);

		XMapWindow(pDisplay, pWindow);

		pContext = ::glXCreateContext(pDisplay, vinfo, NULL, True);
		if (NULL == pContext)
		{
			std::cerr << "glXCreateContext failed" << std::endl;
			return false;
		}

		if (!glXMakeCurrent(pDisplay, pWindow, pContext))
		{
			std::cerr << "DRI not available" << std::endl;
			return false;
		}
		resize(pRect.width(), pRect.height());
		if (!GLSurface::initialize())
		{
			std::cerr << "GL initialization failed" << std::endl;
			return false;
		}
		// Window Attributes
		XGetWindowAttributes(pDisplay, pWindow, &pWndAttr);
		return true;
	}


	void GLXSurface::fullScreen(bool isFullScreen)
	{
		isFullScreen = isFullScreen;
		// TODO
	}


	void GLXSurface::destroy()
	{
	}


	void GLXSurface::makeCurrent()
	{
		glXMakeCurrent(pDisplay, pWindow, pContext);
	}


} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni
