
#include "html.h"


namespace Yuni
{
namespace Markdown
{
namespace Renderer
{

	Html::Html()
	{
		resetDefaultValues();
	}


	void Html::resetDefaultValues()
	{
		// Enable all nodes
		enableAllNodes();

		// Skip if alone
		memset(&paragraphSkipIfAlone, 0, sizeof(paragraphSkipIfAlone));
		paragraphSkipIfAlone[Node::listItem] = true;

		// reset all
		for (unsigned int t = 0; t != static_cast<unsigned int>(Node::maxType); ++t)
		{
			for (unsigned int p = 0; p != static_cast<unsigned int>(partCount); ++p)
				composition[t][p].clear();
		}

		// Document
		composition[Node::body][partBegin] = "<div class=\"content\" id=\"content\">\n\n\n";
		composition[Node::body][partEnd] = "\n\n\n\n</div>\n";


		// Paragraph
		composition[Node::paragraph][partBegin] = "<p>\n";
		composition[Node::paragraph][partEnd]   = "\n</p>\n";
		// Quote
		composition[Node::quote][partBegin]   = "<quote>\n";
		composition[Node::quote][partNewLine] = '\t';
		composition[Node::quote][partEnd]     = "</quote>\n";

		// TOC
		composition[Node::toc][partBegin]   = "<div class=\"toc\">\n";
		composition[Node::toc][partNewLine] = '\t';
		composition[Node::toc][partEnd]     = "</div>\n";

		composition[Node::header1][partBegin] = "<h1>";
		composition[Node::header1][partEnd]   = "</h1>\n";
		composition[Node::header2][partBegin] = "<h2>";
		composition[Node::header2][partEnd]   = "</h2>\n";
		composition[Node::header3][partBegin] = "<h3>";
		composition[Node::header3][partEnd]   = "</h3>\n";
		composition[Node::header4][partBegin] = "<h4>";
		composition[Node::header4][partEnd]   = "</h4>\n";
		composition[Node::header5][partBegin] = "<h5>";
		composition[Node::header5][partEnd]   = "</h5>\n";
		composition[Node::header6][partBegin] = "<h6>";
		composition[Node::header6][partEnd]   = "</h6>\n";

		// Bold
		composition[Node::bold][partBegin]   = "<strong>";
		composition[Node::bold][partEnd]   = "</strong>";

		composition[Node::unorderedList][partBegin]   = "<ul>\n";
		composition[Node::unorderedList][partNewLine] = '\t';
		composition[Node::unorderedList][partEnd]     = "</ul>\n";
		composition[Node::orderedList][partBegin]   = "<ol>\n";
		composition[Node::orderedList][partNewLine] = '\t';
		composition[Node::orderedList][partEnd]     = "</ol>\n";
		composition[Node::listItem][partBegin]   = "<li>";
		composition[Node::listItem][partEnd]     = "</li>\n";

		// Line break
		composition[Node::linebreak][partBegin] = "\n<br />";
	}



	bool Html::render(const Reader& reader)
	{
		// Retrieve the root node
		Node::Ptr root = reader.root();
		if (!root)
			return false;
		renderNode(root);
		return true;
	}


	void Html::renderNode(const Node::Ptr& node)
	{
		assert(!(!node));

		const Node& rawNode = *node;
		const Node::Type type = rawNode.type;
		assert(static_cast<unsigned int>(type) < nodeCount);
		if (!enable[type])
			return;

		std::cout << composition[type][partBegin] << rawNode.innerText;
		
		switch (rawNode.size())
		{
			case 0:
				break;
			case 1:
				{
					const Node::Ptr& child = rawNode.firstChild();
					const Node& rawChild = *child;
					if (paragraphSkipIfAlone[type] && rawChild.type == Node::paragraph)
					{
						const Node::iterator end = rawChild.end();
						for (Node::iterator i = rawChild.begin(); i != end; ++i)
							renderNode(*i);
					}
					else
						renderNode(child);
					break;
				}
			default:
				{
					const Node::iterator end = rawNode.end();
					for (Node::iterator i = rawNode.begin(); i != end; ++i)
						renderNode(*i);
				}
		}

		std::cout << composition[type][partEnd];
	}




} // namespace Renderer
} // namespace Markdown
} // namespace Yuni

