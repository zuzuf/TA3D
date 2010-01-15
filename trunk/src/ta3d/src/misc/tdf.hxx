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
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			return (entry == pTable.end() || entry->second.empty() ? def : entry->second.to<sint32>());
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		return (entry == pTable.end() || entry->second.empty() ? def : entry->second.to<sint32>());
	}


	template<class K>
	sint32 TDFParser::pullAsInt(const K& key)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			return (entry == pTable.end() || entry->second.empty() ? 0 : entry->second.to<sint32>());
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		return (entry == pTable.end() || entry->second.empty() ? 0 : entry->second.to<sint32>());
	}


	template<class K, class V>
	String TDFParser::pullAsString(const K& key, const V& def)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			return entry == pTable.end() ? def : entry->second;
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		return (entry != pTable.end()) ? entry->second : def;
	}


	template<class K>
	String TDFParser::pullAsString(const K& key)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			return entry == pTable.end() ? nullptr : entry->second;
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		return (entry != pTable.end()) ? entry->second : nullptr;
	}




} // namespace TA3D


#endif // __TA3D_XX__TDF_HXX__
