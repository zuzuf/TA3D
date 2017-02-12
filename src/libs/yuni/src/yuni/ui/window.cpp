
#include "../yuni.h"
#include "window.h"
#include "adapter/forvirtual.h"


namespace Yuni
{
namespace UI
{

	Window::~Window()
	{
		destroyBoundEvents();
	}


	String Window::title() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pTitle;
	}


	bool Window::closing() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pClosing;
	}


	void Window::updateComponentWL(const IComponent::ID& /*componentID*/) const
	{
		// FIXME
		//pLocalEvents.onUpdateComponent(pApplicationGUID, componentID);
	}


	void Window::show()
	{
 		ThreadingPolicy::MutexLocker lock(*this);
		if (!pClosing && pAdapter)
			pAdapter->sendShowWindow(pApplicationGUID, pLocalID);
	}


	void Window::hide()
	{
 		ThreadingPolicy::MutexLocker lock(*this);
		if (!pClosing && pAdapter)
			pAdapter->sendHideWindow(pApplicationGUID, pLocalID);
	}


	void Window::close()
	{
 		ThreadingPolicy::MutexLocker lock(*this);
		if (!pClosing)
		{
			pClosing = true;
			if (pAdapter)
				pAdapter->sendHideWindow(pApplicationGUID, pLocalID);
 		}
	}


	void Window::resizeWL(float& width, float& height)
	{
		// If nothing has changed, do nothing
		if (Math::Equals(pWidth, width) && Math::Equals(pHeight, height))
			return;

		IControlContainer::resizeWL(width, height);
		// FIXME
// 		if (!pClosing)
// 			pLocalEvents.onResizeWindow(pApplicationGUID, pLocalID, width, height);
	}




} // namespace UI
} // namespace Yuni

