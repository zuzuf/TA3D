#ifndef __YUNI_APPLICATION_GFX_3D_HXX__
# define __YUNI_APPLICATION_GFX_3D_HXX__


namespace Yuni
{
namespace Application
{

	inline Gfx3D::Gfx3D(int argc, char* argv[])
		:IApplication(argc, argv),
		pTitle("Loading...")
	{
	}


	inline Gfx3D::~Gfx3D()
	{
		// Ensures all notifiers are no longer linked with this class
		destroyBoundEvents();
	}


	inline String Gfx3D::title() const
	{
		ThreadingPolicy::MutexLocker lock(*this);
		return pTitle;
	}


	template<class StringT>
	inline void Gfx3D::title(const StringT& newTitle)
	{
		ThreadingPolicy::MutexLocker lock(*this);
		pTitle = newTitle;
	}



} // namespace Application
} // namespace Yuni


#endif // __YUNI_APPLICATION_GFX_3D_HXX__
