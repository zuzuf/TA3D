
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/group.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <queue>
#include <limits>

using namespace std;

namespace Gui
{

	Group::Group(const ustring &Name, const ustring &Caption, Widget *centralWidget, Widget *parent) : Widget(Name, parent), Caption(Caption)
	{
		if (centralWidget)
			addChild(centralWidget);
		else
			addChild(new Widget(L""));
	}

	Group::~Group()
	{
	}

	void Group::setCentralWidget(Widget *widget)
	{
		set<Widget*> ch = childs;
		for(set<Widget*>::iterator i = ch.begin() ; i != ch.end() ; ++i)
			remove(*i);
		addChild(widget);
	}

	int Group::getOptimalWidth() const
	{
		if (childs.empty())
			return 16;
		const int ow = (*childs.begin())->getOptimalWidth();
		return ow <= -1 ? numeric_limits<int>::min() : 16 + ow;
	}

	int Group::getOptimalHeight() const
	{
		if (childs.empty())
			return Caption.empty() ? 16 : 40;
		const int oh = (*childs.begin())->getOptimalHeight();
		return oh <= -1 ? numeric_limits<int>::min() : (Caption.empty() ? 16 : 40) + oh;
	}

	void Group::updateLayout()
	{
		if (childs.empty())
			return;
		if (Caption.empty())
		{
			getCentralWidget()->setPos(8, 8);
			if (getCentralWidget()->getWidth() != w - 16 || getCentralWidget()->getHeight() != h - 16)
				getCentralWidget()->resize(w - 16, h - 16);
			else
				getCentralWidget()->updateLayout();
		}
		else
		{
			getCentralWidget()->setPos(8, 32);
			if (getCentralWidget()->getWidth() != w - 16 || getCentralWidget()->getHeight() != h - 40)
				getCentralWidget()->resize(w - 16, h - 40);
			else
				getCentralWidget()->updateLayout();
		}
	}

	void Group::draw(SDL_Surface *target)
	{
		const int r = 4;
		arc(target, r + 2, r + 2, r, 180, 270, lightgrey);
		arc(target, r + 2, r + 3, r, 180, 270, lightgrey);
		arc(target, w - r - 3, r + 2, r, 270, 360, lightgrey);
		arc(target, w - r - 3, r + 3, r, 270, 360, lightgrey);
		fillcircle(target, r + 2, r + 2, r - 1, white);
		fillcircle(target, w - r - 3, r + 2, r - 1, white);

		vline(target, 2, r + 2, h - 1, lightgrey);
		vline(target, 3, r + 2, h - 1, white);
		vline(target, w - 3, r + 2, h - 1, lightgrey);
		vline(target, w - 4, r + 2, h - 1, white);
		hline(target, 2, r + 2, w - r - 3, lightgrey);
		fillbox(target, r + 2, 3, w - r - 3, r + 2, white);

		vwhitealphagradientbox(target, 4, r + 2, w - 5, h - 1);

		if (!Caption.empty())
			Font::print(target, (w - Caption.size() * 8) / 2, 8, Caption, black);
	}

	Widget *Group::getCentralWidget()
	{
		if (childs.empty())
			return NULL;
		return *childs.begin();
	}

	void Group::setLayout(Layout *layout)
	{
		getCentralWidget()->setLayout(layout);
	}

	Layout *Group::getLayout()
	{
		return getCentralWidget()->getLayout();
	}

	vector<Widget*> Group::getGroup()
	{
		vector<Widget*> group;
		queue<Widget*> wqueue;
		wqueue.push(this);
		while(!wqueue.empty())
		{
			Widget *cur = wqueue.front();
			wqueue.pop();
			group.push_back(cur);
			for(set<Widget*>::iterator i = cur->childs.begin() ; i != cur->childs.end() ; ++i)
				wqueue.push(*i);
		}
		return group;
	}

}
