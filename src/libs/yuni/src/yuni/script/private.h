#ifndef __YUNI_SCRIPT_SCRIPT_PRIVATE_H__
# define __YUNI_SCRIPT_SCRIPT_PRIVATE_H__


namespace Yuni
{
namespace Private
{
namespace ScriptImpl
{
namespace Bind
{



	/*!
	** \brief Abstract proxy for C++ function binding into languages.
	**
	** \todo document this.
	*/
	struct IBinding
	{
		//! Dtor
		virtual ~IBinding() {}

		//! Returns the contained function's arguement count
		virtual unsigned int argumentCount() const = 0;

		//! Performs the actual function call for Lua implementation
		virtual int performFunctionCall(Script::Lua * /* context */) const = 0;

		// Adding a language ?
		// Insert here more function call overloads for your own script types
	};


	/*!
	** \brief Concrete templated binding implementation.
	*
	** \tparam Fx The right Yuni::Function for the function pointer passed.
	** \tparam ReturnType The passed function return type.
	**
	** This is the only descendant of IBinding. IBinding exists only
	** because we want to be able to call the performFunctionCall method
	** without knowing exactly which instanciation was made of this class.
	*/
	template <class BindT, bool HasReturnType = BindT::hasReturnValue>
	class Binding : public IBinding
	{
	public:
		Binding(const BindT& bind);
		unsigned int argumentCount() const;
		int performFunctionCall(Script::Lua* luaContext) const;

	private:
		//! Callback
		BindT pBind;

	}; // class Binding


	template <class BindT>
	class Binding<BindT, false> : public IBinding
	{
	public:
		Binding(const BindT& bind);
		unsigned int argumentCount() const;
		int performFunctionCall(Script::Lua* luaContext) const;

	private:
		//! Callback
		BindT pBind;

	}; // class Binding





} // namespace Bind
} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni

# include "private.hxx"

#endif // __YUNI_SCRIPT_SCRIPT_PRIVATE_H__
