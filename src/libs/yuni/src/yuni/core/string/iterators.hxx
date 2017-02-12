#ifndef __YUNI_CORE_STRING_ITERATORS_HXX__
# define __YUNI_CORE_STRING_ITERATORS_HXX__

/* !!! Do not use includes here *** */


/*!
** \brief Standard iterator
*/
class iterator
{
	template<typename C1, int Chnk1> friend class StringBase;
	friend class const_iterator;
	friend class reverse_iterator;
	friend class const_reverse_iterator;
public:
	typedef Char  value_type;
	typedef Char* pointer;
	typedef Char& reference;
	typedef const Char& const_reference;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef iterator IteratorType;

public:
	iterator();
	iterator(iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(const reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	iterator(StringBase<Char,Chunk>& str) :pStr(&str), pIndx(0) {}
	iterator(StringBase<Char,Chunk>& str, const Size p) :pStr(&str), pIndx(p) {}

	IteratorType& operator ++ ()
	{
		if (pStr && pIndx < pStr->pSize)
			++pIndx;
		return *this;
	}

	IteratorType& operator -- ()
	{
		if (pIndx > 0)
			--pIndx;
		return *this;
	}

	reference operator * () {return *(pStr->pPtr + pIndx);}
	const_reference operator * () const {return *(pStr->pPtr + pIndx);}

	reference operator -> () {return *(pStr->pPtr + pIndx);}
	const_reference operator -> () const {return *(pStr->pPtr + pIndx);}

	IteratorType operator -- (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType operator ++ (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType& operator += (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator -= (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator += (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType& operator -= (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType operator - (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			--(copy);
		return copy;
	}

	IteratorType operator + (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			++(copy);
		return copy;
	}



	/*! \name Comparisons */
	/*@{ */
	/*! The operator `==` */
	bool operator == (const IteratorType& rhs) const {return pStr == rhs.pStr && pIndx == rhs.pIndx;}
	/*! The operator `!=` */
	bool operator != (const IteratorType& rhs) const {return !(*this == rhs);}
	/*! The operator `<`  */
	bool operator < (const IteratorType& rhs) const {return pIndx < rhs.pIndx;}
	/*! The operator `>`  */
	bool operator > (const IteratorType& rhs) const {return pIndx > rhs.pIndx;}
	/*! The operator `<=`  */
	bool operator <= (const IteratorType& rhs) const {return pIndx <= rhs.pIndx;}
	/*! The operator `>=`  */
	bool operator >= (const IteratorType& rhs) const {return pIndx >= rhs.pIndx;}
	/*@} */


private:
	StringBase<C,Chunk>* pStr;
	Size pIndx;
};


/*!
** \brief const Standard iterator
*/
class const_iterator
{
	template<typename C1, int Chnk1> friend class StringBase;
	friend class iterator;
	friend class reverse_iterator;
	friend class const_reverse_iterator;
public:
	typedef Char  value_type;
	typedef Char* pointer;
	typedef Char& reference;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef const_iterator IteratorType;

public:
	const_iterator();
	const_iterator(iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_iterator(const StringBase<Char,Chunk>& str) :pStr(&str), pIndx(0) {}
	const_iterator(const StringBase<Char,Chunk>& str, const Size p) :pStr(&str), pIndx(p) {}

	IteratorType& operator ++ ()
	{
		if (pStr && pIndx < pStr->pSize)
			++pIndx;
		return *this;
	}

	IteratorType& operator -- ()
	{
		if (pIndx > 0)
			--pIndx;
		return *this;
	}

	const_reference operator * () const {return *(pStr->pPtr + pIndx);}

	const_reference operator -> () const {return *(pStr->pPtr + pIndx);}

	IteratorType operator -- (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType operator ++ (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType& operator += (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator -= (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator += (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType& operator -= (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType operator - (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			--(copy);
		return copy;
	}

	IteratorType operator + (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			++(copy);
		return copy;
	}



	/*! \name Comparisons */
	/*@{ */
	/*! The operator `==` */
	bool operator == (const IteratorType& rhs) const {return pStr == rhs.pStr && pIndx == rhs.pIndx;}
	/*! The operator `!=` */
	bool operator != (const IteratorType& rhs) const {return !(*this == rhs);}
	/*! The operator `<`  */
	bool operator < (const IteratorType& rhs) const {return pIndx < rhs.pIndx;}
	/*! The operator `>`  */
	bool operator > (const IteratorType& rhs) const {return pIndx > rhs.pIndx;}
	/*! The operator `<=`  */
	bool operator <= (const IteratorType& rhs) const {return pIndx <= rhs.pIndx;}
	/*! The operator `>=`  */
	bool operator >= (const IteratorType& rhs) const {return pIndx >= rhs.pIndx;}
	/*@} */


private:
	const StringBase<C,Chunk> * const pStr;
	Size pIndx;
};






/*!
** \brief Reverse iterator
*/
class reverse_iterator
{
	template<typename C1, int Chnk1> friend class StringBase;
	friend class const_iterator;
	friend class iterator;
	friend class const_reverse_iterator;
public:
	typedef Char  value_type;
	typedef Char* pointer;
	typedef Char& reference;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef reverse_iterator IteratorType;

public:
	reverse_iterator();
	reverse_iterator(reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const const_reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(const iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	reverse_iterator(StringBase<Char,Chunk>& str) :pStr(&str), pIndx(str.pSize-1) {}
	reverse_iterator(StringBase<Char,Chunk>& str, const Size p) :pStr(&str), pIndx(p) {}

	IteratorType& operator ++ ()
	{
		if (pStr && pIndx != npos)
			--pIndx;
		return *this;
	}

	IteratorType& operator -- ()
	{
		if (pStr && pIndx < pStr->pSize)
			++pIndx;
		return *this;
	}

	reference operator * () {return *(pStr->pPtr + pIndx);}
	const_reference operator * () const {return *(pStr->pPtr + pIndx);}

	reference operator -> () {return *(pStr->pPtr + pIndx);}
	const_reference operator -> () const {return *(pStr->pPtr + pIndx);}

	IteratorType operator -- (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType operator ++ (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType& operator += (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator -= (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator += (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType& operator -= (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType operator - (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			--(copy);
		return copy;
	}

	IteratorType operator + (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			++(copy);
		return copy;
	}



	/*! \name Comparisons */
	/*@{ */
	/*! The operator `==` */
	bool operator == (const IteratorType& rhs) const {return pStr == rhs.pStr && pIndx == rhs.pIndx;}
	/*! The operator `!=` */
	bool operator != (const IteratorType& rhs) const {return !(*this == rhs);}
	/*! The operator `<`  */
	bool operator < (const IteratorType& rhs) const {return pIndx < rhs.pIndx;}
	/*! The operator `>`  */
	bool operator > (const IteratorType& rhs) const {return pIndx > rhs.pIndx;}
	/*! The operator `<=`  */
	bool operator <= (const IteratorType& rhs) const {return pIndx <= rhs.pIndx;}
	/*! The operator `>=`  */
	bool operator >= (const IteratorType& rhs) const {return pIndx >= rhs.pIndx;}
	/*@} */


private:
	StringBase<C,Chunk>* pStr;
	Size pIndx;
};


/*!
** \brief Const Reverse iterator
*/
class const_reverse_iterator
{
	template<typename C1, int Chnk1> friend class StringBase;
	friend class iterator;
public:
	typedef Char  value_type;
	typedef Char* pointer;
	typedef Char& reference;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef ptrdiff_t  difference_type;
	typedef const_reverse_iterator IteratorType;

public:
	const_reverse_iterator();
	const_reverse_iterator(reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(const reverse_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(const iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(const const_iterator& it) :pStr(it.pStr), pIndx(it.pIndx) {}
	const_reverse_iterator(const StringBase<Char,Chunk>& str) :pStr(&str), pIndx(str.pSize-1) {}
	const_reverse_iterator(const StringBase<Char,Chunk>& str, const Size p) :pStr(&str), pIndx(p) {}

	IteratorType& operator ++ ()
	{
		if (pStr && pIndx != npos)
			--pIndx;
		return *this;
	}

	IteratorType& operator -- ()
	{
		if (pStr && pIndx < pStr->pSize)
			++pIndx;
		return *this;
	}

	const_reference operator * () const {return *(pStr->pPtr + pIndx);}

	const_reference operator -> () const {return *(pStr->pPtr + pIndx);}

	IteratorType operator -- (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType operator ++ (int)
	{
		IteratorType copy(*this);
		--*this;
		return copy;
	}

	IteratorType& operator += (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator -= (int n)
	{
		while (n-- > 0)
			--(*this);
		return *this;
	}

	IteratorType& operator += (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType& operator -= (unsigned int n)
	{
		while (n)
		{
			--(*this);
			--n;
		}
		return *this;
	}

	IteratorType operator - (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			--(copy);
		return copy;
	}

	IteratorType operator + (int n)
	{
		IteratorType copy(*this);
		while (n-- > 0)
			++(copy);
		return copy;
	}


	/*! \name Comparisons */
	/*@{ */
	/*! The operator `==` */
	bool operator == (const IteratorType& rhs) const {return pStr == rhs.pStr && pIndx == rhs.pIndx;}
	/*! The operator `!=` */
	bool operator != (const IteratorType& rhs) const {return !(*this == rhs);}
	/*! The operator `<`  */
	bool operator < (const IteratorType& rhs) const {return pIndx < rhs.pIndx;}
	/*! The operator `>`  */
	bool operator > (const IteratorType& rhs) const {return pIndx > rhs.pIndx;}
	/*! The operator `<=`  */
	bool operator <= (const IteratorType& rhs) const {return pIndx <= rhs.pIndx;}
	/*! The operator `>=`  */
	bool operator >= (const IteratorType& rhs) const {return pIndx >= rhs.pIndx;}
	/*@} */


private:
	const StringBase<C,Chunk> * const pStr;
	Size pIndx;
};




#endif // __YUNI_CORE_STRING_ITERATORS_HXX__
