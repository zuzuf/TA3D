#ifndef __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_HXX__
# define __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_HXX__


namespace Yuni
{
namespace DynamicLibrary
{


	inline Symbol::Symbol()
		:pPtr(NULL)
	{}


	inline Symbol::Symbol(const Symbol::Handle p)
		:pPtr(p)
	{}


	inline Symbol::Symbol(const Symbol& copy)
		:pPtr(copy.pPtr)
	{}


	inline bool Symbol::null() const
	{
		return (NULL == pPtr);
	}


	inline bool Symbol::valid() const
	{
		return (NULL != pPtr);
	}


	inline Symbol& Symbol::operator = (const Symbol& rhs)
	{
		this->pPtr = rhs.pPtr;
		return *this;
	}


	inline Symbol& Symbol::operator = (const Symbol::Handle hndl)
	{
		this->pPtr = hndl;
		return *this;
	}


	inline Symbol::Handle Symbol::ptr() const
	{
		return pPtr;
	}



} // namespace DynamicLibrary
} // namespace Yuni

#endif // __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_HXX__
