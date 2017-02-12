#ifndef __YUNI_CORE_BIT_BIT_ARRAY_H__
# define __YUNI_CORE_BIT_BIT_ARRAY_H__

# include "../../yuni.h"
# include "../customstring.h"
# include "bit.h"
# include <iostream>


namespace Yuni
{
namespace Core
{
namespace Bit
{


	/*!
	** \brief Array of bits
	**
	** This class provides routines for handling an array of bits and is especially
	** efficient for a medium or large amount of bits.
	**
	** Performance Tip: For small arrays of bits (for example < 50), a typical
	**   char[] may be faster, but it will use more memory.
	**
	**
	** \code
	** Core::Bit::Array bits(50);
	** std::cout << bits << "\n";
	**
	** bits.set(12);
	** bits.set(27);
	** bits.set(48);
	** std::cout << bits << "\n";
	**
	** bits.unset(27);
	** std::cout << bits << "\n";
	**
	** bits.unset();
	** std::cout << bits << "\n";
	** \endcode
	*/
	class Array
	{
	public:
		//! Size
		typedef unsigned int Size;
		//! Buffer Type
		typedef CustomString<20, true, false> BufferType;
		enum
		{
			//! Invalide offset
			npos = (unsigned int) (-1),
		};
	public:
		//! \name Constructors & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Array();

		/*!
		** \brief Constructor, with an initial number of bits
		*/
		explicit Array(unsigned int n);

		/*!
		** \brief Constructor, with an initial number of bits and a value for each of them
		*/
		Array(unsigned int n, bool value);

		/*!
		** \brief Copy constructor
		*/
		Array(const Array& copy);

		//! Destructor
		~Array() {}
		//@}


		//! \name Bit manipulation
		//@{
		/*!
		** \brief Unset all bits at once
		*/
		void reset();

		/*!
		** \brief Set/Unset all bits at once
		*/
		void reset(bool value);

		/*!
		** \brief Set the Ith bit
		*/
		void set(unsigned int i);

		/*!
		** \brief Set/Unset the Ith bit
		*/
		void set(unsigned int i, bool value);

		/*!
		** \brief Unset the Ith bit
		*/
		void unset(unsigned int i);

		/*!
		** \brief Unset all bits at once
		**
		** Equivalent to reset().
		*/
		void unset();

		/*!
		** \brief Get if the Ith bit is set
		*/
		bool get(unsigned int i) const;

		/*!
		** \brief Get if the Ith bit is set
		**
		** This routine is provided for STL compatibility.
		*/
		bool test(unsigned int i) const;

		/*!
		** \brief Test if any bit is set
		*/
		bool any() const;

		/*!
		** \brief Test if no bit is set
		*/
		bool none() const;

		/*!
		** \brief Test if all bit are set
		*/
		bool all() const;
		//@}


		//! \name Import/Export
		//@{
		/*!
		** \brief Load an array of bit from a buffer
		**
		** \param buffer Any buffer
		*/
		template<class StringT> void loadFromBuffer(const StringT& buffer);

		/*!
		** \brief Load an array of bit from a buffer
		**
		** \param buffer A raw buffer
		** \param size Size ofthe buffer
		*/
		template<class StringT> void loadFromBuffer(const StringT& buffer, unsigned int size);

		/*!
		** \brief Save the array of bits into another buffer (which may use any type)
		**
		** \param u A buffer
		*/
		template<class AnyBufferT> void saveToBuffer(AnyBufferT& u);
		//@}


		//! \name Lookup
		//@{
		/*!
		** \brief Find the first bit set or unset from a given offset
		**
		** \param offset The offset where to start from
		** \return The bit index. npos if not found
		** \tparam ValueT True to find the first bit set, false for the first unset
		*/
		template<bool ValueT> unsigned int find(unsigned int offset = 0) const;

		/*!
		** \brief Find the first N bit set or unset from a given offset
		**
		** \param count  The number of consecutive bits
		** \param offset The offset where to start from
		** \return The bit index. npos if not found
		** \tparam ValueT True to find the first bit set, false for the first unset
		*/
		template<bool ValueT> unsigned int findN(unsigned int count, unsigned int offset = 0) const;
		//@}

		//! \name Memory management
		//@{
		/*!
		** \brief The number of bits within the buffer
		*/
		unsigned int size() const;

		/*!
		** \brief The number of bits within the buffer
		*/
		unsigned int count() const;

		/*!
		** \brief The size in bytes needed to store all bits within the buffer
		**
		** This value greater of equal to the value returned by size().
		*/
		unsigned int sizeInBytes() const;

		/*!
		** \brief Reserve an amount of bits
		*/
		void reserve(unsigned int n);

		/*!
		** \brief Truncate to an amount of bits
		*/
		void truncate(unsigned int n);

		/*!
		** \brief Resize the internal buffer to a given amount of bit
		*/
		void resize(unsigned int n);
		//@}

		const char* c_str() const;
		const char* data() const;
		char* data();

		//! \name Stream
		//@{
		template<class U> void print(U& out) const;
		//@}

		//! \name Operators
		//@{
		//! The operator `=`
		Array& operator = (const Array& rhs);
		//! The operator `=`
		template<class StringT> Array& operator = (const StringT& rhs);
		//@}


	private:
		unsigned int findFirstSet(unsigned int offset) const;
		unsigned int findFirstUnset(unsigned int offset) const;

	private:
		//! Number of bits into the buffer, requested by the caller
		unsigned int pCount;
		//! Internal buffer
		BufferType pBuffer;

	}; // class Array






} // namespace Bit
} // namespace Core
} // namespace Yuni


# include "array.hxx"



//! \name Operator overload for stream printing
//@{
inline std::ostream& operator << (std::ostream& out, const Yuni::Core::Bit::Array& rhs)
{
	rhs.print(out);
	return out;
}
//@}


#endif // __YUNI_CORE_BIT_BIT_ARRAY_H__
