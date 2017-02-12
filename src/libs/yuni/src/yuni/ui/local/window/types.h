#ifndef __YUNI_UI_LOCAL_STYLE_H__
# define __YUNI_UI_LOCAL_STYLE_H__

# include "../../../core/color/rgb.h"

namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{

	//! Unique numeric Identifier
	typedef unsigned int ID;


	//! Window Styles
	enum WindowStyle
	{
		wsNone = 0,
		wsModal = 1,
		wsToolWindow = 2,
		wsResizeable = 4,
		wsMinimizable = 8,
		wsMaximizable = 16,
		wsFormMovable = 32,
		wsNoBorder = 64,
		wsNoCaption = 128,
		wsNoCloseButton = 256,
	};


	//! Foreground/Background Color
	typedef Yuni::Color::RGB<float>  Color;



} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_STYLE_H__
