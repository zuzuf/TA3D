#ifndef __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_H__
# define __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_H__

# include "../../../yuni.h"
# ifdef YUNI_HAS_STDLIB_H
#	include <stdlib.h>
# endif
# include <string.h>
# include "../../math/math.h"



namespace Yuni
{
namespace Private
{
namespace CustomStringImpl
{

	// Const qualifier from the adapter mode
	template<bool AdapterT, class C> struct QualifierFromAdapterMode { typedef C* Type; };
	template<class C> struct QualifierFromAdapterMode<true, C> { typedef const C* Type; };



	int Compare(const char* const s1, unsigned int l1, const char* const s2, unsigned int l2);
	int CompareInsensitive(const char* const s1, unsigned int l1, const char* const s2, unsigned int l2);

	bool Equals(const char* const s1, const char* const s2, unsigned int len);
	bool EqualsInsensitive(const char* const s1, const char* const s2, unsigned int len);

	bool Glob(const char* const s, unsigned int l1, const char* const pattern, unsigned int patternlen);




	template<class StringT, bool Adapter>
	struct AdapterAssign
	{
		static void Perform(StringT& out, const char* const cstring, typename StringT::Size size)
		{
			out.adaptWithoutChecking(cstring, size);
		}
	};

	template<class StringT>
	struct AdapterAssign<StringT, false>
	{
		static void Perform(StringT& out, const char* const cstring, typename StringT::Size size)
		{
			// fallback
			out.assign(cstring, size);
		}
	};


	template<class StringT, bool Adapter>
	struct Consume
	{
		static void Perform(StringT& out, typename StringT::Size count)
		{
			if (count >= out.size())
				out.clear();
			else
				out.decalOffset(count);
		}
	};

	template<class StringT>
	struct Consume<StringT, false>
	{
		static void Perform(StringT& out, typename StringT::Size count)
		{
			out.erase(0, count);
		}
	};







	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class C>
	struct Data
	{
	public:
		typedef unsigned int Size;
		enum
		{
			chunkSize = ChunkSizeT,
			zeroTerminated = (ZeroTerminatedT ? 1 : 0),
			expandable = 1,
			adapter  = (!chunkSize && expandable && !zeroTerminated),
		};

	public:
		//! \name Constructors & Destructors
		//@{
		/*!
		** \brief Default Constructor
		*/
		Data();

		/*!
		** \brief Copy constructor
		*/
		Data(const Data& rhs);

		//! Destructor
		~Data();
		//@}

		void adapt(const char* const cstring);

		void adapt(const char* const cstring, Size length);

		void clear();


		Size assignWithoutChecking(const C* const block, const Size blockSize)
		{
			// Making sure that we have enough space
			reserve(blockSize + zeroTerminated);
			// Raw copy
			(void)::memcpy(const_cast<char*>(data), block, sizeof(C) * blockSize);
			// New size
			size = blockSize;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return blockSize;
		}


		Size appendWithoutChecking(const C* const block, const Size blockSize)
		{
			// Making sure that we have enough space
			reserve(size + blockSize + zeroTerminated);
			// Raw copy
			(void)::memcpy(const_cast<char*>(data) + size * sizeof(C), block, blockSize * sizeof(C));
			// New size
			size += blockSize;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return blockSize;
		}

		Size assignWithoutChecking(const C c)
		{
			// Making sure that we have enough space
			reserve(1 + zeroTerminated);
			// Raw copy
			(const_cast<char*>(data))[0] = c;
			// New size
			size = 1;
			if (zeroTerminated)
				(const_cast<char*>(data))[1] = C();
			return 1;
		}


		Size appendWithoutChecking(const C c)
		{
			// Making sure that we have enough space
			reserve(size + 1 + zeroTerminated);
			// Raw copy
			(const_cast<char*>(data))[size] = c;
			// New size
			++size;
			if (zeroTerminated)
				(const_cast<char*>(data))[size] = C();
			return 1;
		}


		Size assign(const C* const block, const Size blockSize)
		{
			if (block && blockSize)
				return assignWithoutChecking(block, blockSize);
			clear();
			return 0;
		}


		Size append(const C* const block, const Size blockSize)
		{
			return (block && blockSize) ? appendWithoutChecking(block, blockSize) : 0;
		}

		void put(const C rhs);

		void reserve(Size minCapacity);

		void insert(Size offset, const C* const buffer, const Size len);

		void shrink();


	protected:
		Size size;
		Size capacity;
		//! Our buffer
		typename QualifierFromAdapterMode<(0 != adapter), C>::Type data;
		// Friend
		template<unsigned int SizeT, bool ExpT, bool ZeroT> friend class Yuni::CustomString;

	}; // class Data





	template<unsigned int ChunkSizeT, bool ZeroTerminatedT, class C>
	struct Data<ChunkSizeT, false, ZeroTerminatedT, C>
	{
	public:
		typedef unsigned int Size;
		enum
		{
			chunkSize = ChunkSizeT,
			capacity = ChunkSizeT,
			zeroTerminated = (ZeroTerminatedT ? 1 : 0),
			expandable = 0,
		};

	public:
		Data()
			:size(0)
		{
			// The buffer must be properly initialized
			if (zeroTerminated)
				data[0] = C();
		}

		Data(const Data& rhs)
			:size(rhs.size)
		{
			(void)::memcpy(data, rhs.data, sizeof(C) * (size + zeroTerminated));
		}

		void clear()
		{
			size = 0;
			if (zeroTerminated)
				data[0] = C();
		}

		Size assignWithoutChecking(const C* const block, Size blockSize);

		Size appendWithoutChecking(const C* const block, Size blockSize);

		Size assignWithoutChecking(const C c);

		Size appendWithoutChecking(const C c);


		Size assign(const C* const block, const Size blockSize)
		{
			if (block && blockSize)
				return assignWithoutChecking(block, blockSize);
			clear();
			return 0;
		}

		Size append(const C* const block, const Size blockSize)
		{
			return (block && blockSize) ? appendWithoutChecking(block, blockSize) : 0;
		}

		void put(const C rhs);

		static void reserve(const Size)
		{
			// Do nothing
		}


		void insert(Size offset, const C* const buffer, Size len)
		{
			if (offset + len >= capacity)
			{
				// The new buffer will take the whole space
				(void)::memcpy(data + sizeof(C) * (offset), buffer, sizeof(C) * (capacity - offset));
				size = capacity;
				if (zeroTerminated)
					data[capacity] = C();
				return;
			}
			if (size + len <= capacity)
			{
				// Move the existing block of data
				(void)::memmove(data + sizeof(C) * (offset + len), data + sizeof(C) * (offset), sizeof(C) * (size - offset));
				// Copying the given buffer
				(void)::memcpy (data + sizeof(C) * (offset), buffer, sizeof(C) * len);
				// Updating the size
				size += len;
				// zero-terminated
				if (zeroTerminated)
					data[size] = C();
			}
			else
			{
				// Move the existing block of data
				(void)::memmove(data + sizeof(C) * (offset + len), data + sizeof(C) * (offset), sizeof(C) * (capacity - offset - len));
				// Copying the given buffer
				(void)::memcpy (data + sizeof(C) * (offset), buffer, sizeof(C) * len);
				// Updating the size
				size = capacity;
				// zero-terminated
				if (zeroTerminated)
					data[capacity] = C();
			}
		}

		static void shrink()
		{
			// Do nothing
		}

	protected:
		Size size;
		/*!
		** \brief Static buffer, fixed capacity
		**
		** We always add +1 to allow standard routine write a final 0, and we
		** may need it for zero-terminated strings any way.
		*/
		C data[capacity + 1];

		// Friend
		template<unsigned int SizeT, bool ExpT, bool ZeroT> friend class Yuni::CustomString;

	}; // class Data;







} // namespace CustomStringImpl
} // namespace Private
} // namespace Yuni

# include "traits.hxx"

#endif // __YUNI_CORE_CUSTOMSTRING_TRAITS_TRAITS_H__
