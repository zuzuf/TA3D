/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

#ifndef __TA3D_XX_HASH_TABLE_H__
# define __TA3D_XX_HASH_TABLE_H__

# include <list>
# include <vector>
# include <string>
#include <algorithm>
# include "../stdafx.h"

# define __DEFAULT_HASH_TABLE_SIZE   0x1000      // 4096



namespace TA3D
{
namespace UTILS
{



    template <class T>
    class cBucket
    {
    public:
        String		m_szKey;
        T			m_T_data;

    public:
        cBucket( const String &k, const T &myData) :
            m_szKey(k), m_T_data( myData ) {}

        const String &Key() const { return m_szKey; }
    };


    template <class T>
    struct ModHashFind : public std::binary_function< cBucket<T>, const String, bool >
    {
        bool operator()( const cBucket< T > &d, const String &ref ) const
        {
            return d.Key() == ref;
        }
    };

    template <class T>
    class cHashTable : protected std::vector< std::list< TA3D::UTILS::cBucket<T> > >
    {
    public:
        //! \brief List of buckets
        typedef typename std::list< TA3D::UTILS::cBucket<T> >   BucketsList;
        //! \brief Vector of list of buckets
        typedef typename std::vector< BucketsList >  VectorOfBucketsList;

    public:
        virtual void initTable(const uint32 TableSize);

        //! \name Constructors & Destructor
        //@{
        //! Default Constructor
        cHashTable();
        /*!
        ** \brief Constructor
        ** \param TableSize Initial Size of the table
        */
        cHashTable(const uint32 TableSize);
        //! Destructor
        virtual ~cHashTable();
        //@}

        virtual void emptyHashTable();

        /*!
        ** \brief Test the existence of a key
        ** \param key The key to find
        ** \return True if the key exists, False otherwise
        */
        bool exists(const String& key);

        /*!
        ** \brief Find the value of a key
        ** \param key The key to find
        ** \return The value of the key if found, T(0) otherwise
        */
        T find(const String& key);

        /*!
        ** \brief Insert a new key/value if the key not already exists
        ** \param key The key to insert
        ** \param v Its value
        */
        bool insert(const String& key, T v);

        /*!
        ** \brief Insert or update the value of a given key
        ** \param key The key to insert or update
        ** \param v The new value of the key
        */
        virtual void insertOrUpdate(const String &key, T v);

        /*!
        ** \brief Search for all keys matching a pattern
        **
        ** \param pattern The pattern to search
        ** \param[out] li The results
        */
        uint32 wildCardSearch(const String& pattern, String::List& li);

        /*!
        ** \brief Remove an existing entry
        ** \param key the key to remove
        */
        virtual void remove(const String &key);

        /*!
        ** \brief Call a callback for each key
        ** \param callback The callback
        **
        ** \code
        ** class Predicate
        ** {
        ** public:
        **     bool operator () (const String& key, const String& value)
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
        template<typename C>
        void forEach(C callback);


    protected:
        uint32 pTableSize;
        /*!
        ** \brief Hash of a string
        ** \param key 
        ** \return The hash value of the string
        */
        uint32 generateHash(const String& key) const;

    }; // class cHashTable




    template <class T>
    class clpHashTable : public cHashTable<T>
    {
    public:
        //! \brief List of buckets
        typedef typename std::list< TA3D::UTILS::cBucket<T> >   BucketsList;
        //! \brief Vector of list of buckets
        typedef typename std::vector< BucketsList >  VectorOfBucketsList;

    public:

        // Constructors.
        clpHashTable();

        clpHashTable(const uint32 TableSize, const bool FreeDataOnErase);


        // Destructor
        virtual ~clpHashTable();

        void initTable(const uint32 tableSize, const bool freeDataOnErase);

        /*!
        ** \brief
        */
        void emptyHashTable();

        /*!
        ** \brief
        */
        void insertOrUpdate(const String& key, T v);

        /*!
        ** \brief
        */
        void remove(const String& key);


    private:
        //!
        bool m_bFreeDataOnErase;


    }; // class cHashTable


} // namespace UTILS
} // namespace TA3D 



# include "hashtable.hxx"


#endif // __TA3D_XX_HASH_TABLE_H__
