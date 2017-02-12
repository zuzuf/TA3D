
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/unmanagedlayout.h>
#include <SDL/sgui/widget.h>
#include <vector>
#include <utility>
#include <limits>

using namespace std;

namespace Gui
{

	UnmanagedLayout::UnmanagedLayout()
	{
	}

	UnmanagedLayout::~UnmanagedLayout()
	{
	}

	int UnmanagedLayout::getOptimalWidth() const
	{
		if (parent->childs.empty())
			return parent->w;
		int w = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			w = max(w, (*i)->getX() + (*i)->getOptimalWidth());
		return w == 0 ? numeric_limits<int>::min() : w;
	}

	int UnmanagedLayout::getOptimalHeight() const
	{
		if (parent->childs.empty())
			return parent->h;
		int h = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			h += max(h, (*i)->getY() + (*i)->getOptimalHeight());
		return h == 0 ? numeric_limits<int>::min() : h;
	}

	void UnmanagedLayout::operator()()
	{
	}
}
