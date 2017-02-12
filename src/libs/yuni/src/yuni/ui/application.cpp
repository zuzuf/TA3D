
#include "application.h"
#include <cassert>


namespace Yuni
{
namespace UI
{


	namespace // Anonymous
	{

		IComponent::ID NextID = 0;

	} // namespace Anonymous






	IComponent::ID Application::createID() const
	{
		return NextID++;
	}


	void Application::GenerateGUID(GUID& guid)
	{
		assert(guid.size() == 0 && "The GUID must be empty at this point");

		// FIXME : Use real GUID and not this stupid one (and the mutex must be removed too)
		static int I = 0;
		static Mutex mutex;

		mutex.lock();
		guid = "f81d4fae-7dec-11d0-a765-00a0c91e6bf6";
		CustomString<10, false> t;
		guid.overwriteRight((t << I++));
		mutex.unlock();
	}


	void Application::initialize()
	{
		// Prepare a new GUI for the application
		// The const_cast is quite ugly but after this point, the guid _must_ not be
		// modified
		GenerateGUID(const_cast<GUID&>(pGUID));
	}


	void Application::quit()
	{
		{
			ThreadingPolicy::MutexLocker lock(*this);

			const Window::Map::iterator end = pWindows.end();
			for (Window::Map::iterator it = pWindows.begin(); it != end; ++it)
				it->second->close();
		}
		destroyBoundEvents();
	}


	void Application::show()
	{
		ThreadingPolicy::MutexLocker lock(*this);
		const Window::Map::iterator end = pWindows.end();
		for (Window::Map::iterator it = pWindows.begin(); it != end; ++it)
			it->second->show();
	}


	void Application::add(const Window::Ptr& wnd)
	{
		if (!(!wnd))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows[wnd->id()] = wnd;
		}
	}


	Application& Application::operator += (const Window::Ptr& wnd)
	{
		if (!(!wnd))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows[wnd->id()] = wnd;
		}
		return *this;
	}


	Application& Application::operator += (Window* wnd)
	{
		if (wnd)
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows[wnd->id()] = wnd;
		}
		return *this;
	}


	Application& Application::operator << (const Window::Ptr& wnd)
	{
		if (!(!wnd))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows[wnd->id()] = wnd;
		}
		return *this;
	}


	Application& Application::operator << (Window* wnd)
	{
		if (wnd)
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows[wnd->id()] = wnd;
		}
		return *this;
	}


	void Application::remove(IComponent::ID id)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		pWindows.erase(id);
	}


	void Application::remove(const Window::Ptr& wnd)
	{
		if (!(!wnd))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows.erase(wnd->id());
		}
	}


	Application& Application::operator -= (IComponent::ID id)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		pWindows.erase(id);
		return *this;
	}


	Application& Application::operator -= (Window* wnd)
	{
		if (wnd)
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows.erase(wnd->id());
		}
		return *this;
	}


	Application& Application::operator -= (const Window::Ptr& wnd)
	{
		if (!(!wnd))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pWindows.erase(wnd->id());
		}
		return *this;
	}


	Application::Application()
	{
		initialize();
	}


	Application::~Application()
	{
		quit();
	}


	const GUID& Application::guid() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pGUID;
	}


	const String& Application::name() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pName;
	}




} // namespace UI
} // namespace Yuni
