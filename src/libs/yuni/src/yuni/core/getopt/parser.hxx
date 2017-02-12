#ifndef __YUNI_CORE_GETOPT_PARSER_HXX__
# define __YUNI_CORE_GETOPT_PARSER_HXX__


namespace Yuni
{
namespace GetOpt
{


	inline Parser::Parser()
		:pRemains(NULL), pErrors(0)
	{}


	template<class U>
	void Parser::add(U& var, const char shortName, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true>(var, shortName)
			: (IOption*) new Private::GetOptImpl::Option<U, false>(var, shortName);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}



	template<class U, class S>
	void Parser::add(U& var, const char shortName, const S& longName, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true>(var, shortName, longName)
			: (IOption*) new Private::GetOptImpl::Option<U, false>(var, shortName, longName);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;
		// The long name
		if (!o->longName().empty())
			pLongNames[o->longName().c_str()] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}


	template<class U, class S, class D>
	void Parser::add(U& var, const char shortName, const S& longName, const D& description, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true>(var, shortName, longName, description)
			: (IOption*) new Private::GetOptImpl::Option<U, false>(var, shortName, longName, description);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;
		// The long name
		if (!o->longName().empty())
			pLongNames[o->longName().c_str()] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}


	template<class U>
	void Parser::addFlag(U& var, const char shortName, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true, false>(var, shortName)
			: (IOption*) new Private::GetOptImpl::Option<U, false, false>(var, shortName);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}



	template<class U, class S>
	void Parser::addFlag(U& var, const char shortName, const S& longName, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true, false>(var, shortName, longName)
			: (IOption*) new Private::GetOptImpl::Option<U, false, false>(var, shortName, longName);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;
		// The long name
		if (!o->longName().empty())
			pLongNames[o->longName().c_str()] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}


	template<class U, class S, class D>
	void Parser::addFlag(U& var, const char shortName, const S& longName, const D& description, bool visible)
	{
		// The new option
		IOption* o = (visible)
			? (IOption*) new Private::GetOptImpl::Option<U, true, false>(var, shortName, longName, description)
			: (IOption*) new Private::GetOptImpl::Option<U, false, false>(var, shortName, longName, description);

		// In the list with all other options
		pAllOptions.push_back(o);
		// The short name
		if (shortName != '\0' && shortName != ' ')
			pShortNames[shortName] = o;
		// The long name
		if (!o->longName().empty())
			pLongNames[o->longName().c_str()] = o;

		// The long name of an option must not be equal to 1
		// There is an ambiguity on Windows : /s : a long or short name ?
		assert(o->longName().size() != 1 && "The long name of an option must not be equal to 1 (ambigous on Windows)");
	}



	template<class U>
	inline void Parser::remainingArguments(U& var)
	{
		if (pRemains)
			delete pRemains;
		pRemains = new Private::GetOptImpl::Option<U, false>(var, '\0');
	}


	template<class U>
	void Parser::addParagraph(const U& text)
	{
		pAllOptions.push_back(new Private::GetOptImpl::Paragraph(text));
	}



} // namespace GetOpt
} // namespace Yuni

#endif // __YUNI_CORE_GETOPT_PARSER_H__
