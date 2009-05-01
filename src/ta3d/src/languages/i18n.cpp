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

#include "i18n.h"
#include "../misc/resources.h"
#include "../misc/paths.h"
#include "../TA3D_NameSpace.h"




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
        pTranslations()
    {
        initializeAllLanguages();
    }

    I18N::~I18N()
    {
        LOG_DEBUG(LOG_PREFIX_I18N << "Release.");
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
        pLanguageSuffix += String::ToLower(pCurrentLanguage->englishCaption());
    }

    I18N::Language* I18N::doAddNewLanguage(const String& englishID, const String& translatedName)
    {
        I18N::Language* lng = new Language(pNextLangID, englishID, translatedName);
        LOG_ASSERT(NULL != lng);
        ++pNextLangID;
        pLanguages.push_back(lng);
        return lng;
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
        String s = String::Trim(locale);
        s.toLower();
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
                    LOG_INFO(LOG_PREFIX_I18N << "Switching to `"
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
        String k(key);
        k.toLower();
        k += pLanguageSuffix;
        return (defaultValue.empty())
            ? pTranslations.pullAsString(k, key)
            : pTranslations.pullAsString(k, defaultValue);
    }

    void I18N::translate(String::Vector& out)
    {
        pMutex.lock();
        for (String::Vector::iterator i = out.begin(); i != out.end(); ++i)
            *i = translate(*i);
        pMutex.unlock();
    }

    void I18N::translate(String::List& out)
    {
        pMutex.lock();
        for (String::List::iterator i = out.begin(); i != out.end(); ++i)
            *i = translate(*i);
        pMutex.unlock();
    }


    bool I18N::loadFromFile(const String& filename, const bool emptyBefore, const bool inASCII)
    {
        String res;
        if (!TA3D::VARS::HPIManager->Exists(filename))
        {
            LOG_WARNING(LOG_PREFIX_I18N << "Impossible to load translations from `"
                        << filename << "` (file not found)");
            return false;
        }

        pMutex.lock();
        // Load the file
        bool r = pTranslations.loadFromFile(filename, emptyBefore, inASCII);
        const String &languageEnglishID = pTranslations.pullAsString( "info.name" );
        const String &languageCaption = pTranslations.pullAsString( "info.caption" );
        // This file register a new language
        if (!languageEnglishID.empty() && !languageCaption.empty() && language(languageEnglishID) == NULL)
        {
            (void) doAddNewLanguage(languageEnglishID, languageCaption);
        }
        pMutex.unlock();

        // Success
        if (r)
            LOG_DEBUG(LOG_PREFIX_I18N << "`" << filename << "` loaded.");
        else
            LOG_ERROR(LOG_PREFIX_I18N << "Impossible to load `" << filename << "`");
        return r;
    }

    bool I18N::loadFromResources()
    {
        bool res(false);
        String::Vector list;
        if (Resources::Glob(list, "languages" + Paths::SeparatorAsString + "*.po"))
        {
            for (String::Vector::const_iterator i = list.begin(); i != list.end(); ++i)
            {
                if (loadFromFile("languages" + Paths::SeparatorAsString + Paths::ExtractFileName(*i), false))
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
    }


} // namespace TA3D

