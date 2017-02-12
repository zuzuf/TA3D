#ifndef MENUENTRY_H
#define MENUENTRY_H

#include "widget.h"

namespace Gui
{
	class Menu;

	class MenuEntry : public Widget
	{
	public:
		MenuEntry(const ustring &Name, const ustring &Caption, Widget *parent = NULL);
		MenuEntry(const ustring &Name, Menu *SubMenu, Widget *parent = NULL);
		virtual ~MenuEntry();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void mouseEnter();
		virtual void mouseLeave();
		virtual void mousePressEvent(SDL_Event *e);
		virtual void addChild(Widget &)	{}
		virtual void addChild(Widget *)	{}
		virtual void remove(Widget *)	{}

	private:
		bool bHighlight;

		PROPERTY2(ustring, Caption)
		PROPERTY2P(Menu*, SubMenu)
	};

	inline MenuEntry &MenuEntry_(const ustring &Name, const ustring &Caption, Widget *parent = NULL)
	{
		return *(new MenuEntry(Name, Caption, parent));
	}

	inline MenuEntry &MenuEntry_(const ustring &Name, Menu *SubMenu, Widget *parent = NULL)
	{
		return *(new MenuEntry(Name, SubMenu, parent));
	}

}

#define MENUENTRY(x)	static_cast<Gui::MenuEntry*>(Gui::Widget::get(#x))

#endif // MENUENTRY_H
