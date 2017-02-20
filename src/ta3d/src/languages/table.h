#ifndef __TA3D_LANGUAGES_TABLE_H__
# define __TA3D_LANGUAGES_TABLE_H__

# include <QString>


namespace TA3D
{


	class TranslationTable
	{
	public:
		/*!
		** \brief Update all translations
		*/
		static void Update();

	public:
		//! Translation of `game time`
        static QString gameTime;
		//! Translation of `units`
        static QString units;
		//! Translation of `speed`
        static QString speed;

	};

} // namespace TA3D

#endif // __TA3D_LANGUAGES_TABLE_H__
