#ifndef __YUNI_CORE_IO_IO_HXX__
# define __YUNI_CORE_IO_IO_HXX__

# include <cassert>
# include <ctype.h>

namespace Yuni
{
namespace Private
{
namespace Core
{
namespace IO
{

	Yuni::Core::IO::NodeType TypeOf(const char* p, unsigned int length);
	Yuni::Core::IO::NodeType TypeOfNotZeroTerminated(const char* p, unsigned int length);

} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni




namespace Yuni
{
namespace Core
{
namespace IO
{
namespace Directory
{

	// Forward declaration
	template<typename C> struct Constant;

} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Yuni



namespace Yuni
{
namespace Core
{
namespace IO
{


	template<class StringT>
	inline bool IsAbsolute(const StringT& filename)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, CustomString_InvalidTypeForBuffer);
		YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  CustomString_InvalidTypeForBufferSize);

		const char* const p = Traits::CString<StringT>::Perform(filename);
		if (!p)
			return false;
		// Find the substring
		if (Traits::Length<StringT, unsigned int>::isFixed)
		{
			// We can make some optimisations when the length is known at compile compile time
			// This part of the code should not bring better performances but it should
			// prevent against bad uses of the API, like using a typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::Char* for looking for a single char.

			// The value to find is actually empty, npos will be the unique answer
			if (0 == Traits::Length<StringT, unsigned int>::fixedLength)
				return false;
			// The string is actually a single POD item
			if (1 == Traits::Length<StringT, unsigned int>::fixedLength)
				return *p == '/' || *p == '\\';
		}

		const unsigned int len = Traits::Length<StringT, unsigned int>::Value(filename);
		return (len != 0 && p)           // The string must not be empty
			&& (*p == '/' || *p == '\\'  // Unix-style
				|| (isalpha(*p)          // Windows-style (Drive letter)
					&& len >= 2 && p[1] == ':' && (len == 2 || (p[2] == '\\' || p[2] == '/'))));
	}


	template<class StringT>
	inline bool IsRelative(const StringT& filename)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, CustomString_InvalidTypeForBuffer);
		YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  CustomString_InvalidTypeForBufferSize);
		return !IsAbsolute(filename);
	}


	template<class StringT> inline bool Exists(const StringT& filename)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, IOExists_InvalidTypeForBuffer);
		YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  IOExists_InvalidTypeForBufferSize);
		return (Yuni::Core::IO::typeUnknown != Yuni::Core::IO::TypeOf(filename));
	}


	template<class StringT1, class StringT2>
	void ExtractFilePath(StringT1& out, const StringT2& p, const bool systemDependant)
	{
		if (p.empty())
			out.clear();
		const typename StringT2::size_type pos = (systemDependant)
			? p.find_last_of(IO::Constant<char>::Separator)
			: p.find_last_of(IO::Constant<char>::AllSeparators);
		if (StringT2::npos == pos)
			out.clear();
		else
			out.assign(p, pos);
	}



	template<class StringT1, class StringT2>
	void ExtractFileName(StringT1& out, const StringT2& p, const bool systemDependant)
	{
		if (p.notEmpty())
			out.clear();
		const typename StringT2::size_type pos = (systemDependant)
			? p.find_last_of(IO::Constant<char>::Separator)
			: p.find_last_of(IO::Constant<char>::AllSeparators);
		if (StringT2::npos == pos)
			out.clear();
		else
			out.assign(p.c_str() +  pos + 1);
	}



	template<class StringT1, class StringT2>
	void ExtractFileNameWithoutExtension(StringT1& out, const StringT2& p, const bool systemDependant)
	{
		const typename StringT2::size_type pos = (systemDependant)
			? p.find_last_of(IO::Constant<char>::Separator)
			: p.find_last_of(IO::Constant<char>::AllSeparators);
		const typename StringT2::size_type n = p.find_last_of('.');

		if (StringT2::npos == n && StringT2::npos == pos)
		{
			out = p;
			return;
		}
		if (n == pos)
		{
			out.clear();
			return;
		}
		if (n == StringT2::npos && n > pos + 1)
		{
			if (StringT2::npos == pos)
			{
				out = p;
				return;
			}
			out.assign(p.c_str() + pos + 1);
			return;
		}
		if (pos == StringT2::npos)
		{
			out.assign(p, n);
			return;
		}
		out.assign(p.c_str() + pos + 1, n - pos - 1);
	}


	template<class StringT1, class StringT2>
	bool ExtractExtension(StringT1& out, const StringT2& filename, bool dot, bool clear)
	{
		if (clear)
			out.clear();
		// If the string is empty, the buffer may be invalid (NULL)
		if (filename.size())
		{
			unsigned int i = filename.size();
			do
			{
				--i;
				switch (filename[i])
				{
					case '.':
						{
							if (!dot)
							{
								if (++i >= (unsigned int) filename.size())
									return true;
							}
							out.append(filename.c_str() + i, filename.size() - i);
							return true;
						}
					case '/':
					case '\\':
						return false;
				}
			}
			while (i != 0);
		}
		return false;
	}


	template<class StringT1, class StringT2>
	void MakeAbsolute(StringT1& out, const StringT2& filename, bool clearBefore)
	{
		if (clearBefore)
			out.clear();
		if (IsAbsolute(filename))
		{
			out += filename;
		}
		else
		{
			Core::IO::Directory::Current::Get(out, clearBefore);
			out << Core::IO::Separator << filename;
		}
	}


	template<class StringT1, class StringT2, class StringT3>
	void MakeAbsolute(StringT1& out, const StringT2& filename, const StringT3& currentPath, bool clearBefore)
	{
		if (clearBefore)
			out.clear();
		if (IsAbsolute(filename))
        {
			out += filename;
        }
		else
		{
			out += currentPath;
			out += Core::IO::Separator;
			out += filename;
		}
	}


	template<class StringT1, class StringT2>
	bool ReplaceExtension(StringT1& filename, const StringT2& newExtension)
	{
		// If the string is empty, the buffer may be invalid (NULL)
		if (filename.size())
		{
			unsigned int i = filename.size();
			do
			{
				--i;
				switch (filename[i])
				{
					case '.':
						{
							filename.resize(i);
							filename += newExtension;
							return true;
						}
					case '/':
					case '\\':
						return false;
				}
			}
			while (i != 0);
		}
		return false;
	}


	template<class StringT1, class StringT2>
	void Normalize(StringT1& out, const StringT2& in, unsigned int inLength, bool replaceSlashes)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT2>::valid, Normalize_InvalidTypeForInput);
		YUNI_STATIC_ASSERT(Traits::Length<StringT2>::valid,  Normalize_InvalidTypeForInputSize);

		// Some static checks
		if (Traits::Length<StringT2,unsigned int>::isFixed)
		{
			// The value to find is actually empty, nothing to do
			if (0 == Traits::Length<StringT2,unsigned int>::fixedLength)
			{
				out.clear();
				return;
			}
			// The string is actually a single POD item
			if (1 == Traits::Length<StringT2,unsigned int>::fixedLength)
			{
				out = in;
				if (replaceSlashes)
				{
					# ifdef YUNI_OS_WINDOWS
					out.replace('/', '\\');
					# else
					out.replace('\\', '/');
					# endif
				}
				return;
			}
		}

		// The length of the input
		if (inLength == static_cast<unsigned int>(-1))
			inLength = Traits::Length<StringT2,unsigned int>::Value(in);
		if (!inLength)
		{
			out.clear();
			return;
		}
		if (inLength == 1)
		{
			out = in;
			if (replaceSlashes)
			{
				# ifdef YUNI_OS_WINDOWS
				out.replace('/', '\\');
				# else
				out.replace('\\', '/');
				# endif
			}
			return;
		}
		// From here, we have at least 2 chars

		// From now on, we will work on a mere CString
		const char* input = Traits::CString<StringT2>::Perform(in);
		if (!input)
		{
			out.clear();
			return;
		}

		// Counting slashes
		unsigned int slashes = 0;
		// An index, used at different places
		unsigned int i = 0;
		// We will keep the position of the character after the first slash. It improves
		// a bit performances for relative filenames
		unsigned int start = 0;
		for (; i != inLength; ++i)
		{
			if (input[i] == '/' || input[i] == '\\')
			{
				slashes = 1;
				start = ++i;
				break;
			}
		}
		if (!slashes)
		{
			// Nothing to normalize
			out = in;
			if (replaceSlashes)
			{
				# ifdef YUNI_OS_WINDOWS
				out.replace('/', '\\');
				# else
				out.replace('\\', '/');
				# endif
			}
			return;
		}
		for (; i < inLength; ++i)
		{
			if (input[i] == '/' || input[i] == '\\')
				++slashes;
		}

		// Initializing the output, and reserving the memory to avoid as much as possible calls to realloc
		// In the most cases, the same size than the input is the most appropriate value
		out.reserve(inLength);
		// Copying the begining of the input
		out.assign(input, start);

		// Detecting absolute paths.
		// We only know that we have at least 2 chars, and we can not assume that the input
		// is zero-terminated.
		// For performance reasons (to reduce the calls to malloc/free when pushing an element),
		// we will skip the begining if the path is absolute.
		bool isAbsolute = false;
		// Performing checks only if the first slash is near by the begining.
		if (start < 4)
		{
			if (input[1] == ':' && inLength >= 2 && (input[2] == '\\' || input[2] == '/'))
			{
				// We have an Windows-style path, and it is absolute
				isAbsolute = true;
			}
			else
			{
				// We have an Unix-style path
				if (input[0] == '/' || input[0] == '\\')
					isAbsolute = true;
			}
		}

		// The last known good position
		unsigned int cursor = start;
		// The number of non-relative folders, used when the path is not absolute
		// This value is used to keep the relative segments at the begining
		unsigned int realFolderCount = 0;

		// The stack
		// PreAllocating the stack, to speed up the algoritm by avoiding numerous
		// calls to malloc/free
		struct Stack
		{
		public:
			void operator () (unsigned int c, unsigned int l)
			{
				cursor = c;
				length = l;
			}
		public:
			unsigned int cursor;
			unsigned int length;
		};
		Stack* stack = new Stack[slashes + 1]; // Ex: path/to/somewhere/on/my/hdd
		// Index on the stack
		unsigned int count = 0;

		for (i = start; i < inLength; ++i)
		{
			// Detecting the end of a segment
			if (input[i] == '/' || input[i] == '\\')
			{
				switch (i - cursor)
				{
					case 0:
						// A single slash. Nothing to do
						break;
					case 1:
						{
							// not the current folder `./`
							if (input[cursor] != '.')
							{
								stack[count++](cursor, 2);
								++realFolderCount;
							}
							break;
						}
					case 2:
						{
							// double dot segments
							if (input[cursor] == '.' && input[cursor + 1] == '.')
							{
								if (isAbsolute)
								{
									if (count)
										--count;
								}
								else
								{
									if (realFolderCount)
									{
										--count;
										--realFolderCount;
									}
									else
										stack[count++](cursor, 3);
								}
								break;
							}
							else
							{
								// we have a real folder, so `break` _must_ not
								// be used here.
							}
						}
					default:
						{
							// We have encountered a standard segment
							stack[count++](cursor, i - cursor + 1);
							++realFolderCount;
						}
				}
				// Positioning the cursor to the next character
				cursor = i + 1;
			}
		}

		// Special case : The last segment is a double dot segment
		if (cursor < inLength && inLength - cursor == 2)
		{
			if (input[cursor] == '.' && input[cursor + 1] == '.')
			{
				if (isAbsolute)
				{
					if (count)
						--count;
				}
				else
				{
					if (realFolderCount)
						--count;
					else
						stack[count++](cursor, 2);
				}

				cursor = inLength;
			}
		}

		// Pushing all stored segments
		if (count)
		{
			for (unsigned int j = 0; j != count; ++j)
				out.append(input + stack[j].cursor, stack[j].length);
		}

		// Releasing the memory
		delete[] stack;

		// But it may remain a final segment
		// We know for sure that it can not be a double dot segment
		if (cursor < inLength)
		{
			if (!(inLength - cursor == 1 && input[cursor] == '.'))
				out.append(input + cursor, inLength - cursor);
		}
		// Removing the trailing slash
		if (out.size() > 3)
			out.removeTrailingSlash();

		if (replaceSlashes)
		{
			# ifdef YUNI_OS_WINDOWS
			out.replace('/', '\\');
			# else
			out.replace('\\', '/');
			# endif
		}
	}



	template<class StringT>
	inline NodeType TypeOf(const StringT& filename)
	{
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, TypeOF_InvalidTypeForBuffer);
		YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  TypeOF_InvalidTypeForBufferSize);

		if (Traits::CString<StringT>::zeroTerminated) // static check
		{
			return Yuni::Private::Core::IO::TypeOf(
				Traits::CString<StringT>::Perform(filename),
				Traits::Length<StringT, unsigned int>::Value(filename));
		}
		else
		{
			return Yuni::Private::Core::IO::TypeOf(
				Traits::CString<StringT>::Perform(filename),
				Traits::Length<StringT, unsigned int>::Value(filename));
		}
	}




} // namespace IO
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_IO_IO_HXX__
