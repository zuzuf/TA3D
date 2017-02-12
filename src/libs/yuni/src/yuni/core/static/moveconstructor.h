#ifndef __YUNI_CORE_STATIC_REF_TO_VALUE_H__
# define __YUNI_CORE_STATIC_REF_TO_VALUE_H__



namespace Yuni
{
namespace Static
{


	/*!
	** \brief Move Constructor (C++ idiom)
	**
	** Transfer the ownership of a resource held by an object to a new object.
	** (Also known as the Colvin-Gibbons trick)
	*/
	template <class T>
	class MoveConstructor
	{
	public:
		MoveConstructor(T& r)
			:pReference(r)
		{}

		MoveConstructor(const MoveConstructor& r)
			:pReference(r.pReference)
		{}

		//! Case-operator
		operator T& () const  {return pReference;}


	private:
		T& pReference;
	};


} // namespace Static
} // namespace Yuni

#endif // __YUNI_CORE_STATIC_REF_TO_VALUE_H__
