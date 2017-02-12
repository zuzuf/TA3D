#ifndef __YUNI_EXTRA_MARKDOWN_READER_H__
# define __YUNI_EXTRA_MARKDOWN_READER_H__

# include "../../yuni.h"
# include "../../core/string.h"
# include "fwd.h"
# include "node.h"
# include "../../core/event.h"


namespace Yuni
{
namespace Markdown
{


	class YUNI_DECL  Reader
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Reader();
		//! Destructor
		~Reader();
		//@}


		//! \name Document
		//@{
		template<class StringT> void beginDocument(const StringT& filename);

		template<class StringT> void append(const StringT& text);

		/*!
		** \brief End the parsing of a document
		*/
		void endDocument();
		//@}


		//! \name Error tracking
		//@{
		/*!
		** \brief Set the current filename
		**
		** The current position will be reset.
		*/
		template<class StringT> void hintFilename(const StringT& filename);
		//! Get the current filename
		const String& hintFilename() const;
		//@}


		//! \name Root node
		//@{
		//! Get the root node
		Node::Ptr  root() const;
		//@}


		//! \name Operators
		//@{
		//! Append
		template<class StringT> Reader& operator += (const StringT& text);
		//! Append
		template<class StringT> Reader& operator << (const StringT& text);
		//@}


	public:
		//!
		Event<void ()> onBeginDocument;
		//! Event
		Event<void (const String&)> onParseFilename;
		//!
		Event<void ()> onEndDocument;

	private:
		void beginDocument();
		void internalHintFilename(const StringAdapter& filename);
		void parse(const StringAdapter& text);

	private:
		//! Type for the internal data
		typedef Yuni::Private::Markdown::ReaderData  ReaderDataType;
		//! Flag to indicate if a document has been started (aka internal states are initialized)
		bool pDocumentStarted;
		//! Internal data
		ReaderDataType* pData;

	}; // class Reader






} // namespace Markdown
} // namespace Yuni

# include "reader.hxx"

#endif // __YUNI_EXTRA_MARKDOWN_READER_H__
