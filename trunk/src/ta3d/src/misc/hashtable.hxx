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
            return (ret % table.size());
        }

        template<class T>
                void
                cHashTable<T>::initTable(const uint32 TableSize, const bool /*freeDataOnErase*/)
        {
            if (!table.empty())
                this->emptyHashTable();
            table.resize(TableSize);        // Use default std::list<> constructor
        }

        template<class T>
                void
                cHashTable<T>::emptyHashTable()
        {
            if (table.empty())
                return;
            typename VectorOfBucketsList::iterator iter;
            for(iter = table.begin() ; iter != table.end() ; ++iter)
                iter->clear();
            table.clear();
        }

        template<class T>
                cHashTable<T>::cHashTable()
        {
            initTable(__DEFAULT_HASH_TABLE_SIZE);
        }

        template<class T>
                cHashTable<T>::cHashTable(uint32 TableSize)
        {
            initTable(TableSize);
        }

        template<class T>
                cHashTable<T>::~cHashTable()
        {
            this->emptyHashTable();
        }


        template<class T>
                bool
                cHashTable<T>::exists(const String& key)
        {
            if (table.empty())
                return false;
            const uint32 hash = generateHash(key);
            BucketsList& it = table[hash];
            typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
            return (cur != it.end());
        }


        template<class T>
                T
                cHashTable<T>::find(const String &key)
        {
            if (table.empty())
                return T();
            const uint32 hash = generateHash(key);
            BucketsList& it = table[hash];
            typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
            return ((cur == it.end()) ? (T) 0 : (*cur).m_T_data);
        }


        template<class T>
                bool
                cHashTable<T>::insert(const String& key, T v)
        {
            if(table.empty() || exists(key))
                return false;
            cBucket<T> elt(key, v);
            table[generateHash(key)].push_back(elt);
            return true;
        }


        template<class T>
                void
                cHashTable<T>::insertOrUpdate(const String& key, T v)
        {
            if (table.empty())
                return;
            const uint32 hash = generateHash(key);
            BucketsList& it = table[hash];
            typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));
            if( cur != it.end())
                (*cur).m_T_data = v;
            else
                insert(key, v);
        }


        template<class T>
                uint32
                cHashTable<T>::wildCardSearch(const String& pattern, String::List& li)
        {
            if (table.empty())
                return 0;
            String first;
            String last;
            String::size_type iFind = pattern.find('*');
            if (iFind != String::npos)
            {
                first = pattern.substr(0, iFind);
                first.toLower();
                ++iFind;
                last = pattern.substr(iFind);
                last.toLower();
            }
            else
            {
                first = pattern;
                first.toLower();
                last.clear();
            }

            uint32 nb(0);
            String::size_type firstLen = first.length();
            String::size_type lastLen = last.length();
            for (typename VectorOfBucketsList::iterator iter = table.begin() ; iter != table.end() ; ++iter)
            {
                for (typename BucketsList::iterator cur = iter->begin() ; cur != iter->end() ; ++cur)
                {
                    String f = cur->Key();
                    String::size_type fLen = f.length();
                    if (fLen < firstLen || fLen < lastLen)
                        continue;

                    if (f.substr(0, firstLen) == first)
                    {
                        if (f.substr(fLen - lastLen, lastLen) == last)
                        {
                            li.push_back(f);
                            ++nb;
                        }
                    }
                }
            }
            return nb;
        }




        template<class T>
                void
                cHashTable<T>::remove(const String& key)
        {
            if (table.empty())
                return;
            const uint32 hash = generateHash(key);

            BucketsList& it = table[hash];
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
                clpHashTable<T>::initTable(const uint32 tableSize, const bool freeDataOnErase)
        {
            m_bFreeDataOnErase = freeDataOnErase;
            if (cHashTable<T>::table.size())
                this->emptyHashTable();

            cHashTable<T>::table.resize(tableSize);     // Use std::list<> default constructor
        }


        template<class T>
                void
                clpHashTable<T>::remove(const String& key)
        {
            if (cHashTable<T>::table.empty())
                return;
            const uint32 hash = cHashTable<T>::generateHash(key);

            BucketsList& it = cHashTable<T>::table[hash];
            for (typename BucketsList::iterator cur= it.begin(); cur!= it.end(); ++cur)
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
                clpHashTable<T>::emptyHashTable()
        {
            if (cHashTable<T>::table.empty())
                return;
            for (typename VectorOfBucketsList::iterator iter = cHashTable<T>::table.begin() ; iter != cHashTable<T>::table.end() ; ++iter)
            {
                if (m_bFreeDataOnErase)
                {
                    for (typename BucketsList::iterator cur = iter->begin(); cur != iter->end(); ++cur)
                        delete( (*cur).m_T_data );
                }
                iter->clear();
            }
            cHashTable<T>::table.clear();
        }


        template<class T>
                clpHashTable<T>::clpHashTable()
        {
            initTable(__DEFAULT_HASH_TABLE_SIZE, true);
        }


        template<class T>
                clpHashTable<T>::clpHashTable(const uint32 TableSize, const bool FreeDataOnErase)
        {
            initTable(TableSize, FreeDataOnErase);
        }



        template<class T>
                clpHashTable<T>::~clpHashTable()
        {
            this->emptyHashTable();
        }


        template<class T>
                void
                clpHashTable<T>::insertOrUpdate(const String& key, T v)
        {
            if (cHashTable<T>::table.empty())
                return;
            const uint32 hash = cHashTable<T>::generateHash(key);

            BucketsList& it = cHashTable<T>::table.at(hash);
            typename BucketsList::iterator cur = std::find_if(it.begin(), it.end(), std::bind2nd(ModHashFind<T>(), key));

            if (cur != it.end())
            {
                if (m_bFreeDataOnErase)
                    delete( (*cur).m_T_data );
                (*cur).m_T_data = v;
            }
            else
                cHashTable<T>::insert(key, v);
        }


        template<class T>
                template<typename C>
                void cHashTable<T>::forEach(C callback)
        {
            for (typename VectorOfBucketsList::iterator iter = table.begin(); iter != table.end(); ++iter)
            {
                for (typename BucketsList::iterator cur = iter->begin() ; cur != iter->end(); ++cur)
                {
                    if (!callback(cur->m_szKey, cur->m_T_data))
                        return;
                }
            }

        }



    } // namespace UTILS
} // namespace TA3D

#endif // __TA3D_XX_HASH_TABLE_HXX__
