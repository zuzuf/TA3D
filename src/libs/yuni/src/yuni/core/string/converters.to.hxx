#ifndef __YUNI_CORE_STRING_STD_CONVERTERS_TO_HXX__
# define __YUNI_CORE_STRING_STD_CONVERTERS_TO_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{


	template<typename C>
	struct AutoDetectBaseNumber
	{
		static const C* Value(const C* s, int& base)
		{
			switch (*s)
			{
				case '#' :
					{
						base = 16;
						return s + 1;
					}
				case '0' :
					{
						if (s[1] == 'x' || s[1] == 'X')
						{
							base = 16;
							return s + 2;
						}
					}
			}
			base = 10;
			return s;
		}
	};



	template<>
	struct To<char>
	{
		template<typename C, int Chnk>
		static inline char Value(const StringBase<C,Chnk>& s)  {return s.pSize ? *(s.pPtr) : '\0';}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, C& v)
		{
			if (s.pSize)
			{
				v = (*s.pPtr);
				return true;
			}
			return false;
		}
	};

	template<>
	struct To<char*>
	{
		template<typename C, int Chnk>
		static inline const char* Value(const StringBase<C,Chnk>& s)
		{
			return StringBase<C,Chnk>(s).c_str();
		}

		template<typename C, int Chnk>
		static inline const char* Value(StringBase<C,Chnk>& s)
		{
			return s.c_str();
		}
	};

	template<int N>
	struct To<char[N]>
	{
		template<typename C, int Chnk>
		static inline const char* Value(const StringBase<C,Chnk>& s)
		{
			return StringBase<C,Chnk>(s).c_str();
		}

		template<typename C, int Chnk>
		static inline const char* Value(StringBase<C,Chnk>& s)
		{
			return s.c_str();
		}

	};


	template<>
	struct To<bool>
	{
		template<typename C, int Chnk>
		static bool Value(const StringBase<C,Chnk>& s)
		{
			switch (s.pSize)
			{
				case 0:
					return false;
				case 1:
					return ('1' == *(s.pPtr) || 'Y' == *(s.pPtr) || 'y' == *(s.pPtr)
							|| 'O' == *(s.pPtr) || 'o' == *(s.pPtr) || 't' == *(s.pPtr) || 'T' == *(s.pPtr));
				default:
					{
						if (s.pSize > 4)
							return false;
						char buffer[5] = {0,0,0,0,0};
						# ifdef YUNI_OS_MSVC
						strncpy_s(buffer, 5, s.pPtr, Private::StringImpl::Min((typename StringBase<C,Chnk>::Size)4, s.pSize));
						# else
						strncpy(buffer, s.pPtr, Private::StringImpl::Min((typename StringBase<C,Chnk>::Size)4, s.pSize));
						# endif
						buffer[0] = (C) tolower(buffer[0]);
						buffer[1] = (C) tolower(buffer[1]);
						buffer[2] = (C) tolower(buffer[2]);
						buffer[3] = (C) tolower(buffer[3]);
						return (!strcmp("true", buffer) || !strcmp("on", buffer) || !strcmp("yes", buffer));
					}
			}
			return false;
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, bool& v)
		{
			v = Value(s);
			return true;
		}

	}; // bool


	template<typename U, int N>
	struct To<StringBase<U,N> >
	{
		template<typename C, int Chnk>
		static inline const StringBase<U,N>& Value(const StringBase<C,Chnk>& s)
		{
			return s;
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, StringBase<U,N>& v)
		{
			v = s;
			return true;
		}

	}; // std::string


	template<typename U>
	struct To<std::basic_string<U> >
	{
		template<typename C, int Chnk>
		static inline std::basic_string<U> Value(const StringBase<C,Chnk>& s)
		{
			return s.pSize
				? std::basic_string<U>(s.pPtr, s.pSize)
				: std::basic_string<U>();
		}

		template<typename C, int Chnk>
		static bool Value(const StringBase<C,Chnk>& s, std::basic_string<U>& v)
		{
			v.clear();
			if (s.pSize)
				v.append(s.pPtr, s.pSize);
			return true;
		}

	}; // std::string


	template<>
	struct To<sint16>
	{
		template<typename C, int Chnk>
		static inline sint16 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (sint16)strtol(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, sint16& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (sint16)strtol(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // sint16


	template<>
	struct To<sint32>
	{
		template<typename C, int Chnk>
		static inline sint32 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (sint32)strtol(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, sint32& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (sint32)strtol(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // sint32

	template<>
	struct To<sint64>
	{
		template<typename C, int Chnk>
		static inline sint64 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (sint64)strtol(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, sint64& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (sint64)strtol(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // sint64


	template<>
	struct To<uint16>
	{
		template<typename C, int Chnk>
		static inline uint16 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (uint16)strtoul(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, uint16& v)
		{
			int base;
			C* pend;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (uint16)strtoul(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // uint16


	template<>
	struct To<uint32>
	{
		template<typename C, int Chnk>
		static inline uint32 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (uint32)strtoul(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, uint32& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (uint32)strtoul(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // uint32

	template<>
	struct To<uint64>
	{
		template<typename C, int Chnk>
		static inline uint64 Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (uint64)strtoul(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, uint64& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (uint64)strtoul(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // uint64

	template<>
	struct To<float>
	{
		template<typename C, int Chnk>
		static inline float Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			# ifdef YUNI_OS_MSVC
			// Visual Studio does not support strtof
			return (float)strtod(s.pPtr, &pend);
			# else
			return (float)strtof(s.pPtr, &pend);
			# endif
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, float& v)
		{
			C* pend;
			v = (float)strtof(s.pPtr, &pend);
			return (NULL != pend && '\0' == *pend);
		}

	}; // float



	template<>
	struct To<double>
	{
		template<typename C, int Chnk>
		static inline double Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			return (float)strtod(s.pPtr, &pend);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, double& v)
		{
			C* pend;
			v = (double)strtod(s.pPtr, &pend);
			return (NULL != pend && '\0' == *pend);
		}

	}; // float



# ifdef YUNI_HAS_LONG
	template<>
	struct To<unsigned long>
	{
		template<typename C, int Chnk>
		static inline unsigned long Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (unsigned long)strtoul(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, unsigned long& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (unsigned long)strtoul(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // uint32

	template<>
	struct To<long>
	{
		template<typename C, int Chnk>
		static inline long Value(const StringBase<C,Chnk>& s)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			return (long)strtoul(p, &pend, base);
		}

		template<typename C, int Chnk>
		static inline bool Value(const StringBase<C,Chnk>& s, long& v)
		{
			C* pend;
			int base;
			const C* p = AutoDetectBaseNumber<C>::Value(s.pPtr, base);
			v = (long)strtoul(p, &pend, base);
			return (NULL != pend && '\0' == *pend);
		}

	}; // uint32

# endif


} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STD_CONVERTERS_TO_HXX__

