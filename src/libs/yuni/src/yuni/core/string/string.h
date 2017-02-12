#ifndef __YUNI_CORE_STRING_STRING_H__
# define __YUNI_CORE_STRING_STRING_H__

# include "../../yuni.h"
# include <stdlib.h>
# include <string.h>
# include <string>
# include <cstdarg>
# include <vector>
# include <map>
# include <list>
# include <limits.h>
# include <ostream>
# include <sstream>
# include "../static/remove.h"
# include "forward.h"
# include "../smartptr/smartptr.h"


//! Default separators
# ifndef YUNI_STRING_SEPARATORS
#	define YUNI_STRING_SEPARATORS   " \t\n\r"
# endif



namespace Yuni
{

	


	/*!
	** \brief A String implementation
	**
	** \deprecated This class is deprecated. You should consider CustomString instead
	**
	** The string class provides a useful way to manipulate and store sequences of
	** characters.
	**
	** This class is a template class, you should prefer the convenient alias
	** `Yuni::String`.
	**
	** \code
	**	  Yuni::StringBase<> a("abcd");
	**	  std::cout << a << std::endl;  // display: `abcd`
	**	  Yuni::StringBase<> b(10 + 2);
	**	  std::cout << b << std::endl;  // display: `12`
	**	  Yuni::StringBase<> c(10.3);
	**	  std::cout << c << std::endl;  // display: `10.3`
	**
	**	  // The same with the operator `<<`
	**	  Yuni::StringBase<> d;
	**	  d << "Value : " << 42;
	**	  std::cout << d << std::endl;  // display: `Value : 42`
	** \endcode
	**
	** \code
	**	  Yuni::StringBase<> s = "HelLo wOrLd";
	**	  std::cout << Yuni::String::ToLower(s) << std::endl;  // `hello world`
	**	  std::cout << s << std::endl;  // `HelLo wOrLd`
	**	  std::cout << s.toLower() << std::endl;  // `hello world`
	**	  std::cout << s << std::endl;  // `hello world`
	** \endcode
	**
	** \code
	** 	 Yuni::String::Vector list;
	** 	 list.push_back("BMW");
	** 	 list.push_back("Audi");
	** 	 list.push_back("Ferrari");
	** 	 list.push_back("9FF");
	**
	** 	 std::cout << list << std::endl; // BMW, Audi, Ferrari, 9FF
	**
	** 	 String s;
	** 	 s.append(list, ", ", "`");
	** 	 std::cout << s << std::endl; // `BMW`, `Audi`, `Ferrari`, `9FF`
	** \endcode
	**
	**
	** To know if a string is empty or not :
	** \code
	** Yuni::StringBase<> myString;
	** ...
	**
	** if (!myString)
	** 	;
	** // or
	** if (myString.empty())
	** 	;
	**
	** // Test if not empty
	** if (myString.notEmpty())
	** 	;
	** \endcode
	**
	**
	** \warning This class is not thread-safe
	** \warning This class is a final class, except if `YUNI_STRING_USE_VIRTUAL_DESTRUCTOR` is defined
	** \ingroup Core
	**
	** \tparam C The type of a single character
	** \tparam Chunk Size of a chunk
	*/
	template<typename C /* = char */, int Chunk /* = 80 */>
	class StringBase
	{
	public:
		//! Complete type for the string
		typedef StringBase<C,Chunk> StringType;
		//! Smartptr
		typedef SmartPtr<StringType> Ptr;

		//! A String list
		typedef std::list<StringType> List;
		//! A string list
		typedef std::list<typename StringType::Ptr> ListPtr;
		//! A String vector
		typedef std::vector<StringType> Vector;
		//! A String vector
		typedef std::vector<typename StringType::Ptr> VectorPtr;

		//! The type of object, CharT, stored in the string.
		typedef C Char;
		//! The type of object, CharT, stored in the string.
		typedef C Type;
		//! Size
		typedef size_t Size;

		enum
		{
			//! The largest possible value of type `Size` or `size_type`. That is, Size(-1)
			npos = Size(-1),
		};

		enum
		{
			//! Chunk size
			chunkSize = Chunk,
		};

		//! \name Compatibility with std::string
		//@{
		//! The type of object, CharT, stored in the string
		typedef C value_type;
		//! Pointer to Char
		typedef C* pointer;
		//! Reference to Char
		typedef C& reference;
		//! Const reference to Char
		typedef const C& const_reference;
		//! An unsigned integral type
		typedef size_t size_type;
		//! A signed integral type
		typedef ssize_t difference_type;
		//@}

		//! Char Case
		enum CharCase
		{
			//! The string should remain untouched
			soCaseSensitive,
			//! The string should be converted to lowercase
			soIgnoreCase
		};

		// Standard iterator
		class iterator;
		//! Const iterator
		class const_iterator;
		// Reverse iterator
		class reverse_iterator;
		//! Const reverse iterator
		class const_reverse_iterator;

		// Implementation of iterators
		# include "iterators.hxx"

	public:
		//! \name Case conversions
		//@{
		/*!
		** \brief Copy then Convert the case (lower case) of characters in the string
		** \param u The string to convert
		** \return A new string
		*/
		template<typename U> static StringBase<C,Chunk> ToLower(const U& u);

		/*!
		** \brief Copy then Convert the case (upper case) of characters in the string
		** \param u The string to convert
		** \return A new string
		*/
		template<typename U> static StringBase<C,Chunk> ToUpper(const U& u);
		//@}

		//! \name Searching
		//@{
		/*!
		** \brief Get if a string has at least one occurence of a given character
		** \param c The character to find
		** \param str The String
		*/
		template<typename C1, class U> static bool HasChar(const C1 c, const U& str);

		/*!
		** \brief Get the number of occurences of a char in various string implementations
		** \param c The character to find
		** \param str The String
		*/
		template<typename C1, class U> static Size CountChar(const C1 c, const U& str);
		//@}

		//! \name Comparisons
		//@{
		//! Get if two C-String are equivalent
		static bool Equals(const Char a[], const Char b[]);
		//! Get if two C-String are equivalent
		static bool Equals(const Char a[], const Char b[], const Size maxLen);
		//! Get if two Yuni Strings are equivalent (a == b)
		template<int Chnk1, int Chnk2>
		static bool Equals(const StringBase<Char,Chnk1>& a, const StringBase<Char,Chnk2>& b);

		//! Compare two C-String
		static int Compare(const Char a[], const Char b[], const Size maxLen = npos);
		//! Compare a Yuni string with a C-String
		template<int Chnk1>
		static int Compare(const StringBase<Char,Chnk1>& a, const Char b[], const Size maxLen = npos);
		//! Compare a C-String with a Yuni String
		template<int Chnk1>
		static int Compare(const Char a[], const StringBase<Char,Chnk1>& b, const Size maxLen = npos);
		//! Compare two yuni Strings
		template<int Chnk1, int Chnk2>
		static int Compare(const StringBase<Char,Chnk1>& a, const StringBase<Char,Chnk2>& b, const Size maxLen = npos);

		//! Compare two C-String (ignoring the case, as if it were two lowercase strings)
		static int CompareInsensitive(const Char a[], const Char b[], const Size maxLen = npos);
		//@}


		//! \name Length
		//@{
		/*!
		** \brief Get the length in bytes of any string implementations
		**
		** Various string implementations (C-String, std::string, Yuni::String...)
		*/
		template<typename U> static Size Length(const U& u);
		//@}


		//! \name Formatted string
		//@{
		/*!
		** \brief Formatted string
		**
		** \param f The format of the new string
		** \return A new string
		*/
		template<int Chnk1>
		static StringBase<Char,Chunk> Format(const StringBase<C,Chnk1>& f, ...);
		/*!
		** \brief Formatted string
		**
		** \param f The format of the new string
		** \return A new string
		*/
		template<int Chnk1>
		static StringBase<Char,Chunk> Format(StringBase<C,Chnk1>& f, ...);
		/*!
		** \brief Formatted string
		**
		** \param f The format of the new string
		** \return A new string
		*/
		static StringBase<Char,Chunk> Format(const char f[], ...);
		//@}


		//! \name Misc
		//@{
		/*!
		** \brief Try to find the first occurence of a string from `offset` in a STL container
		**
		** \param list A STL Container
		** \param str The string to find
		** \param offset The offset to start from
		** \return The index of string in the list, npos if not found
		*/
		template<template<class,class> class L, class T, class Alloc>
		static Size FindInList(const L<T,Alloc>& list, const StringBase<Char,Chunk>& str, const Size offset = 0);

		/*!
		** \brief Extract the key and its value from a string in the INI format
		**
		** \code
		**	String k, v;
		**
		**	// -> k='category'
		**	// -> v='core vtol ctrl_v level1 weapon  notsub'
		**	String::ExtractKeyValue("  category=core vtol ctrl_v level1 weapon  notsub ;", k, v)
		**
		**	// -> k='foo'
		**	// -> v='bar'
		**	String::ExtractKeyValue("  foo  = bar  ");
		**
		**	// -> k=''  v=''
		**	String::ExtractKeyValue("  } ", k, v); // used in TDF files
		**
		**	// -> k='['   v='Example of Section'
		**	String::ExtractKeyValue(" [Example of Section] ", k, v);
		**
		**	// -> k='foo'  v='bar'
		**	String::ExtractKeyValue(" foo=bar; // comments here; ", k, v);
		**
		**	// -> k='key'  v=' Allow special chars like semicolon `;` and spaces  '
		**	String::ExtractKeyValue(" key = \" Allow special chars like semicolon `;` and spaces  \"; // comments here; ", k, v);
		** \endcode
		**
		** For compatibility reasons, the parser allows the TDF format :
		** \code
		** [section]
		** {
		** 		key1 = value1;
		** 		key2 = value2;
		** }
		** \endcode
		**
		**
		** \param s Any line (ex: `   category=core vtol ctrl_v level1 weapon  notsub ;`)
		** \param[out] key The key that has been found
		** \param[out] value The associated value
		** \param chcase The key will be converted to lowercase if equals to `soIgnoreCase`
		*/
		template<int Ck1, int Ck2>
		static void ExtractKeyValue(const StringBase& s, StringBase<C,Ck1>& key, StringBase<C,Ck2>& value,
			const enum CharCase chcase = soCaseSensitive);

		/*!
		** \brief Copy the string and convert all escaped characters (O(N))
		**
		** \code
		** std::cout << Yuni::String::ConvertEscapedCharacters("A long string\\nwith two lines") << std::endl;
		** \endcode
		**
		** You should prefer the second form of this method `String::assignFromEscapedCharacters(str)`.
		**
		** \see man printf
		** \bug Not all escaped characters are handled
		** \see assignFromEscapedCharacters()
		*/
		static StringBase<C,Chunk> ConvertEscapedCharacters(const char str[]);
		//
		static StringBase<C,Chunk> ConvertEscapedCharacters(const wchar_t str[]);
		//
		template<typename U>
		static StringBase<C,Chunk> ConvertEscapedCharacters(const std::basic_string<U>& str);
		//
		template<int Chnk1>
		static StringBase<C,Chunk> ConvertEscapedCharacters(const StringBase<C,Chnk1>& str);

		/*!
		** \brief Find the end of a sequence, started and terminated by a given character (usually a quote)
		**
		** This method is not a simple find(), because it takes care of escaped
		** characters
		**
		** \param str The sequence
		** \param quote The character to find, usually a quote
		*/
		static Size FindEndOfSequence(const Char* str, const Char quote, Size maxLen = npos);

		/*!
		** \brief Convert an ascii string into UTF8
		*/
		static StringBase ToUTF8(const C* s);
		template<int Chnk1> static StringBase ToUTF8(const StringBase<C,Chnk1>& s);

		/*!
		** \brief Get the CString part of any string
		**
		** \tparam U Any string type (const char*, std::string, String, ...)
		*/
		template<typename U> static const Char* CString(const U& u);
		//@} Misc


	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		StringBase();

		//! Copy constructor
		StringBase(const StringBase& copy);

		//! Copy constructor
		template<int Chnk1> StringBase(const StringBase<Char,Chnk1>& copy);

		/*!
		** \brief Constructor from a substring
		**
		** \param rhs Another string
		** \param offset Offset where to start from
		** \param len The maximum number of characters to copy
		*/
		template<int Chnk1>
		StringBase(const StringBase<Char,Chnk1>& rhs, const Size pos, Size len = npos);

		/*!
		** \brief Constructor from a substring
		**
		** \param rhs A std::string
		** \param offset Offset where to start from
		** \param len The maximum number of characters to copy
		*/
		template<typename C1>
		StringBase(const std::basic_string<C1>& rhs, const Size pos, Size len = npos);

		/*!
		** \brief Constructor - From a C-String
		** \param str A C-String (may be null)
		*/
		StringBase(const char str[]);

		/*!
		** \brief Constructor - From a C-String
		** \param str A C-String (may be null)
		*/
		StringBase(const wchar_t str[]);

		/*!
		** \brief Constructor - From a single char
		** \param c The char to copy, even if equals to \0
		*/
		StringBase(const Char c);

		/*!
		** \brief Constructor with an empty value
		*/
		StringBase(const NullPtr&);

		/*!
		** \brief Constructor - From a repetition of chars
		**
		** \param n The number of occurences
		** \param c The char (can be equals to \0)
		*/
		StringBase(const Size n, const Char c);

		/*!
		** \brief Constructor - From a std::string
		** \param str A std::string
		*/
		template<typename C1>
		StringBase(const std::basic_string<C1>& rhs);

		/*!
		** \brief Constructor - From any other types
		** \param u Any value that the method `append(U)` can handle
		*/
		template<typename U> explicit StringBase(const U& u);

		/*!
		** \brief Constructor - From a C-String with a maximum number of chars
		**
		** \param str A C-String
		** \param len The maximum number of characters to copy
		*/
		template<typename C1> StringBase(const C1* str, const Size len);

		/*!
		** \brief Constructor - From iterators
		**
		** \code
		** Yuni::StringBase<> s("0123456789");
		** Yuni::StringBase<> t(s.begin(), s.begin() + 5);
		** std::cout << t << std::endl;
		** \endcode
		**
		** \param begin Iterator at the begining of the substring
		** \param end Iterator at the end of the substring
		*/
		template<class IteratorClass>
		StringBase(const IteratorClass& begin, const IteratorClass& end);

		//! Destructor
		YUNI_STRING_VIRTUAL_DESTRUCTOR ~StringBase();
		//@}


		//! \name Iterators
		//@{
		//! Get an iterator pointing to the beginning of the string
		iterator begin();
		const_iterator begin() const;
		//! Get an iterator pointing to the end of the string
		iterator end();
		const_iterator end() const;

		//! Get a reverse iterator to reverse beginning of the string
		reverse_iterator rbegin();
		const_reverse_iterator rbegin() const;
		//! Get reverse iterator to reverse end of the string
		reverse_iterator rend();
		const_reverse_iterator rend() const;
		//@}


		//! \name Assign
		//@{
		/*!
		** \brief Clear the string than append a string
		**
		** \param u Any value that the method `append(U)` can handle
		*/
		template<typename U> void assign(const U& u);

		/*!
		** \brief Set the value of the string with a maximum length
		**
		** \param u Any value that the method `append(U, len)` can handle
		** \param len The maximum number of chars to copy
		*/
		template<typename U> void assign(const U& u, const Size len);

		/*!
		** \brief Set the value of the string from a substring of another one
		*/
		template<int Chnk1>
		void assign(const StringBase<Char,Chnk1>& s, const Size offset, const Size len);

		/*!
		** \brief Set the value of the string from a raw buffer
		**
		** No checks are performed here.
		** \param u A buffer (may be null)
		** \param len Size of the buffer
		** \return Always this
		*/
		StringBase& assignRaw(const char* u, const Size len);
		//@}


		//! \name Append
		//@{
		/*!
		** \brief Append an expression to the end of the string (if the appropriate
		** converter is available)
		**
		** \code
		** Yuni::StringBase<> s;
		** s.append("A C-String;");                  // a C-String
		** s.append(std::string("A Std-string."));   // A std::string
		** s.append(42);                             // an int
		** s.append(12.5);                           // a double
		** \endcode
		**
		** \tparam U Any type
		** \param u Any value
		*/
		template<typename U> void append(const U& u);

		/*!
		** \brief Append an expression with a maximum length to the end of the
		** string (if the appropriate converter is available)
		**
		** \code
		** Yuni::StringBase<> s;
		** s.append("123456789", 3); // will only append the C-String "123"
		** \endcode
		**
		** \tparam U Any type
		** \param u Any value
		** \param len The maximum length
		*/
		template<typename U> void append(const U& u, const Size len);

		/*!
		** \brief Append an expression with a maximum length to the end of the
		** string (if the appropriate converter is available)
		**
		** \code
		** Yuni::StringBase<> s;
		** s.append("123456789", 1, 3); // will only append the C-String "23"
		** \endcode
		**
		** \tparam U Any type
		** \param u Any value
		** \param offset The offset where to start from
		** \param len The maximum length
		*/
		template<typename U> void append(const U& u, const Size offset, const Size len);

		/*!
		** \brief Append a STL container
		**
		** \code
		** Yuni::StringBase<> s;
		** std::vector<int> vect;
		** vect.push_back(10);
		** vect.push_back(30);
		** s.append(vect); // will only append the 5 first values
		** std::cout << s << std::endl; // `10, 30`
		** \endcode
		**
		** \param u Any value
		** \param len The maximum count of items to append
		*/
		template<template<class,class> class L, class TypeL, class Alloc>
		void append(const L<TypeL,Alloc>& u);

		template<template<class,class> class L, class TypeL, class Alloc>
		void append(const L<TypeL,Alloc>& u, const unsigned int max);

		template<template<class,class> class L, class TypeL, class Alloc, typename S, typename E>
		void append(const L<TypeL,Alloc>& u, const S& separator, const E& enclosure,
			const unsigned int max);

		template<template<class,class> class L, class TypeL, class Alloc, typename S>
		void append(const L<TypeL,Alloc>& u, const S& separator, const unsigned int max);

		template<template<class,class> class L, class TypeL, class Alloc, typename S, typename E>
		void append(const L<TypeL,Alloc>& u, const S& separator, const E& enclosure);

		template<template<class,class> class L, class TypeL, class Alloc, typename S>
		void append(const L<TypeL,Alloc>& u, const S& separator);

		//! This function is provided for STL compatibility. It is equivalent to append(u)
		template<typename U> void push_back(const U& u);

		/*!
		** \brief Append a single char to the string
		*/
		void put(const C c);

		/*!
		** \brief Append a raw buffer to the end of the string
		**
		** No checks are performed here.
		** \param u A buffer (may be null)
		** \param len Size of the buffer
		** \return Always this
		*/
		StringBase& appendRaw(const char* u, const Size len);

		//@}


		//! \name Inserting
		//@{
		/*!
		** \brief Prepend any value at the begining of the string
		**
		** This is a convenient method to insert at the begining and is
		** strictly equivalent to `insert(0, u)`.
		** \param u Any value that the method `insert(0, u)` can handle
		*/
		template<typename U> void prepend(const U& u);

		/*!
		** \brief Insert a value at a given offset in the string
		**
		** If the offset is greater than the size of the string, the value
		** will be merely appended to the string (equivalent to `append(u)`).
		**
		** \param offset The offset in the string
		** \param u A value
		*/
		template<typename U> void insert(const Size offset, const U& u);

		/*!
		** \brief Insert a value at a given offset (from an iterator) in the string
		**
		** If the offset is greater than the size of the string, the value
		** will be merely appended to the string (equivalent to `append(u)`).
		**
		** \param it The offset in the string given by the iterator
		** \param u A value
		*/
		template<typename U> void insert(const iterator& it, const U& u);
		template<typename U> void insert(const const_iterator& it, const U& u);
		template<typename U> void insert(const reverse_iterator& it, const U& u);
		template<typename U> void insert(const const_reverse_iterator& it, const U& u);
		//@}

		//! \name Removing
		//@{
		/*!
		** \brief Rease a range of characters
		**
		** \param offset The offset of the first character
		** \param len The number of characters to remove
		** \return Always *this
		*/
		StringBase& erase(const Size offset, const Size len = npos);

		/*!
		** \brief Rease a range of characters
		**
		** The iterator `end` may be located before `begin`.
		** \param begin The begining of the sub string
		** \param end The end of the substring
		** \return Always *this
		*/
		void erase(const iterator& begin, const iterator& end);
		void erase(const const_iterator& begin, const const_iterator& end);
		void erase(const reverse_iterator& begin, const reverse_iterator& end);
		void erase(const const_reverse_iterator& begin, const const_reverse_iterator& end);

		/*!
		** \brief Remove the X first occurences in the string
		**
		** \param u The substring to remove (any type that `find` and `Length` can handle)
		** \param maxOccurences The maxinum number of occurences to remove
		** \param offset The offset where to start from
		** \return The number of occurences
		*/
		template<typename U> Size remove(const U& u, Size offset = 0, const Size maxOccurences = npos);

		/*!
		** \brief Removes white-space from the left of the string
		** \see YUNI_STRING_SEPARATORS
		*/
		void trimLeft();
		/*!
		** \brief Removes white-space from the left of the string
		**
		** Any type `U` is allowed, if the overload for the method `HasChar`
		** if available.
		** \param separators The list of chars considered as white-spaces
		** \see HasChar()
		*/
		template<typename U> void trimLeft(const U& separators);

		/*!
		** \brief Removes white-space from the right end of the string
		** \see YUNI_STRING_SEPARATORS
		*/
		void trimRight();
		/*!
		** \brief Removes white-space from the right end of the string
		**
		** Any type `U` is allowed, if the overload for the method `HasChar`
		** if available.
		** \param separators The list of chars considered as white-spaces
		** \see HasChar()
		*/
		template<typename U> void trimRight(const U& separators);

		/*!
		** \brief Removes white-space from the left and the right end of the string
		*/
		void trim();
		/*!
		** \brief Removes white-space from the left and the right end of the string
		**
		** Any type `U` is allowed, if the overload for the method `HasChar`
		** if available.
		** \param separators The list of chars considered as white-spaces
		** \see HasChar()
		*/
		template<typename U> void trim(const U& separators);

		/*!
		** \brief Remove the last char of the string if not empty
		** \see last()
		*/
		void removeLast();

		/*!
		** \brief Remove the trailing slash or backslash at the end of the string if any
		**
		** This method is nearly equivalent to :
		** \code
		** Yuni::StringBase<> s("/some/path/");
		** if ('\\' == s.last() || '/' == s.last())
		** 	s.removeLast();
		** std::cout << s << std::endl;  // -> /some/path
		** \endcode
		*/
		void removeTrailingSlash();

		/*!
		** \brief Remove `n` characters from the end of the string
		*/
		void chop(const Size n);
		//@}


		//! \name Conversions
		//@{
		/*!
		** \brief Convert the string to any type
		**
		** \code
		** Yuni::StringBase<> s("42");
		** unsigned int u = s.to<unsigned int>();
		** bool b = s.to<bool>();
		** \endcode
		**
		** \note For the conversion be valid, the template specialization
		** of `Private::StringImpl::From` must be available.
		*/
		template<typename U> U to() const;

		/*!
		** \brief Convert the string to any type, and get if the conversion succeeded
		**
		** \code
		** Yuni::StringBase<> s("42");
		** unsigned int result;
		** if (s.to<unsigned int>(result))
		** {
		** 	// Do something
		** }
		** \endcode
		**
		** \note For the conversion be valid, the template specialization
		** of `Private::StringImpl::From` must be available.
		**
		** \param[out] u The variable where to store the result
		*/
		template<typename U> bool to(U& u) const;

		/*!
		** \brief Convert the string to UTF8
		*/
		StringBase& toUTF8();

		//! Get the char at a specific location
		Char at(const Size offset) const;

		//! Conversion to a C-String
		const char* c_str() const throw();

		/*!
		** \brief Return a pointer to the first character in the string
		**
		** This method is provided for compatibility with the STL only and
		** *should* not be used.
		** \internal There is no guarantee that this buffer is zero-terminated
		**  but it is guaranteed that there is at least
		**  `sizeof(Char) * size()` bytes.
		*/
		const Char* data() const throw();
		Char* data() throw();
		//@}


		//! \name Searching
		//@{
		/*!
		** \brief Try to find the position of a substring
		**
		** \param u The substring to find (can be null)
		** \return The position of the substring, npos if not found or empty
		*/
		template<typename U> Size find(const U& u) const;

		/*!
		** \brief Get if the string contains a substring
		**
		** \param u The substring to find (can be null)
		** \return True if `u` can be found in the string
		*/
		template<typename U> bool contains(const U& u) const;

		/*!
		** \brief Try to find the position of a substring starting at a given offset
		**
		** \param u The string to find (can be null)
		** \param offset The index where to start from
		** \return The position of the char, npos if not found
		*/
		template<typename U> Size find(const U& u, const Size offset) const;

		//! Alias for find()
		template<typename U> Size find_first_of(const U& u) const;
		//! Alias for find()
		template<typename U> Size find_first_of(const U& u, const Size offset) const;

		template<typename U> Size find_first_not_of(const U& u) const;
		template<typename U> Size find_first_not_of(const U& u, const Size offset) const;


		/*!
		** \brief Try to find the last position of a substring from the end of the string
		**
		** \param u The substring to find (can be null)
		** \return The position of the substring, npos if not found or empty
		*/
		template<typename U> Size rfind(const U& u) const;

		/*!
		** \brief Try to find the last position of a substring starting at a given offset
		**
		** \param u The string to find (can be null)
		** \param offset The index where to start from
		** \return The position of the char, npos if not found
		*/
		template<typename U> Size rfind(const U& u, const Size offset) const;

		//! Alias for find()
		template<typename U> Size find_last_of(const U& u) const;
		//! Alias for find()
		template<typename U> Size find_last_of(const U& u, const Size offset) const;

		template<typename U> Size find_last_not_of(const U& u) const;
		template<typename U> Size find_last_not_of(const U& u, const Size offset) const;

		//! Get if the string contains a char (O(N))
		bool hasChar(const Char c) const;
		//! Get if the string contains a char
		bool hasChar(const Char c[]) const;
		//! Get if the string contains a char
		template<int Chnk1> bool hasChar(const StringBase<Char,Chnk1>& str) const;

		/*!
		** \brief Get the number of occurences of a char (O(N))
		*/
		Size countChar(const Char c) const;

		/*!
		** \brief Get if a given string can be found at the begining
		**
		** \param s The other string.
		** \param option soIgnoreCase to be case insensitive
		** \return True if `s` has been found at the begining of the string
		*/
		bool startsWith(const Char* s, CharCase option = soIgnoreCase) const;
		template<int Chnk1> bool startsWith(const StringBase<Char,Chnk1>& s, CharCase option = soIgnoreCase) const;

		/*!
		** \brief Get if the string matches a simple pattern ('*' only managed)
		*/
		bool glob(const C* pattern) const;
		template<int Chnk1> bool glob(const StringBase<Char,Chnk1>& pattern) const;
		//@}


		//! \name Replacing
		//@{
		/*!
		** \brief Replace a sub part of the string by another one
		*/
		template<typename U> StringBase& replace(const Size offset, const Size len, const U& by);

		/*!
		** \brief Replace a substring by another thing
		*/
		template<typename U, typename V> StringBase& replace(const U& u, const V& v);

		/*!
		** \brief Replace all occurences of a given char by another one
		**
		** \param from The char to find and replace
		** \param to The replacement
		*/
		StringBase& replace(const Char from, const Char to);

		/*!
		** \brief Replace all occurences of a given char by another one starting from an offset
		**
		** \param from The char to find and replace
		** \param to The replacement
		** \param offset The index where to start from
		*/
		StringBase& replace(const Char from, const Char to, const Size offset);
		//@}


		//! \name Extracting
		//@{
		/*!
		** \brief Get the sub-string of the string
		**
		** \param pos The position of the first character
		*/
		StringBase substr(const Size offset) const;
		/*!
		** \brief Get the sub-string of the string
		**
		** \param pos The position of the first character
		** \param len The length of the substring to extract
		*/
		StringBase substr(const Size offset, const Size len) const;

		/*!
		** \brief Get the sub-string of the string assuming it is an UTF8 charset
		**
		** \param pos The position of the first character
		** \param len The length of the substring to extract
		*/
		StringBase substrUTF8(Size pos, Size len = npos) const;
		//@}


		//! \name Case conversion
		//@{
		/*!
		** \brief Convert the case (lower case) of characters in the string (O(N))
		*/
		StringBase& toLower();
		/*!
		** \brief Convert the case (upper case) of characters in the string (O(N))
		*/
		StringBase& toUpper();
		//@}


		//! \name Format
		//@{
		/*!
		** \brief Reset the current value with a formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		template<int Chnk1> StringBase& format(const StringBase<Char,Chnk1>& f, ...);
		/*!
		** \brief Reset the current value with a formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		template<int Chnk1> StringBase& format(StringBase<Char,Chnk1>& f, ...);
		/*!
		** \brief Reset the current value with a formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		StringBase& format(const char f[], ...);

		/*!
		** \brief Append formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		template<int Chnk1> StringBase& appendFormat(const StringBase<Char,Chnk1>& f, ...);
		/*!
		** \brief Append a formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		template<int Chnk1> StringBase& appendFormat(StringBase<Char,Chnk1>& f, ...);
		/*!
		** \brief Append formatted string
		**
		** \param f The format
		** \return Always *this
		*/
		StringBase& appendFormat(const char f[], ...);

		/*!
		** \brief Append a formatted string to the end of the current string
		*/
		void vappendFormat(const char f[], va_list parg);
		//@}


		//! \name Misc
		//@{
		/*!
		** \brief Get the first char of the string
		** \return The last char of the string if not empty, \0 otherwise
		*/
		Char first() const;

		/*!
		** \brief Get the last char of the string
		** \return The last char of the string if not empty, \0 otherwise
		*/
		Char last() const;

		/*!
		** \brief Convert all backslashes into slashes
		*/
		void convertBackslashesIntoSlashes();

		/*!
		** \brief Convert all slashes into backslashes
		*/
		void convertSlashesIntoBackslashes();

		/*!
		** \brief Set the string from a sequence of escaped characters (O(N))
		**
		** \code
		** Yuni::StringBase<> s;
		** s.assignFromEscapedCharacters("A long string\\nwith two lines");
		** std::cout << s << std::endl;
		** \endcode
		**
		** \param str The original string
		** \see man printf
		** \bug Not all escaped characters are handled
		*/
		void assignFromEscapedCharacters(const char str[]);
		//
		void assignFromEscapedCharacters(const wchar_t str[]);
		//
		template<typename U>
		void assignFromEscapedCharacters(const std::basic_string<U>& str);
		//
		template<int Chnk1>
		void assignFromEscapedCharacters(const StringBase<C,Chnk1>& str);

		/*!
		** \brief Set the string from a sequence of escaped characters (O(N))
		**
		** \param str The original string
		** \param maxLen The maximum length allowed
		** \param offset The offset where to start from
		*/
		template<int Chnk1>
		void assignFromEscapedCharacters(const StringBase<C,Chnk1>& str, Size maxLen,
			const Size offset = 0);


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
		template<int Ck1, int Ck2>
		void extractKeyValue(StringBase<C,Ck1>& key, StringBase<C,Ck2>& value,
			const enum CharCase chcase = soCaseSensitive) const;

		/*!
		** \brief Explode a string into several segments
		**
		** Here is an example of howto convert a string to a list of int :
		** \code
		** std::list<int>  list;
		** String("22::80::443::993").explode(list, ":");
		** std::cout << list << std::endl;
		** \endcode
		**
		** \param[out] out All segments that have been found
		** \param sep Sequence of chars considered as a separator
		** \param emptyBefore True to clear the vector before fulfill it
		** \param keepEmptyElements True to keep empty items
		** \param trimElements Trim each item found
		** \return Always this
		**
		** \warning Do not take care of string representation (with `'` or `"`)
		*/
		template<template<class,class> class U, class UType, class Alloc, typename S>
		void explode(U<UType,Alloc>& out, const S& sep = YUNI_STRING_SEPARATORS,
			const bool emptyBefore = true, const bool keepEmptyElements = false, const bool trimElements = true) const;

		/*!
		** \brief Get the hash value of this string
		*/
		uint32 hashValue() const;

		/*!
		** \brief Ensure that the string length is N
		**
		** If the size < N, then N-size chars will be added.
		** If the size < N, then the string will be truncated to N
		**
		** \param newSize The desired new size of the string
		** \param defaultChar The char to use for all new char
		*/
		void toNChars(const unsigned int newSize, const Char defaultChar = ' ');

		/*!
		** \brief Print the content of the string to an ostream
		**
		** \param[in,out] A stream
		*/
		void print(std::ostream& out) const;
		//@}


		//! \name Memory management
		//@{
		/*!
		** \brief Clear the string
		*/
		StringBase& clear();

		//! Get if the string is empty
		bool empty() const;
		//! Get if the string is not empty
		bool notEmpty() const;

		//! Get the size (number of caracters) in the string
		Size size() const;
		//! Alias for size()
		Size count() const;
		//! Alias for size()
		Size length() const;

		//! Get the size (number of caracters) in the string, assuming it is an UTF8 charset
		Size sizeUTF8() const;
		//! Alias for sizeUTF8()
		Size countUTF8() const;
		//! Alias for sizeUTF8()
		Size lengthUTF8() const;

		/*!
		** \brief Ensure that there is enough allocated space for X caracters
		**
		** \param min The minimum capacity of the buffer (in caracters)
		*/
		void reserve(const Size min);

		/*!
		** \brief Resize the string to 'len' bytes
		**
		** The current content will remain untouched but all extra bytes will not be
		** initialized.
		**
		** \param len The new length of the string
		*/
		void resize(const Size len);

		//! Get the number of characters that the string can currently hold
		Size capacity() const;
		//! Alias for capacity()
		Size max_size() const;

		/*!
		** \brief Truncate the string to the given length
		*/
		void truncate(const Size maxLen);

		/*!
		** \brief Releases any memory not required to store the character data
		**
		** In general, you will rarely ever need to call this function.
		*/
		void shrink();

		/*!
		** \brief Swap the content with another string
		** \param s The other string
		*/
		void swap(StringBase& s);

		/*!
		** \brief Get the allocator used
		**
		** This method is only provided for compatibility with the STL
		*/
		static std::allocator<C> get_allocator();
		//@}


		//! \name Operators
		//@{
		/*!
		** \brief Set the value of the string
		**
		** This method is strictly equivalent to the method append()
		** with one parameter but the string is cleared before.
		** We also need it to override the default operator =, or bad
		** things(c) happen.
		*/
		StringBase& operator = (const StringBase& rhs);

		/*!
		** \brief Set the value of the string
		**
		** This method is strictly equivalent to the method append()
		** with one parameter but the string is cleared before.
		*/
		template<typename U> StringBase& operator = (const U& u);

		/*!
		** \brief Empty the string
		**
		** This method is strictly equivalent to the method clear()
		*/
		StringBase& operator = (const NullPtr&);

		/*!
		** \brief Append a value to the end of the string
		**
		** This method is strictly equivalent to the method append()
		** with one parameter.
		*/
		template<typename U> StringBase& operator += (const U& u);

		/*!
		** \brief Append a null value to the end of the string (actually do nothinh)
		**
		** This method is strictly equivalent to the method append()
		** with one parameter.
		*/
		StringBase& operator += (const NullPtr&);

		/*!
		** \brief Append a value to the end of the string
		**
		** This method is strictly equivalent to the method append()
		** with one parameter.
		*/
		template<typename U> StringBase& operator << (const U& u);

		/*!
		** \brief Append a null value to the end of the string (actually do nothinh)
		**
		** This method is strictly equivalent to the method append()
		** with one parameter.
		*/
		StringBase& operator << (const NullPtr&);

		//! Get if the string is less than a C-String (can be null)
		bool operator < (const Char rhs[]) const;
		//! Get if the string is less than another String
		template<int Chnk1> bool operator < (const StringBase<C,Chnk1>& rhs) const;


		//! Get if the string is greater than a C-String (can be null)
		bool operator > (const Char rhs[]) const;
		//! Get if the string is greater than another String
		template<int Chnk1> bool operator > (const StringBase<C,Chnk1>& rhs) const;

		//! Get if the string is equivalent to a C-String (can be null)
		bool operator == (const Char rhs[]) const;

		//! Get if the string is equivalent to a NULL C-String
		bool operator == (const NullPtr&) const;

		template<int N> bool operator == (const Char rhs[N]) const;
		//! Get if the string is equivalent to another string
		template<int Chnk1>
		bool operator == (const StringBase<Char,Chnk1>& rhs) const;

		//! Get if the string is not equivalent to a C-String (can be null)
		bool operator != (const Char rhs[]) const;
		template<int N> bool operator != (const Char rhs[N]) const;
		//! Get if the string is not equivalent to another string
		template<int Chnk1>
		bool operator != (const StringBase<Char,Chnk1>& rhs) const;
		//! Get if the string is equivalent to a NULL C-String
		bool operator != (const NullPtr&) const;

		/*!
		** \brief Get an iterator at a specific position
		**
		** \param indx The position
		** \return A reference to the character
		*/
		Char& operator [] (const Size indx);
		const Char& operator [] (const Size indx) const;

		/*!
		** \brief Get if the string is empty
		**
		** \code
		** Yuni::StringBase<> s; // here is an empty string
		** if (!s)
		**	// make some stuff here
		** \endcode
		*/
		bool operator ! () const;
		//@}


	protected:
		//! Length of the string
		Size pSize;
		//! The largest possible size of the string
		Size pCapacity;
		//! Pointer to the inner buffer
		Char* pPtr;

		// Friends !
		template<class C1, int Chunk1> friend class StringBase;
		template<class StrBase1, class T1> friend struct Private::StringImpl::Length;
		template<class StrBase1, class T1> friend struct Private::StringImpl::HasChar;
		template<class StrBase1, class T1> friend struct Private::StringImpl::CountChar;
		template<class StrBase1, class T1> friend struct Private::StringImpl::Find;
		template<class StrBase1, class T1> friend struct Private::StringImpl::Remove;
		template<class StrBase1, class T1, bool Equals> friend struct Private::StringImpl::FindFirstOf;
		template<class StrBase1, class T1, bool Equals> friend struct Private::StringImpl::FindLastOf;
		template<typename T1> friend struct Private::StringImpl::From;
		template<typename T1> friend struct Private::StringImpl::To;

	}; // class StringBase







} // namespace Yuni



// Specific implementation for string handling
# include "traits.hxx"

// Standard conversions
# include "converters.from.hxx"
# include "converters.to.hxx"
// Implementation
# include "string.hxx"




//! \name Operator overload for stream printing
//@{

template<typename C, int Chunk>
inline std::ostream& operator << (std::ostream& out, const Yuni::StringBase<C,Chunk>& rhs)
{
	rhs.print(out);
	return out;
}

template<typename C, int Chunk>
inline bool operator == (const Yuni::StringBase<C,Chunk>& rhs, const C* u)
{
	return rhs == u;
}

template<typename C, int Chunk>
inline bool operator == (const C* u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return rhs == u;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C,Chunk>& rhs, const char* u)
{
	return Yuni::StringBase<>(rhs) += u;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C,Chunk>& rhs, const wchar_t* u)
{
	return Yuni::StringBase<>(rhs) += u;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C,Chunk>& rhs, const char u)
{
	return Yuni::StringBase<>(rhs) += u;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C,Chunk>& rhs, const wchar_t u)
{
	return Yuni::StringBase<>(rhs) += u;
}



template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const wchar_t* u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return Yuni::StringBase<>(u) += rhs;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const char* u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return Yuni::StringBase<>(u) += rhs;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const char u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return Yuni::StringBase<>(u) += rhs;
}

template<typename C, int Chunk>
inline Yuni::StringBase<> operator + (const wchar_t u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return Yuni::StringBase<>(u) += rhs;
}




template<typename C, int Chunk, typename U>
inline Yuni::StringBase<> operator + (const std::basic_string<U>& u, const Yuni::StringBase<C,Chunk>& rhs)
{
	return Yuni::StringBase<>(u) += rhs;
}


template<typename C, int Chunk, typename U>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C,Chunk>& rhs, const std::basic_string<U>& u)
{
	return Yuni::StringBase<>(rhs) += u;
}

template<typename C1, int Chunk1, typename C2, int Chunk2>
inline Yuni::StringBase<> operator + (const Yuni::StringBase<C1,Chunk1>& rhs, const Yuni::StringBase<C2,Chunk2>& u)
{
	return Yuni::StringBase<>(rhs) += u;
}

//@}


# include "../customstring.h"


#endif // __YUNI_CORE_STRING_STRING_H__
