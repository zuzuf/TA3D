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
#ifndef __TA3D_XX_I18N_HXX__
# define __TA3D_XX_I18N_HXX__

#include "i18n.h"

namespace TA3D
{

	inline const I18N::Language* I18N::CurrentLanguage()
	{
		return I18N::Instance()->currentLanguage();
	}


	inline const I18N::Language* I18N::currentLanguage()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return pCurrentLanguage;
	}


	inline bool I18N::CurrentLanguage(const QString& l)
	{
		return I18N::Instance()->currentLanguage(l);
	}


	inline bool I18N::AutoLanguage()
	{
		return I18N::Instance()->tryToDetermineTheLanguage();
	}


	inline bool I18N::LoadFromFile(const QString& filename)
	{
		return I18N::Instance()->loadFromFile(filename);
	}


	inline bool I18N::LoadFromResources()
	{
		return I18N::Instance()->loadFromResources();
	}


	inline QString I18N::Translate(const QString& key, const QString& defaultValue)
	{
		return I18N::Instance()->translate(key, defaultValue);
	}


    inline void I18N::Translate(QStringList& out)
	{
		I18N::Instance()->translate(out);
	}


	inline bool I18N::currentLanguage(const QString& n)
	{
		return currentLanguage(language(n));
	}


	inline QString I18N::operator [] (const QString& key)
	{
		return translate(key);
	}





} // namespace TA3D

#endif // __TA3D_XX_I18N_HXX__
