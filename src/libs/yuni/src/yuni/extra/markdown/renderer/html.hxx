#ifndef __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_HXX__
# define __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_HXX__


namespace Yuni
{
namespace Markdown
{
namespace Renderer
{


	inline void Html::discard(Node::Type ndtype)
	{
		assert(static_cast<unsigned int>(ndtype) < nodeCount);
		enable[ndtype] = false;
	}


	inline void Html::enableAllNodes()
	{
		for (unsigned int i = 0; i != nodeCount; ++i)
			enable[i] = true;
	}




} // namespace Renderer
} // namespace Markdown
} // namespace Yuni

#endif // __YUNI_EXTRA_MARKDOWN_RENDERER_HTML_H__
