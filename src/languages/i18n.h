#ifndef __TA3D_XX_I18N_H__
# define __TA3D_XX_I18N_H__

# include "../stdafx.h"
# include "../threads/mutex.h"
# include "../cTAFileParser.h"
# include <vector>


namespace TA3D
{



    /*! \class I18N
    **
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
    class I18N
    {
    public:
        /*! \class Language
        **
        ** \brief Informations about a single language
        */
        class Language
        {
        public:
            typedef String::Vector Locales;
        public:
            //! \name Constructor & Destructor
            //@{
            /*!
            ** \brief Constructor
            ** \param indx Index of the language
            ** \param englishID Name of the language in english
            ** \param caption Name of the language
            */
            Language(const int indx, const String& englishID, const String& caption);
            //! Constructor by copy
            Language(const Language& l);
            //! Destructor
            ~Language() {}
            //@}

            //! Index of this language
            const int index() const { return pIndx;}

            //! Name of this language in english
            const String& englishCaption() const { return pEnglishID; }

            //! Caption of the language
            const String& caption() const { return pCaption; }

        private:
            //! The index for this language
            int pIndx;
            //! The english name
            String pEnglishID;
            //! The name translated into this language
            String pCaption;

        }; // class Language



    public:
        /*!
        ** \brief Get a valid instance of the class I18N
        ** \return An instance of I18N. This must not be NULL
        */
        static I18N* Instance();

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
        static const Language* CurrentLanguage() { return I18N::Instance()->currentLanguage(); }

        /*!
        ** \brief Set the current language
        ** \param l name of the language
        ** \return True if language has been changed, false otherwise
        **
        ** \see I18N::currentLanguage(const String&)
        */
        static bool CurrentLanguage(const String& l) { return I18N::Instance()->currentLanguage(l); }

        /*!
        ** \brief Set the current language
        ** \param i index of the language
        ** \return True if language has been changed, false otherwise
        **
        ** \see I18N::currentLanguage(const int)
        */
        static bool CurrentLanguage(const int i) { return I18N::Instance()->currentLanguage(i); }

        /*!
        ** \brief Try to find the language according the system settings
        **
        ** \see I18N::tryToDetermineTheLanguage()
        */
        static bool AutoLanguage() { return I18N::Instance()->tryToDetermineTheLanguage(); }

        /*!
        ** \brief Load translations from a file
        **
        ** \param filename The file name to open
        ** \return True if the operation succeeded, false otherwise
        **
        ** \see I18N::loadFromFile()
        */
        static bool LoadFromFile(const String& filename) { return I18N::Instance()->loadFromFile(filename); }

        /*!
        ** \brief Load translations from files within the resources folders (*.po)
        **
        ** Note those .po files use the original TA format and not
        ** the GNU Gettext Portable Object format
        **
        ** \see I18N::loadFromResources()
        */
        static bool LoadFromResources() { return I18N::Instance()->loadFromResources(); }

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
        static String Translate(const String& key, const String& defaultValue = "")
        { return I18N::Instance()->translate(key, defaultValue); }

        /*!
        ** \brief Translate a list of keywords
        ** \param[in,out] The list of keywords that will be translated
        */
        static void Translate(String::Vector& out) { I18N::Instance()->translate(out);}

        /*!
        ** \brief Translate a list of keywords
        ** \param[in,out] The list of keywords that will be translated
        */
        static void Translate(String::List& out) { I18N::Instance()->translate(out);}



    public:
        //! Destructor
        ~I18N();

        /*!
        ** \brief Retreive a language according its index
        **
        ** \param indx Index the language
        ** \return A pointer to the language that has been found, NULL otherwise
        */
        Language* language(const int indx);

        /*!
        ** \brief Retrieve a language according its name
        **
        ** \param name Name of the language (can be the english version or the translated one)
        ** \return A pointer to the language that has been found, NULL otherwise
        */
        Language* language(const String& name);

        /*!
        ** \brief Try to find the more appropriated language according a given locale
        ** \param locale The locale
        ** \return The language that has been found, the default one otherwise (must never be null)
        */
        const Language* languageFromLocal(const String& locale);

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
        bool currentLanguage(const String& n) { return currentLanguage(language(n)); }

        /*!
        ** \brief Set the current language according its index
        **
        ** \param i index of the language (may be out of bounds)
        ** \return True if language has been changed, false otherwise
        */
        bool currentLanguage(const int i) { return currentLanguage(language(i)); }

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
        String translate(const String& key, const String& defaultValue = "");

        //! \see translate()
        String operator [] (const String& key) { return translate(key); }

        /*!
        ** \brief Translate a list of keywords
        ** \param[in,out] The list of keywords that will be translated
        */
        void translate(String::Vector& out);
    
        /*!
        ** \brief Translate a list of keywords
        ** \param[in,out] The list of keywords that will be translated
        */
        void translate(String::List& out);


        /*!
        ** \brief Load translations from a file
        **
        ** \param filename The file name to open
        ** \param emptyBefore True to clear all previously loaded translations
        ** \param inASCII Must Convert character from ascii
        ** \return True if the operation succeeded, false otherwise
        */
        bool loadFromFile(const String& filename, const bool emptyBefore = false, const bool inASCII = false);


        /*!
        ** \brief Load translations from files within the resources folders (*.po)
        **
        ** Note those .po files use the original TA format and not
        ** the GNU Gettext Portable Object format
        */
        bool loadFromResources();

    private:
        /*!
        ** \brief Private constructor
        */
        I18N();

        /*!
        ** \brief Initialize the list of available languages
        */
        void initializeAllLanguages();

        /*!
        ** \brief Reset the prefix according the current language
        */
        void resetPrefix();

        /*!
        ** \brief Insert a new language in the list
        */
        Language* doAddNewLanguage(const String& englishID, const String& translatedName);

        /*!
        ** \brief Clear the list of languages
        ** \todo std::vector< boost::shared_ptr<> > would be more efficient...
        */
        void doClearLanguages();

    private:
        //! All availables languages
        typedef std::vector<I18N::Language*>  Languages;

    private:
        //! Singleton instance
        static I18N* pInstance;

        //! Mutex
        static Mutex pMutex;
        //! The next language id
        int pNextLangID;
        
        //! All available languages
        Languages pLanguages;
        //! The default language
        Language* pDefaultLanguage;
        //! The current language
        Language* pCurrentLanguage;
        //! The suffix to use to find the good language
        String pLanguageSuffix;

        //! All translations
        UTILS::cTAFileParser pTranslations;

    }; // class I18N




} // namespace TA3D

#endif // __TA3D_XX_I18N_H__
