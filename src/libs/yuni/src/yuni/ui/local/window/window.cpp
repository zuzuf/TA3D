
#include "window.h"

#ifdef YUNI_OS_MAC
	// TODO: Write window creation for Cocoa. For the moment it does nothing
#else
#	ifdef YUNI_OS_WINDOWS
#		include "wingdi.h"
#   else
#		ifdef YUNI_OS_UNIX
#			include "x11.h"
#		else
#			error No window creation available for your platform!
#		endif
#	endif
#endif



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


	IWindow* Create()
	{
		# ifdef YUNI_WINDOWSYSTEM_MSW
		IWindow* wnd = new WinGDI();
		# endif
		# ifdef YUNI_WINDOWSYSTEM_X11
		IWindow* wnd = new X11();
		# endif
		# ifdef YUNI_OS_MAC
		(void) source;
		IWindow* wnd = nullptr; // new Cocoa::CairoWindow(source, 32, false);
		# endif

		// Try to initialize
		if (!wnd || !wnd->initialize())
		{
			wnd->close();
			delete wnd;
			return nullptr;
		}
		return wnd;
	}


} // namespace Window
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni
