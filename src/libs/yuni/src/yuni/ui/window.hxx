#ifndef __YUNI_UI_WINDOW_HXX__
# define __YUNI_UI_WINDOW_HXX__


namespace Yuni
{
namespace UI
{


	template<class StringT>
	inline Window::Window(const StringT& title)
		:pTitle(title), pClosing(false)
	{
		pClass = "window";
	}


	template<class StringT>
	inline Window::Window(const StringT& title, float width, float height)
		:IControlContainer(width, height),
		pTitle(title),
		pClosing(false)
	{
		pClass = "window";
	}


	template<class StringT>
	inline Window::Window(const StringT& newTitle, float x, float y, float width,
		float height)
		:IControlContainer(x, y, width, height),
		pTitle(title),
		pClosing(false)
	{
		pClass = "window";
	}


	template<class StringT, class T>
	inline Window::Window(const StringT& title, const Point2D<T>& pos, float width, float height)
		:IControlContainer(pos, width, height),
		pTitle(title),
		pClosing(false)
	{
		pClass = "window";
	}


	template<class StringT>
	inline void Window::title(const StringT& newTitle)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		// Resetting with the new value
		titleWL(newTitle);
	}


	template<class StringT>
	inline void Window::titleWL(const StringT& newTitle)
	{
		// Resetting with the new value (without lock)
		pTitle = newTitle;
	}





} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_WINDOW_HXX__
