#ifndef __YUNI_CORE_VALIDATOR_DEFAULT_HXX__
# define __YUNI_CORE_VALIDATOR_DEFAULT_HXX__


namespace Yuni
{
namespace Validator
{
namespace Text
{


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>::ExceptionList()
	{}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>::ExceptionList(const ExceptionList<DefaultPolicy>& rhs)
		:pExceptionList(rhs.pExceptionList)
	{}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<Yuni::Validator::DefaultPolicy OtherDefaultPolicy>
	inline ExceptionList<DefaultPolicy>::ExceptionList(const ExceptionList<OtherDefaultPolicy>& rhs)
		:pExceptionList(rhs.pExceptionList)
	{}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>::ExceptionList(const String::Vector& rhs)
		:pExceptionList(rhs)
	{}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>::ExceptionList(const String::List& rhs)
	{
		if (!rhs.empty())
		{
			const String::List::const_iterator end = rhs.end();
			for (String::List::const_iterator i = rhs.begin(); i != end; ++i)
				pExceptionList.push_back(*i);
		}
	}




	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<typename U>
	bool ExceptionList<DefaultPolicy>::validate(const U& s) const
	{
		if (!pExceptionList.empty())
		{
			const String::Vector::const_iterator end = pExceptionList.end();
			for (String::Vector::const_iterator i = pExceptionList.begin(); i != end; ++i)
			{
				if (*i == s)
					return !DefaultPolicy;
			}
		}
		return DefaultPolicy;
	}



	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<class U>
	inline void ExceptionList<DefaultPolicy>::exception(const U& e)
	{
		pExceptionList.push_back(e);
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<Yuni::Validator::DefaultPolicy OtherDefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator = (const ExceptionList<OtherDefaultPolicy>& rhs)
	{
		pExceptionList = rhs.pExceptionList;
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator = (const String::Vector& rhs)
	{
		pExceptionList = rhs;
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator = (const String::List& rhs)
	{
		pExceptionList.clear();
		if (!rhs.empty())
		{
			const String::List::const_iterator end = rhs.end();
			for (String::List::const_iterator i = rhs.begin(); i != end; ++i)
				pExceptionList.push_back(*i);
		}
		return *this;
	}




	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<typename U>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator += (const U& rhs)
	{
		pExceptionList.push_back(rhs);
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<Yuni::Validator::DefaultPolicy OtherDefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator += (const ExceptionList<OtherDefaultPolicy>& rhs)
	{
		pExceptionList += rhs.pExceptionList;
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator += (const String::Vector& rhs)
	{
                for(const String &s : rhs)
                pExceptionList.push_back(s);
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator += (const String::List& rhs)
	{
		if (!rhs.empty())
		{
			const String::List::const_iterator end = rhs.end();
			for (String::List::const_iterator i = rhs.begin(); i != end; ++i)
				pExceptionList.push_back(*i);
		}
		return *this;
	}



	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<typename U>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator << (const U& rhs)
	{
		pExceptionList.push_back(rhs);
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	template<Yuni::Validator::DefaultPolicy OtherDefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator << (const ExceptionList<OtherDefaultPolicy>& rhs)
	{
		pExceptionList += rhs.pExceptionList;
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	inline ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator << (const String::Vector& rhs)
	{
            for(const String &s : rhs)
                pExceptionList.push_back(s);
		return *this;
	}


	template<Yuni::Validator::DefaultPolicy DefaultPolicy>
	ExceptionList<DefaultPolicy>& ExceptionList<DefaultPolicy>::operator << (const String::List& rhs)
	{
		if (!rhs.empty())
		{
			const String::List::const_iterator end = rhs.end();
			for (String::List::const_iterator i = rhs.begin(); i != end; ++i)
				pExceptionList.push_back(*i);
		}
		return *this;
	}




} // namespace Text
} // namespace Validator
} // namespace Yuni

#endif // __YUNI_CORE_VALIDATOR_DEFAULT_H__
