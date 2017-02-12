
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/menubar.h>
#include <SDL/sgui/window.h>
#include <SDL/sgui/menu.h>
#include <SDL/sgui/renderapi.h>
#include <SDL/sgui/layout.h>
#include <SDL/sgui/font.h>
#include <algorithm>
#include <limits>

using namespace std;

namespace Gui
{

	MenuBar::MenuBar(const ustring &Name, Window *wnd) : Widget(Name, wnd)
	{
		highlight = -1;
	}

	MenuBar::~MenuBar()
	{
		for(uint32 i = 0 ; i < entries.size() ; ++i)
			delete entries[i];
		entries.clear();
	}

	int MenuBar::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int MenuBar::getOptimalHeight() const
	{
		return 24;
	}

	void MenuBar::setLayout(Layout *layout)
	{
		delete layout;
	}

	void MenuBar::addMenu(Menu *menu)
	{
		if (find(entries.begin(), entries.end(), menu) == entries.end())
			entries.push_back(menu);
	}

	void MenuBar::remove(Menu *menu)
	{
		vector<Menu*>::iterator it = find(entries.begin(), entries.end(), menu);
		if (it != entries.end())
			entries.erase(it);
	}

	void MenuBar::draw(SDL_Surface *target)
	{
		gradientbox(target, 0, 0, w - 1, h - 1, 0.0f, -0.5f / h, lightgrey, darkgrey);
		for(vector<Menu*>::iterator i = entries.begin() ; i != entries.end() ; ++i)
		{
			if (dynamic_cast<Menu*>(*i))
			{
				if (static_cast<Menu*>(*i)->getWindow())
				{
					highlight = i - entries.begin();
					break;
				}
			}
		}

		int x = 0;
		for(uint32 i = 0 ; i < entries.size() ; ++i)
		{
			const int w = entries[i]->getCaption().size() * 8 + 16;
			if (highlight == int(i))
			{
				roundedgradientbox(target, x + 4, 2, x + w - 5, h - 3, 4, 0.0f, 1.0f / h, grey, darkgrey);
				Font::print(target, x + 8, 4, entries[i]->getCaption(), white);
			}
			else
				Font::print(target, x + 8, 4, entries[i]->getCaption(), black);
			x += w;
		}
	}

	void MenuBar::mouseMoveEvent(SDL_Event *e)
	{
		const int prev = highlight;
		highlight = -1;
		int x = 0;
		for(uint32 i = 0 ; i < entries.size() ; ++i)
		{
			const int w = entries[i]->getCaption().size() * 8 + 16;
			if (e->button.x >= x && e->button.x < x + w)
			{
				highlight = i;
				break;
			}
			x += w;
		}
		if (highlight != prev)
			refresh();
	}

	void MenuBar::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button != SDL_BUTTON_LEFT)
			return;

		const int prev = highlight;
		highlight = -1;
		int x = 0;
		for(uint32 i = 0 ; i < entries.size() ; ++i)
		{
			const int w = entries[i]->getCaption().size() * 8 + 16;
			if (e->button.x >= x && e->button.x < x + w)
			{
				highlight = i;
				break;
			}
			x += w;
		}
		if (highlight >= 0)
		{
			Window *wnd = dynamic_cast<Window*>(getRoot());
			if (wnd)
			{
				for(uint32 i = 0 ; i < entries.size() ; ++i)
					if (int(i) != highlight)
						entries[i]->hide();
				int ax, ay;
				getAbsolutePos(ax, ay);
				entries[highlight]->setPos(x + ax, ay + h);
				wnd->addFloatting(entries[highlight]);
			}
		}
		if (highlight != prev)
			refresh();
	}

	void MenuBar::mouseLeave()
	{
		if (highlight >= 0)
		{
			highlight = -1;
			refresh();
		}
	}
}
