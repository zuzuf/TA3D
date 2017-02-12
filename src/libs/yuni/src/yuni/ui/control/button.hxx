#ifndef __YUNI_UI_CONTROL_BUTTON_HXX__
# define __YUNI_UI_CONTROL_BUTTON_HXX__


namespace Yuni
{
namespace UI
{

	
	template<class StringT>
	inline Button::Button(const StringT& caption)
		: pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT>
	inline Button::Button(const IControl::Ptr& parent, const StringT& caption)
		: IControl(parent), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT>
	inline Button::Button(const StringT& caption, float width, float height)
		: IControl(width, height), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT>
	inline Button::Button(const IControl::Ptr& parent, const StringT& caption, float width,
		float height)
		: IControl(parent, width, height), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT>
	inline Button::Button(const StringT& caption, float x, float y, float width, float height)
		: IControl(x, y, width, height), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT>
	inline Button::Button(const IControl::Ptr& parent, const StringT& caption, float x, float y,
		float width, float height)
		: IControl(parent, x, y, width, height), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT, class T>
	inline Button::Button(const StringT& caption, Point2D<T>& pos, float width, float height)
		: IControl(pos, width, height), pCaption(caption)
	{
		pClass = "button";
	}


	template<class StringT, class T>
	inline Button::Button(const IControl::Ptr& parent, const StringT& caption, Point2D<T>& pos,
		float width, float height)
		: IControl(parent, pos, width, height), pCaption(caption)
	{
		pClass = "button";
	}



	template<class StringT>
	inline void Button::caption(const StringT& value)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		pCaption = value;
	}




} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_CONTROL_BUTTON_HXX__
