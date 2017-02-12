#ifndef __YUNI_CORE_UNIT_HXX__
# define __YUNI_CORE_UNIT_HXX__



# define YUNI_UNIT_FORMULA  \
	T(  \
		QuantityConversion<Q1,T1,Q,T>::Value(rhs.toSIBaseUnit<P>()) \
		/  \
		Abstract<Q,Child,T,P,N>::RelationToSI() \
	)




namespace Yuni
{
namespace Unit
{

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract()
		:pValue()
	{}


	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const sint16 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const sint32 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const sint64 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const uint16 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const uint32 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const uint64 u)
		:pValue((T)u)
	{}

	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const float u)
		:pValue((T)u)
	{}


	template<class Q, class Child, typename T, typename P, typename N>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const double u)
		:pValue((T)u)
	{}


	template<class Q, class Child, typename T, typename P, typename N>
	template<typename P1>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const Abstract<Q,Child, T,P1,N>& rhs)
		:pValue(rhs.value())
	{}


	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline
	Abstract<Q,Child,T,P,N>::Abstract(const Abstract<Q1,C1, T1,P1,N1>& rhs)
		:pValue(YUNI_UNIT_FORMULA)
	{}



	template<class Q, class Child, typename T, typename P, typename N>
	inline Abstract<Q,Child,T,P,N>&
	Abstract<Q,Child,T,P,N>::operator ++ ()
	{
		++pValue;
		return *this;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline Abstract<Q,Child,T,P,N>
	Abstract<Q,Child,T,P,N>::operator ++ (int)
	{
		Type copy(*this);
		++pValue;
		return copy;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline Abstract<Q,Child,T,P,N>&
	Abstract<Q,Child,T,P,N>::operator -- ()
	{
		--pValue;
		return *this;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline Abstract<Q,Child,T,P,N>
	Abstract<Q,Child,T,P,N>::operator -- (int)
	{
		Type copy(*this);
		--pValue;
		return copy;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const int u) const
	{
		return pValue != (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const unsigned int u) const
	{
		return pValue != (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const float u) const
	{
		return pValue != (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const double u) const
	{
		return pValue != (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue != rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator != (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue != YUNI_UNIT_FORMULA;
	}



	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const int u) const
	{
		return pValue == (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const unsigned int u) const
	{
		return pValue == (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const float u) const
	{
		return pValue == (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const double u) const
	{
		return pValue == (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue == rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator == (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue == YUNI_UNIT_FORMULA;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const int u) const
	{
		return pValue <= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const unsigned int u) const
	{
		return pValue <= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const float u) const
	{
		return pValue <= (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const double u) const
	{
		return pValue <= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const long double u) const
	{
		return pValue <= (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue <= rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator <= (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue <= YUNI_UNIT_FORMULA;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const int u) const
	{
		return pValue >= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const unsigned int u) const
	{
		return pValue >= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const float u) const
	{
		return pValue >= (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const double u) const
	{
		return pValue >= (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue >= rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator >= (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue >= YUNI_UNIT_FORMULA;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const int u) const
	{
		return pValue < (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const unsigned int u) const
	{
		return pValue < (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const float u) const
	{
		return pValue < (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const double u) const
	{
		return pValue < (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue < rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator < (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue < YUNI_UNIT_FORMULA;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const int u) const
	{
		return pValue > (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const unsigned int u) const
	{
		return pValue > (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const float u) const
	{
		return pValue > (T)u;
	}


	template<class Q, class Child, typename T, typename P, typename N>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const double u) const
	{
		return pValue > (T)u;
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class P1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const Abstract<Q,Child, T,P1,N>& rhs) const
	{
		return pValue > rhs.value();
	}

	template<class Q, class Child, typename T, typename P, typename N>
	template<class Q1, typename T1, class C1, typename P1, typename N1>
	inline bool
	Abstract<Q,Child,T,P,N>::operator > (const Abstract<Q1,C1, T1,P1,N1>& rhs) const
	{
		return pValue > YUNI_UNIT_FORMULA;
	}






# define YUNI_UNIT_OPERATOR_ASSIGN(OP,TYPE) \
	template<class Q, class Child, typename T, typename P, typename N> \
	inline Abstract<Q,Child,T,P,N>& \
	Abstract<Q,Child,T,P,N>::operator OP (const TYPE u) \
	{ \
		pValue OP (T)u; \
		return *this; \
	}

# define YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT(OP) \
	template<class Q, class Child, typename T, typename P, typename N> \
	template<class Q1, typename T1, class C1, typename P1, typename N1> \
	inline Abstract<Q,Child,T,P,N>& \
	Abstract<Q,Child,T,P,N>::operator OP (const Abstract<Q1,C1, T1,P1,N1>& rhs) \
	{ \
		pValue OP YUNI_UNIT_FORMULA; \
		return *this; \
	}

# define YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT_PRECISION(OP) \
	template<class Q, class Child, typename T, typename P, typename N> \
	template<typename P1> \
	inline Abstract<Q,Child,T,P,N>& \
	Abstract<Q,Child,T,P,N>::operator OP (const Abstract<Q,Child, T,P1,N>& rhs) \
	{ \
		pValue OP rhs.value(); \
		return *this; \
	}


# define YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(OP) \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, sint16)      \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, sint32)      \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, sint64)      \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, uint16)     \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, uint32)     \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, uint64)     \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, float)        \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, double)       \
	YUNI_UNIT_OPERATOR_ASSIGN(OP, long double)  \
	YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT(OP)      \
	YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT_PRECISION(OP) \


YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(=)
YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(+=)
YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(-=)
YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(/=)
YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES(*=)


#undef YUNI_UNIT_OPERATOR_ASSIGN
#undef YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT
#undef YUNI_UNIT_OPERATOR_ASSIGN_ABSTRACT_PRECISION
#undef YUNI_UNIT_OPERATOR_FOR_ALLSTD_TYPES


} // namespace Unit
} // namespace Yuni

# undef YUNI_UNIT_FORMULA

#endif // __YUNI_CORE_UNIT_HXX__
