#ifndef __YUNI_CORE_CUSTOMSTRING_ITERATOR_HXX__
# define __YUNI_CORE_CUSTOMSTRING_ITERATOR_HXX__


namespace Yuni
{

	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::utf8iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::utf8begin()
	{
		return utf8iterator(*this, 0);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::const_utf8iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::utf8begin() const
	{
		return const_utf8iterator(*this, 0);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::null_iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::utf8end()
	{
		return null_iterator(*this);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::null_iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::utf8end() const
	{
		return null_iterator(*this);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::null_iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::end()
	{
		return null_iterator(*this);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::null_iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::end() const
	{
		return null_iterator(*this);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::begin()
	{
		return iterator(*this, 0);
	}


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	inline typename CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::const_iterator
	CustomString<ChunkSizeT,ExpandableT,ZeroTerminatedT>::begin() const
	{
		return const_iterator(*this, 0);
	}



} // namespace Yuni

#endif // __YUNI_CORE_CUSTOMSTRING_ITERATOR_HXX__


