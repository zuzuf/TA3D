/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005  Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_XX_I18N_H__
# define __TA3D_XX_I18N_H__

# include <yuni/yuni.h>
# include <yuni/core/smartptr/smartptr.h>
# include <stdafx.h>
# include <misc/string.h>
# include <threads/mutex.h>
# include <misc/tdf.h>
# include <vector>



namespace TA3D
{



	/*!
	** \brief Thread-safe singleton to handle internationalization, provides
	** mainly Dictionary-Based Translation Support Tools
	**
	** Here is the best way to initialize this class. An instance of I18N will be
	** automatically created by I18N::Instance().
	** \code
	**      // Try to find the good language
	**      TA3D::I18N::AutoLanguage();
	** \endcode
	** \see I18N::Instance()
	** \see I18N::tryToDetermineTheLanguage()
	**
	**
	**
	** Get the current language
	** \code
	**      std::cout << "The current language is :" << TA3D::I18N::CurrentLanguage()->caption() << std::endl;
	** \endcode
	**
	** Set the current language
	** \code
	**      // Set the current language to `japanese`
	**      TA3D::I18N::CurrentLanguage("japanese");
	**      // Set the current language to `japanese` too
	**      TA3D::I18N::CurrentLanguage("日本語");
	**
	**      // The language can be set if its index is known
	**      TA3D::I18N::CurrentLanguage(2); // arbitrary index
	** \endcode
	**
	** Get all available languages
	** \code
	**      typedef std::vector<TA3D::I18N::Language*> AllLanguages;
	**      AllLanguages all;
	**      TA3D::I18N::RetrieveAllLanguages(all);
	**      for (AllLanguages::const_iterator i = all.begin(); i != all.end(); ++)
	**      {
	**          std::cout << "This language is available :" << (*i)->caption()
	**              << " (" << (*i)->englishCaption()
	**              << ", indx:" << (*i)->index() << ")" << std::endl;
	**      }
	** \endcode
	**
	** Find the translation of a keyword
	** \code
	**      std::cout << TA3D::I18N::Translate("Loading the map") << "..." << std::endl;
	** \endcode
	**
	** Load translations from files
	** \code
	**      // From a single file
	**      TA3D::I18N::LoadFromFile("ta3d.po");
	**
	**      // From the resources folders
	**      TA3D::I18N::LoadFromResources();
	** \endcode
	*/
    class I18N : public Mutex, public zuzuf::ref_count
	{
	public:
		//! The most suitable smart pointer for the class
        typedef zuzuf::smartptr<I18N>	Ptr;

	public:
		/*!
		** \brief Informations about a single language
		*/
		class Language
		{
		public:
            typedef QStringList Locales;

		public:
			//! \name Constructor & Destructor
			//@{
			/*!
			** \brief Constructor
			** \param indx Index of the language
			** \param englishID Name of the language in english
			** \param caption Name of the language
			*/
			Language(const int indx, const QString& englishID, const QString& caption);
			//! Constructor by copy
			Language(const Language& l);
			//! Destructor
			~Language() {}
			//@}

			//! Index of this language
			int index() const { return pIndx;}

			//! Name of this language in english
			const QString& englishCaption() const { return pEnglishID; }

			//! Caption of the language
			const QString& caption() const { return pCaption; }

		private:
			//! The index for this language
			int pIndx;
			//! The english name
			QString pEnglishID;
			//! The name translated into this language
			QString pCaption;

		}; // class Language



	public:
		/*!
		** \brief Get a valid instance of the class I18N
		** \return An instance of I18N. This must not be NULL
		*/
		static I18N::Ptr Instance();

		/*!
		** \brief Destroy the singleton
		*/
		static void Destroy();

		/*!
		** \brief Get the selected language
		** \return The current language. Must not be NULL
		**
		** \see I18N::currentLanguage()
		*/
		static const Language* CurrentLanguage();

		/*!
		** \brief Set the current language
		** \param l name of the language
		** \return True if language has been changed, false otherwise
		**
		** \see I18N::currentLanguage(const QString&)
		*/
		static bool CurrentLanguage(const QString& l);

		/*!
		** \brief Try to find the language according the system settings
		**
		** \see I18N::tryToDetermineTheLanguage()
		*/
		static bool AutoLanguage();

		/*!
		** \brief Load translations from a file
		**
		** \param filename The file name to open
		** \return True if the operation succeeded, false otherwise
		**
		** \see I18N::loadFromFile()
		*/
		static bool LoadFromFile(const QString& filename);

		/*!
		** \brief Load translations from files within the resources folders (*.po)
		**
		** Note those .po files use the original TA format and not
		** the GNU Gettext Portable Object format
		**
		** \see I18N::loadFromResources()
		*/
		static bool LoadFromResources();

		/*!
		** \brief Translate a keyword according the current language
		**
		** \param key The key to translate
		** \param defaultValue The default value if the key has not been found
		** \return The translation if the key exists, otherwise the default value is used.
		** If the default value is empty, the content of the key is used
		**
		** \see I18N::translate()
		*/
		static QString Translate(const QString& key, const QString& defaultValue = nullptr);

		/*!
		** \brief Translate a list of keywords
		** \param[in,out] The list of keywords that will be translated
		*/
        static void Translate(QStringList& out);


	public:
		//! Destructor
		~I18N();

		/*!
		** \brief Retrieve a language according its name
		**
		** \param name Name of the language (can be the english version or the translated one)
		** \return A pointer to the language that has been found, NULL otherwise
		*/
		Language* language(const QString& name);

		/*!
		** \brief Try to find the more appropriated language according a given locale
		** \param locale The locale
		** \return The language that has been found, the default one otherwise (must never be null)
		*/
		const Language* languageFromLocal(const QString& locale);

		/*!
		** \brief Get the default language
		*/
		const Language* defaultLanguage();

		/*!
		** \brief Get the current language
		*/
		const Language* currentLanguage();

		/*!
		** \brief Set the current language
		**
		** \param lng The new language to use. Nothing will be done if equals to NULL
		** \return True if language has been changed, false otherwise
		*/
		bool currentLanguage(const Language* lng);

		/*!
		** \brief Set the current language according its name
		**
		** \param name Name of the language (can be the english version or the translated one)
		** \return True if language has been changed, false otherwise
		*/
		bool currentLanguage(const QString& n);

		/*!
		** \brief Try to find out the language according the system settings
		**
		** If the language can not fully determined, the default one will be
		** used.
		**
		** On Unixes systems, this method should ne be considered as thread-safe
		** due to the function getenv().
		**
		** \todo Make a specific implementation for the Windows platform
		** \todo Make a specific implementation for Mac OS X
		** \link http://trac.ta3d.org/ticket/20
		**
		** \return True if language has been changed, false otherwise
		*/
		bool tryToDetermineTheLanguage();

		/*!
		** \brief Get the list of all available languages
		**
		** \param[out] out A copy of the list of all languages.
		*/
		void retrieveAllLanguages(std::vector<I18N::Language>& out);


		/*!
		** \brief Translate a keyword according the current language
		**
		** \return The translation if the key exists, otherwise the default value is used.
		** If the default value is empty, the content of the key is copied
		*/
		QString translate(const QString& key, const QString& defaultValue = nullptr);

		/*!
		** \brief Translate a list of keywords
		** \param[in,out] The list of keywords that will be translated
		*/
        void translate(QStringList& out);


		/*!
		** \brief Load translations from a file
		**
		** \param filename The file name to open
		** \param emptyBefore True to clear all previously loaded translations
		** \param inASCII Must Convert character from ascii
		** \return True if the operation succeeded, false otherwise
		*/
		bool loadFromFile(const QString& filename, const bool emptyBefore = false, const bool inASCII = false);


		/*!
		** \brief Load translations from files within the resources folders (*.po)
		**
		** Note those .po files use the original TA format and not
		** the GNU Gettext Portable Object format
		*/
		bool loadFromResources();


		//! \see translate()
		QString operator [] (const QString& key);

	private:
		/*!
		** \brief Private constructor
		*/
		I18N();

		/*!
		** \brief Reset the prefix according the current language
		*/
		void resetPrefix();

		/*!
		** \brief Insert a new language in the list
		*/
		Language* addNewLanguageWL(const QString& englishID, const QString& translatedName);


	private:
		//! All availables languages
		typedef std::vector<I18N::Language*>  Languages;

	private:
		//! Singleton instance
		static I18N::Ptr pInstance;

		//! The next language id
		int pNextLangID;

		//! All available languages
		Languages pLanguages;
		//! The default language
		Language* pDefaultLanguage;
		//! The current language
		Language* pCurrentLanguage;
		//! The suffix to use to find the good language
		QString pLanguageSuffix;

		//! All translations
		TDFParser pTranslations;

	}; // class I18N




} // namespace TA3D

# include "i18n.hxx"

#endif // __TA3D_XX_I18N_H__
