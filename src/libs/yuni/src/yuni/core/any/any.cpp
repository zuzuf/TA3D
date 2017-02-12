#include <algorithm>
#include "any.h"


namespace Yuni
{


	Any::Any()
	{
		pTable = Private::Any::Table<Private::Any::Empty>::Get();
		pObject = NULL;
	}


	Any::Any(const Any& rhs)
	{
		pTable = Private::Any::Table<Private::Any::Empty>::Get();
		assign(rhs);
	}


	Any::~Any()
	{
		pTable->staticDelete(&pObject);
	}


	Any& Any::assign(const Any& rhs)
	{
		// Are we copying from the same type (using the same table) ?
		if (pTable == rhs.pTable)
		{
			// If so, we can avoid reallocation
			pTable->move(&rhs.pObject, &pObject);
		}
		else
		{
			reset();
			rhs.pTable->clone(&rhs.pObject, &pObject);
			pTable = rhs.pTable;
		}
		return *this;
	}


	void Any::Swap(Any& one, Any& other)
	{
		std::swap(one.pTable, other.pTable);
		std::swap(one.pObject, other.pObject);
	}


	void Any::reset()
	{
		if (!this->empty())
		{
			pTable->staticDelete(&pObject);
			pTable = Private::Any::Table<Private::Any::Empty>::Get();
			pObject = NULL;
		}
	}



} // namespace Yuni
