#ifndef __YUNI_UI_LOCAL_WINDOW_WINDOW_HXX__
# define __YUNI_UI_LOCAL_WINDOW_WINDOW_HXX__

# include "../../../core/string.h"
# include <cassert>

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

	inline IWindow::IWindow():
		pStyleSet(defaultStyleSet),
		pStayOnTop(false),
		pFullScreen(false),
		pBackgroundColor(0.0f, 0.0f, 1.0f),
		pRefreshRefCount(0)
	{
	}


	inline IWindow::~IWindow()
	{
	}


	inline void IWindow::refresh()
	{
		if (!pRefreshRefCount)
			doRefresh();
	}


	inline void IWindow::forceRefresh()
	{
		doRefresh();
	}


	inline void IWindow::beginUpdate()
	{
		++pRefreshRefCount;
	}


	inline void IWindow::endUpdate()
	{
		assert(pRefreshRefCount > 0);
		if (!--pRefreshRefCount)
          doRefresh();
	}


	template<class StringT>
	inline void IWindow::caption(const StringT& newString)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, IWindowCaption_InvalidTypeForBuffer);
		pCaption = newString;
		doUpdateCaption();
	}


	inline void IWindow::style(unsigned int flags)
	{
		pStyleSet = flags;
		doUpdateStyle();
	}


	inline void IWindow::stayOnTop(bool alwaysOnTop)
	{
		pStayOnTop = alwaysOnTop;
		doUpdateStayOnTop();
	}


	inline void IWindow::backgroundColor(const Color& color)
	{
		pBackgroundColor = color;
		refresh();
	}


	inline void IWindow::backgroundColor(float r, float g, float b)
	{
		pBackgroundColor.red = r;
		pBackgroundColor.green = g;
		pBackgroundColor.blue = b;
		refresh();
	}


	inline void IWindow::onMinimize()
	{
		std::cout << "Caught Minimize event !" << std::endl;
	}


	inline void IWindow::onMaximize()
	{
		std::cout << "Caught Maximize event !" << std::endl;
	}


	inline void IWindow::onRestore()
	{
		std::cout << "Caught Restore event !" << std::endl;
	}


	inline void IWindow::onShow()
	{
		std::cout << "Caught Show event !" << std::endl;
	}


	inline void IWindow::onHide()
	{
		std::cout << "Caught Hide event !" << std::endl;
	}


	inline void IWindow::onResize(float width, float height)
	{
		std::cout << "Caught Resize event to (" << width << "," << height << ") !" << std::endl;
	}


	inline void onMove(float left, float top)
	{
		std::cout << "Caught Move event to (" << left << "," << top << ") !" << std::endl;
	}


	inline void IWindow::close()
	{
		onClose();
	}


	inline void IWindow::onCloseQuery(bool& /*canClose*/)
	{
		std::cout << "Caught Close event !" << std::endl;
	}


	inline void IWindow::onClose()
	{
		std::cout << "Caught Close event !" << std::endl;
	}


	inline const String& IWindow::caption() const
	{
		return pCaption;
	}


	inline unsigned int IWindow::style() const
	{
		return pStyleSet;
	}


	inline bool IWindow::stayOnTop() const
	{
		return pStayOnTop;
	}


	inline const Color& IWindow::backgroundColor() const
	{
		return pBackgroundColor;
	}





} // namespace Window
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_WINDOW_WINDOW_HXX__
