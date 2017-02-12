
# include "wglsurface.h"


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{


	WGLSurface::WGLSurface(HWND handle):
		pHWnd(handle),
		pDC(NULL),
		pRC(NULL)
	{
	}


	WGLSurface::~WGLSurface()
	{
		destroy();
	}


	bool WGLSurface::initialize()
	{
		// Did We Get A Device Context?
		if (!(pDC = GetDC(pHWnd)))
		{
			destroy();
			MessageBox(NULL, "Can't create a GL device context.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		static PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof (PIXELFORMATDESCRIPTOR), 1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA, (BYTE)pBitDepth, 0, 0, 0, 0, 0, 0,        // Color Bits Ignored
				// No Alpha Buffer
				0,
				// Shift Bit Ignored
				0,
				// No Accumulation Buffer
				0,
				// Accumulation Bits Ignored
				0, 0, 0, 0,
				// 16Bit Z-Buffer (Depth Buffer)
				16,
				// No Stencil Buffer
				0,
				// No Auxiliary Buffer
				0,
				// Main Drawing Layer
				PFD_MAIN_PLANE,
				// Reserved
				0,
				// Layer Masks Ignored
				0, 0, 0
			};

		unsigned int pixelFormat = ChoosePixelFormat(pDC, &pfd);
		// Did Windows find a matching pixel format?
		if (!pixelFormat)
		{
			destroy();
			MessageBox(NULL, "Can't find a suitable PixelFormat.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
		// Are We Able To Set The Pixel Format?
		if (!SetPixelFormat(pDC, pixelFormat, &pfd))
		{
			destroy();
			MessageBox(NULL, "Can't set the PixelFormat.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// Are We Able To Get A Rendering Context?
		if (!(pRC = wglCreateContext(pDC)))
		{
			destroy();
			MessageBox(NULL, "Can't create a GL rendering context.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// Try To Activate The Rendering Context
		if (!wglMakeCurrent(pDC, pRC))
		{
			destroy();
			MessageBox(NULL, "Can't activate the GL Rendering Context.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// Show the window
		ShowWindow(pHWnd, SW_SHOW);
		// Slightly higher priority
		SetForegroundWindow(pHWnd);
		// Sets keyboard focus to the window
		SetFocus(pHWnd);

		// Set up our perspective GL screen
		GLSurface::resize(pRect.width(), pRect.height());

		// Initialize our newly created GL window
		if (!GLSurface::initialize())
		{
			destroy();
			MessageBox(NULL, "Initialization failed.", "GL Initialization Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
		return true;
	}


	void WGLSurface::destroy()
	{
		// Do We Have A Rendering Context?
		if (pRC)
		{
			// Are We Able To Release The DC And RC Contexts?
			if (!wglMakeCurrent(NULL, NULL))
				MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
			// Are We Able To Delete The RC?
			if (!wglDeleteContext(pRC))
				MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
			pRC = NULL;
		}
		// Are We Able To Release The DC?
		if (pDC && !ReleaseDC(pHWnd, pDC))
		{
			MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
			pDC = NULL;
		}
	}


	void WGLSurface::refresh()
	{
		::SwapBuffers(pDC);
	}


	void WGLSurface::makeCurrent()
	{
		// If the surface is not initialized, it will just call :
		// wglMakeCurrent(NULL, NULL) which is okay.
		wglMakeCurrent(pDC, pRC);
	}



} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni
