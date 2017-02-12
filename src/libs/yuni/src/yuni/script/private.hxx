#ifndef __YUNI_SCRIPT_SCRIPT_PRIVATE_HXX__
# define __YUNI_SCRIPT_SCRIPT_PRIVATE_HXX__

# include "args.hxx"


namespace Yuni
{
namespace Private
{
namespace ScriptImpl
{
namespace Bind
{

	template <class BindT, bool HasReturnType>
	inline Binding<BindT,HasReturnType>::Binding(const BindT& bind)
		:pBind(bind)
	{}



	template<class BindT, bool HasReturnType>
	inline unsigned int Binding<BindT,HasReturnType>::argumentCount() const
	{
		return pBind.argumentCount;
	}


	template<class BindT, bool HasReturnType>
	int Binding<BindT,HasReturnType>::performFunctionCall(Script::Lua* luaContext) const
	{
		Argument<Script::Lua*,typename BindT::ReturnType>::Push(
			luaContext,
			pBind. template callWithArgumentGetter<Script::Lua*, Argument>(luaContext));
		return 1;
	}



	template <class BindT>
	inline Binding<BindT,false>::Binding(const BindT& bind)
		:pBind(bind)
	{}



	template<class BindT>
	unsigned int Binding<BindT,false>::argumentCount() const
	{
		return pBind.argumentCount;
	}


	template<class BindT>
	inline int Binding<BindT,false>::performFunctionCall(Script::Lua* luaContext) const
	{
		pBind. template callWithArgumentGetter<Script::Lua*, Argument>(luaContext);
		return 0;
	}




} // namespace Bind
} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_SCRIPT_SCRIPT_PRIVATE_HXX__
