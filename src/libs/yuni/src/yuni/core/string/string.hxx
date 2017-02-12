#ifndef __YUNI_CORE_STRING_HXX__
# define __YUNI_CORE_STRING_HXX__

# include <cassert>
# include <iostream>




namespace Yuni
{


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>::StringBase()
		:pSize(0), pCapacity(0), pPtr(NULL)
	{}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>::StringBase(const NullPtr&)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{}



	template<typename C, int Chunk>
	StringBase<C,Chunk>::StringBase(const StringBase<C,Chunk>& copy)
		:pSize(copy.pSize)
	{
		if (pSize)
		{
			pCapacity = pSize + 1;
			pPtr = (Char*)::malloc( sizeof(Char) * pCapacity);
			memcpy(pPtr, copy.pPtr, sizeof(Char) * pSize);
			pPtr[pSize] = '\0';
		}
		else
		{
			pPtr = NULL;
			pCapacity = 0;
		}
	}

	template<typename C, int Chunk>
	template<int Chk1>
	StringBase<C,Chunk>::StringBase(const StringBase<C,Chk1>& copy)
		:pSize(copy.pSize)
	{
		if (pSize)
		{
			pCapacity = pSize + 1;
			pPtr = (Char*)::malloc( sizeof(Char) * pCapacity);
			memcpy(pPtr, copy.pPtr, sizeof(Char) * pSize);
			pPtr[pSize] = '\0';
		}
		else
		{
			pPtr = NULL;
			pCapacity = 0;
		}
	}



	template<typename C, int Chunk>
	template<int Chk1>
	inline StringBase<C,Chunk>::StringBase(const StringBase<C,Chk1>& rhs, const typename StringBase<C,Chunk>::Size offset,
		typename StringBase<C,Chunk>::Size len)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From< StringBase<C, Chk1> >::Append(*this, rhs, offset, len);
	}


	template<typename C, int Chunk>
	template<typename C1>
	inline StringBase<C,Chunk>::StringBase(const std::basic_string<C1>& rhs, const typename StringBase<C,Chunk>::Size offset,
		typename StringBase<C,Chunk>::Size len)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From<std::basic_string<C1> >::Append(*this, rhs, offset, len);
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>::StringBase(const char* str)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From<typename Static::Remove::Const<char*>::Type>::Append(*this, str);
	}


	template<typename C, int Chunk>
	StringBase<C,Chunk>::StringBase(const wchar_t* str)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From<typename Static::Remove::Const<wchar_t*>::Type>::Append(*this, str);
	}



	template<typename C, int Chunk>
	StringBase<C,Chunk>::StringBase(const Char c)
		:pSize(1), pCapacity(Chunk)
	{
		pPtr = (Char*)::malloc(sizeof(Char) * pCapacity);
		*pPtr = c;
		*(pPtr + 1) = '\0';
	}

	template<typename C, int Chunk>
	StringBase<C,Chunk>::StringBase(typename StringBase<C,Chunk>::Size n, const Char c)
		:pSize(n), pCapacity(0), pPtr(NULL)
	{
		if (n)
		{
			do
			{
				pCapacity += Chunk;
			} while (pCapacity < n);
			pPtr = (Char*)::malloc(sizeof(Char) * pCapacity);
			for (typename StringBase<C,Chunk>::Size i = 0; i < n; ++i)
				pPtr[i] = c;
			pPtr[pSize] = '\0';
		}
	}



	template<typename C, int Chunk>
	template<typename C1>
	inline StringBase<C,Chunk>::StringBase(const std::basic_string<C1>& rhs)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From<std::basic_string<C1> >::Append(*this, rhs);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>::StringBase(const U& u)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
	}

	template<typename C, int Chunk>
	template<typename C1>
	inline StringBase<C,Chunk>::StringBase(const C1* str, const typename StringBase<C,Chunk>::Size len)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		if (str && len)
		{
			Private::StringImpl::From<C1*>::AppendRaw(*this, str,
				Private::StringImpl::Min(
					Private::StringImpl::Length<StringBase<C,Chunk>, C1*>::Value(str), len));
		}
	}


	template<typename C, int Chunk>
	template<class IteratorClass>
	inline StringBase<C,Chunk>::StringBase(const IteratorClass& begin, const IteratorClass& end)
		:pSize(0), pCapacity(0), pPtr(NULL)
	{
		if (begin.pStr && begin.pIndx < begin.pStr->pSize && end.pIndx >= begin.pIndx)
			append(begin.pStr->pPtr + begin.pIndx, end.pIndx - begin.pIndx + 1);
	}



	template<typename C, int Chunk>
	inline StringBase<C,Chunk>::~StringBase()
	{
		::free(pPtr);
		# ifndef NDEBUG
		pPtr = NULL;
		# endif
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>& StringBase<C,Chunk>::clear()
	{
		if (pSize)
		{
			pSize = 0;
			pPtr[0] = '\0';
		}
		return *this;
	}



	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::empty() const
	{
		return !pSize;
	}

	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::notEmpty() const
	{
		return (0 != pSize);
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::size() const
	{
		return pSize;
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::count() const
	{
		return pSize;
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::length() const
	{
		return pSize;
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::sizeUTF8() const
	{
		typename StringBase<C,Chunk>::Size len(0);
		for (typename StringBase<C,Chunk>::Size i = 0; i < pSize; ++i)
		{
			if (((unsigned char)pPtr[i]) >= 0xC0 || ((unsigned char)pPtr[i]) < 0x80)
				++len;
		}
		return len;
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::countUTF8() const
	{
		return sizeUTF8();
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::lengthUTF8() const
	{
		return sizeUTF8();
	}




	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::capacity() const
	{
		return this->pCapacity;
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::max_size() const
	{
		return this->pCapacity;
	}




	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::reserve(const Size min)
	{
		// Really something to do ?
		if (this->pCapacity <= min)
		{
			// Looking for the good capacity
			do
			{
				this->pCapacity += Chunk;
			}
			while (this->pCapacity < min);

			// ReAllocating
			this->pPtr = (Char*) ::realloc(this->pPtr, sizeof(Char) * this->pCapacity);
		}
	}


	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::resize(const Size len)
	{
		this->reserve(len + 1);
		this->pSize = len;
		this->pPtr[len] = '\0';
	}



	template<typename C, int Chunk>
	void
	StringBase<C,Chunk>::shrink()
	{
		// Have we have a buffer to shrink ?
		if (pCapacity)
		{
			if (!pSize)
			{
				// The string is empty.
				pCapacity = 0;
				::free(pPtr);
				pPtr = NULL;
			}
			else
			{
				// Shrink the string as much as possible
				pCapacity = pSize + 1;
				pPtr = (Char*)::realloc(pPtr, sizeof(Char) * pCapacity);
				// Ensures the string is still zero-terminated
				pPtr[pSize] = '\0';
			}
		}
	}



	template<typename C, int Chunk>
	inline const char*
	StringBase<C,Chunk>::c_str() const throw()
	{
		return pPtr;
	}

	template<typename C, int Chunk>
	inline const C*
	StringBase<C,Chunk>::data() const throw()
	{
		return pPtr;
	}

	template<typename C, int Chunk>
	inline C*
	StringBase<C,Chunk>::data() throw()
	{
		return pPtr;
	}



	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator = (const StringBase<C,Chunk>& rhs)
	{
		assign(rhs);
		return *this;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator = (const NullPtr&)
	{
		clear();
		return *this;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator += (const NullPtr&)
	{
		/* Do nothing */
		return *this;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator << (const NullPtr&)
	{
		/* Do nothing */
		return *this;
	}




	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator += (const U& u)
	{
		append(u);
		return *this;
	}

	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator << (const U& u)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		return *this;
	}

	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::operator = (const U& u)
	{
		clear();
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		return *this;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::assignRaw(const char* u, const typename StringBase<C,Chunk>::Size len)
	{
		// Resizing the string to zero
		clear();
		// Appending the raw string
		if (u && len)
			Private::StringImpl::From<char*>::AppendRaw(*this, u, len);
		return *this;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::appendRaw(const char* u, const typename StringBase<C,Chunk>::Size len)
	{
		if (u && len)
			Private::StringImpl::From<char*>::AppendRaw(*this, u, len);
		return *this;
	}



	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::assign(const U& u)
	{
		clear();
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::assign(const U& u, const typename StringBase<C,Chunk>::Size len)
	{
		clear();
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u, len);
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline void
	StringBase<C,Chunk>::assign(const StringBase<C,Chnk1>& s, const Size offset, const Size len)
	{
		if (offset < s.pSize && len)
			assign(s.pPtr + offset, Private::StringImpl::Min(len, s.pSize - offset));
		else
			clear();
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator ! () const
	{
		return !pSize;
	}


	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::print(std::ostream& out) const
	{
		if (pSize)
			out.write(pPtr, static_cast<std::streamsize>(sizeof(Char) * pSize));
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::iterator
	StringBase<C,Chunk>::begin()
	{
		return iterator(*this);
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::iterator
	StringBase<C,Chunk>::end()
	{
		return iterator(*this, pSize);
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::const_iterator
	StringBase<C,Chunk>::begin() const
	{
		return const_iterator(*this);
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::const_iterator
	StringBase<C,Chunk>::end() const
	{
		return const_iterator(*this, pSize);
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::reverse_iterator
	StringBase<C,Chunk>::rbegin()
	{
		return reverse_iterator(*this);
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::reverse_iterator
	StringBase<C,Chunk>::rend()
	{
		return reverse_iterator(*this, Size(-1));
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::const_reverse_iterator
	StringBase<C,Chunk>::rbegin() const
	{
		return const_reverse_iterator(*this);
	}

	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::const_reverse_iterator
	StringBase<C,Chunk>::rend() const
	{
		return const_reverse_iterator(*this, Size(-1));
	}


	template<typename C, int Chunk>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::countChar(const C c) const
	{
		return Private::StringImpl::CountChar<StringBase<C,Chunk>, StringBase<C,Chunk>
			> :: Value(*this, c);
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	bool
	StringBase<C,Chunk>::hasChar(const StringBase<C,Chnk1>& str) const
	{
		for (Size i = 0; i < pSize; ++i)
		{
			if (str.hasChar(pPtr[i]))
				return true;
		}
		return false;
	}



	template<typename C, int Chunk>
	bool
	StringBase<C,Chunk>::startsWith(const Char* s, CharCase option) const
	{
		if (s && '\0' != *s)
		{
			Size len = Length(s);
			if (len <= pSize)
			{
				return (soIgnoreCase == option) ? !CompareInsensitive(pPtr, s, len)
					: !Compare(pPtr, s, len);
			}
		}
		return false;
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	bool
	StringBase<C,Chunk>::startsWith(const StringBase<C,Chnk1>& s, CharCase option) const
	{
		if (pSize && s.pSize <= pSize)
		{
			return (soIgnoreCase == option) ? !CompareInsensitive(pPtr, s.pPtr, s.pSize)
				: !Compare(pPtr, s.pPtr, s.pSize);
		}
		return false;
	}


	template<typename C, int Chunk>
	bool
	StringBase<C,Chunk>::glob(const C* pattern) const
	{
		// TODO This method should be completly removed
		Size len = Length(pattern);
		if (pattern && len)
		{
			if (pSize)
			{
				Size e = 0;
				Size prev = npos;
				for (Size i = 0 ; i < pSize; ++i)
				{
					if ('*' == pattern[e])
					{
						if (e + 1 == len)
							return true;
						while (pattern[e+1] == '*')
							++e;
						if (e + 1 == len)
							return true;

						prev = e;
						if (pattern[e + 1] == pPtr[i])
							e += 2;
					}
					else
					{
						if (pattern[e] == pPtr[i])
							++e;
						else
							if (prev != npos)
								e = prev;
							else
								return false;
					}
				}
				return (e == len);
			}
			return false;
		}
		return empty();
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	bool
	StringBase<C,Chunk>::glob(const StringBase<C,Chnk1>& pattern) const
	{
		// TODO This method should be completly removed
		if (pattern.pSize)
		{
			if (pSize)
			{
				Size e = 0;
				Size prev = npos;
				for (Size i = 0 ; i < pSize; ++i)
				{
					if ('*' == pattern.pPtr[e])
					{
						if (e + 1 == pattern.pSize)
							return true;
						while (pattern.pPtr[e+1] == '*')
							++e;
						if (e + 1 == pattern.pSize)
							return true;

						prev = e;
						if (pattern.pPtr[e + 1] == pPtr[i])
							e += 2;
					}
					else
					{
						if (pattern.pPtr[e] == pPtr[i])
							++e;
						else
							if (prev != npos)
								e = prev;
							else
								return false;
					}
				}
				return (e == pattern.pSize);
			}
			return false;
		}
		return empty();
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_first_of(const U& u) const
	{
		return Private::StringImpl:: template FindFirstOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, true>
			::Value(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_first_of(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl:: template FindFirstOf<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, true>
			::Value(*this, u, offset);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_first_not_of(const U& u) const
	{
		return Private::StringImpl:: template FindFirstOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, false>
			::Value(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_first_not_of(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl:: template FindFirstOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, false>
			::Value(*this, u, offset);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_last_of(const U& u) const
	{
		return Private::StringImpl:: template FindLastOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, true>
			::Value(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_last_of(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl:: template FindLastOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, true>
			::Value(*this, u, offset);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_last_not_of(const U& u) const
	{
		return Private::StringImpl:: template FindLastOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, false>
			::Value(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find_last_not_of(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl:: template FindLastOf< StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type, false>
			::Value(*this, u, offset);
	}



	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find(const U& u) const
	{
		return Private::StringImpl::Find<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type>::Value(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline bool
	StringBase<C,Chunk>::contains(const U& u) const
	{
		return (npos !=
			Private::StringImpl::Find<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type>::Value(*this, u));
	}



	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::find(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl::Find<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type>::Value(*this, u, offset);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::rfind(const U& u) const
	{
		return Private::StringImpl::Find<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type>::ReverseValue(*this, u);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::rfind(const U& u, typename StringBase<C,Chunk>::Size offset) const
	{
		return Private::StringImpl::Find<StringBase<C,Chunk>, typename Static::Remove::Const<U>::Type>::ReverseValue(*this, u, offset);
	}



	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::Length(const U& u)
	{
		return Private::StringImpl::Length<
			StringBase<C,Chunk>,
			typename Static::Remove::Const<U>::Type> :: Value(u);
	}



	template<typename C, int Chunk>
	inline int
	StringBase<C,Chunk>::Compare(const Char* a, const Char* b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return (a && b) ? Private::StringImpl::Impl<C,Chunk>::Compare(a, b, maxLen) : -1;
	}


	template<typename C, int Chunk>
	inline int
	StringBase<C,Chunk>::CompareInsensitive(const Char* a, const Char* b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return (a && b) ? Private::StringImpl::Impl<C,Chunk>::CompareInsensitive(a, b, maxLen) : -1;
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline int
	StringBase<C,Chunk>::Compare(const StringBase<Char,Chnk1>& a, const C* b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return a.pSize && b ? Private::StringImpl::Impl<C,Chunk>::Compare(a.pPtr, b, maxLen > a.pSize ? a.pSize : maxLen) : 1;
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline int
	StringBase<C,Chunk>::Compare(const C* a, const StringBase<Char,Chnk1>& b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return a && b.pSize ? Private::StringImpl::Impl<C,Chunk>::Compare(a, b.c_str(), maxLen) : -1;
	}

	template<typename C, int Chunk>
	template<int Chnk1, int Chnk2>
	inline int
	StringBase<C,Chunk>::Compare(const StringBase<Char,Chnk1>& a, const StringBase<Char,Chnk2>& b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return a.pSize && b.pSize ? Private::StringImpl::Impl<C,Chunk>::Compare(a.pPtr, b.c_str(), maxLen > a.pSize ? a.pSize : maxLen) : -1;
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator < (const C* rhs) const
	{
		return (rhs ? (-1 == Private::StringImpl::Impl<C,Chunk>::Compare(pPtr, rhs, pSize)) : false);
	}

	template<typename C, int Chunk>
	template<int Chnk1>
	inline bool
	StringBase<C,Chunk>::operator < (const StringBase<C,Chnk1>& rhs) const
	{
		return Private::StringImpl::Impl<C,Chunk>::Compare(pPtr, rhs.pPtr,
			Private::StringImpl::Min(pSize, rhs.pSize)) < 0;
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator > (const C* rhs) const
	{
		return (rhs ? (1 == Private::StringImpl::Impl<C,Chunk>::Compare(pPtr, rhs, pSize)) : true);
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline bool
	StringBase<C,Chunk>::operator > (const StringBase<C,Chnk1>& rhs) const
	{
		return Private::StringImpl::Impl<C,Chunk>::Compare(pPtr, rhs.pPtr,
			Private::StringImpl::Min(pSize, rhs.pSize)) > 0;
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator == (const C* rhs) const
	{
		return (::strlen(rhs) == pSize) && Private::StringImpl::Impl<C,Chunk>::StrictlyEquals(pPtr, rhs);
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator == (const NullPtr&) const
	{
		return !pSize;
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator != (const NullPtr&) const
	{
		return (0 != pSize);
	}


	template<typename C, int Chunk>
	template<int N>
	inline bool
	StringBase<C,Chunk>::operator == (const C rhs[N]) const
	{
		return ((N - 1) == pSize) && Private::StringImpl::Impl<C,Chunk>::Equals(pPtr, rhs, pSize);
	}



	template<typename C, int Chunk>
	template<int Chnk1>
	inline bool
	StringBase<C,Chunk>::operator == (const StringBase<C,Chnk1>& rhs) const
	{
		return (rhs.size() == pSize) && StringBase<C,Chunk>::Equals(*this, rhs);
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::operator != (const C* rhs) const
	{
		return (::strlen(rhs) != pSize) || !Private::StringImpl::Impl<C,Chunk>::Equals(pPtr, rhs, pSize);
	}


	template<typename C, int Chunk>
	template<int N>
	inline bool
	StringBase<C,Chunk>::operator != (const C rhs[N]) const
	{
		return !(((N - 1) == pSize) && Private::StringImpl::Impl<C,Chunk>::Equals(pPtr, rhs, pSize));
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline bool
	StringBase<C,Chunk>::operator != (const StringBase<C,Chnk1>& rhs) const
	{
		return (rhs.size() != pSize) || !StringBase<C,Chunk>::Equals(*this, rhs);
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::Equals(const Char* a, const Char* b)
	{
		return Private::StringImpl::Impl<C,Chunk>::Equals(a, b);
	}


	template<typename C, int Chunk>
	inline bool
	StringBase<C,Chunk>::Equals(const Char* a, const Char* b, const typename StringBase<C,Chunk>::Size maxLen)
	{
		return Private::StringImpl::Impl<C,Chunk>::Equals(a, b, maxLen);
	}

	template<typename C, int Chunk>
	template<int Chnk1, int Chnk2>
	inline bool
	StringBase<C,Chunk>::Equals(const StringBase<C,Chnk1>& a, const StringBase<C,Chnk2>& b)
	{
		return (a.pSize == b.pSize) &&
			(!a.pSize || Private::StringImpl::Impl<C,Chunk>::StrictlyEquals(a.pPtr, b.pPtr, a.pSize));
	}



	template<typename C, int Chunk>
	template<int Chnk1>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::format(const StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		this->clear();
		StringBase<C,Chunk> s(f);
		vappendFormat(s.c_str(), parg);
		va_end(parg);
		return *this;
	}

	template<typename C, int Chunk>
	template<int Chnk1>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::format(StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		this->clear();
		vappendFormat(f.c_str(), parg);
		va_end(parg);
		return *this;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::format(const char* f, ...)
	{
		va_list parg;
		va_start(parg, f);
		this->clear();
		vappendFormat(f, parg);
		va_end(parg);
		return *this;
	}



	template<typename C, int Chunk>
	template<int Chnk1>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::appendFormat(const StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		StringBase<C,Chunk> s(f);
		vappendFormat(s.c_str(), parg);
		va_end(parg);
		return *this;
	}

	template<typename C, int Chunk>
	template<int Chnk1>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::appendFormat(StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		vappendFormat(f.c_str(), parg);
		va_end(parg);
		return *this;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::appendFormat(const char* f, ...)
	{
		va_list parg;
		va_start(parg, f);
		vappendFormat(f, parg);
		va_end(parg);
		return *this;
	}




	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::vappendFormat(const char* f, va_list parg)
	{
		Private::StringImpl::vsprintf(*this, f, parg);
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	StringBase<C,Chunk>
	StringBase<C,Chunk>::Format(const StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		StringBase<> s;
		s.vappendFormat(f.c_str(), parg);
		va_end(parg);
		return s;
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::Format(StringBase<C,Chnk1>& f, ...)
	{
		va_list parg;
		va_start(parg, f);
		StringBase<> s;
		s.vappendFormat(f.c_str(), parg);
		va_end(parg);
		return s;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::Format(const char* f, ...)
	{
		va_list parg;
		va_start(parg, f);
		StringBase<> s;
		s.vappendFormat(f, parg);
		va_end(parg);
		return s;
	}


	template<typename C, int Chunk>
	template<class U>
	inline void
	StringBase<C,Chunk>::push_back(const U& u)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
	}

	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::put(const C c)
	{
		Private::StringImpl::From<C>::Append(*this, c);
	}


	template<typename C, int Chunk>
	template<class U>
	inline void
	StringBase<C,Chunk>::append(const U& u)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
	}

	template<typename C, int Chunk>
	template<class U>
	inline void
	StringBase<C,Chunk>::append(const U& u, const typename StringBase<C,Chunk>::Size len)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u, len);
	}

	template<typename C, int Chunk>
	template<class U>
	inline void
	StringBase<C,Chunk>::append(const U& u, const typename StringBase<C,Chunk>::Size offset, const typename StringBase<C,Chunk>::Size len)
	{
		Private::StringImpl::From<typename Static::Remove::Const<U>::Type
			>::Append(*this, u, offset, len);
	}

	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc>
	inline void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list, const unsigned int max)
	{
		append(list, ", ", max);
	}

	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc>
	inline void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list)
	{
		append(list, ", ");
	}

	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc, typename S, typename E>
	void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list, const S& separator, const E& enclosure, const unsigned int max)
	{
		if (!list.empty() && max)
		{
			// Adding the first item
			typename L<TypeL,Alloc>::const_iterator i = list.begin();
			append(enclosure);
			append(*i);
			append(enclosure);
			++i;

			// All other items
			const typename L<TypeL,Alloc>::const_iterator end = list.end();
			unsigned int pos(1);
			for (; pos != max && i != end; ++i, ++pos)
			{
				append(separator);
				append(enclosure);
				append(*i);
				append(enclosure);
			}
		}
	}


	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc, typename S, typename E>
	void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list, const S& separator, const E& enclosure)
	{
		if (!list.empty())
		{
			// Adding the first item
			typename L<TypeL,Alloc>::const_iterator i = list.begin();
			append(enclosure);
			append(*i);
			append(enclosure);
			++i;

			// All other items
			const typename L<TypeL,Alloc>::const_iterator end = list.end();
			for (; i != end; ++i)
			{
				append(separator);
				append(enclosure);
				append(*i);
				append(enclosure);
			}
		}
	}



	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc, typename S>
	void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list, const S& separator, const unsigned int max)
	{
		if (!list.empty() && max)
		{
			// Adding the first item
			typename L<TypeL,Alloc>::const_iterator i = list.begin();
			append(*i);
			++i;

			// All other items
			const typename L<TypeL,Alloc>::const_iterator end = list.end();
			unsigned int pos(1);
			for (; pos != max && i != end; ++i, ++pos)
			{
				append(separator);
				append(*i);
			}
		}
	}

	template<typename C, int Chunk>
	template<template<class,class> class L, class TypeL, class Alloc, typename S>
	void
	StringBase<C,Chunk>::append(const L<TypeL,Alloc>& list, const S& separator)
	{
		if (!list.empty())
		{
			// Adding the first item
			typename L<TypeL,Alloc>::const_iterator i = list.begin();
			append(*i);
			++i;

			// All other items
			const typename L<TypeL,Alloc>::const_iterator end = list.end();
			for (; i != end; ++i)
			{
				append(separator);
				append(*i);
			}
		}
	}



	template<typename C, int Chunk>
	template<typename C1, class U>
	inline bool
	StringBase<C,Chunk>::HasChar(const C1 c, const U& u)
	{
		return Private::StringImpl::HasChar<
			StringBase<C,Chunk>,
			typename Static::Remove::Const<U>::Type> :: Value(u, c);
	}


	template<typename C, int Chunk>
	template<typename C1, class U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::CountChar(const C1 c, const U& u)
	{
		return Private::StringImpl::CountChar<
			StringBase<C,Chunk>,
			typename Static::Remove::Const<U>::Type> :: Value(u, c);
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&  StringBase<C,Chunk>::toLower()
	{
		for (typename StringBase<C,Chunk>::Size i = 0; i < pSize; ++i)
			pPtr[i] = (C)tolower(pPtr[i]);
		return *this;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&  StringBase<C,Chunk>::toUpper()
	{
		for (typename StringBase<C,Chunk>::Size i = 0; i < pSize; ++i)
			pPtr[i] = (C)toupper(pPtr[i]);
		return *this;
	}



	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::ToLower(const U& u)
	{
		return StringBase<C,Chunk>(u).toLower();
	}

	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::ToUpper(const U& u)
	{
		return StringBase<C,Chunk>(u).toUpper();
	}


	template<typename C, int Chunk>
	template<template<class,class> class L, class T, class Alloc>
	typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::FindInList(const L<T,Alloc>& list, const StringBase<Char,Chunk>& str, const typename StringBase<C,Chunk>::Size offset)
	{
		if (!list.empty())
		{
			unsigned int pos(0);
			const typename L<T,Alloc>::const_iterator end = list.end();
			for (typename L<T,Alloc>::const_iterator i = list.begin(); i != end; ++i)
			{
				if (pos >= offset && StringBase<C,Chunk>::Equals(str, *i))
					return pos;
				++pos;
			}
		}
		return StringBase<C,Chunk>::npos;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::ConvertEscapedCharacters(const char str[])
	{
		return (str && '\0' != *str)
			? ConvertEscapedCharacters(StringBase<C,Chunk>(str))
			: StringBase<C,Chunk>();
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::ConvertEscapedCharacters(const wchar_t str[])
	{
		return (str && '\0' != *str)
			? ConvertEscapedCharacters(StringBase<C,Chunk>(str))
			: StringBase<C,Chunk>();
	}

	template<typename C, int Chunk>
	template<typename U>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::ConvertEscapedCharacters(const std::basic_string<U>& str)
	{
		return str.empty()
			? StringBase<C,Chunk>()
			: ConvertEscapedCharacters(StringBase<C,Chunk>(str));
	}



	template<typename C, int Chunk>
	template<int Chnk1>
	StringBase<C,Chunk>
	StringBase<C,Chunk>::ConvertEscapedCharacters(const StringBase<C,Chnk1>& str)
	{
		if (str.empty())
			return StringBase<C,Chunk>();
		// Preparing the copy
		StringBase<C,Chunk> ret;
		ret.assignFromEscapedCharacters(str);
		return ret;
	}

	template<typename C, int Chunk>
	template<int Chnk1>
	inline void
	StringBase<C,Chunk>::assignFromEscapedCharacters(const StringBase<C,Chnk1>& str)
	{
		assignFromEscapedCharacters(str, str.pSize);
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	void
	StringBase<C,Chunk>::assignFromEscapedCharacters(const StringBase<C,Chnk1>& str, typename StringBase<C,Chunk>::Size maxLen,
		const typename StringBase<C,Chunk>::Size offset)
	{
		clear();
		if (!str.pSize || offset > str.pSize || !maxLen)
			return;
		if (maxLen == npos)
		{
			maxLen = str.pSize - offset;
		}
		else
		{
			maxLen += offset;
			if (maxLen > str.pSize)
				maxLen = str.pSize;
		}
		// Preparing the copy
		pSize = maxLen - offset;
		reserve(pSize);

		// Browsing all char
		Size retPos(0);
		for (Size i = offset; i < maxLen; ++i, ++retPos)
		{
			if ('\\' == str.pPtr[i] && i + 1 != maxLen)
			{
				switch (str.pPtr[i + 1])
				{
					case 'r'  : pPtr[retPos] = '\r'; break;
					case 'n'  : pPtr[retPos] = '\n'; break;
					case '\\' : pPtr[retPos] = '\\'; break;
					case ';'  : pPtr[retPos] = ';'; break;
					case 'a'  : pPtr[retPos] = '\a'; break;
					case 'f'  : pPtr[retPos] = '\f'; break;
					case 't'  : pPtr[retPos] = '\t'; break;
					case '\'' : pPtr[retPos] = '\''; break;
					case '"'  : pPtr[retPos] = '"'; break;
					default   : pPtr[retPos] = str.pPtr[i]; continue;
				}
				--pSize;
				++i;
				continue;
			}
			pPtr[retPos] = str.pPtr[i];
		}
		if (pSize)
			pPtr[pSize] = '\0';
	}






	template<typename C, int Chunk>
	uint32
	StringBase<C,Chunk>::hashValue() const
	{
		if (pSize)
		{
			uint32 hash(0);
			for (typename StringBase<C,Chunk>::Size i = 0; i != pSize; ++i)
				hash = (hash << 5) - hash + *(pPtr + i);
			return hash;
		}
		return 0;
	}


	template<typename C, int Chunk>
	void StringBase<C,Chunk>::convertSlashesIntoBackslashes()
	{
		for (typename StringBase<C,Chunk>::Size i = 0; i < pSize; ++i)
		{
			if ('/' == pPtr[i])
				pPtr[i] = '\\';
		}
	}

	template<typename C, int Chunk>
	void StringBase<C,Chunk>::convertBackslashesIntoSlashes()
	{
		for (typename StringBase<C,Chunk>::Size i = 0; i < pSize; ++i)
		{
			if ('\\' == pPtr[i])
				pPtr[i] = '/';
		}
	}


	template<typename C, int Chunk>
	template<int Ck1, int Ck2>
	inline void
	StringBase<C,Chunk>::ExtractKeyValue(const StringBase<C,Chunk>& s, StringBase<C,Ck1>& key, StringBase<C,Ck2>& value,
		const enum CharCase chcase)
	{
		s.extractKeyValue(key, value, chcase);
	}



	template<typename C, int Chunk>
	template<int Ck1, int Ck2>
	inline void
	StringBase<C,Chunk>::extractKeyValue(StringBase<C,Ck1>& key, StringBase<C,Ck2>& value, const enum CharCase chcase) const
	{
		// ReInitializing
		key.clear();
		value.clear();

		if (!pSize) // The string is empty, Nothing to do
			return;

		unsigned int left(0);
		while (left != pSize && HasChar(pPtr[left], YUNI_STRING_SEPARATORS))
			++left;

		if (left == pSize) // The string is actually an empty string (composed only by separators)
			return;

		// Section
		if ('{' == pPtr[left])
		{
			key.append('{');
			return ;
		}
		if ('}' == pPtr[left])
		{
			key.append('}');
			return ;
		}
		if ('[' == pPtr[left])
		{
			key.append('[');
			++left;
			typename StringBase<C,Chunk>::Size right = find(']', left);
			if (right != npos && right != left)
			{
				value.append(*this, left, right - left);
				value.trim();
			}
			return;
		}

		// If not a section, it should be a key/value
		// Looking for the symbol `=`
		typename StringBase<C,Chunk>::Size equal = find_first_of("=/;", left);
		if (equal == npos || equal == left || '=' != pPtr[equal])
			return;

		// Getting our key
		key.assign(*this, left, equal - left);
		key.trimRight();
		if (chcase == soIgnoreCase)
			key.toLower();

		// Looking for the first interesting char
		typename StringBase<C,Chunk>::Size leftValue(equal);
		++leftValue; // After the symbol `=`
		while (leftValue < pSize && HasChar(pPtr[leftValue], YUNI_STRING_SEPARATORS))
			++leftValue;
		if (leftValue < pSize) // Empty value
		{
			switch (pPtr[leftValue])
			{
				case ';':
					// Empty value
					break;
				case '"':
				case '\'':
					{
						// Value enclosed in a string
						++leftValue;
						const typename StringBase<C,Chunk>::Size next
							= FindEndOfSequence(pPtr + leftValue, pPtr[leftValue-1], pSize - leftValue);
						if (next != npos)
							value.assignFromEscapedCharacters(*this, next, leftValue);
						else
							value.append(pPtr + leftValue, pSize - leftValue);
						break;
					}
				case '/':
					// Empty value if we have a comment otherwise '/' is a valid entry
					if (leftValue + 1 >= pSize || pPtr[leftValue + 1] == '/')
						break;
				default:
					{
						// Standard value
						const typename StringBase<C,Chunk>::Size semicolon = find_first_of(';', leftValue);
						value.append(*this, leftValue, semicolon - leftValue);
						value.trimRight();
						break;
					}
			}
		}
	}



	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::trimRight()
	{
		if (pSize)
		{
			while (pSize && HasChar(pPtr[pSize-1], YUNI_STRING_SEPARATORS))
				--pSize;
			pPtr[pSize] = '\0';
		}
	}


	template<typename C, int Chunk>
	template<typename U>
	inline void StringBase<C,Chunk>::trimRight(const U& separators)
	{
		if (pSize)
		{
			while (pSize && HasChar(pPtr[pSize-1], separators))
				--pSize;
			pPtr[pSize] = '\0';
		}
	}


	template<typename C, int Chunk>
	void StringBase<C,Chunk>::trimLeft()
	{
		if (pSize && HasChar(*pPtr, YUNI_STRING_SEPARATORS))
		{
			unsigned int pos(1);
			while (pos < pSize && HasChar(pPtr[pos], YUNI_STRING_SEPARATORS))
				++pos;
			pSize -= pos;
			for (unsigned int i = 0; i != pSize; ++i)
				pPtr[i] = pPtr[pos + i];
			pPtr[pSize] = '\0';
		}
	}

	template<typename C, int Chunk>
	template<typename U>
	void StringBase<C,Chunk>::trimLeft(const U& separators)
	{
		if (pSize && HasChar(*pPtr, separators))
		{
			unsigned int pos(1);
			while (pos < pSize && HasChar(pPtr[pos], separators))
				++pos;
			pSize -= pos;
			for (unsigned int i = 0; i != pSize; ++i)
				pPtr[i] = pPtr[pos + i];
			pPtr[pSize] = '\0';
		}
	}


	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::trim()
	{
		trimRight();
		trimLeft();
	}

	template<typename C, int Chunk>
	template<typename U>
	void StringBase<C,Chunk>::trim(const U& separators)
	{
		trimRight(separators);
		trimLeft(separators);
	}


	template<typename C, int Chunk>
	inline C StringBase<C,Chunk>::first() const
	{
		return (pSize) ? *pPtr : '\0';
	}

	template<typename C, int Chunk>
	inline C StringBase<C,Chunk>::last() const
	{
		return (pSize) ? pPtr[pSize-1] : '\0';
	}

	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::removeLast()
	{
		if (pSize)
		{
			--pSize;
			pPtr[pSize] = '\0';
		}
	}

	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::removeTrailingSlash()
	{
		if (pSize && ('/' == pPtr[pSize-1] || '\\' == pPtr[pSize-1]))
		{
			--pSize;
			pPtr[pSize] = '\0';
		}
	}

	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::truncate(const typename StringBase<C,Chunk>::Size maxLen)
	{
		if (pSize > maxLen)
		{
			pSize = maxLen;
			pPtr[pSize] = '\0';
		}
	}


	template<typename C, int Chunk>
	inline void StringBase<C,Chunk>::chop(const typename StringBase<C,Chunk>::Size n)
	{
		if (pSize && pSize >= n)
		{
			pSize -= n;
			pPtr[pSize] = '\0';
		}
	}


	template<typename C, int Chunk>
	typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::FindEndOfSequence(const C* str, const C quote, typename StringBase<C,Chunk>::Size maxLen)
	{
		if (str)
		{
			bool escape = false;
			typename StringBase<C,Chunk>::Size pos(0);
			while (pos != maxLen && '\0' != str[pos])
			{
				if ('\\' == str[pos])
					escape = !escape;
				else
				{
					if (quote == str[pos] && escape)
						return pos;
					escape = false;
				}
				++pos;
			}
		}
		return StringBase<C,Chunk>::npos;
	}

	template<typename C, int Chunk>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::substr(const typename StringBase<C,Chunk>::Size offset) const
	{
		return (offset < pSize)
			// Substring
			? StringBase<C,Chunk>().appendRaw(pPtr + offset, pSize - offset)
			// Empty string
			: StringBase<C,Chunk>();
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>
	StringBase<C,Chunk>::substr(const typename StringBase<C,Chunk>::Size offset,
		typename StringBase<C,Chunk>::Size len) const
	{
		return (offset < pSize && len)
			// Substring
			? StringBase<C,Chunk>().appendRaw(pPtr + offset, Private::StringImpl::Min(pSize - offset, len))
			// Empty string
			: StringBase<C,Chunk>();
	}



	template<typename C, int Chunk>
	template<typename U>
	StringBase<C,Chunk>&
	StringBase<C,Chunk>::replace(const typename StringBase<C,Chunk>::Size offset, const typename StringBase<C,Chunk>::Size len, const U& u)
	{
		if (offset < pSize && len)
		{
			erase(offset, len);
			insert(offset, u);
		}
		return *this;
	}


	template<typename C, int Chunk>
	template<typename U, typename V>
	StringBase<C,Chunk>&
	StringBase<C,Chunk>::replace(const U& u, const V& v)
	{
		const typename StringBase<C,Chunk>::Size lenU = Length(u);
		if (lenU)
		{
			const typename StringBase<C,Chunk>::Size lenV = Length(v);
			typename StringBase<C,Chunk>::Size offset = find(u);
			while (npos != offset)
			{
				replace(offset, lenU, v);
				offset = find(u, offset + lenV);
			}
		}
		return *this;
	}

	template<typename C, int Chunk>
	StringBase<C,Chunk>&
	StringBase<C,Chunk>::replace(const C from, const C to)
	{
		for (Size i = 0; i != pSize; ++i)
		{
			if (from == pPtr[i])
				pPtr[i] = to;
		}
		return *this;
	}

	template<typename C, int Chunk>
	StringBase<C,Chunk>&
	StringBase<C,Chunk>::replace(const C from, const C to, const typename StringBase<C,Chunk>::Size offset)
	{
		for (Size i = offset; i < pSize; ++i)
		{
			if (from == pPtr[i])
				pPtr[i] = to;
		}
		return *this;
	}


	template<typename C, int Chunk>
	inline std::allocator<C>
	StringBase<C,Chunk>::get_allocator()
	{
		return std::allocator<C>();
	}


	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::swap(StringBase<C,Chunk>& s)
	{
		std::swap(s.pSize, pSize);
		std::swap(s.pCapacity, pCapacity);
		std::swap(s.pPtr, pPtr);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline U
	StringBase<C,Chunk>::to() const
	{
		return Private::StringImpl::To<typename Static::Remove::Const<U>::Type>::Value(*this);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline bool
	StringBase<C,Chunk>::to(U& u) const
	{
		return Private::StringImpl::To<typename Static::Remove::Const<U>::Type>::Value(*this, u);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::insert(const typename StringBase<C,Chunk>::Size offset, const U& u)
	{
		if (offset >= pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, offset);
	}


	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::insert(const iterator& it, const U& u)
	{
		if (it.pIndx >= pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, it.pIndx);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::insert(const const_iterator& it, const U& u)
	{
		if (it.pIndx >= pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, it.pIndx);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::insert(const reverse_iterator& it, const U& u)
	{
		if (it.pIndx >= pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, it.pIndx);
	}

	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::insert(const const_reverse_iterator& it, const U& u)
	{
		if (it.pIndx >= pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, it.pIndx);
	}



	template<typename C, int Chunk>
	template<typename U>
	inline void
	StringBase<C,Chunk>::prepend(const U& u)
	{
		if (!pSize)
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Append(*this, u);
		else
			Private::StringImpl::From<typename Static::Remove::Const<U>::Type>::Insert(*this, u, 0);
	}


	template<typename C, int Chunk>
	StringBase<C,Chunk>&
	StringBase<C,Chunk>::erase(const typename StringBase<C,Chunk>::Size offset, const typename StringBase<C,Chunk>::Size len)
	{
		if (offset < pSize && len)
		{
			if (offset + len > pSize)
				pSize = offset;
			else
			{
				memmove(pPtr + sizeof(C) * (offset), pPtr + sizeof(C) * (offset + len), sizeof(C) * (pSize-offset-len));
				pSize -= len;
			}
			pPtr[pSize] = '\0';
		}
		return *this;
	}


	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::erase(const iterator& begin, const iterator& end)
	{
		if (end.pIndx > begin.pIndx)
			erase(begin.pIndx, end.pIndx - begin.pIndx + 1);
		else
			erase(end.pIndx, begin.pIndx - end.pIndx + 1);
	}

	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::erase(const const_iterator& begin, const const_iterator& end)
	{
		if (end.pIndx > begin.pIndx)
			erase(begin.pIndx, end.pIndx - begin.pIndx + 1);
		else
			erase(end.pIndx, begin.pIndx - end.pIndx + 1);
	}

	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::erase(const reverse_iterator& begin, const reverse_iterator& end)
	{
		if (end.pIndx > begin.pIndx)
			erase(begin.pIndx, end.pIndx - begin.pIndx + 1);
		else
			erase(end.pIndx, begin.pIndx - end.pIndx + 1);
	}

	template<typename C, int Chunk>
	inline void
	StringBase<C,Chunk>::erase(const const_reverse_iterator& begin, const const_reverse_iterator& end)
	{
		if (end.pIndx > begin.pIndx)
			erase(begin.pIndx, end.pIndx - begin.pIndx + 1);
		else
			erase(end.pIndx, begin.pIndx - end.pIndx + 1);
	}



	template<typename C, int Chunk>
	template<typename U>
	inline typename StringBase<C,Chunk>::Size
	StringBase<C,Chunk>::remove(const U& u, typename StringBase<C,Chunk>::Size offset, const typename StringBase<C,Chunk>::Size maxOccurences)
	{
		return Private::StringImpl::Remove<
			StringBase<C,Chunk>,
			typename Static::Remove::Const<U>::Type> :: Run(*this, u, offset, maxOccurences);
	}



	template<typename C, int Chunk>
	inline C&
	StringBase<C,Chunk>::operator [] (const typename StringBase<C,Chunk>::Size indx)
	{
		return *(pPtr + indx);
	}

	template<typename C, int Chunk>
	inline const C&
	StringBase<C,Chunk>::operator [] (const typename StringBase<C,Chunk>::Size indx) const
	{
		return *(pPtr + indx);
	}



	template<typename C, int Chunk>
	template<template<class,class> class U, class UType, class Alloc, typename S>
	void
	StringBase<C,Chunk>::explode(U<UType,Alloc>& out, const S& sep, const bool emptyBefore, const bool keepEmptyElements, const bool trimElements) const
	{
		// Empty the container
		if (emptyBefore)
			out.clear();
		// String empty
		if (this->notEmpty())
		{
			typename StringBase<C,Chunk>::Size indx(0);
			typename StringBase<C,Chunk>::Size len(0);
			while (true)
			{
				typename StringBase<C,Chunk>::Size newIndx = this->find_first_of(sep, indx);
				if (StringBase<C,Chunk>::npos == newIndx)
				{
					StringBase<C,Chunk> segment(*this, indx, StringBase<>::npos);
					if (trimElements)
						segment.trim();
					if (segment.notEmpty() || keepEmptyElements)
						out.push_back(segment.to<UType>());
					return;
				}

				if ((newIndx && (len = newIndx - indx)) || keepEmptyElements)
				{
					StringBase<C,Chunk> segment(*this, indx, len);
					if (trimElements)
						segment.trim();
					if (segment.notEmpty() || keepEmptyElements)
						out.push_back(segment.to<UType>());
				}
				indx = newIndx + 1;
			}
		}
	}


	template<typename C, int Chunk>
	inline C
	StringBase<C,Chunk>::at(const typename StringBase<C,Chunk>::Size offset) const
	{
		return (offset < pSize) ? pPtr[offset] : '\0';
	}


	template<typename C, int Chunk, typename U>
	static typename StringBase<C,Chunk>::Size ASCIItoUTF8(const U c, U* out)
	{
		if (c < 0x80)
		{
			*out = c;
			return 1;
		}
		if (c < 0xC0)
		{
			out[0] = 0xC2;
			out[1] = c;
			return 2;
		}
		out[0] = 0xC3;
		out[1] = (U)(c - 0x40);
		return 2;
	}


	template<typename C, int Chunk>
	StringBase<C,Chunk>
	StringBase<C,Chunk>::ToUTF8(const C* s)
	{
		if (!s || '\0' == *s)
			return StringBase<>();
		typedef unsigned char uchar;
		uchar tmp[4];
		Size newSize = 1;
		for (uchar *p = (uchar*)s; *p; ++p)
			newSize += ASCIItoUTF8<C,Chunk,uchar>(*p, tmp);

		StringBase<C,Chunk> ret;
		ret.reserve(newSize + 1);
		ret.pSize = newSize;

		uchar* q = (uchar*)ret.pPtr;
		for (uchar* p = (uchar*)s; *p; ++p)
			q += ASCIItoUTF8<C,Chunk,uchar>(*p, q);
		*q = '\0';
		return ret;
	}


	template<typename C, int Chunk>
	template<int Chnk1>
	StringBase<C,Chunk>
	StringBase<C,Chunk>::ToUTF8(const StringBase<C,Chnk1>& s)
	{
		if (!s.pSize)
			return StringBase<>();
		typedef unsigned char uchar;
		uchar tmp[4];
		Size newSize = 1;
		for (uchar *p = (uchar*)s.pPtr; *p; ++p)
			newSize += ASCIItoUTF8<C,Chunk,uchar>(*p, tmp);

		StringBase<C,Chunk> ret;
		ret.reserve(newSize + 1);
		ret.pSize = newSize;

		uchar* q = (uchar*)ret.pPtr;
		for (uchar* p = (uchar*)s.pPtr; *p; ++p)
			q += ASCIItoUTF8<C,Chunk,uchar>(*p, q);
		*q = '\0';
		return ret;
	}


	template<typename C, int Chunk>
	inline StringBase<C,Chunk>&
	StringBase<C,Chunk>::toUTF8()
	{
		const StringBase<> tmp = ToUTF8(*this);
		clear();
		append(tmp);
		return *this;
	}


	template<typename C, int Chunk>
	StringBase<C,Chunk>
	StringBase<C,Chunk>::substrUTF8(typename StringBase<C,Chunk>::Size pos, typename StringBase<C,Chunk>::Size len) const
	{
		if (len == npos)
			len = sizeUTF8() - pos;
		typedef unsigned char uchar;
		StringBase<> res;
		int utf8_pos = 0;
		for(; pos > 0; --pos)
		{
			if (((uchar)pPtr[utf8_pos]) >= 0xC0)
			{
				++utf8_pos;
				while (((uchar)pPtr[utf8_pos]) >= 0x80 && ((uchar)pPtr[utf8_pos]) < 0xC0)
					++utf8_pos;
			}
			else
				++utf8_pos;
		}

		for(; len > 0; --len)
		{
			if (((uchar)pPtr[utf8_pos]) >= 0x80)
			{
				res << (char)pPtr[utf8_pos];
				++utf8_pos;
				while (((uchar)pPtr[utf8_pos]) >= 0x80 && ((uchar)pPtr[utf8_pos]) < 0xC0)
				{
					res << (char)pPtr[utf8_pos];
					++utf8_pos;
				}
			}
			else
			{
				res << ((char)pPtr[utf8_pos]);
				++utf8_pos;
			}
		}
		return res;
	}




	template<typename C, int Chunk>
	template<typename U>
	inline const C* StringBase<C,Chunk>::CString(const U& u)
	{
		return Private::StringImpl::CString<C, U>::Extractor(u);
	}


	template<typename C, int Chunk>
	void StringBase<C,Chunk>::toNChars(const unsigned int newSize, const Char defaultChar)
	{
		if (pSize < newSize)
		{
			for (unsigned int i = pSize; i != newSize; ++i)
				append(defaultChar);
		}
		else
			pSize = newSize;
	}



} // namespace Yuni

#endif // __YUNI_CORE_STRING_HXX__
