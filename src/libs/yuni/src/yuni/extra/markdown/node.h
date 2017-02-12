#ifndef __YUNI_EXTRA_MARKDOWN_NODE_H__
# define __YUNI_EXTRA_MARKDOWN_NODE_H__

# include "../../yuni.h"
# include "../../core/tree/treeN.h"
# include "../../core/string.h"


namespace Yuni
{
namespace Markdown
{

	/*!
	** \brief A node for a markdown parse tree
	*/
	class Node : public Core::TreeN<Node, Policy::SingleThreaded>
	{
	public:
		//! Our ancestor
		typedef Core::TreeN<Node, Policy::SingleThreaded> AncestorType;
		//! Smart pointer
		typedef AncestorType::Ptr  Ptr;

		//! Type for a single node
		enum Type
		{
			unknown = 0,
			//! A markdown document
			document,
			//! A document header
			head,
			//! TOC (Table Of Content)
			toc,
			//! Body for a document
			body,
			//! Header (level 1, block)
			header1,
			//! Header (level 2, block)
			header2,
			//! Header (level 3, block)
			header3,
			//! Header (level 4, block)
			header4,
			//! Header (level 5, block)
			header5,
			//! Header (level 6, block)
			header6,
			//! Raw text
			text,
			//! Paragraph
			paragraph,
			//! Quote
			quote,
			//! Emphasis
			emphasis,
			//! Bold (inline)
			bold,
			//! Horizontal line
			hzLine,
			//! Unordered list
			unorderedList,
			//! Ordered list
			orderedList,
			//! List item (ordered or unordered)
			listItem,
			//! Line break (br)
			linebreak,
			//! Pragma directive
			pragma,
			//! Number of items
			maxType,
		};

	public:
		//! Convert a node type into a mere c-string
		static const char* TypeToCString(Type t)
		{
			static const char* values[maxType] =
			{
				"unknown",
				"document",
				"head",
				"toc",
				"body",
				"header1",
				"header2",
				"header3",
				"header4",
				"header5",
				"header6",
				"text",
				"paragraph",
				"quote",
				"emphasis",
				"bold",
				"hzLine",
				"unorderedList",
				"orderedList",
				"listItem",
				"linebreak",
				"pragma",
			};
			assert(static_cast<unsigned int>(t) < static_cast<unsigned int>(maxType));
			return values[static_cast<unsigned int>(t)];
		}

		//! Get if a type is a block
		static bool ShouldAppendAParagraphNode(Type t)
		{
			static const bool values[maxType] =
			{
				false, // "unknown",
				false, // "document",
				false, // "head"
				false, // "toc"
				true,  // "body"
				false, // "header1",
				false, // "header2",
				false, // "header3",
				false, // "header4",
				false, // "header5",
				false, // "header6",
				false, // "text",
				false, // "paragraph",
				true,  // "quote",
				false, // "emphasis",
				false, // "bold",
				false, // "hzLine",
				false, // "unorderedList",
				false, // "orderedList",
				true,  // "listItem",
				false, // "linebreak"
				false, // "pragma",
			};
			assert(static_cast<unsigned int>(t) < static_cast<unsigned int>(maxType));
			return values[static_cast<unsigned int>(t)];
		}


	public:
		Node()
			:type(unknown)
		{
			// FIXME (#302) artificially increase the reference counter - memory leak
			for (unsigned int i = 0; i != 50; ++i)
				addRef();
		}

		explicit Node(const Type t)
			:type(t)
		{
			// FIXME (#302) artificially increase the reference counter - memory leak
			for (unsigned int i = 0; i != 50; ++i)
				addRef();
		}

		template<class StringT> Node(const Type t, const StringT& text)
			:type(t), innerText(text)
		{
			// FIXME (#302) artificially increase the reference counter - memory leak
			for (unsigned int i = 0; i != 50; ++i)
				addRef();
		}



		~Node()
		{
			// delete all children nodes
			clear();
		}


		template<class StreamT> void dump(StreamT& out, unsigned int level = 0) const
		{
			const char* const id = TypeToCString(type);

			for (unsigned int i = 1; i < level; ++i)
				out << "    ";
			out << "<" << id << '>';

			if (type == Node::text)
			{
				if (innerText.notEmpty())
					out << "<![CDATA[" << innerText << "]]>";
			}
			else
			{
				out << '\n';
				if (innerText.notEmpty())
				{
					for (unsigned int i = 1; i < level; ++i)
						out << "    ";
					out << "    <![CDATA[" << innerText << "]]>\n";
				}

				if (pChildrenCount)
				{
					const iterator end;
					for (iterator i = begin(); i != end; ++i)
						i->dump(out, level + 1);
				}

				for (unsigned int i = 1; i < level; ++i)
					out << "    ";
			}
			out << "</" << id << ">\n";
		}


		void flattenText(String& out) const;

	public:
		Type type;
		//! Text of the node
		String innerText;
		//! ID of the node
		String id;

	}; // class Node





} // namespace Markdown
} // namespace Yuni

#endif // __YUNI_EXTRA_MARKDOWN_NODE_H__
