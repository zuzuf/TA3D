
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/menu.h>
#include <SDL/sgui/menuentry.h>
#include <SDL/sgui/window.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <SDL/sgui/vboxlayout.h>

using namespace std;

namespace Gui
{

	Menu::Menu(const ustring &Name, const ustring &Caption, Widget *parent) : Floatting(Name, parent), bCanBeHidden(false), Caption(Caption)
	{
		Widget::setLayout(new VBoxLayout);
	}

	Menu::~Menu()
	{
	}

	int Menu::getOptimalWidth() const
	{
		int w = 0;
		for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			w = max<int>(w, (*i)->getOptimalWidth());
		return w;
	}

	int Menu::getOptimalHeight() const
	{
		int h = 0;
		for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			h += max<int>(0, (*i)->getOptimalHeight());
		return h;
	}

	void Menu::draw(SDL_Surface *target)
	{
		const int r = 4;
		fillroundedbox(target, 0, 0, w - 1, h - 1, r, lightgrey);
		roundedbox(target, 0, 0, w - 1, h - 1, r, grey);

		arc(target, r + 1, r + 1, r, 180, 270, white);
		arc(target, w - r - 2, r + 1, r, 270, 370, white);
		arc(target, r + 1, h - r - 2, r, 90, 180, white);
		hline(target, 1, r, w - r - 1, white);
		vline(target, 1, r, h - r - 1, white);
	}

	void Menu::mouseEnter()
	{
		bCanBeHidden = true;
		refresh();
	}

	void Menu::mouseLeave()
	{
		Widget::mouseLeave();
		if (bCanBeHidden)
		{
			bCanBeHidden = false;
			for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
			{
				if (dynamic_cast<MenuEntry*>(*i) && static_cast<MenuEntry*>(*i)->getSubMenu())
				{
					if (static_cast<MenuEntry*>(*i)->getSubMenu()->getWindow())
						return;
				}
			}
			hide();
		}
		refresh();
	}

	void Menu::setLayout(Layout *layout)
	{
		delete layout;
	}

	void Menu::addEntry(const ustring &Name, const ustring &Caption)
	{
		addChild(MenuEntry_(Name, Caption));
	}

	void Menu::addEntry(const ustring &Name, Menu *menu)
	{
		addChild(MenuEntry_(Name, menu));
	}

	void Menu::onHide()
	{
		for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
		{
			if (dynamic_cast<MenuEntry*>(*i) && static_cast<MenuEntry*>(*i)->getSubMenu())
			{
				if (static_cast<MenuEntry*>(*i)->getSubMenu()->getWindow())
					static_cast<MenuEntry*>(*i)->getSubMenu()->hide();
			}
		}
	}
}
