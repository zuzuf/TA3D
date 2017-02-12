#ifndef __YUNI_CORE_STRING_STRING_TRAITS_REMOVE_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_REMOVE_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{


	template<class StrBase, typename U>
	struct Remove
	{
		static typename StrBase::Size
		Run(StrBase& str, const U& u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			assert(&str != &u && "Undefined behavior");

			// The length of the substring to find
			const typename StrBase::Size len(StrBase::Length(u));

			if (len)
			{
				// The number of occurences
				typename StrBase::Size n(0);

				// Loop
				while (offset < str.pSize && n != maxOccurences)
				{
					// Looking for the next substring
					offset = str.find(u, offset);

					// Nothing, go away
					if (offset == StrBase::npos)
						return n;

					// There is a substring to remove
					str.erase(offset, len);
					// An occurence has been found
					++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase, typename W, int N>
	struct Remove<StrBase, W[N]>
	{
		static typename StrBase::Size
		Run(StrBase& str, const W* u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			assert(str.pPtr != u && "Undefined behavior");
			// The number of occurences
			typename StrBase::Size n(0);

			// Loop
			while (offset < str.pSize && n != maxOccurences)
			{
				// Looking for the next substring
				offset = Find<StrBase, W*>::RawValue(str, u, N - 1, offset);

				// Nothing, go away
				if (offset == StrBase::npos)
					return n;

				// There is a substring to remove
				str.erase(offset, N - 1);
				// An occurence has been found
				++n;
			}
			return n;
		}
	};


	template<class StrBase, typename W>
	struct Remove<StrBase, std::basic_string<W> >
	{
		static typename StrBase::Size
		Run(StrBase& str, const std::basic_string<W>& u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			if (!u.empty())
			{
				// The number of occurences
				typename StrBase::Size n(0);

				// Loop
				while (offset < str.pSize && n != maxOccurences)
				{
					// Looking for the next substring
					offset = Find<StrBase, W*>::RawValue(str, u.c_str(), u.size(), offset);

					// Nothing, go away
					if (offset == StrBase::npos)
						return n;

					// There is a substring to remove
					str.erase(offset, u.size());
					// An occurence has been found
					++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase, typename W, int Chnk1>
	struct Remove<StrBase, StringBase<W,Chnk1> >
	{
		static typename StrBase::Size
		Run(StrBase& str, const StringBase<W,Chnk1>& u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			assert(&str != &u && "Undefined behavior");
			if (u.pSize)
			{
				// The number of occurences
				typename StrBase::Size n(0);

				// Loop
				while (offset < str.pSize && n != maxOccurences)
				{
					// Looking for the next substring
					offset = Find<StrBase, W*>::RawValue(str, u.pPtr, u.pSize, offset);

					// Nothing, go away
					if (offset == StrBase::npos)
						return n;

					// There is a substring to remove
					str.erase(offset, u.pSize);
					// An occurence has been found
					++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase, typename W>
	struct Remove<StrBase, std::basic_string<W>* >
	{
		static typename StrBase::Size
		Run(StrBase& str, const std::basic_string<W>* u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			if (u && !u->empty())
			{
				// The number of occurences
				typename StrBase::Size n(0);

				// Loop
				while (offset < str.pSize && n != maxOccurences)
				{
					// Looking for the next substring
					offset = Find<StrBase, W*>::RawValue(str, u->c_str(), u->size(), offset);

					// Nothing, go away
					if (offset == StrBase::npos)
						return n;

					// There is a substring to remove
					str.erase(offset, u->size());
					// An occurence has been found
					++n;
				}
				return n;
			}
			return 0;
		}
	};



	template<class StrBase, typename W, int Chnk1>
	struct Remove<StrBase, StringBase<W,Chnk1>* >
	{
		static typename StrBase::Size
		Run(StrBase& str, const StringBase<W,Chnk1>* u, typename StrBase::Size offset,
			const typename StrBase::Size maxOccurences)
		{
			assert(&str != u && "Undefined behavior");
			if (u && u->pSize)
			{
				// The number of occurences
				typename StrBase::Size n(0);

				// Loop
				while (offset < str.pSize && n != maxOccurences)
				{
					// Looking for the next substring
					offset = Find<StrBase, W*>::RawValue(str, u->pPtr, u->pSize, offset);

					// Nothing, go away
					if (offset == StrBase::npos)
						return n;

					// There is a substring to remove
					str.erase(offset, u->pSize);
					// An occurence has been found
					++n;
				}
				return n;
			}
			return 0;
		}
	};



} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_REMOVE_HXX__
