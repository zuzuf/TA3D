
#include "button.h"


namespace Yuni
{
namespace UI
{


	Button::Button()
	{
		pClass = "button";
	}


	Button::Button(const IControl::Ptr& parent)
		: IControl(parent)
	{
		pClass = "button";
	}


	Button::~Button()
	{
		destroyBoundEvents();
	}


	String Button::caption() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return pCaption;
	}




} // namespace UI
} // namespace Yuni

