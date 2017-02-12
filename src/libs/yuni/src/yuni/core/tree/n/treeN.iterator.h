#ifndef __YUNI_CORE_TREE_N_ITERATOR_H__
# define __YUNI_CORE_TREE_N_ITERATOR_H__

// !!! Do not use includes here  !!!



class iterator
{
public:
	YUNI_CORE_TREE_N_ITERATOR_PRE(iterator);

public:
	//! \name Constructors
	//@{
	iterator() :pNode() {}
	iterator(const iterator& it)  :pNode(it.pNode)  {}
	iterator(const NodeT& p) :pNode(p) {}
	//@}


	IteratorType& operator ++ ()
	{
		pNode = pNode->pNextSibling;
		return *this;
	}

	IteratorType& operator -- ()
	{
		pNode = pNode->pPreviousSibling;
		return *this;
	}

	YUNI_CORE_TREE_N_ITERATOR_COMMON_PUBLIC_METHODS

protected:
	YUNI_CORE_TREE_N_ITERATOR_PROTECTED;

}; // name iterator




#endif // __YUNI_CORE_TREE_N_ITERATOR_H__
