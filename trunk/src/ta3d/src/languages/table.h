#ifndef __TA3D_LANGUAGES_TABLE_H__
# define __TA3D_LANGUAGES_TABLE_H__

# include <yuni/core/string.h>


namespace TA3D
{


	class TranslationTable
	{
	public:
		//! A String with a smaller buffer than usual strings
		typedef Yuni::StringBase<char, 40> ShortString;

	public:
		/*!
		** \brief Update all translations
		*/
		static void Update();

	public:
		//! Translation of `game time`
		static ShortString gameTime;
		//! Translation of `units`
		static ShortString units;
		//! Translation of `speed`
		static ShortString speed;

	};

} // namespace TA3D

#endif // __TA3D_LANGUAGES_TABLE_H__
