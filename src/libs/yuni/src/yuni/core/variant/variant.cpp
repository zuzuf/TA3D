#include "variant.h"


namespace Yuni
{

	Variant::~Variant()
	{
		if (pData)
		{
			delete pData;
			pData = NULL;
		}
	}


	void Variant::assign(const Variant& rhs)
	{
		if (pData)
			delete pData;
		pData = (rhs.pData) ? rhs.pData->clone() : NULL;
	}

	void Variant::clear()
	{
		if (pData)
		{
			delete pData;
			pData = NULL;

		}
	}

} // namespace Yuni
