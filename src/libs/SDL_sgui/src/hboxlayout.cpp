
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/hboxlayout.h>
#include <SDL/sgui/widget.h>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <limits>

using namespace std;

namespace Gui
{

	HBoxLayout::HBoxLayout()
	{
	}

	HBoxLayout::~HBoxLayout()
	{
	}

	void HBoxLayout::operator ()()
	{
		vector< pair<Widget*, uint32> > childs;
		for(map<Widget*, uint32>::iterator i = wmap.begin() ; i != wmap.end() ; ++i)
			childs.push_back(*i);

		sort(childs.begin(), childs.end(), comp);

		vector<int> optim;
		int nblanks = 0;
		int tw = 0;
		for(vector< pair<Widget*, uint32> >::iterator i = childs.begin() ; i != childs.end() ; ++i)
		{
			optim.push_back(i->first->getOptimalWidth());
			if (optim.back() <= -1)
				++nblanks;
			else
				tw += optim.back();
		}

		int n = 0;
		float fx0 = 0.0f, fx1;
		for(vector< pair<Widget*, uint32> >::iterator i = childs.begin() ; i != childs.end() ; ++i)
		{
			const int ow = optim[n++];
			if (ow >= 0)
			{
				if (tw < parent->getWidth())
				{
					if (nblanks)
						fx1 = fx0 + ow + 1;
					else
						fx1 = fx0 + float(parent->getWidth()) * ow / tw;
				}
				else
					fx1 = fx0 + float(parent->getWidth()) * ow / tw;
			}
			else
			{
				if (tw < parent->getWidth())
					fx1 = fx0 + float(parent->getWidth() - tw) / nblanks;
				else
					fx1 = fx0;
			}
			int x0 = int(fx0 + 0.5f);
			int x1 = int(fx1 + 0.5f);
			fx0 = fx1;

			i->first->setPos(x0, 0);
			i->first->resize(x1 - x0, parent->getHeight());
		}
	}

	Widget &operator|(Widget *w1, Widget &w2)
	{
		if (typeid(*w1) == typeid(Widget) && typeid(*w1->getLayout()) == typeid(HBoxLayout))
		{
			w1->addChild(&w2);
			return *w1;
		}
		Widget *widget = new Widget(L"");
		widget->setLayout(new HBoxLayout);
		widget->addChild(w1);
		widget->addChild(&w2);
		return *widget;
	}

	int HBoxLayout::getOptimalWidth() const
	{
		if (parent->childs.empty())
			return parent->w;
		int w = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			w += max(0, (*i)->getOptimalWidth());
		return w == 0 ? numeric_limits<int>::min() : w;
	}

	int HBoxLayout::getOptimalHeight() const
	{
		if (parent->childs.empty())
			return parent->h;
		int h = 0;
		for(set<Widget*>::iterator i = parent->childs.begin() ; i != parent->childs.end() ; ++i)
			h = max(h, (*i)->getOptimalHeight());
		return h == 0 ? numeric_limits<int>::min() : h;
	}

}
