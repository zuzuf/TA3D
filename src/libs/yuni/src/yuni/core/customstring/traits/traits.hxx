#ifndef __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_HXX__
# define __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_HXX__



namespace Yuni
{
namespace Private
{
namespace CustomStringImpl
{

	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::Data()
		:size(0), capacity(0), data(NULL)
	{}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::Data(const Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>& rhs)
		:size(rhs.size), capacity(rhs.size), data(NULL)
	{
		if (size)
		{
			if (chunkSize != 0)
			{
				capacity += zeroTerminated;
				data = (C*)::malloc(sizeof(C) * capacity);
				(void)::memcpy(const_cast<void*>(static_cast<const void*>(data)), rhs.data, sizeof(C) * size);
				if (zeroTerminated)
					(const_cast<char*>(data))[size] = C();
			}
			else
			{
				// this string is a string adapter
				data = rhs.data;
			}
		}
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::~Data()
	{
		// Release the internal buffer if allocated
		// The string is a string adapter only if the chunk size if null
		// When the string is an adapter, the variable is const
		if (chunkSize != 0)
			::free(const_cast<void*>(static_cast<const void*>(data)));
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::adapt(const char* const cstring)
	{
		data = const_cast<char*>(cstring);
		capacity = size = (data ? ::strlen(data) : 0);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::adapt(const char* const cstring, Size length)
	{
		data = const_cast<char*>(cstring);
		capacity = size = length;
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::clear()
	{
		if (zeroTerminated)
		{
			if (size)
			{
				size = 0;
				*(const_cast<char*>(data)) = C();
			}
		}
		else
			size = 0;
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::shrink()
	{
		if (data)
		{
			if (size)
			{
				capacity = size + zeroTerminated;
				data = (char*)::realloc(const_cast<char*>(data), capacity);
			}
			else
			{
				capacity = 0;
				::free(const_cast<void*>(static_cast<const void*>(data)));
				data = nullptr;
			}
		}
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::insert(Size offset, const C* const buffer, const Size len)
	{
		// Reserving enough space to insert the buffer
		reserve(len + size + zeroTerminated);
		// Move the existing block of data
		(void)::memmove(const_cast<char*>(data) + sizeof(C) * (offset + len), const_cast<char*>(data) + sizeof(C) * (offset), sizeof(C) * (size - offset));
		// Copying the given buffer
		(void)::memcpy (const_cast<char*>(data) + sizeof(C) * (offset), buffer, sizeof(C) * len);
		// Updating the size
		size += len;
		// zero-terminated
		if (zeroTerminated)
			(const_cast<char*>(data))[size] = C();
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	inline void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::put(const C rhs)
	{
		// Making sure that we have enough space
		reserve(size + 1 + zeroTerminated);
		// Raw copy
		(const_cast<char*>(data))[size] = rhs;
		// New size
		++size;
		if (zeroTerminated)
			(const_cast<char*>(data))[size] = C();
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	void
	Data<ChunkSizeT,ExpandableT,ZeroTerminatedT,C>::reserve(Size minCapacity)
	{
		if (adapter)
			return;
		minCapacity += zeroTerminated;
		if (capacity < minCapacity)
		{
			// This loop may be a little faster than the following replacement code :
			// capacity = minCapacity + minCapacity % chunkSize;
			// Especially when chunkSize is not a power of 2
			Size newcapacity = capacity;
			do
			{
				// Increase the capacity until we have enough space
				newcapacity += chunkSize;
			}
			while (newcapacity < minCapacity);

			// Realloc the internal buffer
			C* newdata = (C*)::realloc(const_cast<char*>(data), (size_t)(sizeof(C) * newcapacity));
			// The returned value can be NULL
			if (!newdata)
				throw "Yuni::CustomString: Impossible to realloc";
			{
				data = newdata;
				capacity = newcapacity;
				if (zeroTerminated)
					(const_cast<char*>(data))[size] = C();
			}
		}
	}


	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	typename Data<ChunkSizeT,false,ZeroTerminatedT,C>::Size
	Data<ChunkSizeT,false,ZeroTerminatedT,C>::assignWithoutChecking(const C* const block,
		typename Data<ChunkSizeT,false,ZeroTerminatedT,C>::Size blockSize)
	{
		// We have to trunk the size if we are outer limits
		// This condition is a little faster than the folowing replacement code :
		// blockSize = Math::Min<Size>(blockSize, capacity - size);
		//
		// We have better performance if the two cases have their own code block
		// (perhaps because of the constant values)
		//
		if (blockSize > capacity)
		{
			// The new blocksize is actually the capacity itself
			// The capacity can not be null
			// Raw copy
			(void)::memcpy(const_cast<char*>(data), block, sizeof(C) * capacity);
			// New size
			size = capacity;
			if (zeroTerminated)
				(const_cast<char*>(data))[capacity] = C();
			return capacity;
		}
		// else
		{
			// Raw copy
			(void)::memcpy(const_cast<char*>(data), block, sizeof(C) * blockSize);
			// New size
			size = blockSize;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return blockSize;
		}
	}


	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	typename Data<ChunkSizeT,false,ZeroTerminatedT,C>::Size
	Data<ChunkSizeT,false,ZeroTerminatedT,C>::appendWithoutChecking(const C* const block, Size blockSize)
	{
		// We have to trunk the size if we are outer limits
		// This condition is a little faster than the folowing replacement code :
		// blockSize = Math::Min<Size>(blockSize, capacity - size);
		//
		// We have better performance if the two cases have their own code block
		// (perhaps because of the constant values)
		//
		if (blockSize + size > capacity)
		{
			// Computing the new block size to copy
			blockSize = capacity - size;
			// The performance are a bit better if we interupt the process sooner
			if (!blockSize)
				return 0;
			// Raw copy
			(void)::memcpy(data + size * sizeof(C), block, sizeof(C) * blockSize);
			// New size
			size = capacity;
			if (zeroTerminated)
				(const_cast<char*>(data))[capacity] = C();
			return blockSize;
		}
		// else
		{
			// Raw copy
			(void)::memcpy(data + size * sizeof(C), block, sizeof(C) * blockSize);
			// New size
			size += blockSize;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return blockSize;
		}
	}


	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	inline typename Data<ChunkSizeT,false,ZeroTerminatedT,C>::Size
	Data<ChunkSizeT,false,ZeroTerminatedT,C>::assignWithoutChecking(const C c)
	{
		data[0] = c;
		size = 1;
		if (zeroTerminated)
			(const_cast<char*>(data))[1] = C();
		return 1;
	}


	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	typename Data<ChunkSizeT,false,ZeroTerminatedT,C>::Size
	Data<ChunkSizeT,false,ZeroTerminatedT,C>::appendWithoutChecking(const C c)
	{
		if (size == capacity)
			return 0;
		// else
		{
			data[size] = c;
			++size;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return 1;
		}
	}


	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	void
	Data<ChunkSizeT,false,ZeroTerminatedT,C>::put(const C rhs)
	{
		// Making sure that we have enough space
		if (size != capacity)
		{
			// Raw copy
			data[size] = rhs;
			// New size
			++size;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
		}
	}





} // namespace CustomStringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_HXX__
