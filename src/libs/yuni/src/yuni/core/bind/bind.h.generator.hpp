#ifndef __YUNI_CORE_BIND_BIND_H__
# define __YUNI_CORE_BIND_BIND_H__

# include "../../yuni.h"
# include "../../thread/policy.h"
# include "../static/types.h"
# include "../static/assert.h"
# include "../static/remove.h"
# include "../smartptr.h"
# include "traits.h"
# include "../dynamiclibrary/symbol.h"

<%
require File.dirname(__FILE__) + '/../../../tools/generators/commons.rb'
generator = Generator.new()
%>
<%=generator.thisHeaderHasBeenGenerated("bind.h.generator.hpp")%>


namespace Yuni
{


	/*!
	** \brief A delegate implementation
	**
	** How to bind a mere function :
	** \code
	** #include <iostream>
	** #include <yuni/bind.h>
	**
	** static int Foo(int value)
	** {
	** 	std::cout << "Foo: " << value << std::endl;
	** 	return 0;
	** }
	**
	** int main()
	** {
	** 	Yuni::Bind<int (int)> callback;
	** 	callback.bind(&Foo);
	** 	callback(42);
	** 	callback(61);
	** 	callback(-1)
	** 	return 0;
	** }
	** \endcode
	**
	** How to bind a member of an object :
	** \code
	** #include <iostream>
	** #include <yuni/bind.h>
	**
	** class Foo
	** {
	** public:
	**	int bar(int value)
	**	{
	** 		std::cout << "Foo::bar  : " << value << std::endl;
	** 		return 0;
	**	}
	**	int bar2(int value)
	**	{
	** 		std::cout << "Foo::bar2 : " << value << std::endl;
	** 		return 0;
	**	}
	** };
	**
	** int main()
	** {
	**	Foo foo;
	** 	Yuni::Bind<int (int)> callback;
	** 	callback.bind(foo, &Foo::bar);
	** 	callback(42);
	** 	callback(61);
	** 	callback(-1)
	** 	callback.bind(foo, &Foo::bar2);
	** 	callback(42);
	** 	callback(61);
	** 	callback(-1)
	** 	return 0;
	** }
	** \endcode
	**
	** This class is thread-safe, this is guaranteed by the use of smartptr.
	**
	** \note This class does not take care of deleted objects. It is the responsibility
	** of the user to unbind the delegate before the linked object is delete and/or
	** to not invoke the delegate when the object does not exist.
	**
	** \note It is safe to provide a null pointer when binding the delegate
	** \note It is always safe to invoke the delegate when unbound.
	**
	** \tparam P The prototype of the targetted function/member
	*/
	template<class P = void (), class Dummy = void>
	class Bind
	{
	public:
		// This class can not be used like that. We must use one the specialization
		// (see below).
		// All definitions in this class are only here for the documentation
		YUNI_STATIC_ASSERT(false, Bind_BadFunctionOrMemberSignature);

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		Bind();
		/*!
		** \brief Copy constructor
		*/
		Bind(const Bind& rhs);
		/*!
		** \brief Copy constructor
		*/
		Bind(const Yuni::DynamicLibrary::Symbol& symbol);
		/*!
		** \brief Destructor
		*/
		~Bind();
		//@}

	}; // class Bind<>














	//
	// --- Specializations for Bind<> ---
	//


<%
(0..(generator.argumentCount)).each do |i|
[ ["class R" + generator.templateParameterList(i), "R ("+generator.list(i) + ")", "void"],
  ["class R" + generator.templateParameterList(i), "R (*)(" + generator.list(i) + ")", "void"],
  ["class ClassT, class R" + generator.templateParameterList(i), "R (ClassT::*)(" + generator.list(i) + ")", "ClassT"] ].each do |tmpl|
%>

	/*
	** \brief Bind to a function/member with <%=generator.xArgumentsToStr(i)%> (Specialization)
	*/
	template<<%=tmpl[0]%>>
	class Bind<<%=tmpl[1]%>, <%=tmpl[2]%>>
	{
	public:
		//! The Bind Type
		typedef Bind<<%=tmpl[1]%>, <%=tmpl[2]%>> Type;
		//! The Bind Type
		typedef Bind<<%=tmpl[1]%>, <%=tmpl[2]%>> BindType;

<%=generator.include("yuni/core/bind/bind.h.generator.commonstypes.hpp", i) %>

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		Bind();
		/*!
		** \brief Copy constructor
		*/
		Bind(const Bind& rhs);
		/*!
		** \brief Constructor from a library symbol
		*/
		Bind(const Yuni::DynamicLibrary::Symbol& symbol);
		/*!
		** \brief Destructor
		*/
		~Bind();
		//@}

		//! \name Bind
		//@{
		/*!
		** \brief Bind to a function
		**
		** \param pointer A pointer-to-function
		*/
		void bind(R (*pointer)(<%=generator.list(i)%>));


		/*!
		** \brief Bind to a function with a custom and persistent user data
		**
		** \tparam U The type of the user data
		** \param pointer A pointer-to-function
		** \param userdata The userdata that will be copied and stored
		*/
		template<class U>
		void bind(R (*pointer)(<%=generator.list(i,'A', "", ", ")%>U), typename WithUserData<U>::ParameterType userdata);

		/*!
		** \brief Bind to an object member
		**
		** \tparam C Any class
		** \param c A pointer to an object (can be null)
		** \param member A pointer-to-member
		*/
		template<class C> void bind(C* c, R (C::*member)(<%=generator.list(i)%>));

		/*!
		** \brief Bind to an object member
		**
		** \tparam C Any smartptr
		** \param c A pointer to an object (can be null)
		** \param member A pointer-to-member
		*/
		template<class C,
			template <class> class OwspP, template <class> class ChckP, class ConvP,
			template <class> class StorP, template <class> class ConsP>
		void bind(const SmartPtr<C, OwspP,ChckP,ConvP,StorP,ConsP> c, R (C::*member)(<%=generator.list(i)%>));
		template<class C,
			template <class> class OwspP, template <class> class ChckP, class ConvP,
			template <class> class StorP, template <class> class ConsP>
		void bind(const SmartPtr<C, OwspP,ChckP,ConvP,StorP,ConsP> c, R (C::*member)(<%=generator.list(i)%>) const);

		/*!
		** \brief Bind to a const object member
		**
		** \tparam C Any class
		** \param c A pointer to an object (can be null)
		** \param member A pointer-to-member
		*/
		template<class C> void bind(const C* c, R (C::*member)(<%=generator.list(i)%>) const);


		/*!
		** \brief Bind to an object member with a custom and persistent user data
		**
		** \tparam U The type of the user data
		** \tparam C Any class
		** \param c A pointer to an object (can be null)
		** \param member A pointer-to-member
		*/
		template<class U, class C>
		void bind(C* c, R (C::*member)(<%=generator.list(i,'A', "", ", ")%>U), typename WithUserData<U>::ParameterType userdata);
		template<class U, class C>
		void bind(const C* c, R (C::*member)(<%=generator.list(i,'A', "", ", ")%>U) const, typename WithUserData<U>::ParameterType userdata);

		/*!
		** \brief Bind to an object member with a custom and persistent user data
		**
		** \tparam C Any smartptr
		** \param c A pointer to an object (can be null)
		** \param member A pointer-to-member
		*/
		template<class U, class C,
			template <class> class OwspP, template <class> class ChckP, class ConvP,
			template <class> class StorP, template <class> class ConsP>
		void bind(const SmartPtr<C, OwspP,ChckP,ConvP,StorP,ConsP> c, R (C::*member)(<%=generator.list(i,'A', "", ", ")%>U),
			typename WithUserData<U>::ParameterType userdata);
		template<class U, class C,
			template <class> class OwspP, template <class> class ChckP, class ConvP,
			template <class> class StorP, template <class> class ConsP>
		void bind(const SmartPtr<C, OwspP,ChckP,ConvP,StorP,ConsP> c, R (C::*member)(<%=generator.list(i,'A', "", ", ")%>U) const,
			typename WithUserData<U>::ParameterType userdata);


		/*!
		** \brief Bind from another Bind object
		*/
		void bind(const Bind& rhs);

		/*!
		** \brief Bind from a library symbol
		*/
		void bind(const Yuni::DynamicLibrary::Symbol& symbol);
		//@} // Bind


		//! \name Unbind
		//@{
		/*!
		** \brief Unbind
		**
		** It is safe to call this method several times
		*/
		void unbind();

		//! \see unbind
		void clear();
		//@}


		//! \name Invoke
		//@{
		/*!
		** \brief Invoke the delegate
		**
		** The operator () can be used instead.
		*/
		R invoke(<%=generator.variableList(i)%>) const;

		/*!
		** \brief Invoke the bind using a getter for the arguments.
		**
		** Nothing will happen if the pointer is null
		** However, the returned value may not be what we shall expect
		** (the default constructor of the returned type is used in this case).
		*/
		template<class UserTypeT, template<class UserTypeGT, class ArgumentIndexTypeT> class ArgGetterT>
		R callWithArgumentGetter(UserTypeT userdata) const;
		//@}


		//! \name Print
		//@{
		/*!
		** \brief Print the value to the std::ostream
		*/
		void print(std::ostream& out) const;
		//@}


		//! \name Inheritance
		//@{
		/*!
		** \brief Get the raw pointer to the binded object (if any)
		**
		** If bound to a class, the return value will never be null. There is no way
		** to know statically the type of the object.
		** \warning It is the responsability to the user to use this method with care
		**
		** \return A non-null pointer if bound to a class
		*/
		const void* object() const;

		//! Get if the attached class is a descendant of 'IEventObserverBase'
		bool isDescendantOfIEventObserverBase() const;
		//! Get if the attached class is a real descendant of 'IEventObserverBase'
		bool isDescendantOf(const IEventObserverBase* obj) const;

		/*!
		** \brief Get the pointer to the binded object (if any) cast into IEventObserverBase
		**
		** \warning This method should never be used by the user
		** \return A non-null pointer if bound to a class
		*/
		const IEventObserverBase* observerBaseObject() const;
		//@}


		//! \name Operators
		//@{
		/*!
		** \brief Invoke the delegate
		** \see invoke()
		*/
		R operator () (<%=generator.variableList(i)%>) const;
		//! Assignment with another Bind object
		Bind& operator = (const Bind& rhs);
		//! Assignment with a pointer-to-function
		Bind& operator = (R (*pointer)(<%=generator.list(i)%>));
		//! Assignment with a library symbol
		Bind& operator = (const Yuni::DynamicLibrary::Symbol& symbol);

		//! Comparison with a pointer-to-function
		bool operator == (R (*pointer)(<%=generator.list(i)%>)) const;
		//! Comparison with a pointer-to-object
		template<class U> bool operator == (const U* object) const;
		//@}

	private:
		//! Empty callback when not binded (returns a default value)
		R emptyCallback(<%=generator.list(i)%>);
		//! Empty callback when not binded (returns void)
		void emptyCallbackReturnsVoid(<%=generator.list(i)%>);

	private:
		//! The holder type
		typedef Private::BindImpl::IPointer<R(<%=generator.list(i)%>)> IHolder;

		/*!
		** \brief Pointer to function/member
		** \internal The smartptr is used to guarantee the thread-safety, and to avoid
		** expensive copies
		*/
		SmartPtr<IHolder> pHolder;

		// A friend !
		template<class R1, class B1> friend struct Private::BindImpl::Unbind;

	}; // class Bind<R(<%=generator.list(i,'A')%>)>




<% end end %>






} // namespace Yuni

# include "bind.hxx"


template<class P, class DummyT>
inline std::ostream& operator << (std::ostream& out, const Yuni::Bind<P,DummyT>& rhs)
{
	rhs.print(out);
	return out;
}


// Comparison with any pointer-to-object
template<class U, class P, class DummyT>
inline bool operator == (const U* object, const Yuni::Bind<P,DummyT>& bind)
{
	return (bind == object);
}

// Comparison with any pointer-to-object
template<class U, class P, class DummyT>
inline bool operator != (const U* object, const Yuni::Bind<P,DummyT>& bind)
{
	return (bind != object);
}


<% (0..(generator.argumentCount)).each do |i| %>
template<class R, class P, class DummyT<%=generator.templateParameterList(i)%>>
inline bool operator == (R (*pointer)(<%=generator.list(i)%>), const Yuni::Bind<P,DummyT>& bind)
{
	return (bind == pointer);
}

template<class R, class P, class DummyT<%=generator.templateParameterList(i)%>>
inline bool operator != (R (*pointer)(<%=generator.list(i)%>), const Yuni::Bind<P,DummyT>& bind)
{
	return (bind != pointer);
}


<% end %>
#endif // __YUNI_CORE_BIND_BIND_H__
