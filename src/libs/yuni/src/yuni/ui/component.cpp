
#include "component.h"
#include "adapter/localforvirtual.h"

namespace Yuni
{
namespace UI
{


	void IComponent::resize(float width, float height)
	{
		ThreadingPolicy::MutexLocker lock(*this);

		// The new size
		float nwWidth  = (width  > 0.f) ? width  : 0.f;
		float nwHeight = (height > 0.f) ? height : 0.f;

		resizeWL(nwWidth, nwHeight);

		// assigning the new size.
		pWidth  = nwWidth;
		pHeight = nwHeight;
	}


	void IComponent::release()
	{
		{
			ThreadingPolicy::MutexLocker locker(*this);
			if (--pRefCount > 0)
				return;
			detachWL();
		}
		delete this;
	}



} // namespace UI
} // namespace Yuni

