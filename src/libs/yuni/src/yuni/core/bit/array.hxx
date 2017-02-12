#ifndef __YUNI_CORE_BIT_BIT_ARRAY_HXX__
# define __YUNI_CORE_BIT_BIT_ARRAY_HXX__


namespace Yuni
{
namespace Core
{
namespace Bit
{

	inline Array::Array()
		:pCount(0)
	{}

	inline Array::Array(unsigned int n)
	{
		resize(n);
		unset();
	}

	inline Array::Array(unsigned int n, bool value)
	{
		resize(n);
		reset(value);
	}


	inline void Array::unset()
	{
		(void)::memset(pBuffer.data(), 0, (size_t)pBuffer.sizeInBytes());
	}



	inline void Array::reset()
	{
		(void)::memset(pBuffer.data(), 0, (size_t)pBuffer.sizeInBytes());
	}


	inline void Array::reset(bool value)
	{
		(void)::memset(pBuffer.data(), (value ? 0xFF : 0), (size_t)pBuffer.sizeInBytes());
	}


	inline void Array::set(unsigned int i)
	{
		YUNI_BIT_SET(pBuffer.data(), i);
	}


	inline bool Array::get(unsigned int i) const
	{
		# ifdef YUNI_OS_MSVC
		return (YUNI_BIT_GET(pBuffer.data(), i)) ? true : false;
		# else
		return YUNI_BIT_GET(pBuffer.data(), i);
		# endif
	}


	inline bool Array::test(unsigned int i) const
	{
		# ifdef YUNI_OS_MSVC
		return (YUNI_BIT_GET(pBuffer.data(), i)) ? true : false;
		# else
		return YUNI_BIT_GET(pBuffer.data(), i);
		# endif
	}


	inline void Array::set(unsigned int i, bool value)
	{
		if (value)
			YUNI_BIT_SET(pBuffer.data(), i);
		else
			YUNI_BIT_UNSET(pBuffer.data(), i);
	}


	inline void Array::unset(unsigned int i)
	{
		YUNI_BIT_UNSET(pBuffer.data(), i);
	}


	inline void Array::reserve(unsigned int n)
	{
		pBuffer.reserve((n >> 3) + 1);
	}


	inline void Array::truncate(unsigned int n)
	{
		pBuffer.truncate(((pCount = n) >> 3) + 1);
	}


	inline void Array::resize(unsigned int n)
	{
		pBuffer.resize(((pCount = n) >> 3) + 1);
	}


	inline unsigned int Array::sizeInBytes() const
	{
		return (unsigned int)pBuffer.sizeInBytes();
	}

	inline unsigned int Array::size() const
	{
		return pCount;
	}

	inline unsigned int Array::count() const
	{
		return pCount;
	}

	inline const char* Array::c_str() const
	{
		return pBuffer.c_str();
	}

	inline const char* Array::data() const
	{
		return pBuffer.data();
	}

	inline char* Array::data()
	{
		return pBuffer.data();
	}


	template<class StringT>
	inline void Array::loadFromBuffer(const StringT& u)
	{
		// Assert, if a C* container can not be found at compile time
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, BitArray_InvalidTypeForBuffer);
		// Assert, if the length of the container can not be found at compile time
		YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  BitArray_InvalidTypeForBufferSize);

		if (Traits::Length<StringT,Size>::isFixed)
		{
			// We can make some optimisations when the length is known at compile compile time
			// This part of the code should not bring better performances but it should
			// prevent against bad uses of the API, like using a C* for looking for a single char.

			// The value to find is actually empty, nothing to do
			if (0 == Traits::Length<StringT,Size>::fixedLength)
			{
				pBuffer.clear();
				pCount = 0;
				return;
			}
		}
		pCount = Traits::Length<StringT,Size>::Value(u);
		pBuffer.assign(Traits::CString<StringT>::Perform(u), pCount);
	}


	template<class StringT>
	inline void Array::loadFromBuffer(const StringT& u, unsigned int size)
	{
		// Assert, if a C* container can not be found at compile time
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, BitArray_InvalidTypeForBuffer);

		pCount = size;
		pBuffer.assign(Traits::CString<StringT>::Perform(u), pCount);
	}


	template<class AnyBufferT>
	inline void Array::saveToBuffer(AnyBufferT& u)
	{
		if (pCount)
			u.assign(pBuffer.c_str(), pBuffer.sizeInBytes());
		else
			u.clear();
	}


	inline Array& Array::operator = (const Array& rhs)
	{
		pBuffer = rhs.pBuffer;
		pCount = rhs.pCount;
		return *this;
	}


	template<class StringT> inline Array& Array::operator = (const StringT& rhs)
	{
		loadFromBuffer(rhs);
		return *this;
	}


	template<class U>
	inline void Array::print(U& out) const
	{
		for (unsigned int i = 0; i != pCount; ++i)
			out.put((YUNI_BIT_GET(pBuffer.data(), i)) ? '1' : '0');
	}


	template<bool ValueT>
	unsigned int Array::findN(unsigned int count, unsigned int offset) const
	{
		while (npos != (offset = find<ValueT>(offset)))
		{
			if (offset + count > pCount)
				return npos;

			bool ok = true;

			// Checking if the block is large enough for our needs
			// The first block is already valid
			for (unsigned int j = 1; j < count; ++j)
			{
				if (ValueT != get(offset + j))
				{
					ok = false;
					break;
				}
			}
			if (ok)
				return offset;
			++offset;
		}
		return npos;
	}


	template<bool ValueT>
	inline unsigned int Array::find(unsigned int offset) const
	{
		return ValueT ? findFirstSet(offset) : findFirstUnset(offset);
	}




} // namespace Bit
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_BIT_BIT_ARRAY_HXX__
