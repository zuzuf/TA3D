#ifndef __TA3D_XX__TDF_HXX__
# define __TA3D_XX__TDF_HXX__


namespace TA3D
{


	template<class K>
	sint32 TDFParser::pullAsInt(const K& key, const sint32 def)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			if (!pTable.exists(keyToFind))
				return def;
			String iterFind = pTable.find(keyToFind);
			return (iterFind.empty() ? def : iterFind.to<sint32>());
		}
		if (!pTable.exists(key))
			return def;
		String iterFind = pTable.find(key);
		return (iterFind.empty() ? def : iterFind.to<sint32>());
	}


	template<class K>
	sint32 TDFParser::pullAsInt(const K& key)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			if (!pTable.exists(keyToFind))
				return 0;
			String iterFind = pTable.find(keyToFind);
			return (iterFind.empty() ? 0 : iterFind.to<sint32>());
		}
		if (!pTable.exists(key))
			return 0;
		String iterFind = pTable.find(key);
		return (iterFind.empty() ? 0 : iterFind.to<sint32>());
	}


	template<class K, class V>
	String TDFParser::pullAsString(const K& key, const V& def)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			if (!pTable.exists(keyToFind))
				return def;
			return pTable.find(keyToFind);
		}
		return (pTable.exists(key)) ? pTable.find(key) : def;
	}


	template<class K>
	String TDFParser::pullAsString(const K& key)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			if (!pTable.exists(keyToFind))
				return nullptr;
			return pTable.find(keyToFind);
		}
		return (pTable.exists(key)) ? pTable.find(key) : nullptr;
	}




} // namespace TA3D


#endif // __TA3D_XX__TDF_HXX__
