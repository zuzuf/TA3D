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

#include <yuni/yuni.h>
#include <yuni/core/io/file.h>
#include "i18n.h"
#include <misc/resources.h>
#include <misc/paths.h>
#include <TA3D_NameSpace.h>




namespace TA3D
{

	I18N::Ptr I18N::pInstance = NULL;



	I18N::Language::Language(const int indx, const QString& englishID, const QString& caption)
		:pIndx(indx), pEnglishID(englishID), pCaption(caption)
	{}


	I18N::Language::Language(const Language& l)
		:pIndx(l.pIndx), pEnglishID(l.pEnglishID), pCaption(l.pCaption)
	{}


	I18N::Ptr I18N::Instance()
	{
		// We use here a double-check lock, which would be good enough
		// for this kind of class
		if (!pInstance)
		{
			ThreadingPolicy::MutexLocker locker;
			if (!pInstance)
				pInstance = new I18N();
		}
		return pInstance;
	}


	I18N::I18N()
		:pNextLangID(0), pDefaultLanguage(NULL), pCurrentLanguage(NULL),
		pTranslations()
	{
		// English
		pDefaultLanguage = addNewLanguageWL("english", "English");
		pCurrentLanguage = pDefaultLanguage;
		resetPrefix();
	}

	I18N::~I18N()
	{
		LOG_DEBUG(LOG_PREFIX_I18N << "Release.");
		if (!pLanguages.empty())
		{
			for (Languages::iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
				delete *i;
			pLanguages.clear();
		}
	}


	void I18N::Destroy()
	{
		ThreadingPolicy::MutexLocker locker;
		pInstance = NULL;
	}

	void I18N::resetPrefix()
	{
		pLanguageSuffix = ".";
		pLanguageSuffix += ToLower(pCurrentLanguage->englishCaption());
	}

	I18N::Language* I18N::addNewLanguageWL(const QString& englishID, const QString& translatedName)
	{
		I18N::Language* lng = new Language(pNextLangID, englishID, translatedName);
		++pNextLangID;
		pLanguages.push_back(lng);
		return lng;
	}

	I18N::Language* I18N::language(const QString& name)
	{
        if (name.isEmpty())
			return NULL;
		ThreadingPolicy::MutexLocker locker(*this);
		for (Languages::const_iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
		{
			if (name == (*i)->caption() || name == (*i)->englishCaption())
				return (*i);
		}
		return NULL;
	}

	const I18N::Language* I18N::languageFromLocal(const QString& locale)
	{
		LOG_ASSERT(NULL != pDefaultLanguage /* initializeAllLanguages() must be called before */ );
        if (locale.isEmpty())
			return pDefaultLanguage;
        const QString s(Substr(locale, 0, 5).trimmed().toLower());

		// French
		if (s.contains("fr_fr") || s.contains("fr_ca") || s.contains("fr_ch") || s.contains("fr_be")
			|| s.contains("fr_eu") || s.contains("fr_lu"))
			return language("french");

		// German
		if (s.contains("de_de") || s.contains("de_ch") || s.contains("de_be") || s.contains("de_eu")
			|| s.contains("de_at"))
			return language("german");

		// Spanish
		if (s.contains("es_ar") || s.contains("es_bo") || s.contains("es_cl") || s.contains("es_co")
			|| s.contains("es_do") || s.contains("es_ec") || s.contains("es_eu") || s.contains("es_gt")
			|| s.contains("es_hn") || s.contains("es_mx") || s.contains("es_pa") || s.contains("es_pe")
			|| s.contains("es_py") || s.contains("es_sv") || s.contains("es_us") || s.contains("es_uy")
			|| s.contains("es_ve") || s.contains("es_mx"))
			return language("spanish");

		// Italian
		if (s.contains("it_it") || s.contains("it_eu"))
			return language("italian");

		// japanese
		if (s.contains("jp_jp"))
			return language("japanese");

		// Default
		return pDefaultLanguage; // should be english
	}

	const I18N::Language* I18N::defaultLanguage()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return pDefaultLanguage;
	}


	bool I18N::currentLanguage(const I18N::Language* lng)
	{
		if (lng)
		{
			ThreadingPolicy::MutexLocker locker(*this);
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
		QString locale = getenv("LC_ALL");
        if (locale.isEmpty())		locale = getenv("LANG");
		LOG_INFO(LOG_PREFIX_I18N << "locale = " << locale);
		return currentLanguage(languageFromLocal(locale));
# else
		return currentLanguage(pDefaultLanguage);
# endif
	}


	void I18N::retrieveAllLanguages(std::vector<I18N::Language>& out)
	{
		out.clear();
		ThreadingPolicy::MutexLocker locker(*this);
		for (Languages::const_iterator i = pLanguages.begin(); i != pLanguages.end(); ++i)
			out.push_back(*(*i));
	}

	QString I18N::translate(const QString& key, const QString& defaultValue)
	{
        if (key.isEmpty())
			return defaultValue;

        const QString k(key.toLower() + pLanguageSuffix);
		ThreadingPolicy::MutexLocker locker(*this);
        return (defaultValue.isEmpty())
            ? pTranslations.pullAsString(k, key)
            : pTranslations.pullAsString(k, defaultValue);
	}

    void I18N::translate(QStringList& out)
	{
		ThreadingPolicy::MutexLocker locker(*this);
        for (QString &i : out)
            i = translate(i);
	}


	bool I18N::loadFromFile(const QString& filename, const bool emptyBefore, const bool inASCII)
	{
		if (!VFS::Instance()->fileExists(filename))
		{
			LOG_WARNING(LOG_PREFIX_I18N << "Impossible to load translations from `"
						<< filename << "` (file not found)");
			return false;
		}

		ThreadingPolicy::MutexLocker locker(*this);
		// Load the file
		bool r = pTranslations.loadFromFile(filename, emptyBefore, inASCII);
        const QString &languageEnglishID = pTranslations.pullAsString( "info.name" );
        const QString &languageCaption = pTranslations.pullAsString( "info.caption" );
		// This file register a new language
        if (!languageEnglishID.isEmpty() && !languageCaption.isEmpty() && language(languageEnglishID) == NULL)
		{
			(void) addNewLanguageWL(languageEnglishID, languageCaption);
		}

		// Success
		if (r)
			LOG_DEBUG(LOG_PREFIX_I18N << "`" << filename << "` loaded.");
		else
			LOG_ERROR(LOG_PREFIX_I18N << "Impossible to load `" << filename << "`");
		return r;
	}

	bool I18N::loadFromResources()
	{
		// Retrieve the list of all .po files
        QString path = "languages/*.po";
        QStringList list;
		VFS::Instance()->getFilelist(path, list);

        if (!list.isEmpty())
		{
			bool res = false;
            for (const QString &i : list)
			{
                path = "languages/" + Paths::ExtractFileName(i);
				if (loadFromFile(path, false))
				{
					// The operation will succeed if at least one .po has been loaded
					res = true;
				}
			}
			return res;
		}
		return false;
	}






} // namespace TA3D

