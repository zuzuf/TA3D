#ifndef __YUNI_EXTRA_MARKDOWN_DATA_H__
# define __YUNI_EXTRA_MARKDOWN_DATA_H__

# include "../../../yuni.h"
# include "../../../core/string.h"
# include "signature.h"
# include "../fwd.h"


namespace Yuni
{
namespace Private
{
namespace Markdown
{

	class ReaderData
	{
	public:
		//! Node
		typedef Yuni::Markdown::Node  Node;

		enum
		{
			defaultLinePosition   = 1,
			defaultColumnPosition = 0,
			debug                 = Yuni::Markdown::markdownDebug,
			stackLimit            = Yuni::Markdown::stackLimit,
		};

	public:
		ReaderData()
		{}

		~ReaderData()
		{}

		void reset();

		void parse(const StringAdapter& text);

		void flush();

		const Node::Ptr& root() const {return pStack[0];}

	public:
		unsigned int column;
		unsigned int line;
		String filename;

	private:
		void parseLine();
		unsigned int signatureForTheCurrentLine();
		bool executePragma();
		void updateStack();
		void pushNewNode();
		void appendParagraphText(const StringAdapter& text);
		void postParseTree(const Node::Ptr& node);
		void appendTOCEntry(const Node& node, unsigned int level);

	private:
		//! Map ID
		typedef std::map<String, Node::Ptr> MapID;

	private:
		CustomString<4096> pLine;
		bool pHasLineBreak;
		unsigned int pQuoteLevel;
		Signature pCurrentLineSignature;
		Signature pLastSignature;
		unsigned int pStackSize;
		Node::Ptr pStack[stackLimit];
		//! Reference to the TOC (Table Of Content)
		Node::Ptr pTOC;
		//!
		MapID pMapID;

	}; // ReaderData





} // namespace Markdown
} // namespace Private
} // namespace Yuni

#endif // __YUNI_EXTRA_MARKDOWN_DATA_H__
