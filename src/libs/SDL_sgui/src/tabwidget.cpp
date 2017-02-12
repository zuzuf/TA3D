#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/tabwidget.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <algorithm>
#include <limits>

using namespace std;

namespace Gui
{

	TabWidget::TabWidget(const ustring &Name, Widget *parent) : Widget(Name, parent)
	{
		CurrentTab = 0;
		FrameLess = false;
	}

	TabWidget::~TabWidget()
	{

	}

	int TabWidget::addTab(const ustring &title, Widget *widget)
	{
		for(vector<pair<wstring, Widget*> >::iterator i = tabs.begin() ; i != tabs.end() ; ++i)
			if (i->second == widget)
				return i - tabs.begin();
		tabs.push_back(make_pair(title, widget));
		Widget::addChild(widget);
		return tabs.size() - 1;
	}

	void TabWidget::removeTab(int idx)
	{
		if (idx < 0 || idx >= int(tabs.size()))
			return;

		Widget::remove(tabs[idx].second);
		tabs.erase(tabs.begin() + idx);
		const int ct = clamp<int>(CurrentTab, 0, tabs.size() - 1);
		if (ct != CurrentTab)
			setCurrentTab(ct);
		refresh();
	}

	const wstring &TabWidget::getTabName(int idx)
	{
		if (idx < 0 || idx >= int(tabs.size()))
		{
			static wstring null;
			return null;
		}
		return tabs[idx].first;
	}

	Widget *TabWidget::getTabWidget(int idx)
	{
		if (idx < 0 || idx >= int(tabs.size()))
			return NULL;
		return tabs[idx].second;
	}

	void TabWidget::setTabName(int idx, const ustring &Name)
	{
		if (idx < 0 || idx >= int(tabs.size()))
			return;
		tabs[idx].first = Name;
		refresh();
	}

	void TabWidget::setTabWidget(int idx, Widget *widget)
	{
		if (idx < 0 || idx >= int(tabs.size()))
			return;
		tabs[idx].second = widget;
		if (idx == CurrentTab)
			updateLayout();
	}

	int TabWidget::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int TabWidget::getOptimalHeight() const
	{
		return numeric_limits<int>::min();
	}

	void TabWidget::draw(SDL_Surface *target)
	{
		if (FrameLess)
			return;
		const int r = 4;

		for(int i = 0 ; i < int(tabs.size()) ; ++i)
		{
			if (i == CurrentTab)	continue;
			const int x0 = i * 80 + 2;
			const int x1 = x0 + 80;
			const int y0 = 2;
			const int y1 = 24 + r;
			roundedgradientbox(target, x0, y0, x1 - 1, y1, r, 0.0f, -0.05f, lightgrey - darkgrey, darkgrey);
			arc(target, x0 + r, y0 + 1 + r, r, 180, 270, grey);
			roundedbox(target, x0 + 1, y0 + 1, x1 - 1, y1, r, verylightgrey);
			roundedbox(target, x0, y0, x1 - 1, y1, r, grey);
			vline(target, x1, 1 + y0 + r, y1 - r, darkgrey);
			if (tabs[i].first.empty())
				continue;
			SDL_Surface sub = SubSurface(target, x0 + 8, y0 + 4, x1 - x0 - 17, 16);
			Font::print(&sub, (sub.w - tabs[i].first.size() * 8) / 2, 0, tabs[i].first, black);
		}
		if (tabs.empty())
			CurrentTab = 0;

		fillroundedbox(target, 2, 24, w - 3, h - 3, r, lightgrey);
		arc(target, w - 2 - r, h - 3 - r, r, 0, 90, darkgrey);
		arc(target, 2 + r, 25 + r, r, 180, 270, grey);
		roundedbox(target, 3, 25, w - 3, h - 3, r, white);
		roundedbox(target, 2, 24, w - 3, h - 3, r, grey);
		hline(target, h - 2, 3 + r, w - 2 - r, black);
		vline(target, w - 2, 25 + r, h - 2 - r, black);
		arc(target, w - 2 - r, h - 2 - r, r, 0, 90, black);

		const int x0 = 2 + CurrentTab * 80;
		const int x1 = x0 + 80;
		const int y0 = 0;
		const int y1 = 24;
		fillroundedbox(target, x0, y0, x1 - 1, y1 + r, r, lightgrey);
		arc(target, x0 + r + 1, y0 + 1 + r, r, 180, 270, white);
		arc(target, x1 - r - 1, y0 + 1 + r, r, 270, 360, white);
		hline(target, y0 + 1, x0 + r, x1 - r, white);
		vline(target, x0 + 1, y0 + r, CurrentTab == 0 ? y1 + 3 : y1, white);

		arc(target, x0 + r, y0 + r, r, 180, 270, grey);
		arc(target, x1 - r, y0 + r, r, 270, 360, grey);
		hline(target, y0, x0 + r, x1 - r, grey);
		vline(target, x0, y0 + r, CurrentTab == 0 ? y1 + 2 : y1, grey);
		vline(target, x1 - 1, y0 + r, y1 + 1, grey);

		vline(target, x1, 1 + y0 + r, y1, darkgrey);
		if (!tabs.empty() && !tabs[CurrentTab].first.empty())
		{
			SDL_Surface sub = SubSurface(target, x0 + 8, y0 + 4, x1 - x0 - 17, 16);
			Font::print(&sub, (sub.w - tabs[CurrentTab].first.size() * 8) / 2, 0, tabs[CurrentTab].first, black);
		}
	}

	void TabWidget::updateLayout()
	{
		if (bUpdatingLayout)
			return;

		if (tabs.empty())
			return;

		bUpdatingLayout = true;

		if (FrameLess)
		{
			tabs[CurrentTab].second->w = w;
			tabs[CurrentTab].second->h = h;
			tabs[CurrentTab].second->x = 0;
			tabs[CurrentTab].second->y = 0;
		}
		else
		{
			tabs[CurrentTab].second->w = w - 16;
			tabs[CurrentTab].second->h = h - 40;
			tabs[CurrentTab].second->x = 8;
			tabs[CurrentTab].second->y = 32;
		}
		tabs[CurrentTab].second->updateLayout();

		if (parent)
			parent->updateLayout();

		refresh();

		bUpdatingLayout = false;
	}

	void TabWidget::event(SDL_Event *e)
	{
		if (tabs.empty())
			return;
		SDL_Event ev = *e;

		switch (ev.type)
		{
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (!tabs.empty())
			{
				if ((getMouseX(e) < 8 || getMouseX(e) >= w - 8 || getMouseY(e) < 32 || getMouseY(e) >= h - 8) && !FrameLess)
				{
					if (tabs[CurrentTab].second->bMouseIn)
					{
						tabs[CurrentTab].second->bMouseIn = false;
						tabs[CurrentTab].second->mouseLeave();
					}
				}
				else if (!tabs[CurrentTab].second->bMouseIn)
				{
					tabs[CurrentTab].second->bMouseIn = true;
					tabs[CurrentTab].second->mouseEnter();
				}
			}
			if (!FrameLess && (getMouseX(e) < 8 || getMouseX(e) >= w - 8 || getMouseY(e) < 32 || getMouseY(e) >= h - 8))
			{
				if (e->type == SDL_MOUSEBUTTONDOWN)
					mousePressEvent(e);
				else if (e->type == SDL_MOUSEBUTTONUP)
					mouseReleaseEvent(e);
				return;
			}
			getMouseX(ev) -= tabs[CurrentTab].second->x;
			getMouseY(ev) -= tabs[CurrentTab].second->y;
			break;
		}
		tabs[CurrentTab].second->event(&ev);
	}

	void TabWidget::mouseEnter()
	{
		if (FrameLess && !tabs.empty())
		{
			tabs[CurrentTab].second->bMouseIn = true;
			tabs[CurrentTab].second->mouseEnter();
		}
	}

	void TabWidget::paint(SDL_Surface *target)
	{
		if (bRefresh)
		{
			draw(target);
			if (!tabs.empty())
			{
				tabs[CurrentTab].second->bRefresh = true;
				tabs[CurrentTab].second->bRefreshChain = true;
			}
			bRefreshChain = true;
		}
		if (bRefreshChain && !tabs.empty()
			&& (tabs[CurrentTab].second->bRefresh || tabs[CurrentTab].second->bRefreshChain))
		{
			SDL_Surface sub = SubSurface(target, tabs[CurrentTab].second->x, tabs[CurrentTab].second->y, tabs[CurrentTab].second->w, tabs[CurrentTab].second->h);
			tabs[CurrentTab].second->paint(&sub);
		}
		bRefresh = false;
		bRefreshChain = false;
	}

	void TabWidget::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button != SDL_BUTTON_LEFT)
			return;
		if (e->button.x >= 2 && e->button.y >= 2 && e->button.y <= 24 && !FrameLess)
		{
			const int idx = (e->button.x - 2) / 80;
			if (idx < int(tabs.size()))
				setCurrentTab(idx);
		}
	}

	void TabWidget::onSetCurrentTab()
	{
		if (!tabs.empty())
		{
			CurrentTab = clamp<int>(CurrentTab, 0, tabs.size() - 1);
			for(int i = 0 ; i < int(tabs.size()) ; ++i)
				if (i != CurrentTab && tabs[i].second->bMouseIn)
				{
					tabs[i].second->bMouseIn = false;
					tabs[i].second->mouseLeave();
				}
			if (FrameLess)
			{
				tabs[CurrentTab].second->w = w;
				tabs[CurrentTab].second->h = h;
				tabs[CurrentTab].second->x = 0;
				tabs[CurrentTab].second->y = 0;
			}
			else
			{
				tabs[CurrentTab].second->w = w - 16;
				tabs[CurrentTab].second->h = h - 40;
				tabs[CurrentTab].second->x = 8;
				tabs[CurrentTab].second->y = 32;
			}
			tabs[CurrentTab].second->updateLayout();
			updateLayout();
		}
	}

	void TabWidget::onSetFrameLess()
	{
		onSetCurrentTab();
		refresh();
	}

	void TabWidget::remove(Widget *widget)
	{
		for(int i = 0 ; i < int(tabs.size()) ; ++i)
			if (tabs[i].second == widget)
			{
				removeTab(i);
				return;
			}
	}
}
