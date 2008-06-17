#ifndef __TA3D_XX_HASH_TABLE_HXX__
# define __TA3D_XX_HASH_TABLE_HXX__

# include "../logs/logs.h"
# include <list>


namespace TA3D
{
namespace UTILS
{


    template<class T>
    uint32
    cHashTable<T>::GetHash( const String &key ) const
    {
        uint32 HashValue = 0;
        for(uint32 i = 0; i < (uint32)key.length();  ++i)
            HashValue = key[i] + ( HashValue << 5 ) - HashValue;
            return (HashValue % m_u32TableSize);
    }

    template<class T>
    void
    cHashTable<T>::InitTable( uint32 TableSize )
    {
        if (std::vector< std::list< TA3D::UTILS::cBucket<T> > >::size())
            this->EmptyHashTable();
        std::list< cBucket<T> > empty_list;
        resize(TableSize, empty_list);
        m_u32TableSize = TableSize;
    }

    template<class T>
    void
    cHashTable<T>::EmptyHashTable()
    {
        if (m_u32TableSize == 0 || !std::vector< std::list< TA3D::UTILS::cBucket<T> > >::size())
            return;
        typename std::vector< std::list< TA3D::UTILS::cBucket<T> > >::iterator iter;
        for(iter = std::vector< std::list< TA3D::UTILS::cBucket<T> > >::begin() ; iter != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::end() ; ++iter)   
            iter->clear();
        std::vector< std::list< TA3D::UTILS::cBucket<T> > >::clear();
        m_u32TableSize = 0; // Ugly bug without this
    }

    template<class T>
    cHashTable<T>::cHashTable()
    {
        InitTable(__DEFAULT_HASH_TABLE_SIZE);
    }

    template<class T>
    cHashTable<T>::cHashTable(uint32 TableSize)
    {
        InitTable( TableSize );
    }

    template<class T>
    cHashTable<T>::~cHashTable()
    {
        this->EmptyHashTable();
    }


    template<class T>
    bool
    cHashTable<T>::Exists(const String& key)
    {
        if (!m_u32TableSize)
            return false;
        uint32 hash = GetHash(key);
        typename std::list< cBucket<T> >::iterator cur =
            std::find_if(std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
                        std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(),
                        std::bind2nd(ModHashFind<T>(), key));
        return ((cur == std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end())
                ? false : true);
    }


    template<class T>
    T
    cHashTable<T>::Find( const String &key )
    {
        if (m_u32TableSize == 0)
            return (T) 0;
        uint32 hash = GetHash( key );
        typename std::list< cBucket<T> >::iterator cur =
            std::find_if(std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
                        std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(),
                        std::bind2nd( ModHashFind<T>(), key ) );
        return ((cur == std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end()) 
                ? (T) 0 : (*cur).m_T_data);
    }


    template<class T>
    bool
    cHashTable<T>::Insert(const String& key, T v)
    {
        if(m_u32TableSize == 0)
            return false;
        if (Exists(key))
            return false;
        cBucket<T> elt( key, v );
        std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(GetHash(key)).push_back(elt);
        return true;
    }


    template<class T>
    void
    cHashTable<T>::InsertOrUpdate(const String& key, T v)
    {
        if (m_u32TableSize == 0)
            return;
        uint32 hash = GetHash( key );
        typename std::list< cBucket<T> >::iterator cur =
            std::find_if( std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(),
                        std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(),
                        std::bind2nd( ModHashFind<T>(), key));
        if( cur != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end() )
            (*cur).m_T_data = v;
        else
            Insert(key, v);
    }


    template<class T>
    uint32
    cHashTable<T>::WildCardSearch( const String &Search, std::list< String > *li)
    {
        LOG_ASSERT(li != NULL);

        if(m_u32TableSize == 0)
            return 0;
        String first, last;
        std::basic_string <char>::size_type iFind;
        static const std::basic_string <char>::size_type NotFound = -1;
        uint32 nb=0;

        iFind = Search.find( "*" );

        if (iFind != NotFound) 
        {
            first = Lowercase(Search.substr(0,(iFind)));
            last = Lowercase(Search.substr( iFind+1));
        }
        else
        {
            first = Lowercase(Search);
            last = "";
        }

        for (typename std::vector< std::list< TA3D::UTILS::cBucket<T> > >::iterator iter = std::vector< std::list< TA3D::UTILS::cBucket<T> > >::begin() ; iter != std::vector< std::list< TA3D::UTILS::cBucket<T> > >::end() ; iter++ )   
        {
            for( typename std::list< cBucket<T> >::iterator cur = iter->begin() ; cur != iter->end() ; ++cur)
            {
                String f = cur->Key();

                if (f.length() < first.length() || f.length() < last.length())
                    continue;

                if (f.substr(0,first.length()) == first)  
                {
                    if( f.substr( f.length() - last.length(), last.length() ) == last)
                    {
                        li->push_back(f);
                        nb++;
                    }
                }
            }
        }

        return nb;
    }


    template<class T>
    void
    cHashTable<T>::Remove(const String& key)
    {
        if(m_u32TableSize == 0)
            return;
        uint32 hash = GetHash(key);

        typename std::list< cBucket<T> >::iterator cur;

        for( cur=std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).begin(); cur!=std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at(hash).end(); cur++)
        {
            if (cur->Key() == key)
            {
                std::vector< std::list< TA3D::UTILS::cBucket<T> > >::at( hash ).erase( cur );
                return;
            }
        }
    }


}
}

#endif // __TA3D_XX_HASH_TABLE_HXX__
