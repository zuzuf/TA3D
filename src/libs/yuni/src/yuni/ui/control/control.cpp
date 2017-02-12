
#include "control.h"
#include "controlcontainer.h"


namespace Yuni
{
namespace UI
{

	IControl::IControl()
		:pParent(nullptr),
		pDepth(0)
	{}


	IControl::IControl(IControl::Ptr newParent)
		:pParent(nullptr)
	{
		parentWL(newParent);
	}


	IControl::IControl(float width, float height)
		:IComponent(width, height),
		pParent(nullptr),
		pDepth(0)
	{}


	IControl::IControl(IControl::Ptr newParent, float width, float height)
		:IComponent(width, height),
		pParent(nullptr),
		pDepth(0)
	{
		parentWL(newParent);
	}


	IControl::IControl(float x, float y, float width, float height)
		:IComponent(x, y, width, height),
		pParent(nullptr),
		pDepth(0)
	{}


	IControl::IControl(IControl::Ptr newParent, float x, float y,
		float width, float height)
		:IComponent(x, y, width, height),
		pParent(nullptr)
	{
		parentWL(newParent);
	}


	IControl::~IControl()
	{
		ThreadingPolicy::MutexLocker lock(*this);
		// We are dying, tell the parent we cannot be his child anymore
		if (pParent)
			(*pParent) -= pLocalID;
	}


	void IControl::parentWL(const IControl::Ptr& newParent)
	{
		IControl* newParentAsControl = Ptr::WeakPointer(newParent);
		IControlContainer* newParentAsContainer = dynamic_cast<IControlContainer*>(newParentAsControl);
		if (newParentAsContainer != newParentAsControl || pParent == newParentAsContainer)
			return;

		// If we already had a parent, tell him we do not want to be his child anymore
		if (pParent)
			(*pParent) -= pLocalID;
		if (!newParent)
		{
			pParent = nullptr;
			pDepth = 0;
		}
		else
		{
			pParent = newParentAsContainer;
			pDepth  = 1 + pParent->depth();
			(*pParent) += this;
		}
	}


	void IControl::updateComponentWL(const IComponent::ID& componentID) const
	{
		// If the tree is not rooted in a window / application
		// there is no local representation to update, so give up.
		if (pParent)
			pParent->updateComponentWL(componentID);
	}


	void IControl::update() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		updateComponentWL(pLocalID);
	}


	IControl::Ptr IControl::parent() const
	{
		return (IControl*)pParent;
	}


	IControl::Map IControl::children() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pChildren;
	}


	bool IControl::hasParent() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return NULL != pParent;
	}


	unsigned int IControl::depth() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pDepth;
	}


	inline void IControl::parent(IControl::Ptr newParent)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		parentWL(newParent);
	}



} // namespace UI
} // namespace Yuni

