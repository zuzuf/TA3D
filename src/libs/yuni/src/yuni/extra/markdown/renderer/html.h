#ifndef __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_H__
# define __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_H__

# include "../../../yuni.h"
# include "../node.h"
# include "../../../core/string.h"
# include "../reader.h"
# include <cassert>


namespace Yuni
{
namespace Markdown
{
namespace Renderer
{


	class Html
	{
	public:
		enum
		{
			partBegin = 0,
			partEnd,
			partNewLine,
			partCount
		};
		enum
		{
			nodeCount = Node::maxType,
		};

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Html();
		//! Destructor
		~Html() {}
		//@}


		//! Reset all default values
		void resetDefaultValues();

		//! \name Enable / Disable nodes
		//@{
		/*!
		** \brief Ignore a type of node
		*/
		void discard(Node::Type ndtype);
		//! Consider all types of node
		void enableAllNodes();
		//@}


		//! \name Rendering
		//@{
		bool render(const Reader& reader);
		//@}

	public:
		//! Tags composition
		String composition[nodeCount][partCount];
		//! Flag to enable/disable each type of node
		bool enable[nodeCount];
		bool paragraphSkipIfAlone[nodeCount];

	private:
		void renderNode(const Node::Ptr& node);

	}; // class Html





} // namespace Renderer
} // namespace Markdown
} // namespace Yuni

# include "html.hxx"

#endif // __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_H__
