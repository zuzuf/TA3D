
#include "x11.h"


#ifdef YUNI_WINDOWSYSTEM_X11


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{
namespace Window
{


	bool X11::initialize()
	{
		// Connect to the X server to open the local display
		if (!(pDisplay = XOpenDisplay(NULL)))
			return false;

		int screen = DefaultScreen(pDisplay);
		int depth;
		Visual* visual = FindARGBVisual(pDisplay, screen, depth);
		::Window root = DefaultRootWindow(pDisplay);

		XSetWindowAttributes attributes;
		attributes.colormap = XCreateColormap(pDisplay, root, visual, AllocNone);
		attributes.background_pixel = BlackPixel(pDisplay, screen);
		attributes.border_pixel = BlackPixel(pDisplay, screen);
		attributes.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;

		if (pFullScreen)
		{
			attributes.override_redirect = True;
		}

		pWindow = XCreateWindow(pDisplay, root, (int)pLeft, (int)pTop,
			(unsigned int)pWidth, (unsigned int)pHeight, 0,
			depth, CopyFromParent, visual,
			CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &attributes);

		XMapWindow(pDisplay, pWindow);
		XStoreName(pDisplay, pWindow, pCaption.c_str());

		XWindowAttributes wndAttr;
		XGetWindowAttributes(pDisplay, pWindow, &wndAttr);
		return true;
	}


	Visual* X11::FindARGBVisual(Display* dpy, int screen, int& depth)
	{
		XRenderPictFormat* format;

		XVisualInfo tmplate;
		tmplate.screen = screen;
		tmplate.depth = 32;
		// Temporarily removed, does not compile with a C++ compiler...
		//	tmplate.class = TrueColor;

		int	nvi;
		XVisualInfo* xvi = XGetVisualInfo (dpy, VisualScreenMask | VisualDepthMask | VisualClassMask, &tmplate, &nvi);
		if (!xvi)
			return 0;

		Visual* visual = 0;
		for (int i = 0; i < nvi; ++i)
		{
			format = XRenderFindVisualFormat(dpy, xvi[i].visual);
			if (format->type == PictTypeDirect && format->direct.alphaMask)
			{
				visual = xvi[i].visual;
				break;
			}
		}

		depth = xvi->depth;

		XFree(xvi);
		return visual;
	}



	void X11::move(float left, float top)
	{
		pLeft = left;
		pTop = top;
		XMoveWindow(pDisplay, pWindow, (int)left, (int)top);
	}


	void X11::moveRelative(float left, float top)
	{
		pLeft += left;
		pTop += top;
		XMoveWindow(pDisplay, pWindow, (int)left, (int)top);
	}


	void X11::resize(float width, float height)
	{
		pWidth = width;
		pTop = height;
		XResizeWindow(pDisplay, pWindow, (unsigned int)width, (unsigned int)height);
	}


	void X11::show()
	{
		// TODO
	}


	void X11::hide()
	{
		// TODO
	}


	void X11::minimize()
	{
		// TODO
	}


	void X11::maximize()
	{
		// TODO
	}


	void X11::restore()
	{
		// TODO
	}


	void X11::bringToFront()
	{
		XRaiseWindow(pDisplay, pWindow);
	}


	void X11::sendToBack()
	{
		XLowerWindow(pDisplay, pWindow);
	}


	bool X11::pollEvents()
	{
		XEvent event;

		// Event loop
		while (XPending(pDisplay) > 0)
		{
			XNextEvent(pDisplay, &event);
			switch (event.type)
			{
				case Expose:
					break;

				case ConfigureNotify:
					{
						// Resize : Only if our window-size changed
						if ((int)pWidth != event.xconfigure.width
							|| (int)pHeight != event.xconfigure.height)
							onResize((float)event.xconfigure.width, (float)event.xconfigure.height);
						// Move: Only if our window position changed
						if ((int)pLeft != event.xconfigure.x
							|| (int)pTop != event.xconfigure.y)
							onMove((float)event.xconfigure.x, (float)event.xconfigure.y);
						break;
					}

				// Close
				case ClientMessage:
					{
						if (*XGetAtomName(pDisplay, event.xclient.message_type) == *"WM_PROTOCOLS")
						{
							onClose();
							return true;
						}
						break;
					}
				default:
					break;
			}
		}

		return false;
	}


	void X11::doRefresh()
	{
		XClearWindow(pDisplay, pWindow);
	}


	void X11::doRefreshRect(float left, float top, float width, float height)
	{
		// Last argument as "false" means no Expose event will be generated
		XClearArea(pDisplay, pWindow, (int)left, (int)top, (unsigned int)width, (unsigned int)height, false);
	}


	void X11::doUpdateStyle()
	{
		// TODO : translate pStyleSet to the correct values
		unsigned long valueMask;
		XSetWindowAttributes attributes;

		XChangeWindowAttributes(pDisplay, pWindow, valueMask, &attributes);
	}


	void X11::doUpdateStayOnTop()
	{
		// This code uses EWMH (Extended Window Manager Hints)
		// We add a _NET_WM_STATE_ABOVE state to the window
		Atom _NET_WM_STATE_ABOVE = XInternAtom(pDisplay, "_NET_WM_STATE_ABOVE", false);
		int op = pStayOnTop ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
		netwm_set_state(pDisplay, pWindow, op, _NET_WM_STATE_ABOVE);
	}


	void X11::doUpdateFullScreen()
	{
		// This code uses EWMH (Extended Window Manager Hints)
		// We add a _NET_WM_STATE_FULLSCREEN state to the window
		Atom _NET_WM_STATE_FULLSCREEN = XInternAtom(pDisplay, "_NET_WM_STATE_FULLSCREEN", false);
		int op = pFullScreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
		netwm_set_state(pDisplay, pWindow, op,_NET_WM_STATE_FULLSCREEN);
	}



} // namespace Window
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni


#endif // YUNI_WINDOWSYSTEM_X11
