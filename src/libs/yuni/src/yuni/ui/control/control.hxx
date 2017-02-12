#ifndef __YUNI_UI_CONTROL_CONTROL_HXX__
# define __YUNI_UI_CONTROL_CONTROL_HXX__


namespace Yuni
{
namespace UI
{



	template<class T>
	inline IControl::IControl(Point2D<T>& pos, float width, float height)
		:IComponent(pos, width, height),
		pParent(nullptr),
		pDepth(0)
	{}


	template<class T>
	inline IControl::IControl(IControl::Ptr newParent, const Point2D<T>& pos,
		float width, float height)
		:IComponent(pos, width, height),
		pParent(nullptr)
	{
		parentWL(newParent);
	}



	inline void IControl::detachWL()
	{
		parentWL(nullptr);
	}

	inline void IControl::addChildWL(const IControl::Ptr&)
	{}


	inline bool IControl::removeChildWL(const IControl::Ptr&)
	{
		return false;
	}





} // namespace UI
} // namespacs Yuni

#endif // __YUNI_UI_CONTROL_CONTROL_HXX__
