#ifndef __YUNI_CORE_STRING_STRING_TRAITS_FIND_LAST_OF_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_FIND_LAST_OF_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{


	template<typename C, int Chnk, bool Equals>
	struct FindLastOf<StringBase<C,Chnk>, C, Equals>
	{
		static typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C toFind)
		{
			if (s.pSize)
			{
				typename StringBase<C,Chnk>::Size i = s.pSize;
				--i;
				do
				{
					if (Equals == (toFind == s.pPtr[i]))
						return i;
				}
				while (--i);
			}
			return (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

		static typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C toFind, const typename StringBase<C,Chnk>::Size offset)
		{
			if (offset < s.pSize)
			{
				typename StringBase<C,Chnk>::Size i = offset;
				do
				{
					if (Equals == (toFind == s.pPtr[i]))
						return i;
				}
				while (--i);
			}
			return (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

	};

	template<typename C, int Chnk, bool Equals>
	struct FindLastOf<StringBase<C,Chnk>, C*, Equals>
	{
		static typename StringBase<C,Chnk>::Size RawValue(const StringBase<C,Chnk>& s, const C* toFind,
			typename StringBase<C,Chnk>::Size offset, /* offset in `s` */
			typename StringBase<C,Chnk>::Size len     /* Length of `toFind` */)
		{
			if (len && toFind && offset < s.pSize)
			{
				typename StringBase<C,Chnk>::Size j;
				typename StringBase<C,Chnk>::Size i = offset;
				do
				{
					for (j = 0; j != len; ++j)
					{
						if (Equals == (toFind[j] == s.pPtr[i]))
							return i;
					}
				}
				while (--i);
			}
			return (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}


		static inline typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C* toFind)
		{
			return s.pSize
				? RawValue(s, toFind, s.pSize - 1, StringBase<C,Chnk>::Length(toFind))
				: (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

		static typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C toFind, const typename StringBase<C,Chnk>::Size offset)
		{
			return RawValue(s, toFind, offset, StringBase<C,Chnk>::Length(toFind));
		}

	};

	template<typename C, int Chnk, bool Equals, int N>
	struct FindLastOf<StringBase<C,Chnk>, C[N], Equals>
	{
		static typename StringBase<C,Chnk>::Size RawNValue(const StringBase<C,Chnk>& s, const C* toFind,
			typename StringBase<C,Chnk>::Size offset /* offset in `s` */)
		{
			if ((N - 1) && toFind && offset < s.pSize)
			{
				typename StringBase<C,Chnk>::Size j;
				typename StringBase<C,Chnk>::Size i = offset;
				do
				{
					for (j = 0; j != (N - 1); ++j)
					{
						if (Equals == (toFind[j] == s.pPtr[i]))
							return i;
					}
				}
				while (--i);
			}
			return (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}


		static inline typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C* toFind)
		{
			return s.pSize ? RawNValue(s, toFind, s.pSize - 1) : (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

		static inline typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C* toFind, const typename StringBase<C,Chnk>::Size offset)
		{
			return RawNValue(s, toFind, offset);
		}
	};


	template<typename C, int Chnk, bool Equals, int Cnk1>
	struct FindLastOf<StringBase<C,Chnk>, StringBase<C, Cnk1>, Equals>
	{
		static inline typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const StringBase<C,Cnk1>& toFind)
		{
			return s.pSize
				? FindLastOf<StringBase<C,Chnk>, C*, Equals>::RawValue(s, toFind.pPtr, s.pSize - 1, toFind.pSize)
				: (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

		static typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C toFind, const typename StringBase<C,Chnk>::Size offset)
		{
			return FindLastOf<StringBase<C,Chnk>, C*, Equals>::RawValue(s, toFind.pPtr, offset, toFind.pSize);
		}
	};

	template<typename C, int Chnk, bool Equals>
	struct FindLastOf<StringBase<C,Chnk>, std::basic_string<C>, Equals>
	{
		static inline typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const std::basic_string<C>& toFind)
		{
			return s.pSize
				? FindLastOf<StringBase<C,Chnk>, C*, Equals>::RawValue(s, toFind.c_str(), s.pSize - 1, toFind.size())
				: (typename StringBase<C,Chnk>::Size) StringBase<C,Chnk>::npos;
		}

		static typename StringBase<C,Chnk>::Size Value(const StringBase<C,Chnk>& s, const C toFind, const typename StringBase<C,Chnk>::Size offset)
		{
			return FindLastOf<StringBase<C,Chnk>, C*, Equals>::RawValue(s, toFind.c_str(), offset, toFind.size());
		}
	};




} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_FIND_LAST_OF_HXX__
