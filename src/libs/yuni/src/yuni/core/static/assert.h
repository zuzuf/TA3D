#ifndef __YUNI_CORE_STATIC_ASSERT_H__
# define __YUNI_CORE_STATIC_ASSERT_H__

# include "../preprocessor/std.h"


/*!
** \def YUNI_STATIC_ASSERT(X,ID)
** \brief Assert at compile time
**
**
** YUNI_STATIC_ASSERT is like assert for C/C++ (man 3 assert), except that the
** test is done at compile time.
**
** Here is an example to produce an error to prevent compilation from a missing
** implementation :
** \code
** #include <iostream>
** #include <yuni/core/static/assert.h>
**
**
** template<typename T>
** struct Dummy
** {
** 	void foo()
** 	{
** 		YUNI_STATIC_ASSERT(false, TheImplementationIsMissing);
** 	}
** };
**
** template<>
** struct Dummy<int>
** {
** 	void foo()
** 	{
** 		std::cout << "Called Dummy<int>::foo()" << std::endl;
** 	}
** };
**
** int main()
** {
** 	// As the static assert is within a method and not the class itself,
** 	// it is possible to instanciate the class in all cases.
** 	// But this is not true for the method `foo()`.
**
** 	Dummy<int> dummy0;
** 	// The following statement will compile
** 	dummy0.foo();
**
** 	Dummy<unsigned int> dummy1;
** 	// But this one will fail
** 	dummy1.foo();
**
** 	// Error with gcc 3.x :
** 	// ./main.cpp: In member function `void Dummy<T>::foo() [with T = unsigned int]':
** 	//	./main.cpp:36:   instantiated from here
** 	//	./main.cpp:11: error: creating array with size zero (` Assert_TheImplementationIsMissing')
**
** 	// Error with gcc 4.x :
** 	// ./main.cpp: In member function ‘void Dummy<T>::foo() [with T = unsigned int]’:
** 	//	./main.cpp:36:   instantiated from here
** 	//	./main.cpp:11: error: creating array with negative size (‘(StaticAssert_TheImplementationIsMissing::._67)-0x000000001’)
** 	return 0;
** }
** \endcode
**
** This macro might be used anywhere.
** <br />
** When using several static asserts within the same scope, each ID must be
** unique.
**
** \param X An expression
** \param ID An unique ID for the scope
*/
# define YUNI_STATIC_ASSERT(X,ID)  \
		struct StaticAssert_##ID { \
			enum { Assert_##ID = Yuni::Static::Assert<(0 != (X))>::result }; \
		}; \
		typedef char invokeStaticAssert_##ID[StaticAssert_##ID::Assert_##ID]




namespace Yuni
{
namespace Static
{

	template<int N> struct Assert   { enum {result =  1}; }; // No error

	template<> struct Assert<false> { enum {result = -1}; }; // Error


} // namespace Static
} // namespaec Yuni

#endif // __YUNI_CORE_STATIC_ASSERT_H__
