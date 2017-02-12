#ifndef __YUNI_UI_APPLICATION_HXX__
# define __YUNI_UI_APPLICATION_HXX__


namespace Yuni
{
namespace UI
{


	template<class StringT>
	inline Application::Application(const StringT& name)
		:pName(name)
	{
		initialize();
	}





} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_APPLICATION_HXX__
