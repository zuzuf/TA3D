#ifndef __YUNI_CORE_GETOPT_OPTION_H__
# define __YUNI_CORE_GETOPT_OPTION_H__

# include "../string.h"
# include "../customstring.h"


/*!
** \brief The maximum length for a long name of an option
*/
# define YUNI_GETOPT_LONGNAME_MAX_LENGTH  42




namespace Yuni
{
namespace Private
{
namespace GetOptImpl
{

	// Forward declaration
	class Tokenizer;


	template<class T>
	class Value
	{
	public:
		static bool Add(T& out, const char* c_str, const String::size_type len)
		{
			out = (len) ? String(c_str, len).to<T>() : T();
			return true;
		}

	};


	template<class C, int Chnk>
	class Value<StringBase<C, Chnk> >
	{
	public:
		static bool Add(StringBase<C,Chnk>& out, const char* c_str, const String::size_type len)
		{
			if (len)
				out.assign(c_str, len);
			else
				out.clear();
			return true;
		}
	};

	template<unsigned int ChunkT, bool ExpT, bool ZeroT>
	class Value<CustomString<ChunkT, ExpT, ZeroT> >
	{
	public:
		static bool Add(CustomString<ChunkT,ExpT, ZeroT>& out, const char* c_str, const String::size_type len)
		{
			out.assign(c_str, len);
			return true;
		}
	};



	template<class C, class Traits, class Alloc>
	class Value<std::basic_string<C, Traits, Alloc> >
	{
	public:
		static bool Add(std::basic_string<C,Traits,Alloc>& out, const char* c_str, const String::size_type len)
		{
			if (len)
				out += c_str;
			else
				out.clear();
			return true;
		}
	};

	template<template<class,class> class L, class T, class Alloc>
	class Value<L<T, Alloc> >
	{
	public:
		static bool Add(L<T,Alloc>& out, const char* c_str, const String::size_type len)
		{
			if (len)
				out.push_back(String(c_str, len).to<T>());
			else
				out.push_back(T());
			return true;
		}
	};


	template<template<class, class> class L, class C, int Chnk, class Alloc>
	class Value<L<StringBase<C,Chnk>, Alloc> >
	{
	public:
		static bool Add(L<StringBase<C, Chnk>, Alloc>& out, const char* c_str, const String::size_type len)
		{
            StringBase<C,Chnk> s;
            s.assign(c_str, len);
			out.push_back(s);
			return true;
		}
	};


	template<template<class, class> class L, unsigned int ChunkT, bool ExpT, bool ZeroT, class Alloc>
	class Value<L<CustomString<ChunkT,ExpT, ZeroT>, Alloc> >
	{
	public:
		static bool Add(L<CustomString<ChunkT,ExpT,ZeroT>, Alloc>& out, const char* c_str, const String::size_type len)
		{
			out.push_back(CustomString<ChunkT,ExpT,ZeroT>(c_str, len));
			return true;
		}
	};



	template<template<class, class> class L, class C, class Traits, class AllocS, class Alloc>
	class Value<L<std::basic_string<C, Traits, AllocS>, Alloc> >
	{
	public:
		static bool Add(L<std::basic_string<C, Traits, AllocS>, Alloc>& out, const char* c_str, const String::size_type len)
		{
			if (len)
				out.push_back(std::string(c_str, len));
			else
				out.push_back(std::string());
			return true;
		}
	};


	template<class T>
	class Flag
	{
	public:
		static void Enabled(T& out)
		{
			out = T(1);
		}
	};


	template<> class Flag<bool>
	{
	public:
		static void Enable(bool& out)
		{
			out = true;
		}
	};

	template<> class Flag<sint16>
	{
	public:
		static void Enable(sint16& out)
		{
			out = 1;
		}
	};

	template<> class Flag<sint32>
	{
	public:
		static void Enable(sint32& out)
		{
			out = 1;
		}
	};

	template<> class Flag<sint64>
	{
	public:
		static void Enable(sint64& out)
		{
			out = 1;
		}
	};

	template<> class Flag<uint16>
	{
	public:
		static void Enable(uint16& out)
		{
			out = 1u;
		}
	};

	template<> class Flag<uint32>
	{
	public:
		static void Enable(uint32& out)
		{
			out = 1u;
		}
	};

	template<> class Flag<uint64>
	{
	public:
		static void Enable(uint64& out)
		{
			out = 1u;
		}
	};





	template<class C, int Chnk>
	class Flag<StringBase<C, Chnk> >
	{
	public:
		static void Enable(StringBase<C,Chnk>& out)
		{
			out = "true";
		}
	};


	template<unsigned int ChunkT, bool ExpT, bool ZeroT>
	class Flag<CustomString<ChunkT,ExpT,ZeroT> >
	{
	public:
		static void Enable(CustomString<ChunkT,ExpT,ZeroT>& out)
		{
			out = "true";
		}
	};


	template<class C, class Traits, class Alloc>
	class Flag<std::basic_string<C, Traits, Alloc> >
	{
	public:
		static void Enable(std::basic_string<C,Traits,Alloc>& out)
		{
			out = "true";
		}
	};

	template<template<class,class> class L, class T, class Alloc>
	class Flag<L<T, Alloc> >
	{
	public:
		static void Enable(L<T,Alloc>& out)
		{
			out.push_back(T(1));
		}
	};


	template<template<class, class> class L, class C, int Chnk, class Alloc>
	class Flag<L<StringBase<C,Chnk>, Alloc> >
	{
	public:
		static void Enable(L<StringBase<C, Chnk>, Alloc>& out)
		{
			out.push_back("true");
		}
	};

	template<template<class, class> class L, unsigned int ChunkT, bool ExpT, bool ZeroT, class Alloc>
	class Flag<L<CustomString<ChunkT,ExpT,ZeroT>, Alloc> >
	{
	public:
		static void Enable(L<CustomString<ChunkT,ExpT,ZeroT>, Alloc>& out)
		{
			out.push_back("true");
		}
	};



	template<template<class, class> class L, class C, class Traits, class AllocS, class Alloc>
	class Flag<L<std::basic_string<C, Traits, AllocS>, Alloc> >
	{
	public:
		static void Enable(L<std::basic_string<C, Traits, AllocS>, Alloc>& out)
		{
			out.push_back("true");
		}
	};


	/*!
	** \brief Display the help for an option in particulare, with text formatting
	**
	** \param[in,out] out The stream where to write data
	** \param shortName The short name of the option
	** \param longName The long name of the option
	** \param description The description of the option
	*/
	void DisplayHelpForOption(std::ostream& out, const char shortName, const String& longName,
		const String& description, bool requireParameter = false);

	/*!
	** \brief Display a text paragraph
	**
	** \param[in,out] out The stream where to write data
	** \param text The text
	*/
	void DisplayTextParagraph(std::ostream& out, const String& text);




	class IOption
	{
	public:
		IOption()
			:pShortName('\0')
		{}
		IOption(const IOption& rhs)
			:pShortName(rhs.pShortName), pLongName(rhs.pLongName),
			pDescription(rhs.pDescription)
		{}

		explicit IOption(const char s)
			:pShortName(s)
		{}

		template<class StringT>
		IOption(const char s, const StringT& name)
			:pShortName(s), pLongName(name)
		{
			assert("A long name of an option must not exceed `YUNI_GETOPT_LONGNAME_MAX_LENGTH` characters"
				&& pLongName.size() <= YUNI_GETOPT_LONGNAME_MAX_LENGTH);
		}

		template<class StringT1, class StringT2>
		IOption(const char s, const StringT1& name, const StringT2& description)
			:pShortName(s), pLongName(name), pDescription(description)
		{
			assert("A long name of an option must not exceed `YUNI_GETOPT_LONGNAME_MAX_LENGTH` characters"
				&& pLongName.size() <= YUNI_GETOPT_LONGNAME_MAX_LENGTH);
		}

		virtual ~IOption()
        {
        }

		/*!
		** \brief Add a value
		**
		** \param c_str A pointer to the beginining of the CString (must not be null)
		** \param len Length of the string (can be zero)
		** \return True if the operation succeded, false otherwise
		*/
		virtual bool addValue(const char* c_str, const String::size_type len) = 0;

		virtual void enableFlag() = 0;

		virtual bool requireAdditionalParameter() const = 0;

		/*!
		** \brief Get the short name of the option
		*/
		char shortName() const {return pShortName;}

		/*!
		** \brief Get the long name of the option
		*/
		const String& longName() const {return pLongName;}

		virtual void helpUsage(std::ostream& out) const = 0;

	protected:
		//! The short name of the option
		const char pShortName;
		//! The long name
		const String pLongName;
		//! Description
		const String pDescription;

	}; // class IOption





	/*!
	** \brief A single command line option
	*/
	template<class T, bool Visible, bool AdditionalParam = true>
	class Option : public IOption
	{
	public:
		enum
		{
			//! True if the option is hidden from the help usage
			flagVisible = Visible,
		};

	public:
		//! \name Constructors & Destructor
		//@{
		Option(const Option& rhs)
			:IOption(rhs), pVariable(rhs.pVariable)
		{}

		Option(T& var, const char c)
			:IOption(c), pVariable(var)
		{}

		template<class StringT>
		Option(T& var, const StringT& name)
			:IOption(name), pVariable(var)
		{}

		template<class StringT>
		Option(T& var, const char c, const StringT& name)
			:IOption(c, name), pVariable(var)
		{}

		template<class S1, class S2>
		Option(T& var, const char s, const S1& name, const S2& description)
			:IOption(s, name, description), pVariable(var)
		{}

		//! Destructor
		virtual ~Option() {}
		//@}


		/*!
		** \brief Add a value
		**
		** \param c_str A pointer to the beginining of the CString (must not be null)
		** \param len Length of the string (can be zero)
		** \return True if the operation succeded, false otherwise
		*/
		virtual bool addValue(const char* c_str, const String::size_type len)
		{
			return Private::GetOptImpl::Value<T>::Add(pVariable, c_str, len);
		}

		virtual void enableFlag()
		{
			Private::GetOptImpl::Flag<T>::Enable(pVariable);
		}

		virtual void helpUsage(std::ostream& out) const
		{
			if (Visible)
				DisplayHelpForOption(out, pShortName, pLongName, pDescription, AdditionalParam);
		}

		virtual bool requireAdditionalParameter() const {return AdditionalParam;}

	private:
		//! The destination variable, where to add values
		T& pVariable;

	}; // class Option



	/*!
	** \brief A text paragraph
	*/
	class Paragraph : public IOption
	{
	public:
		//! \name Constructors & Destructor
		//@{
		Paragraph(const Paragraph& rhs)
			:IOption(rhs)
		{}

		template<class S>
		Paragraph(const S& description)
			:IOption(' ', nullptr, description)
		{}

		//! Destructor
		virtual ~Paragraph() {}
		//@}


		/*!
		** \brief Add a value
		**
		** \param c_str A pointer to the beginining of the CString (must not be null)
		** \param len Length of the string (can be zero)
		** \return True if the operation succeded, false otherwise
		*/
		virtual bool addValue(const char*, const String::size_type)
		{
			/* Do nothing - This is not an option */
			return false;
		}

		virtual void helpUsage(std::ostream& out) const
		{
			DisplayTextParagraph(out, pDescription);
		}

		virtual void enableFlag()
		{
			// Do nothing
		}

		virtual bool requireAdditionalParameter() const {return false;}

	}; // class Paragraph



	// Forward declaration
	class Context;


} // namespace GetOptImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_GETOPT_OPTION_H__
