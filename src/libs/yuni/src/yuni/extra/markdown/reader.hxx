#ifndef __YUNI_EXTRA_MARKDOWN_HXX__
# define __YUNI_EXTRA_MARKDOWN_HXX__


namespace Yuni
{
namespace Markdown
{


	template<class StringT>
	inline void Reader::append(const StringT& text)
	{
		StringAdapter adapter;
		adapter.adapt(text);
		if (adapter.notEmpty())
			parse(adapter);
	}


	template<class StringT>
	inline void Reader::hintFilename(const StringT& filename)
	{
		StringAdapter adapter;
		adapter.adapt(filename);
		internalHintFilename(adapter);
	}


	template<class StringT>
	inline void Reader::beginDocument(const StringT& filename)
	{
		// begin the document
		beginDocument();
		// filename
		hintFilename(filename);
	}


	template<class StringT>
	inline Reader& Reader::operator += (const StringT& text)
	{
		append(text);
		return *this;
	}


	template<class StringT>
	inline Reader& Reader::operator << (const StringT& text)
	{
		append(text);
		return *this;
	}




} // namespace Markdown
} // namespace Yuni

#endif // __YUNI_EXTRA_MARKDOWN_HXX__
