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

#ifndef __TA3D_XX__TDF_H__
# define __TA3D_XX__TDF_H__

# include "string.h"
# include "hash_table.h"
# include <stack>
# include <zuzuf/smartptr.h>


# define TDFPARSER_HASHTABLE_SIZE       0x400

namespace TA3D
{


	/*!
	** \brief
	**
	** This class is not thread-safe.
	*/
    class TDFParser : public zuzuf::ref_count
	{
	public:
        typedef zuzuf::smartptr<TDFParser>	Ptr;
	public:
		//! \name Constructors & Destructor
		//@{

		//! Constructor by default
		TDFParser();

		/*!
		** \brief Contructor
		**
		** \param filename The absolute filename to load
		** \param caSensitive Get if all keys are case sensitive or not
		** \param toUTF8 Convert this buffer from ASCII to the UTF8 charset
		** \param gadgetMode Mystic mode
		**
		** \see loadFromFile()
		*/
		explicit TDFParser(const QString& filename, const bool caSensitive = false, const bool toUTF8 = false,
                           const bool gadgetMode = false, const bool realFS = false, const bool widgetMode = false);

		//! Destructor
		~TDFParser();

		//@} Constructors & Destructor

		/*!
		** \brief Clear all stored keys
		*/
		void clear();


		//! \name Import Data
		//@{
		/*!
		** \brief Load data from a file
		**
		** \param filename The absolute filename to load
		** \param clear Clear the all previoulsy stored keys
		** \param toUTF8 Convert this buffer from ASCII to the UTF8 charset
		** \param gadgetMode Mystic mode
		** \return True if the operation succeeded, False otherwise
		*/
		bool loadFromFile(const QString& filename, const bool clear = false, const bool toUTF8 = false, const bool gadgetMode = false, const bool realFS = false, const bool widgetMode = false);

		/*!
		** \brief Load data from a buffer
		**
		** \param caption The caption of this buffer to display when a parse error is encountered
		** \param data The buffer
		** \param size Size of the buffer
		** \param clear Clear the all previoulsy stored keys
		** \param toUTF8 Convert this buffer from ASCII to the UTF8 charset
		** \param gadgetMode Mystic mode
		** \return True if the operation succeeded, False otherwise
		*/
		bool loadFromMemory(const QString& caption, const char* data, uint64 size, const bool clear = false,
							const bool toUTF8 = false, const bool gadgetMode = false, const bool widgetMode = false);
		//@}


		//! \name Keys & Values
		//@{
		/*!
		** \brief Get the value for a given key
		** \param key The key
		** \param def The default value if the key could not be found
		** \return The value of the key that has been found, def otherwise
		*/
        sint32  pullAsInt(const QString& key, const sint32 def);
        sint32  pullAsInt(const QString& key);

		/*!
		** \brief Get the value for a given key
		** \param key The key
		** \param def The default value if the key could not be found
		** \return The value of the key that has been found, def otherwise
		*/
		float  pullAsFloat(const QString& key, const float def = 0.0f);

		/*!
		** \brief Get the value for a given key
		** \param key The key
		** \param def The default value if the key could not be found
		** \return The value of the key that has been found, def otherwise
		*/
        QString pullAsString(const QString& key, const QString& def);
        QString pullAsString(const QString& key);

		/*!
		** \brief Get the value for a given key
		** \param key The key
		** \param def The default value if the key could not be found
		** \return The value of the key that has been found, def otherwise
		*/
		bool pullAsBool(const QString& key, const bool def = false);

		/*!
		** \brief Get the value for a given key
		** \param key The key
		** \param def The default value if the key could not be found
		** \return The value of the key that has been found, def otherwise
		*/
		uint32  pullAsColor(const QString& key, const uint32 def = 0);

		/*!
		** \brief Insert or update the value of a key
		** \param key The key
		** \param value The new value of the key
		*/
		void insertOrUpdate(const QString& key, const QString& value) {pTable[key] = value;}

		/*!
		** \brief Remove a key if exists
		** \param key The key to remove
		*/
		void remove(const QString& key) {pTable.erase(key);}

		bool exists(const QString& key) {return pTable.count(key) != 0;}

		/*!
		** \brief Set the special section
		** \param special section name
		*/
        void setSpecialSection(const QString &section);

		/*!
		** \brief Call a callback for each key
		** \param callback The callback
		**
		** \code
		** class Predicate
		** {
		** public:
		**     bool operator () (const QString& key, const QString& value)
		**     {
		**         std::cout << "Key: " << key << ", value: " << value << std::endl;
		**         return true; // False to stop the process
		**     }
		** };
		**
		** int main(void)
		** {
		**     TA3D::TDFParser p;
		**     p.loadFromFile("gui/mdrn_save_menu.tdf");
		**     p.forEach(Predicate());
		**     return 0;
		** }
		** \endcode
		*/
		template<typename C> void forEach(C callback)
		{
			for(TA3D::UTILS::HashMap<QString>::Dense::iterator it = pTable.begin() ; it != pTable.end() ; ++it)
				callback(it.key(), it.value());
		}

		//@}


	private:
		//! The hash table
		TA3D::UTILS::HashMap<QString>::Dense pTable;

		//! CharCase
		bool pIgnoreCase;

		//! Special section for which we keep track of keys
        QString special_section;

	}; // class TDFParser





} // namespace TA3D

#endif // __TA3D_XX__TDF_H__
