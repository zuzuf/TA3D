
#include "desktop.h"

namespace Yuni
{
namespace UI
{


	Desktop::Desktop()
	{
	}


	Desktop::~Desktop()
	{
		destroyBoundEvents();
	}


	void Desktop::add(const Application::Ptr& app)
	{
		if (!(!app))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pApps[app->guid()] = app;
		}
	}


	Desktop& Desktop::operator += (const Application::Ptr& app)
	{
		if (!(!app))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pApps[app->guid()] = app;
		}
		return *this;
	}


	Desktop& Desktop::operator << (const Application::Ptr& app)
	{
		if (!(!app))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pApps[app->guid()] = app;
		}
		return *this;
	}


	Desktop& Desktop::operator -= (const Application::Ptr& app)
	{
		if (!(!app))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pApps.erase(app->guid());
		}
		return *this;
	}


	void Desktop::remove(const Application::Ptr& app)
	{
		if (!(!app))
		{
			ThreadingPolicy::MutexLocker lock(*this);
			pApps.erase(app->guid());
		}
	}


	void Desktop::quit()
	{
		ThreadingPolicy::MutexLocker lock(*this);
		const Application::Map::iterator end = pApps.end();
		for (Application::Map::iterator it = pApps.begin(); it != end; ++it)
			(it->second)->quit();
	}


	void Desktop::show()
	{
		ThreadingPolicy::MutexLocker lock(*this);
		const Application::Map::iterator end = pApps.end();
		for (Application::Map::iterator it = pApps.begin(); it != end; ++it)
			(it->second)->show();
	}




} // namespace UI
} // namespace Yuni
