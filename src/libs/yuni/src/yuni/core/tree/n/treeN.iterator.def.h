#ifndef __YUNI_CORE_TREE_N_ITERATOR_DEF_H__
# define __YUNI_CORE_TREE_N_ITERATOR_DEF_H__


/*!
** \internal Those defines are quite ugly, however, it should simplify the
** reading of the real algorithm for each iterator.
**
** \internal Those defines are removed in `treeN.iterator.undef.h`
*/



# define YUNI_CORE_TREE_N_ITERATOR_PRE(CLASS)     \
		\
		/*! Itself */ \
		typedef CLASS IteratorType;  \
		/*! A node */ \
		typedef Ptr  NodeT;  \
		\
		/*! \name  STL Compatibility */ \
		/*@{*/ \
		typedef NodeT   value_type;  \
		typedef NodeT*  pointer; \
		typedef NodeT&  reference;  \
		typedef std::bidirectional_iterator_tag iterator_category;  \
		typedef ptrdiff_t  difference_type;  \
		/*@} */  \
		\
		/*! Reference */ \
		typedef NodeT&	ReferenceType;  \
		/*! Pointer */ \
		typedef NodeT*  PointerType




# define YUNI_CORE_TREE_N_ITERATOR_COMMON_PUBLIC_METHODS  \
		\
		NodeT& operator * () {return pNode;}  \
		const NodeT& operator * () const {return pNode;}  \
		\
		NodeT& operator -> () {return pNode;}  \
		const NodeT& operator -> () const {return pNode;}  \
		\
		IteratorType operator -- (int)  \
		{  \
			IteratorType copy(*this);  \
			--*this;  \
			return copy;  \
		}  \
		\
		IteratorType operator ++ (int)  \
		{  \
			IteratorType copy(*this);  \
			--*this;  \
			return copy;  \
		}  \
		\
		IteratorType& operator += (int n)  \
		{  \
			while (n-- > 0)  \
				--(*this);  \
			return *this;  \
		}  \
		\
		IteratorType& operator -= (int n)  \
		{  \
			while (n-- > 0)  \
				--(*this);  \
			return *this;  \
		}  \
		\
		IteratorType& operator += (unsigned int n)  \
		{  \
			while (n)  \
			{  \
				--(*this);  \
				--n;  \
			}  \
			return *this;  \
		}  \
		\
		IteratorType& operator -= (unsigned int n)  \
		{  \
			while (n)  \
			{  \
				--(*this);  \
				--n;  \
			}  \
			return *this;  \
		}  \
		\
		IteratorType operator - (int n)  \
		{  \
			IteratorType copy(*this);  \
			while (n-- > 0)  \
				--(copy);  \
			return copy;  \
		}  \
		\
		\
		/*! \name Comparisons */  \
		/*@{ */  \
		/*! The operator `==` */  \
		bool operator == (const IteratorType& rhs) const {return pNode == rhs.pNode;}  \
		/*! The operator `!=` */   \
		bool operator != (const IteratorType& rhs) const {return pNode != rhs.pNode;}  \
		/*! The operator `<`  */   \
		bool operator < (const IteratorType& rhs) const {return pNode < rhs.pNode;}    \
		/*! The operator `>`  */   \
		bool operator > (const IteratorType& rhs) const {return pNode > rhs.pNode;}    \
		/*! The operator `<=`  */  \
		bool operator <= (const IteratorType& rhs) const {return pNode <= rhs.pNode;}  \
		/*! The operator `>=`  */  \
		bool operator >= (const IteratorType& rhs) const {return pNode >= rhs.pNode;}  \
		/*@} */





# define YUNI_CORE_TREE_N_ITERATOR_PROTECTED  \
		/*! The current node */ \
		NodeT  pNode





#endif // __YUNI_CORE_TREE_N_ITERATOR_DEF_H__
