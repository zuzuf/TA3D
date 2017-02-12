#ifndef __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_H__
# define __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_H__

# include <iostream>


namespace Yuni
{
namespace System
{

/*!
** \brief API for dealing with the attributes of characters written to the console screen buffer
*/
namespace Console
{

	enum Color
	{
		//! None
		none = 0,
		//! Black
		black,
		//! Red
		red,
		//! Green
		green,
		//! Yellow
		yellow,
		//! Blue
		blue,
		//! Purple (or Magenta on Windows)
		purple,
		//! Lightblue (or cyan on Windows)
		lightblue,
		//! Gray
		gray,
		//! White
		white,
	};



	/*!
	** \brief Set the text color from a static constant
	*/
	template<int C>
	struct TextColor
	{
		template<class U> static void Set(std::ostream& out);
	};

	/*!
	** \brief Set the text color of the console
	**
	** \param[in,out] out An ostream (std::cout or std::cerr)
	** \param color The new color
	*/
	template<class U> void SetTextColor(std::ostream& out, const Color color);

	/*!
	** \brief Reset the text color to its default value
	**
	** \param[in,out] out An ostream (std::cout or std::cerr)
	*/
	template<class U> void ResetTextColor(std::ostream& out);




} // namespace Console
} // namespace System
} // namespace Yuni

# include "console.hxx"

#endif // __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_H__
