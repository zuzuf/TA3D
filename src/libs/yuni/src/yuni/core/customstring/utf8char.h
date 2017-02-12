#ifndef __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_H__
# define __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_H__



namespace Yuni
{
namespace UTF8
{

	enum Error
	{
		//! No error
		errNone = 0,
		//! Out of bound (offset >= size)
		errOutOfBound,
		//! Invalid lead character
		errInvalidLead,
		//! Not enough data
		errNotEnoughData,
		//! The sequence is incomplete
		errIncompleteSequence,
		errInvalidCodePoint,
	};


	class Char
	{
	public:
		/*!
		** \brief Get the length in bytes of the UTF8 character
		**
		** This information is provided by the lead character (= the first char given by @p)
		** \param p Address of a potential utf8 char
		** \return The size in bytes of the UTF8 char at the address @p (1,2,3, or 4, 0 if invalid).
		*/
		static unsigned int Size(const void* p);

		/*!
		** \brief Extract the first unsigned char from a raw buffer
		*/
		static unsigned char Mask8Bits(const void* p);
		static unsigned char Mask8Bits(const char p);

		/*!
		** \brief Is the UTF-8 a simple ascii char ?
		*/
		static bool IsASCII(unsigned char c);

		/*!
		** \brief Check if the two first bits are set
		*/
		static bool IsTrail(unsigned char c);

		enum
		{
			//! The maximum valid code point
			codePointMax = (uint32) 0x0010ffffu
		};

	public:
		Char()
			:pValue(0)
		{}
		Char(const Char& rhs)
			:pValue(rhs.pValue)
		{}
		explicit Char(char c)
			:pValue((unsigned int) c)
		{}

		/*!
		** \brief The size of the UTF8 character, in bytes
		*/
		unsigned int size() const;

		uint32 value() const {return pValue;}

		template<class StreamT> void write(StreamT& out) const;

		void reset()
		{
			pValue  = 0;
		}

		Char& operator = (const Char& rhs)
		{
			pValue = rhs.pValue;
			return *this;
		}

		Char& operator = (const char c)
		{
			pValue  = (uint32) c;
			return *this;
		}

		Char& operator = (const unsigned char c)
		{
			pValue  = (uint32) c;
			return *this;
		}

		bool operator == (const char c) const
		{
			return (pValue < 0x80 && static_cast<char>(pValue) == c);
		}

		bool operator != (const char c) const
		{
			return !(*this == c);
		}

		bool operator == (const unsigned char c) const
		{
			return (pValue < 0x80 && static_cast<unsigned char>(pValue) == c);
		}

		bool operator != (const unsigned char c) const
		{
			return !(*this == c);
		}

		operator char () const
		{
			return (pValue < 0x80) ? static_cast<char>(pValue) : '\0';
		}

	private:
		//! The UTF-8 character
		uint32 pValue;
		// A friend !
		template<unsigned int, bool, bool> friend class Yuni::CustomString;
	};





} // namespace UTF8
} // namespace Yuni

# include "utf8char.hxx"

#endif // __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_H__
