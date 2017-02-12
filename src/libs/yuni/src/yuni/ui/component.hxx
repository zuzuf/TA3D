#ifndef __YUNI_UI_COMPONENT_HXX__
# define __YUNI_UI_COMPONENT_HXX__

# include <cassert>


namespace Yuni
{
namespace UI
{

	inline IComponent::IComponent()
		: pLocalID(Yuni::UI::ID::New()),
		pAdapter(nullptr),
		pPosition(50, 50),
		pWidth(50),
		pHeight(50),
		pRefCount(0)
	{}


	inline IComponent::IComponent(float width, float height)
		: pLocalID(Yuni::UI::ID::New()),
		pAdapter(nullptr),
		pPosition(50, 50),
		pWidth((width > 0.f) ? width : 0.f),
		pHeight((height > 0.f) ? height : 0.f),
		pRefCount(0)
	{}


	inline IComponent::IComponent(float x, float y, float width, float height)
		: pLocalID(Yuni::UI::ID::New()),
		pAdapter(nullptr),
		pPosition(x, y),
		pWidth((width > 0.f) ? width : 0.f),
		pHeight((height > 0.f) ? height : 0.f),
		pRefCount(0)
	{}


	template<class T>
	inline IComponent::IComponent(const Point2D<T>& pos, float width, float height)
		: pLocalID(Yuni::UI::ID::New()),
		pAdapter(nullptr),
		pPosition(pos),
		pWidth((width > 0.f) ? width : 0.f),
		pHeight((height > 0.f) ? height : 0.f),
		pRefCount(0)
	{}


	inline IComponent::~IComponent()
	{
		assert(0 == pRefCount);
	}


	inline IComponent::ID IComponent::id()
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pLocalID;
	}


	inline float IComponent::width() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pWidth;
	}


	inline float IComponent::height() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pHeight;
	}


	inline Point2D<float> IComponent::position() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pPosition;
	}


	inline void IComponent::size(float& width, float& height) const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		width  = pWidth;
		height = pHeight;
	}


	inline float IComponent::x() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pPosition.x;
	}


	inline float IComponent::y() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pPosition.y;
	}


	inline void IComponent::resizeWL(float&, float&)
	{
		// do nothing
	}


	inline void IComponent::addRef()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		//++pRefCount;
		// FIXME This value is artificially increased due to a design flaw in smartptr
		pRefCount += 50;
	}


	inline void IComponent::detachWL()
	{
	}




} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_COMPONENT_HXX__
