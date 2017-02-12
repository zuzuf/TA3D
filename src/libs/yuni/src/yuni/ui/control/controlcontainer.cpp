
#include "controlcontainer.h"


namespace Yuni
{
namespace UI
{


	IControlContainer::IControlContainer()
	{}


	IControlContainer::IControlContainer(float width, float height)
		:IControl(width, height)
	{}


	IControlContainer::IControlContainer(float x, float y, float width, float height)
		:IControl(x, y, width, height)
	{}



	void IControlContainer::resizeWL(float& newWidth, float& newHeight)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		// TODO: Resize children to fit in the parent's new dimensions
		pWidth = newWidth;
		pHeight = newHeight;
	}


	void IControlContainer::addChildWL(const IControl::Ptr& child)
	{
		pChildren[child->id()] = child;
	}


	bool IControlContainer::removeChildWL(const IControl::Ptr& child)
	{
		return removeChildWL(child->id());
	}


	bool IControlContainer::removeChildWL(IComponent::ID childID)
	{
		const IControl::Map::iterator it = pChildren.find(childID);
		const bool exists = (it != pChildren.end());
		if (exists)
			pChildren.erase(it);
		return exists;
	}



	void IControlContainer::add(const IControl::Ptr& child)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		addChildWL(child);
	}


	IControlContainer& IControlContainer::operator += (const IControl::Ptr& child)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		addChildWL(child);
		return *this;
	}


	IControlContainer& IControlContainer::operator << (const IControl::Ptr& child)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		addChildWL(child);
		return *this;
	}


	bool IControlContainer::remove(IComponent::ID id)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return removeChildWL(id);
	}


	bool IControlContainer::remove(const IControl::Ptr& child)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return removeChildWL(child);
	}


	IControlContainer& IControlContainer::operator -= (IComponent::ID id)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		removeChildWL(id);
		return *this;
	}


	IControlContainer& IControlContainer::operator -= (const IControl::Ptr& child)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		removeChildWL(child);
		return *this;
	}




} // namespace UI
} // namespace Yuni

