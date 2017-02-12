
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/menuentry.h>
#include <SDL/sgui/menu.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/window.h>
#include <SDL/sgui/renderapi.h>

using namespace std;

namespace Gui
{

	MenuEntry::MenuEntry(const ustring &Name, const ustring &Caption, Widget *parent) : Widget(Name, parent), Caption(Caption), SubMenu(NULL)
	{
		bHighlight = false;
	}

	MenuEntry::MenuEntry(const ustring &Name, Menu *SubMenu, Widget *parent) : Widget(Name, parent), Caption(), SubMenu(SubMenu)
	{
		bHighlight = false;
	}

	MenuEntry::~MenuEntry()
	{
		if (SubMenu)
			delete SubMenu;
		SubMenu = NULL;
	}

	int MenuEntry::getOptimalWidth() const
	{
		if (SubMenu)
			return 32 + SubMenu->getCaption().size() * 8;
		return 16 + Caption.size() * 8;
	}

	int MenuEntry::getOptimalHeight() const
	{
		if (SubMenu == NULL && Caption.empty())
			return 12;
		return 24;
	}

	void MenuEntry::draw(SDL_Surface *target)
	{
		if (SubMenu == NULL && Caption.empty())
		{
			const int mx = w / 2;
			for(int x = 8 ; x < w - 8 ; ++x)
			{
				int alpha = max<int>(0, 255 - abs(x - mx) * 255 / ((w - 16) / 2));
				uint32 c = getpixel(target, x, h / 2 - 1);
				Uint8 r, g, b;
				SDL_GetRGB(c, target->format, &r, &g, &b);
				r = (r * (255 - alpha) + alpha * 127) / 255;
				g = (g * (255 - alpha) + alpha * 127) / 255;
				b = (b * (255 - alpha) + alpha * 127) / 255;
				putpixel(target, x, h / 2 - 1, SDL_MapRGB(target->format, r, g, b));
				c = getpixel(target, x, h / 2);
				SDL_GetRGB(c, target->format, &r, &g, &b);
				r = (r * (255 - alpha) + alpha * 255) / 255;
				g = (g * (255 - alpha) + alpha * 255) / 255;
				b = (b * (255 - alpha) + alpha * 255) / 255;
				putpixel(target, x, h / 2, SDL_MapRGB(target->format, r, g, b));
			}
		}
		else
		{
			if (bHighlight)
				roundedgradientbox(target, 2, 2, w - 3, h - 3, 4, 0.0f, 1.0f / h, grey, darkgrey);
			if (SubMenu)
			{
				Font::print(target, 8, 4, SubMenu->getCaption(), black);
				Font::print(target, w - 16, 4, ">", black);
			}
			else
				Font::print(target, 8, 4, Caption, black);
		}
	}

	void MenuEntry::mouseEnter()
	{
		if (!bHighlight)
		{
			bHighlight = true;
			refresh();
		}
	}

	void MenuEntry::mouseLeave()
	{
		if (bHighlight)
		{
			bHighlight = false;
			refresh();
		}
	}

	void MenuEntry::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			emit();
			Window *wnd = dynamic_cast<Window*>(getRoot());
			if (SubMenu && wnd)
			{
				int ax, ay;
				getAbsolutePos(ax, ay);
				SubMenu->setPos(w + ax, ay);
				wnd->addFloatting(SubMenu);
			}
		}
	}

	void MenuEntry::onSetCaption()
	{
		if (SubMenu)
		{
			delete SubMenu;
			SubMenu = NULL;
		}
	}

	void MenuEntry::onSetSubMenu()
	{
		Caption.clear();
	}
}
