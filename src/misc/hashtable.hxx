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
    cHashTable<T>::generateHash(const String& key) const
    {
        uint32 ret(0);
        for (String::const_iterator i = key.begin(); i != key.end(); ++i)
            ret = *i + (ret << 5 ) - ret;
        return (ret % pTableSize);
    }

    template<class T>
    void
    cHashTable<T>::InitTable(const uint32 TableSize)
    {
        if (!VectorOfBucketsList::empty())
            this->EmptyHashTable();
        std::list< cBucket<T> > empty_list; 
        resize(TableSize, empty_list);
        pTableSize = TableSize;
    }

    template<class T>
    void
    cHashTable<T>::EmptyHashTable()
    {
        if (pTableSize == 0 || VectorOfBucketsList::empty())
            return;
        typename VectorOfBucketsList::iterator iter;
        for(iter = VectorOfBucketsList::begin() ; iter != VectorOfBucketsList::end() ; ++iter)   
            iter->clear();
        VectorOfBucketsList::clear();
        pTableSize = 0;
    }

    template<class T>
    cHashTable<T>::cHashTable()
    {
        InitTable(__DEFAULT_HASH_TABLE_SIZE);
    }

    template<class T>
    cHashTable<T>::cHashTable(uint32 TableSize)
    {
        InitTable(TableSize);
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
        if (!pTableSize)
            return false;
        const uint32 hash = generateHash(key);
        BucketsList& it = (*(static_cast<VectorOfBucketsList*>(this)))[hash];
        typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
        return (cur != it.end());
    }


    template<class T>
    T
    cHashTable<T>::Find(const String &key)
    {
        if (pTableSize == 0)
            return (T) 0;
        const uint32 hash = generateHash(key);
        BucketsList& it = (*(static_cast<VectorOfBucketsList*>(this)))[hash];
        typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
        return ((cur == it.end()) ? (T) 0 : (*cur).m_T_data);
    }


    template<class T>
    bool
    cHashTable<T>::Insert(const String& key, T v)
    {
        if(pTableSize == 0 || Exists(key))
            return false;
        cBucket<T> elt(key, v);
        (*(static_cast<VectorOfBucketsList*>(this)))[generateHash(key)].push_back(elt);
        return true;
    }


    template<class T>
    void
    cHashTable<T>::InsertOrUpdate(const String& key, T v)
    {
        if (!pTableSize)
            return;
        const uint32 hash = generateHash(key);
        BucketsList& it = (*(static_cast<VectorOfBucketsList*>(this)))[hash];
        typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
        if( cur != it.end())
            (*cur).m_T_data = v;
        else
            Insert(key, v);
    }


    template<class T>
    uint32
    cHashTable<T>::WildCardSearch(const String& Search, String::List* li)
    {
        LOG_ASSERT(li != NULL);

        if(pTableSize == 0)
            return 0;
        String::size_type iFind;
        String first;
        String last;

        iFind = Search.find("*");
        if (iFind != String::npos) 
        {
            first = Lowercase(Search.substr(0, iFind));
            ++iFind;
            last = Lowercase(Search.substr(iFind));
        }
        else
        {
            first = Lowercase(Search);
            last = "";
        }

        uint32 nb(0);
        for (typename VectorOfBucketsList::iterator iter = VectorOfBucketsList::begin() ; iter != VectorOfBucketsList::end(); ++iter)   
        {
            for (typename BucketsList::iterator cur = iter->begin() ; cur != iter->end(); ++cur)
            {
                String f = cur->Key();
                if (f.length() < first.length() || f.length() < last.length())
                    continue;

                if (f.substr(0,first.length()) == first)  
                {
                    if (f.substr(f.length() - last.length(), last.length()) == last)
                    {
                        li->push_back(f);
                        ++nb;
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
        if (!pTableSize)
            return;
        const uint32 hash = generateHash(key);

        BucketsList& it = (*(static_cast<VectorOfBucketsList*>(this)))[hash];
        for (typename BucketsList::iterator cur = it.begin(); cur != it.end(); ++cur)
        {
            if (cur->Key() == key)
            {
                it.erase(cur);
                return;
            }
        }
    }

    template<class T>
    void
    clpHashTable<T>::InitTable(const uint32 tableSize, const bool freeDataOnErase )
    {
        m_bFreeDataOnErase = freeDataOnErase;
        if (cHashTable<T>::size())
            this->EmptyHashTable();

        BucketsList empty_list;
        cHashTable<T>::resize(tableSize, empty_list);
        cHashTable<T>::pTableSize = tableSize;
    }


    template<class T>
    void
    clpHashTable<T>::Remove(const String& key)
    {
        if (!cHashTable<T>::pTableSize)
            return;
        const uint32 hash = cHashTable<T>::generateHash(key);

        BucketsList& it = (*(static_cast<VectorOfBucketsList*>(this)))[hash];
        for( typename BucketsList::iterator cur= it.begin(); cur!= it.end(); ++cur)
        {
            if (cur->Key() == key)   
            {
                if (!m_bFreeDataOnErase)
                    delete ((*cur).m_T_data);
                it.erase(cur);
                return;
            }
        }
    }

    template<class T>
    void
    clpHashTable<T>::EmptyHashTable()
    {
        if (!cHashTable<T>::pTableSize || cHashTable<T>::empty())
            return;
        for (typename cHashTable<T>::iterator iter = cHashTable<T>::begin() ; iter != cHashTable<T>::end() ; ++iter)   
        {
            if( m_bFreeDataOnErase )   
            {
                for (typename BucketsList::iterator cur = iter->begin() ; cur != iter->end() ; ++cur)
                    delete( (*cur).m_T_data );
            }
            iter->clear();
        }
        cHashTable<T>::clear();
        cHashTable<T>::pTableSize = 0;
    }


    template<class T>
    clpHashTable<T>::clpHashTable()
    {
        InitTable(__DEFAULT_HASH_TABLE_SIZE, true);
    }


    template<class T>
    clpHashTable<T>::clpHashTable(const uint32 TableSize, const bool FreeDataOnErase)
    {
        InitTable(TableSize, FreeDataOnErase);
    }



    template<class T>
    clpHashTable<T>::~clpHashTable()
    {
        this->EmptyHashTable();
    }


    template<class T>
    void
    clpHashTable<T>::InsertOrUpdate( const String &key, T v )
    {
        if (!cHashTable<T>::pTableSize)
            return;
        const uint32 hash = cHashTable<T>::generateHash(key);

        BucketsList& it = cHashTable<T>::at(hash);
        typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));

        if (cur != it.end())
        {
            if (m_bFreeDataOnErase)
                delete( (*cur).m_T_data );
            (*cur).m_T_data = v;
        }
        else
            cHashTable<T>::Insert(key, v);
    }


} // namespace UTILS
} // namespace TA3D

#endif // __TA3D_XX_HASH_TABLE_HXX__
