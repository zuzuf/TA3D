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

#ifndef __CLASS_HASH_TABLE
#define __CLASS_HASH_TABLE

#pragma once

namespace TA3D
{
   namespace UTILS
   {
      #define __DEFAULT_HASH_TABLE_SIZE   0x1000      // 4096

      template <class T>
      class cBucket
      {
         public:
         String		m_szKey;
         T			m_T_data;

         public:
         cBucket( const String &k, const T &myData) :
         m_szKey( k ),
         m_T_data( myData ) { }

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

         uint32 GetHash( const String &key ) const
         {
            uint32 HashValue = 0;

            for( uint32 i=0; i < (uint32)key.length(); i++ )
               HashValue = key[i] + ( HashValue<<5 ) - HashValue;

            return ( HashValue % m_u32TableSize );
         }

      public:
         virtual void InitTable( uint32 TableSize )
         {
            if( std::vector< std::list< TA3D::UTILS::cBucket<T> > >::size() )
               this->EmptyHashTable();

            std::list< cBucket<T> > empty_list;
            resize( TableSize, empty_list );

            m_u32TableSize = TableSize;
         }

         // Constructors.
         cHashTable()
         {
            InitTable( __DEFAULT_HASH_TABLE_SIZE );
         }

         cHashTable( uint32 TableSize )
         {
            InitTable( TableSize );
         }

         // Destructor
         virtual ~cHashTable()
         {
            this->EmptyHashTable();
         }

         virtual void EmptyHashTable()
         {
         	if(m_u32TableSize == 0)	return;
            if( !std::vector< std::list< TA3D::UTILS::cBucket<T> > >::size() )
               return;

            for( typename std::vector< std::list< TA3D::UTILS::cBucket<T> > >::iterator iter = std::vector< std::list< TA3D::UTILS::cBucket<T> > >::begin() ; iter != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::end() ; iter++ )   
               iter->clear();
      
            std::vector< std::list< TA3D::UTILS::cBucket<T> > >::clear();

            m_u32TableSize = 0;				// Ugly bug without this
         }

         bool Exists( const String &key )
         {
         	if(m_u32TableSize == 0)	return false;
            uint32 hash = GetHash( key );

            typename std::list< cBucket<T> >::iterator cur = std::find_if( std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
               std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(), std::bind2nd( ModHashFind<T>(), key ) );
      
            return ( (cur == std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end()) ? false : true );
         }
            
         T Find( const String &key )
         {
         	if(m_u32TableSize == 0)	return (T) 0;
            uint32 hash = GetHash( key );

            typename std::list< cBucket<T> >::iterator cur = std::find_if( std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
               std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(), std::bind2nd( ModHashFind<T>(), key ) );

            return ( (cur == std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end()) ? (T) 0 : (*cur).m_T_data );
         }

         bool Insert( const String &key, T v )
         {
         	if(m_u32TableSize == 0)	return false;
            if( Exists( key ) )
               return false;

            cBucket<T> elt( key, v );

            std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at( GetHash( key ) ).push_back( elt );

            return true;
         }

         virtual void InsertOrUpdate( const String &key, T v )
         {
         	if(m_u32TableSize == 0)	return;
            uint32 hash = GetHash( key );

            typename std::list< cBucket<T> >::iterator cur = std::find_if( std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
               std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(), std::bind2nd( ModHashFind<T>(), key ) );

            if( cur != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end() )
               (*cur).m_T_data = v;
            else
               Insert( key, v );
         }
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
         uint32 WildCardSearch( const String &Search, std::list< String > *li)
         {
         	if(m_u32TableSize == 0)	return 0;
            String first, last;
            std::basic_string <char>::size_type iFind;
            static const std::basic_string <char>::size_type NotFound = -1;
            uint32 nb=0;

            iFind = Search.find( "*" );

            if( iFind != NotFound )   {
               first = Lowercase( Search.substr(0,(iFind)) );
               last = Lowercase( Search.substr( iFind+1) );
               }
            else {
               first = Lowercase( Search );
               last = "";
               }

            for( typename std::vector< std::list< TA3D::UTILS::cBucket<T> > >::iterator iter = std::vector< std::list< TA3D::UTILS::cBucket<T> > >::begin() ; iter != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::end() ; iter++ )   
            {
               for( typename std::list< cBucket<T> >::iterator cur = iter->begin() ; cur != iter->end() ; cur++ )   
               {
                  String f = cur->Key();

                  if( f.length() < first.length() || f.length() < last.length() )
                     continue;
                  
                  if( f.substr(0,first.length() ) == first )   {
                     if( f.substr( f.length() - last.length(), last.length() ) == last )   {
                        li->push_back( f );
                        nb++;
                        }
                     }
                  }
               }

            return nb;
         }

         virtual void Remove( const String &key )
         {
         	if(m_u32TableSize == 0)	return;
            uint32 hash = GetHash( key );

            typename std::list< cBucket<T> >::iterator cur;

            for( cur=std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(); cur!=std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(); cur++) {
               if( cur->Key() == key )   {
                  std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at( hash ).erase( cur );
                  break;
                  }
               }
         }
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
#endif
