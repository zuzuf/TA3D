
# include "node.h"


namespace Yuni
{
namespace Markdown
{


	void Node::flattenText(String& out) const
	{
		if (innerText.notEmpty())
		{
			if (out.notEmpty())
				out += ' ';
			out += innerText;
		}
		if (pChildrenCount)
		{
			const iterator end;
			for (iterator i = begin(); i != end; ++i)
				i->flattenText(out);
		}
	}




} // namespace Markdown
} // namespace Yuni

