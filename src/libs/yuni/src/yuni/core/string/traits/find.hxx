#ifndef __YUNI_CORE_STRING_STRING_TRAITS_FIND_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_FIND_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{



	template<class StrBase1>
	struct Find<StrBase1, char>
	{
		static typename StrBase1::Size Value(const StrBase1& str, const char t)
		{
			for (typename StrBase1::Size i = 0; i != str.pSize; ++i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size Value(const StrBase1& str, const char t, const typename StrBase1::Size offset)
		{
			for (typename StrBase1::Size i = offset; i < str.pSize; ++i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const char t)
		{
			for (typename StrBase1::Size i = str.pSize - 1; i != (typename StrBase1::Size) StrBase1::npos; --i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const char t, const typename StrBase1::Size offset)
		{
			for (typename StrBase1::Size i = ((offset != (typename StrBase1::Size) StrBase1::npos) ? offset : str.pSize - 1);
				i != (typename StrBase1::Size) StrBase1::npos; --i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}

	}; // specialization on `char`




	template<class StrBase1>
	struct Find<StrBase1, wchar_t>
	{
		static typename StrBase1::Size Value(const StrBase1& str, const wchar_t t)
		{
			for (typename StrBase1::Size i = 0; i != str.pSize; ++i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size Value(const StrBase1& str, const wchar_t t, const typename StrBase1::Size offset)
		{
			for (typename StrBase1::Size i = offset; i < str.pSize; ++i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const wchar_t t)
		{
			for (typename StrBase1::Size i = str.pSize - 1; i != (typename StrBase1::Size) StrBase1::npos; --i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const wchar_t t, const typename StrBase1::Size offset)
		{
			for (typename StrBase1::Size i = ((offset != (typename StrBase1::Size) StrBase1::npos) ? offset : str.pSize - 1);
				i != (typename StrBase1::Size) StrBase1::npos; --i)
			{
				if (t == str.pPtr[i])
					return i;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}

	};

	template<class StrBase1, typename W, int N>
	struct Find<StrBase1, W[N]>
	{
		static typename StrBase1::Size Value(const StrBase1& str, const W* t, typename StrBase1::Size offset = 0)
		{
			if (t && '\0' != *t && offset < str.pSize)
			{
				while (1)
				{
					// Trying to find the next occurenceof the first char
					offset = Find<StrBase1,W>::Value(str, *t, offset);
					if (StrBase1::npos == offset || offset + N - 1 > str.pSize)
						return (typename StrBase1::Size) StrBase1::npos;
					if (!memcmp(str.pPtr + offset, t, (N - 1) * sizeof(W)))
						return offset;
					++offset;
				}
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const W* t, typename StrBase1::Size offset = StrBase1::npos)
		{
			if (t && '\0' != *t && str.notEmpty())
			{
				if (offset == (typename StrBase1::Size) StrBase1::npos)
					offset = str.pSize - 1;
				while ((typename StrBase1::Size) StrBase1::npos != offset)
				{
					// Trying to find the next occurenceof the first char
					offset = str.find_last_of(*t, offset);
					if ((typename StrBase1::Size) StrBase1::npos == offset || offset + N - 1 > str.pSize)
						return (typename StrBase1::Size) StrBase1::npos;
					if (!memcmp(str.pPtr + offset, t, (N - 1) * sizeof(W)))
						return offset;
					--offset;
				}
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}

	};

	template<class StrBase1, typename W>
	struct Find<StrBase1, W*>
	{
		static typename StrBase1::Size RawValue(const StrBase1& str, const W* t,
			const typename StrBase1::Size length, typename StrBase1::Size offset)
		{
			while (1)
			{
				// Trying to find the next occurenceof the first char
				offset = str.find(*t, offset);
				if ((typename StrBase1::Size) StrBase1::npos == offset || offset + length > str.pSize)
					return (typename StrBase1::Size) StrBase1::npos;
				if (!memcmp(str.pPtr + offset, t, length * sizeof(W)))
					return offset;
				++offset;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}

		static typename StrBase1::Size Value(const StrBase1& str, const W* t, typename StrBase1::Size offset = 0)
		{
			return (t && '\0' != *t && offset < str.pSize)
				? RawValue(str, t, Length<StrBase1,W*>::Value(t), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseRawValue(const StrBase1& str, const W* t,
			const typename StrBase1::Size length, typename StrBase1::Size offset)
		{
			if (offset >= str.pSize)
				offset = str.pSize - 1;
			while (1)
			{
				// Trying to find the next occurenceof the first char
				offset = str.find(*t, offset);
				if ((typename StrBase1::Size) StrBase1::npos == offset || offset + length > str.pSize)
					return (typename StrBase1::Size) StrBase1::npos;
				if (!memcmp(str.pPtr + offset, t, length * sizeof(W)))
					return offset;
				++offset;
			}
			return (typename StrBase1::Size) StrBase1::npos;
		}

		static typename StrBase1::Size ReverseValue(const StrBase1& str, const W* t, typename StrBase1::Size offset = StrBase1::npos)
		{
			return (t && '\0' != *t && str.pSize)
				? ReverseRawValue(str, t, Length<StrBase1,W*>::Value(t), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}

	};

	template<class StrBase1, typename W>
	struct Find<StrBase1, std::basic_string<W> >
	{
		static typename StrBase1::Size Value(const StrBase1& str, const std::basic_string<W>& t, typename StrBase1::Size offset = 0)
		{
			return (!t.empty() && offset < str.pSize)
				? Find<StrBase1, W*>::RawValue(str, t.c_str(), t.size(), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const std::basic_string<W>& t, typename StrBase1::Size offset = StrBase1::npos)
		{
			return (!t.empty() && str.pSize)
				? Find<StrBase1, W*>::ReverseRawValue(str, t.c_str(), t.size(), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}

	};

	template<class StrBase1, typename W>
	struct Find<StrBase1, std::basic_string<W>* >
	{
		static typename StrBase1::Size Value(const StrBase1& str, const std::basic_string<W>* t, typename StrBase1::Size offset = 0)
		{
			return (t && !t->empty() && offset < str.pSize)
				? Find<StrBase1, W*>::RawValue(str, t->c_str(), t->size(), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const std::basic_string<W>* t, typename StrBase1::Size offset = StrBase1::npos)
		{
			return (t && !t->empty() && str.pSize)
				? Find<StrBase1, W*>::ReverseRawValue(str, t->c_str(), t->size(), offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}

	};


	template<class StrBase1, typename W, int N>
	struct Find<StrBase1, Yuni::StringBase<W,N> >
	{
		static typename StrBase1::Size Value(const StrBase1& str, const Yuni::StringBase<W,N>& t, typename StrBase1::Size offset = 0)
		{
			return (t.pSize && offset < str.pSize)
				? Find<StrBase1, W*>::RawValue(str, t.pPtr, t.pSize, offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const Yuni::StringBase<W,N>& t, typename StrBase1::Size offset = StrBase1::npos)
		{
			return (t.pSize && str.pSize)
				? Find<StrBase1, W*>::ReverseRawValue(str, t.pPtr, t.pSize, offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
	};

	template<class StrBase1, typename W, int N>
	struct Find<StrBase1, Yuni::StringBase<W,N>* >
	{
		static typename StrBase1::Size Value(const StrBase1& str, const Yuni::StringBase<W,N>* t, typename StrBase1::Size offset = 0)
		{
			return (t && t->pSize && offset < str.pSize)
				? Find<StrBase1, W*>::RawValue(str, t->pPtr, t->pSize, offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
		static typename StrBase1::Size ReverseValue(const StrBase1& str, const Yuni::StringBase<W,N>* t, typename StrBase1::Size offset = StrBase1::npos)
		{
			return (t && t->pSize && str.pSize)
				? Find<StrBase1, W*>::ReverseRawValue(str, t->pPtr, t->pSize, offset)
				: (typename StrBase1::Size) StrBase1::npos;
		}
	};






} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_FIND_HXX__
