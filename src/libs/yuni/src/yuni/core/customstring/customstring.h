#ifndef __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_H__
# define __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_H__

# include "../../yuni.h"
# include "../static/remove.h"
# include "../static/assert.h"
# include "../traits/cstring.h"
# include "../traits/length.h"
# include "../smartptr.h"

# include <cstdio>
# ifdef YUNI_HAS_STDARG_H
#	include <stdarg.h>
# endif
# include <string>
# ifdef YUNI_HAS_VECTOR
#	include <vector>
# endif
# include <list>

# include "utf8char.h"
# include "../iterator.h"
# include "traits/traits.h"
# include "traits/append.h"
# include "traits/assign.h"
# include "traits/fill.h"
# include "traits/vnsprintf.h"
# include "traits/into.h"




namespace Yuni
{

	//! A convenient standard string implementation
	typedef CustomString<> String;



	/*!
	** \brief Character string
	**
	** The class provides a useful way to manipulate and store sequences of
	** characters.
	**
	** The class is a template class, you may prefer the convenient alias
	** `Yuni::String` most of the time.
	**
	** The supported external types are the following :
	**  - C
	**  - char*
	**  - C[]
	**  - std::basic_string<char>
	**  - Yuni::CustomString<...>
	**  - SmartPtr<std::basic_string<char>, ...>
	**  - SmartPtr<CustomString<...>, ...>
	**
	** Example for iterating through all character (the recommended way)
	** \code
	** String t = "こんにちは";
	** const String::const_utf8iterator end = t.utf8end();
	** for (String::const_utf8iterator i = t.utf8begin(); i != end; ++i)
	** 	std::cout << "char at offset " << i.offset() << ": " << *i << std::endl;
	** \endcode
	**
	** Example for convertions :
	** \code
	** String s;
	** s << "example for double = " << 42.6;
	** std::cout << s << std::endl;
	**
	** s = "42";
	** int i = s.to<int>();
	** std::cout << "Convertion without check for int: " << i << std::endl;
	** if (s.to(i))
	** {
	** 	std::cout << "Convertion with check for int: " << i << std::endl;
	** }
	** else
	** 	std::cout << "Convertion failed for int " << std::endl;
	**
	** Color::RGB<> rgb(142, 230, 12);
	** s.clear() << "example for rgb = " << rgb;
	** std::cout << s << std::endl;
	**
	** Color::RGBA<> rgba;
	** s = " rgb( 42, 58, 234)";
	** s.to(rgba);
	** std::cout << "Convertion from string to rgba : " << rgba << std::endl;
	** s = " rgba( 42, 58, 234, 67)";
	** s.to(rgba);
	** std::cout << "Convertion from string to rgba : " << rgba << std::endl;
	** \endcode
	**
	** \warning This class is not thread-safe
	** \tparam ChunkSizeT The size for a single chunk (> 3)
	** \tparam ExpandableT True to make a growable string. Otherwise it will be a
	**   string with a fixed-length capacity (equals to ChunkSizeT)
	** \tparam ZeroTerminatedT True to make the string zero-terminated
	*/
	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	class CustomString
		:protected Private::CustomStringImpl::Data<ChunkSizeT,ExpandableT,ZeroTerminatedT, char>
	{
	public:
		//! POD type
		typedef char Char;
		//! Type for the POD type
		typedef char Type;

		//! Ancestor
		typedef Private::CustomStringImpl::Data<ChunkSizeT,ExpandableT,ZeroTerminatedT, char>  AncestorType;
		//! Size type
		typedef typename AncestorType::Size Size;
		//! Self
		typedef CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>  CustomStringType;

		//! \name Compatibility with std::string
		//@{
		//! The type of object, charT, stored in the string
		typedef char value_type;
		//! Pointer to char
		typedef char* pointer;
		//! Reference to char
		typedef char& reference;
		//! Const reference to char
		typedef const char& const_reference;
		//! An unsigned integral type
		typedef Size size_type;
		//! A signed integral type
		typedef ssize_t difference_type;
		//@}

		//! Smartptr
		typedef SmartPtr<CustomStringType> Ptr;
		//! A String vector
		typedef std::vector<CustomStringType> Vector;
		//! A String vector
		typedef std::vector<Ptr> VectorPtr;
		//! A String list
		typedef std::list<CustomStringType> List;
		//! A string list
		typedef std::list<Ptr> ListPtr;

		enum
		{
			//! Size for a single chunk
			chunkSize      = AncestorType::chunkSize,
			//! Invalid offset
			npos           = (Size)(-1),
			//! A non-zero value if the string must be zero terminated
			zeroTerminated = AncestorType::zeroTerminated,
			//! A non-zero value if the string can be expanded
			expandable     = AncestorType::expandable,
			//! True if the string is a string adapter (only read-only operations are allowed)
			adapter        = (!chunkSize && expandable && !zeroTerminated),
		};
		//! char Case
		enum charCase
		{
			//! The string should remain untouched
			soCaseSensitive,
			//! The string should be converted to lowercase
			soIgnoreCase
		};

		//! Self, which can be written
		typedef typename Static::If<adapter || !zeroTerminated || (!expandable && chunkSize > 512),
			CustomString<>, CustomStringType>::RetTrue  WritableType;

		// Checking for a minimal chunk size
		YUNI_STATIC_ASSERT(adapter || chunkSize > 3, CustomString_MinimalChunkSizeRequired);

	public:
		//! \name CString comparison
		//@{
		/*!
		** \brief Compare two string like strcmp()
		**
		** The comparison is done using unsigned characters.
		** \return An integer greater than, equal to, or less than 0, according as the string is greater than,
		**   equal to, or less than the given string
		*/
		static int Compare(const char* const s1, unsigned int l1, const char* const s2, unsigned int l2);

		/*!
		** \brief Compare two string like strcmp() (insensitive)
		**
		** The comparison is done using unsigned characters.
		** \return An integer greater than, equal to, or less than 0, according as the string is greater than,
		**   equal to, or less than the given string
		*/
		static int CompareInsensitive(const char* const s1, unsigned int l1, const char* const s2, unsigned int l2);
		//@}


		//! \name Faster implementation of some commons routines
		//@{
		//! Upper case to lower case letter conversion (man 3 tolower)
		static int ToLower(int c);
		//! Lower case to upper case letter conversion (man 3 toupper)
		static int ToUpper(int c);
		//! White-space character test
		static bool IsSpace(int c);
		//! decimal-digit character test
		static bool IsDigit(int c);
		//! decimal-digit character test (without zero)
		static bool IsDigitNonZero(int c);
		//! alphabetic character test
		static bool IsAlpha(int c);
		//@}


	private:
		// Implements the following iterator models for CustomString<>
		# include "iterator.h"

	public:
		//! \name Iterators
		//@{
		//! Iterator
		typedef IIterator<typename Model::ByteIterator, false>  iterator;
		//! Iterator (const)
		typedef IIterator<typename Model::ByteIterator, true>   const_iterator;
		//! Iterator for UTF8 characters
		typedef IIterator<typename Model::UTF8Iterator, false>  utf8iterator;
		//! Iterator for UTF8 characters (const)
		typedef IIterator<typename Model::UTF8Iterator, true>   const_utf8iterator;
		//! Null iterator
		typedef IIterator<typename Model::NullIterator, true>   null_iterator;
		//@}

	public:
		//! \name Constructors & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		CustomString();

		/*!
		** \brief Copy constructor
		*/
		CustomString(const CustomString& rhs);

		/*!
		** \brief Constructor from a mere CString
		*/
		CustomString(const char* const block, const Size blockSize);

		/*!
		** \brief Constructor from a copy of a substring of 's'
		**
		** The substring is the portion of str that begins at the character position
		** 'offset'.
		*/
		template<unsigned int SizeT, bool ExpT, bool ZeroT>
		CustomString(const CustomString<SizeT,ExpT,ZeroT>& s, Size offset);

		/*!
		** \brief Constructor from a copy of a substring of 's'
		**
		** The substring is the portion of str that begins at the character position
		** 'offset' and takes up to 'n' characters (it takes less than n if the end
		** of 's' is reached before).
		*/
		template<unsigned int SizeT, bool ExpT, bool ZeroT>
		CustomString(const CustomString<SizeT,ExpT,ZeroT>& s, Size offset, Size n /*= npos*/);

		/*!
		** \brief Constructor from a copy of a substring of 's' (std::string)
		**
		** The substring is the portion of str that begins at the character position
		** 'offset'.
		*/
		template<class TraitsT, class AllocT>
		CustomString(const std::basic_string<char,TraitsT,AllocT>& s, Size offset);

		/*!
		** \brief Constructor from a copy of a substring of 's' (std::string)
		**
		** The substring is the portion of str that begins at the character position
		** 'offset' and takes up to 'n' characters (it takes less than n if the end
		** of 's' is reached before).
		*/
		template<class TraitsT, class AllocT>
		CustomString(const std::basic_string<char,TraitsT,AllocT>& s, Size offset, Size n /*= npos*/);

		/*!
		** \brief Constructor by copy from iterator
		**
		** \param begin An iterator pointing to the begining of a sequence
		** \param end  An iterator pointing to the end of a sequence
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2>
		CustomString(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end);

		/*!
		** \brief Constructor by copy from iterator
		**
		** \param begin An iterator pointing to the begining of a sequence
		** \param end  An iterator pointing to the end of a sequence
		** \param separator A string to add between each item
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2, class StringT>
		CustomString(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end, const StringT& separator);

		/*!
		** \brief Constructor with a default value
		*/
		template<class U> CustomString(const U& rhs);

		/*!
		** \brief Construct a string formed by a repetition of the character c, n times
		*/
		CustomString(size_t n, char c);

		/*!
		** \brief Construct a string formed by a repetition of the character c, n times
		*/
		CustomString(size_t n, unsigned char c);

		/*!
		** \brief Destructor
		*/
		~CustomString();
		//@}


		//! \name Iterators
		//@{
		//! Get an iterator on UTF8 characters pointing to the beginning of the string
		utf8iterator utf8begin();
		//! Get an iterator on UTF8 characters pointing to the beginning of the string
		const_utf8iterator utf8begin() const;

		//! Get an iterator on UTF8 characters pointing to the end of the string
		null_iterator utf8end();
		//! Get an iterator on UTF8 characters pointing to the end of the string
		null_iterator utf8end() const;

		//! Get an iterator pointing to the beginning of the string
		iterator begin();
		//! Get an iterator pointing to the beginning of the string
		const_iterator begin() const;

		//! Get an iterator pointing to the end of the string
		null_iterator end();
		//! Get an iterator pointing to the end of the string
		null_iterator end() const;
		//@}


		//! \name Append / Assign / Fill
		//@{
		/*!
		** \brief Assign a new value to the string
		**
		** \param rhs Any supported value
		*/
		template<class U> void assign(const U& rhs);

		/*!
		** \brief Copy a raw C-String
		**
		** \param cstr A C-String
		** \param size Size of the given string
		*/
		template<class StringT>
		void assign(const StringT& str, const Size size);

		/*!
		** \brief Copy a raw C-String
		**
		** \param cstr A C-String
		** \param size Size of the given string
		** \param offset Offset of the first character to copy
		*/
		template<class StringT>
		void assign(const StringT& str, const Size size, const Size offset);

		/*!
		** \brief Assign to the string all items within
		**
		** The type held by the iterator can be anything as long as the type can
		** be converted by the string (see specializations in the namespace
		** `Yuni::Extension::CustomString`).
		**
		** \param begin An iterator pointing to the begining of a sequence
		** \param end  An iterator pointing to the end of a sequence
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2>
		void assign(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end);

		/*!
		** \brief Assign to the string all items within
		**
		** The type held by the iterator can be anything as long as the type can
		** be converted by the string (see specializations in the namespace
		** `Yuni::Extension::CustomString`).
		**
		** \code
		** String s = "string: こんにちは";
		** String::const_utf8iterator a = s.utf8begin() + 9;
		** String::const_utf8iterator a = s.utf8begin() + 11;
		** String sub1(a, b);
		** std::cout << sub1 << std::endl; // んに
		**
		** String sub2;
		** sub2.append(a, b, ", ");
		** std::cout << sub2 << std::endl; // ん, に
		** \endcode
		**
		** \param begin An iterator pointing to the begining of a sequence
		** \param end  An iterator pointing to the end of a sequence
		** \param separator The string separator to use between each item
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2, class StringT>
		void assign(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end,
			const StringT& separator);

		/*!
		** \brief Assign to thestring all items within
		**
		** The type held by the iterator can be anything as long as the type can
		** be converted by the string (see specializations in the namespace
		** `Yuni::Extension::CustomString`).
		**
		** \code
		** String s = "string: こんにちは";
		** String::const_utf8iterator a = s.utf8begin() + 9;
		** String::const_utf8iterator a = s.utf8begin() + 11;
		** String sub1(a, b);
		** std::cout << sub1 << std::endl; // んに
		**
		** String sub2;
		** sub2.append(a, b, ", ");
		** std::cout << sub2 << std::endl; // ん, に
		** \endcode
		**
		** \param begin An iterator pointing to the begining of a sequence
		** \param end  An iterator pointing to the end of a sequence
		** \param separator The string separator to use between each item
		** \param enclosure The enclosure string
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2, class StringT, class EnclosureT>
		void assign(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end,
			const StringT& separator, const EnclosureT& enclosure);


		/*!
		** \brief Append to the end of the string a new value
		**
		** \param rhs Any supported value
		*/
		template<class U> void append(const U& rhs);

		/*!
		** \brief Append to the end of the string all items within
		**
		** The type held by the iterator can be anything as long as it can
		** be converted by the string (see specializations in the namespace
		** Yuni::Extension::CustomString).
		**
		** \see namespace Yuni::Extension::CustomString
		** \param begin  An iterator pointing to the begining of a sequence
		** \param end    An iterator pointing to the end of a sequence
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2>
		void append(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end);

		/*!
		** \brief Append to the end of the string all items within
		**
		** The type held by the iterator can be anything as long as it can
		** be converted by the string (see specializations in the namespace
		** Yuni::Extension::CustomString).
		**
		** \code
		** String s = "string: こんにちは";
		** String::const_utf8iterator a = s.utf8begin() + 9;
		** String::const_utf8iterator a = s.utf8begin() + 11;
		** String sub1(a, b);
		** std::cout << sub1 << std::endl; // んに
		**
		** String sub2;
		** sub2.append(a, b, ", ");
		** std::cout << sub2 << std::endl; // ん, に
		** \endcode
		**
		** \see namespace Yuni::Extension::CustomString
		**
		** \param begin     An iterator pointing to the begining of a sequence
		** \param end       An iterator pointing to the end of a sequence
		** \param separator The string separator to use between each item
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2, class StringT>
		void append(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end,
			const StringT& separator);

		/*!
		** \brief Append to the end of the string all items within a range
		**
		** The type held by the iterator can be anything as long as it can
		** be converted by the string (see specializations in the namespace
		** Yuni::Extension::CustomString).
		**
		** \code
		** String s = "string: こんにちは";
		** String::const_utf8iterator a = s.utf8begin() + 9;
		** String::const_utf8iterator a = s.utf8begin() + 11;
		** String sub1(a, b);
		** std::cout << sub1 << std::endl; // んに
		**
		** String sub2;
		** sub2.append(a, b, ", ", '"');
		** std::cout << sub2 << std::endl; // "ん", "に"
		** \endcode
		**
		** \see namespace Yuni::Extension::CustomString
		**
		** \param begin     An iterator pointing to the begining of a sequence
		** \param end       An iterator pointing to the end of a sequence
		** \param separator The string separator to use between each item
		** \param enclosure The enclosure string
		*/
		template<class ModelT, bool ConstT, class ModelT2, bool ConstT2, class StringT, class EnclosureT>
		void append(const IIterator<ModelT,ConstT>& begin, const IIterator<ModelT2,ConstT2>& end,
			const StringT& separator, const EnclosureT& enclosure);

		/*!
		** \brief Append to the end of the string a new value
		**
		** \param rhs  Any supported value
		** \param size Size of the container
		*/
		template<class StringT>
		void append(const StringT& s, const Size size);

		/*!
		** \brief Append to the end of the string a new value
		**
		** \param s      Any supported value
		** \param size   Size of the container
		** \param offset Offset of the first character to append
		*/
		template<class StringT>
		void append(const StringT& s, const Size size, const Size offset);



		// Equivalent to append, provided for compatibility issues with other
		// Yuni containers
		//! \see template<class U> append(const U&, const Size)
		template<class U> void write(const U& cstr);
		//! \see template<class U> append(const U&, const Size)
		template<class U> void write(const U& cstr, const Size size);

		/*!
		** \brief Append a single signed char
		*/
		void put(const char c);
		/*!
		** \brief Append a single unsigned char
		*/
		void put(const unsigned char c);
		// equivalent to append, provided for compatibility with other containers
		template<class U> void put(const U& rhs);

		/*!
		** \brief Insert a raw C-String at a given position in the string
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param cstr A C-String
		** \param size Size of the string
		** \return True if the given string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		bool insert(const Size offset, const char* const cstr, const Size size);

		/*!
		** \brief Insert a single item at a given position in the string
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param c A single item
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		bool insert(const Size offset, const char c);

		/*!
		** \brief Insert an arbitrary C-String at a given position in the string
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param u Any CString container
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		template<class StringT> bool insert(const Size offset, const StringT& s);

		/*!
		** \brief Insert an arbitrary C-String at a given position in the string
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param s Any CString container
		** \param size The size to use for the given container
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		template<class StringT> bool insert(const Size offset, const StringT& u, const Size size);

		/*!
		** \brief Insert any arbitrary string at a given offset provided by an iterator
		**
		** If the offset is greater than the size of the string, the value
		** will be merely appended to the string.
		*/
		template<class ModelT, bool ConstT, class StringT>
		void insert(const IIterator<ModelT,ConstT>& it, const StringT& string);

		/*!
		** \brief Insert a raw C-String at the begining of in the string
		**
		** \param cstr A C-String
		** \param size Size of the string
		** \return True if the given string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		bool prepend(const char* const cstr, const Size size);

		/*!
		** \brief Insert a single item at the begining of the string
		**
		** \param c A single item
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		bool prepend(const char c);

		/*!
		** \brief Insert an arbitrary C-String at the begining of the string
		**
		** \param u Any CString container
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		template<class StringT> bool prepend(const StringT& s);

		/*!
		** \brief Insert an arbitrary C-String at the begining the string
		**
		** \param s Any CString container
		** \param size The size to use for the given container
		** \return True if the string has been inserted, false otherwise
		**   (size == 0 or offset out of bounds)
		*/
		template<class StringT> bool prepend(const StringT& u, const Size size);

		/*!
		** \brief Overwrite a region of the string
		**
		** The size of the string will remain untouched in any cases.
		**
		** \param offset Position of the first character of the region in the string
		** \param s A CString
		*/
		template<class StringT> void overwrite(const Size offset, const StringT& s);

		/*!
		** \brief Overwrite a region of the string
		**
		** The size of the string will remain untouched in any cases.
		**
		** \param s A CString
		*/
		template<class StringT> void overwrite(const StringT& s);

		/*!
		** \brief Overwrite a region of the string
		**
		** The size of the string will remain untouched in any cases.
		**
		** \param offset Position of the first character of the region in the string
		** \param cstr A C-String
		** \param size Size of 'cstr'
		*/
		void overwrite(const Size offset, const char* const cstr, const Size size);

		/*!
		** \brief Overwrite a region of the string from the right of the string
		**
		** The size of the string will remain untouched in any cases.
		**
		** \param region A CString container
		*/
		template<class StringT> void overwriteRight(const StringT& s);

		/*!
		** \brief Overwrite a region of the string from the right of the string
		**
		** The size of the string will remain untouched in any cases.
		**
		** \param offset Position of the first character (from the right) of the region
		**   in the string
		** \param region A CString container
		*/
		template<class StringT> void overwriteRight(const Size offset, const StringT& s);


		/*!
		** \brief Overwrite a region of the string from the center of the string
		**
		** The size of the string will remain untouched in any cases.
		** \code
		** String s;
		** s.resize(13, '-');
		** s.overwriteCenter(" Title ");
		** std::cout << s << std::endl; // '--- Title ---'
		** \endcode
		**
		** \param region A CString container
		*/
		template<class StringT> void overwriteCenter(const StringT& s);


		/*!
		** \brief Fill the entire string with a given pattern
		**
		** \code
		** String s = "some text here";
		** s.fill('.');
		** std::cout << s << std::endl; // '..............'
		**
		** s.resize(20);
		** s.fill('.');
		** s.overwrite(0,  "Chapter 1 ");
		** s.overwriteRight(" 4");
		** std::cout << s << std::endl; // 'Chapter 1  ....... 4'
		** \endcode
		**
		** \param pattern The pattern
		*/
		template<class StringT> void fill(const StringT& pattern);

		/*!
		** \brief Fill the entire string with a given pattern from a given offset
		**
		** \param offset Position of the first character where to start from
		** \param pattern The pattern
		*/
		template<class StringT> void fill(Size offset, const StringT& pattern);

		//! Equivalent to append()
		template<class U> void push_back(const U& u);

		//! Equivalent to prepend()
		template<class U> void push_front(const U& u);
		//@}


		//! \name Search / Replace
		//@{
		/*!
		** \brief Find the offset of a sub-string
		**
		** \param cstr An arbitrary string
		** \return True if sub-string is found, false otherwise
		*/
		bool contains(char c) const;

		/*!
		** \brief Find the offset of a sub-string (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \return True if sub-string is found, false otherwise
		*/
		bool icontains(char c) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return True if sub-string is found, false otherwise
		*/
		bool contains(const char* const cstr, const Size len) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes) (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return True if sub-string is found, false otherwise
		*/
		bool icontains(const char* const cstr, const Size len) const;

		/*!
		** \brief Find the offset of any supported CString
		**
		** \param cstr Any supported CString
		** \return True if sub-string is found, false otherwise
		*/
		template<class StringT> bool contains(const StringT& s) const;

		/*!
		** \brief Find the offset of any supported CString (ignoring the case)
		**
		** \param cstr Any supported CString
		** \return True if sub-string is found, false otherwise
		*/
		template<class StringT> bool icontains(const StringT& s) const;


		/*!
		** \brief Find the offset of a sub-string
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size find(char c) const;

		/*!
		** \brief Find the offset of a sub-string
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size find(char c, Size offset) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size find(const char* const cstr, Size offset, Size len) const;

		/*!
		** \brief Find the offset of any supported CString
		**
		** \param cstr Any supported CString
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		template<class StringT> Size find(const StringT& s, Size offset = 0) const;


		/*!
		** \brief Find the offset of a sub-string (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size ifind(char c) const;

		/*!
		** \brief Find the offset of a sub-string (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size ifind(char c, Size offset) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes) (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size ifind(const char* const cstr, Size offset, Size len) const;

		/*!
		** \brief Find the offset of any supported CString (ignoring the case)
		**
		** \param cstr Any supported CString
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		template<class StringT> Size ifind(const StringT& s, Size offset = 0) const;


		/*!
		** \brief Find the offset of a sub-string
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size rfind(char c) const;

		/*!
		** \brief Find the offset of a sub-string
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size rfind(char c, Size offset) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size rfind(const char* const cstr, Size offset, Size len) const;

		/*!
		** \brief Find the offset of any supported CString
		**
		** \param cstr Any supported CString
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		template<class StringT> Size rfind(const StringT& s, Size offset = npos) const;


		/*!
		** \brief Find the offset of a sub-string (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size irfind(char c) const;

		/*!
		** \brief Find the offset of a sub-string (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size irfind(char c, Size offset) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes) (ignoring the case)
		**
		** \param cstr An arbitrary string
		** \param len Size of the given cstr
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size irfind(const char* const cstr, Size offset, Size len) const;

		/*!
		** \brief Find the offset of any supported CString (ignoring the case)
		**
		** \param cstr Any supported CString
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		template<class StringT> Size irfind(const StringT& s, Size offset = npos) const;

		/*!
		** \brief Get if the string contains at least one occurence of a given char
		*/
		bool hasChar(char c) const;

		/*!
		** \brief Get if the string contains at least one occurence of a given unsigned char
		*/
		bool hasChar(unsigned char c) const;

		/*!
		** \brief Get the number of occurrences of a single char
		*/
		unsigned int countChar(char c) const;

		/*!
		** \brief Get the number of occurrences of a single unsigned char
		*/
		unsigned int countChar(unsigned char c) const;

		/*!
		** \brief Find the offset of a sub-string from the left
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param cstr An arbitrary string character
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size indexOf(Size offset, const char cstr) const;

		/*!
		** \brief Find the offset of a raw sub-string with a given length (in bytes) from the left
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param cstr An arbitrary C-string
		** \param len Size of the given string
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		Size indexOf(Size offset, const char* const cstr, const Size len) const;

		/*!
		** \brief Find the offset of any supported CString from the left
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered
		** \param cstr Any supported String
		** \return The offset of the first sub-string found, `npos` if not found
		*/
		template<class StringT> Size indexOf(Size offset, const StringT& s) const;

		/*!
		** \brief Searches the string for an individual character
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size find_first_of(char c, Size offset = 0) const;

		/*!
		** \brief Searches the string for an individual character (case insensitive)
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size ifind_first_of(char c, Size offset = 0) const;

		/*!
		** \brief Searches the string for any of the characters that are part of `seq`
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size find_first_of(const StringT& seq, Size offset = 0) const;

		/*!
		** \brief Searches the string for any of the characters that are part of `seq`
		**   (ignoring the case)
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size ifind_first_of(const StringT& seq, Size offset = 0) const;

		/*!
		** \brief Searches the string for the first character that is not `c`
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size find_first_not_of(char c, Size offset = 0) const;

		/*!
		** \brief Searches the string for the first character that is not `c` (case insensitive)
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size ifind_first_not_of(char c, Size offset = 0) const;

		/*!
		** \brief Searches the string for any of the characters that are not part of `seq`
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size find_first_not_of(const StringT& seq, Size offset = 0) const;

		/*!
		** \brief Searches the string for any of the characters that are not part of `seq` (case insensitive)
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size ifind_first_not_of(const StringT& seq, Size offset = 0) const;



		/*!
		** \brief Searches the string from the end for an individual character
		**
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size find_last_of(char c) const;

		/*!
		** \brief Searches the string from the end for an individual character (ignoring the case)
		**
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size ifind_last_of(char c) const;

		/*!
		** \brief Searches the string from the end for an individual character
		**
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		Size find_last_of(char c, Size offset) const;

		/*!
		** \brief Searches the string from the end for any of the characters that are part of `seq`
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size find_last_of(const StringT& seq, Size offset = npos) const;

		/*!
		** \brief Searches the string from the end for any of the characters that are part of `seq`
		**   (ignoring the case)
		**
		** \param seq An arbitrary string
		** \param offset Position of the first character in the string to be taken
		**   into consideration for possible matches. A value of 0 means that the
		**   entire string is considered.
		** \return The position of the first occurrence in the string (zero-based)
		**   npos if not found
		*/
		template<class StringT> Size ifind_last_of(const StringT& seq, Size offset = npos) const;

		/*!
		** \brief Get if a given string can be found at the begining
		*/
		bool startsWith(const char* const cstr, const Size len) const;

		/*!
		** \brief Get if a given string can be found at the begining
		*/
		template<class StringT> bool startsWith(const StringT& s) const;

		/*!
		** \brief Get if a given char can be found at the begining
		*/
		bool startsWith(char c) const;

		/*!
		** \brief Get if a given string can be found at the begining (case insensitive)
		*/
		bool istartsWith(const char* const cstr, const Size len) const;

		/*!
		** \brief Get if a given string can be found at the begining (case insensitive)
		*/
		template<class StringT> bool istartsWith(const StringT& s) const;

		/*!
		** \brief Get if a given char can be found at the begining (case insensitive)
		*/
		bool istartsWith(char c) const;

		/*!
		** \brief Get if a given string can be found at the end
		*/
		bool endsWith(const char* const cstr, const Size len) const;

		/*!
		** \brief Get if a given string can be found at the end
		** \param s Any string
		*/
		template<class StringT> bool endsWith(const StringT& s) const;

		/*!
		** \brief Get if a given string can be found at the end
		** \param c Any char
		*/
		bool endsWith(char c) const;

		/*!
		** \brief Get if a given string can be found at the end (case insensitive)
		*/
		bool iendsWith(const char* const cstr, const Size len) const;

		/*!
		** \brief Get if a given string can be found at the end (case insensitive)
		** \param s Any string
		*/
		template<class StringT> bool iendsWith(const StringT& s) const;

		/*!
		** \brief Get if a given string can be found at the end (case insensitive)
		** \param c Any char
		*/
		bool iendsWith(char c) const;
		//@}


		//! \brief Erase
		//@{
		/*!
		** \brief Remove iup to `n` characters from the end of the string
		*/
		void chop(unsigned int n);

		/*!
		** \brief Remove the last char
		*/
		void removeLast();

		/*!
		** \brief Remove the trailing slash or backslash at the end of the string (if any)
		**
		** Pseudo-code:
		** \code
		** Yuni::String s("/some/path/");
		** if ('\\' == s.last() || '/' == s.last())
		** 	s.removeLast();
		** std::cout << s << std::endl;  // -> /some/path
		** \endcode
		*/
		void removeTrailingSlash();
		//@}


		//! \name Trimming
		//@{
		/*!
		** \brief Remove all white-spaces (" \t\r\n") from the begining and the end
		**   of the string
		*/
		void trim();
		/*!
		** \brief Remove all white-spaces from the begining and the end of the string
		*/
		void trim(const char c);
		/*!
		** \brief Removes all items equal to one of those in 'whitespaces' from the
		**   end of the string
		*/
		template<class StringT> void trim(const StringT& whitespaces);

		/*!
		** \brief Removes all items equal to one of those in 'u' from the end of the string
		*/
		template<class StringT> void trimRight(const StringT& whitespaces);

		/*!
		** \brief Remove all items equal to 'c' from the end of the string
		*/
		void trimRight(const char c);

		/*!
		** \brief Removes all items equal to one of those in 'u' from the begining
		**   of the string
		*/
		template<class StringT> void trimLeft(const StringT& whitespaces);
		/*!
		** \brief Remove all items equal to 'c' from the begining of the string
		*/
		void trimLeft(const char c);
		//@}


		//! \name Remove / Erase
		//@{
		/*!
		** \brief Empty the string
		**
		** The size will be reset to 0 but the internal data will not be freed.
		** If you want to reduce the memory used by the string, use 'shrink()'
		** instead.
		** \see shrink()
		**
		** \return Always *this
		*/
		CustomString& clear();

		/*!
		** \brief Erase a part of the string
		**
		** \param offset The offset (zero-based) of the first item to erase
		** \param len The length (in number of items) to erase
		*/
		void erase(const Size offset, const Size len);

		/*!
		** \brief Erase a part of the string
		**
		** \param offset The offset (zero-based) of the first item to erase
		** \param len The length (in number of items) to erase
		*/
		template<class ModelT, bool ConstT>
		void erase(const IIterator<ModelT,ConstT>& offset, const Size len);

		/*!
		** \brief Replace all occurences of a string by another one
		**
		** \param from The string to find
		** \param to   The string to replace with
		*/
		template<class StringT1, class StringT2>
		void replace(const StringT1& from, const StringT2& to);

		/*!
		** \brief Replace all occurences of a string by another one
		**
		** \param offset The offset where to start from
		** \param from The string to find
		** \param to   The string to replace with
		*/
		template<class StringT1, class StringT2>
		void replace(Size offset, const StringT1& from, const StringT2& to);

		/*!
		** \brief Replace all occurences of a given char by another one
		**
		** \param from The character to search
		** \param to   The replacement
		*/
		void replace(char from, char to);

		/*!
		** \brief Replace all occurences of a given char by another one
		**
		** \param offset The offset where to start from
		** \param from The character to search
		** \param to   The replacement
		*/
		void replace(Size offset, char from, char to);

		/*!
		** \brief Replace all occurences of a given char by another one (case insensitive)
		**
		** \param from The character to search
		** \param to   The replacement
		*/
		void ireplace(char from, char to);

		/*!
		** \brief Replace all occurences of a given char by another one (case insensitive)
		**
		** \param offset The offset where to start from
		** \param from The character to search
		** \param to   The replacement
		*/
		void ireplace(Size offset, char from, char to);

		/*!
		** \brief Replace all occurences of a string by another one (case insensitive)
		**
		** \param from The string to find
		** \param to   The string to replace with
		*/
		template<class StringT1, class StringT2>
		void ireplace(const StringT1& from, const StringT2& to);

		/*!
		** \brief Replace all occurences of a string by another one (case insensitive)
		**
		** \param offset The offset where to start from
		** \param from The string to find
		** \param to   The string to replace with
		*/
		template<class StringT1, class StringT2>
		void ireplace(Size offset, const StringT1& from, const StringT2& to);

		/*!
		** \brief Remove the 'n' first characters
		*/
		void consume(Size n);
		//@}


		//! \name Case conversion
		//@{
		/*!
		** \brief Convert the case (lower case) of characters in the string (O(N))
		*/
		CustomString& toLower();
		/*!
		** \brief Convert the case (upper case) of characters in the string (O(N))
		*/
		CustomString& toUpper();
		//@}


		//! \name Comparisons
		//@{
		/*!
		** \brief Get if the string is equals to another one
		**
		** This method is equivalent to the operator '=='
		*/
		template<class StringT> bool equals(const StringT& rhs) const;

		/*!
		** \brief Get if the string is equals to another one (ignoring case)
		**
		** This method is equivalent to the operator '=='
		*/
		template<class StringT> bool equalsInsensitive(const StringT& rhs) const;

		/*!
		** \brief Compare the string with another one
		**
		** The comparison is done using unsigned characters.
		** \return An integer greater than, equal to, or less than 0, according as the string is greater than,
		**   equal to, or less than the given string
		*/
		template<class StringT> int compare(const StringT& rhs) const;

		/*!
		** \brief Compare the string with another one (ignoring the case)
		**
		** The comparison is done using unsigned characters.
		** \return An integer greater than, equal to, or less than 0, according as the string is greater than,
		**   equal to, or less than the given string
		*/
		template<class StringT> int compareInsensitive(const StringT& rhs) const;
		//@}


		//! \name Conversions
		//@{
		/*!
		** \brief Convert the string into something else
		**
		** The supported types (by default) are :
		** - std::string
		** - const char* (equivalent to `c_str()`)
		** - numeric (int, long, unsigned int, double...)
		** - bool
		*/
		template<class U> U to() const;

		/*!
		** \brief Convert the string into something else
		**
		** This method is strictly equivalent to `to()`, except
		** that we know if the conversion succeeded or not.
		**
		** \param[out] out The variable where to store the result of the conversion
		** \return True if the conversion succeeded. False otherwise
		*/
		template<class U> bool to(U& out) const;
		//@}


		//! \name Iterating through the string
		//@{
		/*!
		** \brief Get the next UTF-8 character
		**
		** \code
		** String t = "An UTF8 string : こんにちは !";
		** std::cout << "string            : " << t             << "\n";
		** std::cout << "valid             : " << t.utf8valid() << "\n";
		** std::cout << "raw size          : " << t.size()      << "\n";
		** std::cout << "nb of characters  : " << t.utf8size()  << "\n";
		**
		** // Iterating through the string
		** String::Size offset = 0;
		** UTF8::Char c;
		** std::cout << "All chars: ";
		** do
		** {
		** 	if (offset)
		** 		std::cout << ", ";
		** 	if (UTF8::errNone != t.utf8next<false>(offset, c))
		** 	{
		** 		std::cout << "<EOF>\n";
		** 		break;
		** 	}
		**
		** 	std::cout << c;
		** }
		** while (true);
		** \endcode
		**
		** \tparam InvalidateOffsetIfErrorT True to automatically set the offset
		**   parameter to `(Size)-1` when the result is not `errNone`
		** \param[out] offset Offset in the string
		** \param[out] out    The UTF-8 char
		** \return True if an UTF8 char has been found, false otherwise (@offset may become invalid)
		*/
		template<bool InvalidateOffsetIfErrorT>
		UTF8::Error utf8next(Size& offset, UTF8::Char& out) const;
		//@}


		//! \name Memory management
		//@{
		/*!
		** \brief Get the item at a given position in a safe way
		**
		** Contrary to the operator [], it is safe to use an invalid offset
		** \return The item at position 'offset', a default value if the offset is out of bound
		*/
		int at(const Size offset) const;

		/*!
		** \brief Truncate the string to the given length
		**
		** Nothing will be done if the new size if greater than the current one.
		** \param newSize The new size (in bytes)
		*/
		void truncate(const Size newSize);

		/*!
		** \brief Ensure that there is enough allocated space for X caracters
		**
		** \param min The minimum capacity of the string (in bytes)
		*/
		void reserve(Size minCapacity);

		/*!
		** \brief Resize the string to 'len' bytes
		**
		** The current content will remain untouched but all extra bytes will not be
		** initialized.
		** If the string can not be expanded, the new size will not be greater than 'ChunkSize'.
		**
		** \param len The new length (in bytes) of the string
		*/
		void resize(const Size len);

		/*!
		** \brief Resize the string to 'len' bytes and fill the new content (if any)
		**
		** The new content (if any) will be filled with 'pattern'.
		** If the string can not be expanded, the new size will not be greater than 'ChunkSize'.
		**
		** \code
		** String s;
		** s.resize(6, '.');
		** std::cout << s << std::endl; // '......'
		** s.resize(4, "useless pattern");
		** std::cout << s << std::endl; // '....'
		** s.resize(8, "-a");
		** std::cout << s << std::endl; // '....-a-a'
		**
		** s = "s: ";
		** s.resize(14, "\\/");
		** std::cout << s << std::endl; // 's: \/\/\/\/\/ ', note the space at the end
		** \endcode
		**
		** \param len The new length (in bytes) of the string
		** \parent pattern The pattern to use to fill the new content
		** \see fill()
		*/
		template<class StringT> void resize(const Size len, const StringT& pattern);

		/*!
		** \brief Releases any memory not required to store the character data
		**
		** If the string is empty, the internal buffer will be freed. Otherwise
		** the buffer will be reallocated to reduce as much as possible the amount
		** of memory used by the string.
		** It does not modify the size of the string, only its capacity.
		** This method has no effect when the template parameter 'ExpandableT'
		** is true.
		*/
		void shrink();

		/*!
		** \brief Perform a full check about UTF8 validity
		**
		** This check will iterate through the whole string to
		** detect any bad-formed UTF8 character.
		*/
		bool utf8valid() const;

		/*!
		** \brief Perform a full check about UTF8 validity
		**
		** This check will iterate through the whole string to
		** detect any bad-formed UTF8 character.
		** \param[out] offset The offset in the string of the misformed UTF8 character
		** \return UTF8::errNone if the string is valid
		*/
		UTF8::Error utf8valid(Size& offset) const;


		/*!
		** \brief Perform a fast check about UTF8 validity
		**
		** Contrary to `utf8valid()`, this check is only based on the first
		** code point of an UTF8 sequence. Consequently, it does not perform
		** a full compliance test and you should prefer `utf8valid()`.
		** As a consequence it may report that the string is a valid UTF8 string
		** even if it is not the case, but it may be good enough in some cases.
		** \see utf8valid()
		*/
		bool utf8validFast() const;

		/*!
		** \brief Compute the number of UTF-8 characters
		**
		** \code
		** String s = "こんにちは";
		** std::cout << "Size in bytes:    " << s.size() << std::endl; // 15
		** std::cout << "Nb of UTF8 chars: " << s.utf8size() << std::endl; // 5
		** \endcode
		** The returned value is computed at each call to this routine
		** \return The number of UTF8 character ( <= size )
		*/
		Size utf8size() const;

		/*!
		** \brief Get the current size of the string (in bytes)
		**
		** The returned value is less than or equal to the capacity, and
		** greater or equal to the number of UTF8 characters in the string.
		*/
		Size size() const;
		//! \see size()
		Size length() const;

		//! \see size()
		size_t sizeInBytes() const;

		//! the maximum number of characters that the string object can hold (for STL compliance)
		size_t max_size() const;

		/*!
		** \brief Get if the cstr is empty
		**
		** \code
		** CustomString<> s;
		** s.empty();          // returns true
		** s.null();           // returns true
		**
		** s = "hello world";  // returns false
		** s.empty();          // returns false
		** s.null();           // returns false
		**
		** s.clear();
		** s.empty();          // returns true
		** s.null();           // returns false
		**
		** s.shrink();
		** s.empty();          // returns true
		** s.null();           // returns true
		** \endcode
		*/
		bool empty() const;

		/*!
		** \brief Get if the cstr is null
		**
		** A null cstr means that no space is reserved, and that the
		** method `data()` will return NULL.
		**
		** \code
		** CustomString<> s;
		** s.empty();          // returns true
		** s.null();           // returns true
		**
		** s = "hello world";  // returns false
		** s.empty();          // returns false
		** s.null();           // returns false
		**
		** s.clear();
		** s.empty();          // returns true
		** s.null();           // returns false
		**
		** s.shrink();
		** s.empty();          // returns true
		** s.null();           // returns true
		** \endcode
		*/
		bool null() const;

		/*!
		** \brief Get if the string is not empty (the exact opposite of `empty()`)
		*/
		bool notEmpty() const;

		/*!
		** \brief Get the current capacity of the string (in bytes)
		** \return The amount of memory used by the string
		*/
		Size capacity() const;

		//! \see capacity()
		size_t capacityInBytes() const;

		/*!
		** \brief A pointer to the original cstr (might be NULL)
		** \see null()
		*/
		const char* c_str() const;

		/*!
		** \brief A pointer to the original cstr (might be NULL)
		** \see null()
		*/
		const char* data() const;
		char* data();
		//@}


		//! \name Formatted buffer
		//@{
		/*!
		** \brief Reset the current value with a formatted string
		**
		** The format is the standard printf format.
		** \param format The format, reprensented by a zero-terminated string
		** \return Always *this
		*/
		template<class StringT> CustomString& format(const StringT& format, ...);

		/*!
		** \brief Append formatted string
		**
		** The format is the standard printf format.
		** \param format The format, reprensented by a zero-terminated string
		** \return Always *this
		*/
		template<class StringT> CustomString& appendFormat(const StringT& format, ...);

		/*!
		** \brief Append a formatted string to the end of the current string
		**
		** The format is the standard printf format.
		** \param format The format, represented by a zero-terminated C-String
		*/
		void vappendFormat(const char* const format, va_list args);
		//@}


		//! \name Misc
		//@{
		/*!
		** \brief Get the first char of the string
		** \return The last char of the string if not empty, \0 otherwise
		*/
		char first() const;

		/*!
		** \brief Get the last char of the string
		** \return The last char of the string if not empty, \0 otherwise
		*/
		char last() const;


		/*!
		** \brief Get if the string matches a simple pattern ('*' only managed)
		**
		** \param pattern A pattern
		** \warning This method should not be used in a new code and will be removed
		**   as soon as possible
		** \TODO To be removed as soon as possible
		*/
		template<class StringT> bool glob(const StringT& pattern) const;

		/*!
		** \brief Convert all backslashes into slashes
		*/
		void convertBackslashesIntoSlashes();

		/*!
		** \brief Convert all slashes into backslashes
		*/
		void convertSlashesIntoBackslashes();

		/*!
		** \brief Explode a string into several segments
		**
		** Here is an example of how to convert a string to a list of int :
		** \code
		** std::list<int>  list;
		** String("22::80::443::993").explode(list, ":");
		** std::cout << list << std::endl;
		** \endcode
		**
		** \param[out] out All segments that have been found
		** \param sep Sequence of chars considered as a separator
		** \param keepEmptyElements True to keep empty items
		** \param trimElements Trim each item found
		** \param emptyBefore True to clear the vector before fulfill it
		**
		** \warning Do not take care of string representation (with `'` or `"`)
		** \deprecated Consider `split` instead. It will be removed in the next v0.2
		*/
		template<template<class,class> class U, class UType, class Alloc, typename StringT>
		YUNI_DEPRECATED("Consider `split` instead (will be removed in the next v0.2)",
		void explode(U<UType,Alloc>& out, const StringT& sep,
			bool emptyBefore = true, const bool keepEmptyElements = false, const bool trimElements = true) const);

		/*!
		** \brief Split a string into several segments
		**
		** Here is an example of how to convert a string to a list of int :
		** \code
		** std::list<int>  list;
		** String("22::80::443::993").split(list, ":");
		** std::cout << list << std::endl;
		** \endcode
		**
		** \param[out] out All segments that have been found
		** \param sep Sequence of chars considered as a separator
		** \param keepEmptyElements True to keep empty items
		** \param trimElements Trim each item found
		** \param emptyBefore True to clear the vector before fulfill it
		**
		** \warning This method does not take care of string representation (with `'` or `"`)
		*/
		template<template<class,class> class U, class UType, class Alloc, typename StringT>
		void split(U<UType,Alloc>& out, const StringT& sep,
			bool keepEmptyElements = false, bool trimElements = true, bool emptyBefore = true) const;

		/*!
		** \brief Dupplicate N times the content of the string
		*/
		void dupplicate(int n);


		/*!
		** \brief Extract the key and its value from a string (mainly provided by TDF
		** files or Ini files)
		**
		** Simple Ini file structure
		** \code
		** [section]
		** key = value
		** \endcode
		**
		** More complex :
		** \code
		** [section]
		** a = b; // Put your comments here
		** b =
		** c = ; // b = c, empty values
		** return cariage = A long string\non two lines
		** "key" = "All characters are allowed here, like semicolons; :)"
		** \endcode
		**
		** \param s A line (ex: `   category=core vtol ctrl_v level1 weapon  notsub ;`)
		** \param[out] key The key that has been found
		** \param[out] value The associated value
		** \param chcase The key will be converted to lowercase if equals to `soIgnoreCase`
		**
		** \see ExtractKeyvalue()
		*/
		template<class StringT1, class StringT2>
		void extractKeyValue(StringT1& key, StringT2& value, bool ignoreCase = false) const;

		//@}


		//! \name Adaptor only
		//@{
		/*!
		** \brief Adapt from a mere C-String
		*/
		void adapt(const char* cstring, Size length);

		/*!
		** \brief Adapt from any known string
		*/
		template<class StringT> void adapt(const StringT& string);
		//@}


		//! \name Operators
		//@{
		//! The operator `[]`, for accessing to a single char (the offset must be valid)
		const char& operator [] (const Size offset) const;
		//! The operator `[]`, for accessing to a single char (the offset must be valid)
		char& operator [] (const Size offset);

		//! The operator `+=` (append)
		template<class U> CustomString& operator += (const U& rhs);
		//! The operator `<<` (append)
		template<class U> CustomString& operator << (const U& rhs);

		//! The operator `=` (assign - copy)
		CustomString& operator = (const CustomString& rhs);
		//! The operator `=` (assign)
		template<class U> CustomString& operator = (const U& rhs);

		//! The operator `<`
		template<class StringT> bool operator <  (const StringT& rhs) const;
		//! The operator `>`
		template<class StringT> bool operator >  (const StringT& rhs) const;

		//! The operator `<=`
		template<class StringT> bool operator <= (const StringT& rhs) const;
		//! The operator `>=`
		template<class StringT> bool operator >= (const StringT& rhs) const;

		//! The operator `==`
		bool operator == (const CustomString& rhs) const;
		//! The operator `==`
		template<class StringT> bool operator == (const StringT& rhs) const;

		//! The operator `!=`
		template<class StringT> bool operator != (const StringT& rhs) const;

		//! The operator `!`  (if (!s) ... - equivalent to if (s.empty()))
		bool operator ! () const;

		//! The operator *=, to dupplicate N times the content of the string
		CustomString& operator *= (int n);
		//@}


	protected:
		//! Assign without checking for pointer validity
		Size assignWithoutChecking(const char* const block, const Size blockSize);
		//! Append without checking for pointer validity
		Size appendWithoutChecking(const char* const block, const Size blockSize);
		//! Append without checking for pointer validity
		Size appendWithoutChecking(const char c);
		//! Assign without checking for pointer validity
		Size assignWithoutChecking(const char c);
		//! Adapt without any check
		void adaptWithoutChecking(const char* const cstring, Size size);
		//! Decal the inner data pointer (must only be used when the class is an adapter)
		void decalOffset(Size count);

	private:

		/*!
		** \brief Set the string from a sequence of escaped characters (O(N))
		**
		** \param str The original string
		** \param maxLen The maximum length allowed
		** \param offset The offset where to start from
		*/
		void assignFromEscapedCharacters(const char* const str, Size maxLen, const Size offset);


	private:
		// our friends !
		template<class, class, int> friend class Private::CustomStringImpl::From;
		template<class, class> friend class Yuni::Extension::CustomString::Append;
		template<class, class> friend class Yuni::Extension::CustomString::Assign;
		template<class, class> friend class Yuni::Extension::CustomString::Fill;
		template<class, bool>  friend struct Private::CustomStringImpl::AdapterAssign;
		template<class, bool>  friend struct Private::CustomStringImpl::Consume;
		template<unsigned int, bool, bool> friend class CustomString;

	}; // class CustomString




	/*!
	** \brief String adapters
	**
	** This is a convenient typedef for declaring a string adapter.
	** A string adapter allow you to perform all read-only operations
	** provided by a string to an arbitrary raw buffer, without copying it.
	** This may be extremly useful to reduce memory consumption and to reduce
	** some useless memory allocation.
	**
	** \code
	** StringAdapter s;
	** s.adapt("Here is a raw C-string");
	** std::cout << "length     : " << s.size() << std::endl;
	** std::cout << "find 'raw' : " << s.find("raw") << std::endl;
	** \endcode
	**
	** Using a sub-string as it were a real string :
	** \code
	** String s = "abcdefghijklmnopqrst";
	** StringAdapter adapter (s.begin() + 2, s.begin() + 9);
	** std::cout << adapter << " (size: " << adapter.size() << ")" << std::endl;
	** \endcode
	*/
	typedef CustomString<0, true, false>  StringAdapter;





} // namespace Yuni

# include "iterator.hxx"
# include "customstring.hxx"
# include "operators.hxx"

#endif // __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_H__
