
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/layout.h>
#include <SDL/sgui/widget.h>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <limits>

using namespace std;

namespace Gui
{

	Layout::Layout() : nextID(0), parent(NULL)
	{
	}

	Layout::~Layout()
	{
	}

	void Layout::addWidget(Widget *widget)
	{
		if (wmap.count(widget))
			return;
		wmap[widget] = nextID++;
		(*this)();
	}

	void Layout::remove(Widget *widget)
	{
		if (wmap.count(widget) == 0)
			return;
		wmap.erase(widget);
		(*this)();
	}

	bool comp(const pair<Widget*, uint32> &a, const pair<Widget*, uint32> &b)
	{
		return a.second < b.second;
	}

	void Layout::operator()()
	{
		vector< pair<Widget*, uint32> > childs;
		for(map<Widget*, uint32>::iterator i = wmap.begin() ; i != wmap.end() ; ++i)
			childs.push_back(*i);

		sort(childs.begin(), childs.end(), comp);

		int x = 0;
		int y = 0;
		for(vector< pair<Widget*, uint32> >::iterator i = childs.begin() ; i != childs.end() ; ++i)
		{
			i->first->resize(i->first->getOptimalWidth(), i->first->getOptimalHeight());
			i->first->setPos(x, 0);
			x += i->first->getWidth();
			y = max(y, i->first->getHeight());
		}
		parent->resize(max(x, parent->getWidth()), max(y, parent->getHeight()));
	}

	void Layout::clear()
	{
		wmap.clear();
	}

	Widget &operator+(Widget *w1, Widget &w2)
	{
		if (typeid(*w1) == typeid(Widget) && typeid(*w1->getLayout()) == typeid(Layout))
		{
			w1->addChild(&w2);
			return *w1;
		}
		Widget *widget = new Widget(L"");
		widget->addChild(w1);
		widget->addChild(&w2);
		return *widget;
	}

	int Layout::getOptimalWidth() const
	{
		if (parent->childs.empty())
			return parent->w;
		int w = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			w += max(0, (*i)->getOptimalWidth());
		return w == 0 ? numeric_limits<int>::min() : w;
	}

	int Layout::getOptimalHeight() const
	{
		if (parent->childs.empty())
			return parent->h;
		int h = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			h = max(h, (*i)->getOptimalHeight());
		return h == 0 ? numeric_limits<int>::min() : h;
	}

}
