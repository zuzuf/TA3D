
#include "i18n.h"
#include "../misc/resources.h"
#include "../misc/paths.h"

#define TA3D_LOG_SECTION_I18N_PREFIX "[i18n] "



namespace TA3D
{

    I18N* I18N::pInstance = NULL;
    Mutex I18N::pMutex;



    I18N::Language::Language(const int indx, const String& englishID, const String& caption)
        :pIndx(indx), pEnglishID(englishID), pCaption(caption)
    {}

    I18N::Language::Language(const Language& l)
        :pIndx(l.pIndx), pEnglishID(l.pEnglishID), pCaption(l.pCaption)
    {}

    I18N* I18N::Instance()
    {
        // We use here a double-check lock, which would be good enough
        // for this kind of class
        if (pInstance == NULL)
        {
            MutexLocker locker(pMutex);
            if (pInstance == NULL)
                pInstance = new I18N();
        }
        return pInstance;
    }

    I18N::I18N()
        :pNextLangID(0), pDefaultLanguage(NULL), pCurrentLanguage(NULL),
        pTranslations(8192)
    {
        initializeAllLanguages();
    }

    I18N::~I18N()
    {
        LOG_DEBUG(TA3D_LOG_SECTION_I18N_PREFIX << "Release.");
        doClearLanguages();
    }


    void I18N::Destroy()
    {
        pMutex.lock();
        if (pInstance != NULL)
        {
            delete pInstance;
            pInstance = NULL;
        }
        pMutex.unlock();
    }

    void I18N::resetPrefix()
    {
        pLanguageSuffix = ".";
        pLanguageSuffix += Lowercase(pCurrentLanguage->englishCaption());
    }

    I18N::Language* I18N::doAddNewLanguage(const String& englishID, const String& translatedName)
    {
        I18N::Language* lng = new Language(pNextLangID, englishID, translatedName);
        LOG_ASSERT(NULL != lng);
        ++pNextLangID;
        pLanguages.push_back(lng);
        return lng;
    }

    I18N::Language* I18N::language(const int indx)
    {
        if (indx < 0)
            return NULL;
        MutexLocker locker(pMutex);
        return (indx < (int)pLanguages.size()) ? pLanguages[indx] : NULL;
    }

    I18N::Language* I18N::language(const String& name)
    {
        if (name.empty())
            return NULL;
        MutexLocker locker(pMutex);
        for (Languages::const_iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
        {
            if (name == (*i)->caption() || name == (*i)->englishCaption())
                return (*i);
        }
        return NULL;
    }

    const I18N::Language* I18N::languageFromLocal(const String& locale)
    {
        LOG_ASSERT(NULL != pDefaultLanguage /* initializeAllLanguages() must be called before */ );
        if (locale.empty())
            return pDefaultLanguage;
        String s = Lowercase(TrimString(locale));
        const String::size_type n = String::npos;

        // French
        if (n != s.find("fr_fr") || n != s.find("fr_ca") || n != s.find("fr_ch") || n != s.find("fr_be")
            || n != s.find("fr_eu") || n != s.find("fr_lu"))
            return language("french");

        // German
        if (n != s.find("de_de") || n != s.find("de_ch") || n != s.find("de_be") || n != s.find("de_eu")
            || n != s.find("de_at"))
            return language("german");

        // Spanish
        if (n != s.find("es_ar") || n != s.find("es_bo") || n != s.find("es_cl") || n != s.find("es_co")
            || n != s.find("es_do") || n != s.find("es_ec") || n != s.find("es_eu") || n != s.find("es_gt")
            || n != s.find("es_hn") || n != s.find("es_mx") || n != s.find("es_pa") || n != s.find("es_pe")
            || n != s.find("es_py") || n != s.find("es_sv") || n != s.find("es_us") || n != s.find("es_uy")
            || n != s.find("es_ve") || n != s.find("es_mx"))
            return language("spanish");

        // Italian
        if (n != s.find("it_it") || n != s.find("it_eu"))
            return language("italian");

        // japanese
        if (n != s.find("jp_jp"))
            return language("japanese");

        // Default
        return pDefaultLanguage; /* should be english */
    }
    
    const I18N::Language* I18N::defaultLanguage()
    {
        MutexLocker locker(pMutex);
        return pDefaultLanguage;
    }
        
    
    const I18N::Language* I18N::currentLanguage()
    {
        MutexLocker locker(pMutex);
        return pCurrentLanguage;
    }

    bool I18N::currentLanguage(const I18N::Language* lng)
    {
        if (lng)
        {
            MutexLocker locker(pMutex);
            if (lng->englishCaption() != pCurrentLanguage->englishCaption())
            {
                pCurrentLanguage = language(lng->caption());
                if (pCurrentLanguage->englishCaption() == lng->englishCaption())
                {
                    resetPrefix();
                    LOG_INFO(TA3D_LOG_SECTION_I18N_PREFIX << "Switching to `"
                             << pCurrentLanguage->caption() << "`"
                             << " (" << pCurrentLanguage->englishCaption() << ")");
                    return true;
                }
            }
        }
        return false;
    }

    bool I18N::tryToDetermineTheLanguage()
    {
        # ifndef TA3D_PLATFORM_WINDOWS
        String locale = getenv("LC_ALL");
        return currentLanguage(languageFromLocal(locale));
        # else
        return currentLanguage(pDefaultLanguage);
        # endif
    }

    
    void I18N::retrieveAllLanguages(std::vector<I18N::Language>& out)
    {
        out.clear();
        pMutex.lock();
        for (Languages::const_iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
            out.push_back(*(*i));
        pMutex.unlock();
    }

    String I18N::translate(const String& key, const String& defaultValue)
    {
        if (key.empty())
            return defaultValue;
        MutexLocker locker(pMutex);
        String k = Lowercase(key);
        k += pLanguageSuffix;
        return (defaultValue.empty())
            ? pTranslations.PullAsString(k, key)
            : pTranslations.PullAsString(k, defaultValue);
    }

    void I18N::translate(std::vector<String>& out)
    {
        pMutex.lock();
        for (std::vector<String>::iterator i = out.begin(); i != out.end(); ++i)
            *i = translate(*i);
        pMutex.unlock();
    }

    void I18N::translate(std::list<String>& out)
    {
        pMutex.lock();
        for (std::list<String>::iterator i = out.begin(); i != out.end(); ++i)
            *i = translate(*i);
        pMutex.unlock();
    }

    
    bool I18N::loadFromFile(const String& filename, const bool emptyBefore, const bool inASCII)
    {
        if (!Paths::Exists(filename))
        {
            LOG_WARNING(TA3D_LOG_SECTION_I18N_PREFIX << "Impossible to load translations from `"
                        << filename << "` (file not found)");
            return false;
        }

        pMutex.lock();
        // Load the file
        try
        {
            pTranslations.Load(filename, emptyBefore, inASCII);
        }
        catch (...)
        {
            LOG_WARNING(TA3D_LOG_SECTION_I18N_PREFIX << "Exception caught for loadFromFile(`" << filename << "`)");
        }
        pMutex.unlock();

        // Success
        LOG_DEBUG(TA3D_LOG_SECTION_I18N_PREFIX << "`" << filename << "` loaded.");
        return true;
    }

    bool I18N::loadFromResources()
    {
        bool res(false);
        std::vector<String> list;
        if (Resources::Glob(list, "languages" + Paths::SeparatorAsString + "*.po"))
        {
            for (std::vector<String>::const_iterator i = list.begin(); i != list.end(); ++i)
            {
                if (loadFromFile(*i, false))
                    res = true;
            }
        }
        return res;
    }

    void I18N::doClearLanguages()
    {
        for (Languages::iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
            delete (*i);
        pLanguages.clear();
    }

    void I18N::initializeAllLanguages()
    {
        MutexLocker locker(pMutex);
        doClearLanguages();
        pNextLangID = 0;

        // English
        pDefaultLanguage = doAddNewLanguage("english", "English"); 
        pCurrentLanguage = pDefaultLanguage;
        resetPrefix();
        //French
        (void) doAddNewLanguage("french", "Français"); 
        // German
        (void) doAddNewLanguage("german", "Deutsch"); 
        // Spanish
        (void) doAddNewLanguage("spanish", "Español"); 
        // Italian
        (void) doAddNewLanguage("italian", "Italiano"); 
        // japanese
        (void) doAddNewLanguage("japanese", "日本語"); 
    }


} // namespace TA3D

