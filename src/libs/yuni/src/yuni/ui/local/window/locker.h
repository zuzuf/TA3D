
#ifndef __YUNI_UI_LOCAL_WINDOW_LOCKER_H__
# define __YUNI_UI_LOCAL_WINDOW_LOCKER_H__

# include "../../../yuni.h"
# include "../../../core/noncopyable.h"
# include "window.h"


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


	/*!
	** \brief Automatic locker, used to update a window
	**
	** This locker uses the RAII paradigm (Resource Acquisition Is Initialization)
	** It allows multiple updates of the same resource at the same time.
	**
	** This is the proper way to update a window :
	** \code
	**    void changeWindow(IWindow::Ptr wnd)
	**    {
	**        Locker lock(wnd);
	**        wnd->stayOnTop(true);
	**    } // Here the locker gets out of scope and releases the resource.
	** \endcode
	*/
	class Locker : private Yuni::NonCopyable<Locker>
	{
	public:
		//! Explicit constructor, acquires the resource
		explicit Locker(const IWindow::Ptr& window)	:
			pWindow(window)
		{
			pWindow->beginUpdate();
		}

		//! Destructor, releases the resource
		~Locker()
		{
			pWindow->endUpdate();
		}


	private:
		//! The window that is locked for update
		IWindow::Ptr pWindow;
	};





} // namespace Window
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_WINDOW_LOCKER_H__
