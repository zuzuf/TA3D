#ifndef __YUNI_UI_CONTROL_CONTROL_CONTAINER_HXX__
# define __YUNI_UI_CONTROL_CONTROL_CONTAINER_HXX__


namespace Yuni
{
namespace UI
{

	template<class T>
	inline IControlContainer::IControlContainer(const Point2D<T>& pos, float width, float height)
		:IControl(pos, width, height)
	{}


	inline IControlContainer::~IControlContainer()
	{}



} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_CONTROL_CONTROL_CONTAINER_HXX__
