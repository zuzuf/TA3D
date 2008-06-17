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
        struct ModHashFind : public std::binary_function< cBucket< T >, const String, bool >
    {
        bool operator()( const cBucket< T > &d, const String &ref ) const
        {
            return d.Key() == ref;
        }
    };

    template <class T>
        class cHashTable : protected std::vector< std::list< TA3D::UTILS::cBucket<T> > >
    {
    private:

    protected:
        uint32         m_u32TableSize;

        uint32 GetHash( const String &key ) const;

    public:
        virtual void InitTable(uint32 TableSize);

        //! \name Constructors & Destructor
        //{
        //! Default Constructor
        cHashTable();
        /*!
        ** \brief Constructor
        ** \param TableSize Initial Size of the table
        */
        cHashTable(uint32 TableSize);
        //! Destructor
        virtual ~cHashTable();
        //}

        virtual void EmptyHashTable();

        /*!
        ** \brief Test the existence of a key
        ** \param key The key to find
        ** \return True if the key exists, False otherwise
        */
        bool Exists(const String& key);

        /*!
        ** \brief Find the value of a key
        ** \param key The key to find
        ** \return The value of the key if found, T(0) otherwise
        */
        T Find(const String& key);

        /*!
        ** \brief Insert a new key/value if the key not already exists
        ** \param key The key to insert
        ** \param v Its value
        */
        bool Insert(const String& key, T v);

        /*!
        ** \brief Insert or update the value of a given key
        ** \param key The key to insert or update
        ** \param v The new value of the key
        */
        virtual void InsertOrUpdate(const String &key, T v);
            
#ifndef TA3D_PLATFORM_WINDOWS
    public:
        String Lowercase( const String &sstring )
        {
            String Return;

            Return.resize (sstring.length());
            for( uint32 i = 0; i < sstring.length(); i++)
                Return[i] = tolower (sstring[i]);

            return (Return);
        }
#endif
    public:
        /*!
        ** \brief
        */
        uint32 WildCardSearch( const String &Search, std::list<String>* li);

        /*!
        ** \brief Remove an existing entry
        ** \param key the key to remove
        */
        virtual void Remove( const String &key );

    }; // class cHashTable

    template <class T>
        class clpHashTable : public cHashTable<T>
    {
    private:
        bool                           m_bFreeDataOnErase;

    public:
        void InitTable( uint32 TableSize, bool FreeDataOnErase )
        {
            m_bFreeDataOnErase = FreeDataOnErase;
            if( cHashTable<T>::size() )
                this->EmptyHashTable();

            std::list< cBucket<T> > empty_list;
            cHashTable<T>::resize( TableSize, empty_list );

            cHashTable<T>::m_u32TableSize = TableSize;
        }

        // Constructors.
        clpHashTable()
        {
            InitTable( __DEFAULT_HASH_TABLE_SIZE, true );
        }

        clpHashTable( uint32 TableSize, bool FreeDataOnErase )
        {
            InitTable( TableSize, FreeDataOnErase );
        }


        // Destructor
        virtual ~clpHashTable()
        {
            this->EmptyHashTable();
        }

        void EmptyHashTable()
        {
            if(cHashTable<T>::m_u32TableSize == 0)	return;
            if( !cHashTable<T>::size() )
                return;

            for( typename cHashTable<T>::iterator iter = cHashTable<T>::begin() ; iter != cHashTable<T>::end() ; iter++ )   
            {
                if( m_bFreeDataOnErase )   
                {
                    for( typename std::list< cBucket<T> >::iterator cur = iter->begin() ; cur != iter->end() ; cur++ )
                        delete( (*cur).m_T_data );
                }

                iter->clear();
            }

            cHashTable<T>::clear();
            cHashTable<T>::m_u32TableSize = 0;
        }

        void InsertOrUpdate( const String &key, T v )
        {
            if(cHashTable<T>::m_u32TableSize == 0)	return;
            uint32 hash = cHashTable<T>::GetHash( key );

            typename std::list< cBucket<T> >::iterator cur = std::find_if( cHashTable<T>::at(hash).begin(),
                                                                           cHashTable<T>::at(hash).end(), std::bind2nd( ModHashFind<T>(), key ) );

            if( cur != cHashTable<T>::at(hash).end() )
            {
                if( m_bFreeDataOnErase )
                    delete( (*cur).m_T_data );

                (*cur).m_T_data = v;
            }
            else
                cHashTable<T>::Insert( key, v );
        }

        void Remove( const String &key )
        {
            if(cHashTable<T>::m_u32TableSize == 0)	return;
            uint32 hash = cHashTable<T>::GetHash( key );

            for( typename std::list< cBucket<T> >::iterator cur= cHashTable<T>::at(hash).begin(); cur!= cHashTable<T>::at(hash).end(); cur++)
            {
                if( cur->Key() == key )   
                {
                    if( !m_bFreeDataOnErase )
                        delete( (*cur).m_T_data  );

                    cHashTable<T>::at( hash ).erase( cur );
                    break;
                }
            }
        }
    }; // class cHashTable


} // namespace UTILS
} // namespace TA3D 



# include "hashtable.hxx"


#endif // __TA3D_XX_HASH_TABLE_H__
