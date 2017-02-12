#ifndef MENUBAR_H
#define MENUBAR_H

#include "widget.h"
#include <vector>

namespace Gui
{
	class Window;
	class Menu;

	class MenuBar: public Widget
	{
		friend class Window;
	public:
		MenuBar(const ustring &Name, Window *wnd = NULL);
		virtual ~MenuBar();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		virtual void setLayout(Layout *layout);

		virtual void addMenu(Menu *menu);
		inline void addMenu(Menu &menu)	{	addMenu(&menu);	}
		virtual void remove(Menu *);

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void mouseMoveEvent(SDL_Event *e);
		virtual void mousePressEvent(SDL_Event *e);
		virtual void mouseLeave();
		virtual void addChild(Widget &)	{}
		virtual void addChild(Widget *)	{}
		virtual void remove(Widget *)	{}

	private:
		int highlight;
		std::vector<Menu*> entries;
	};

	inline MenuBar &MenuBar_(const ustring &Name, Window *parent = NULL)
	{
		return *(new MenuBar(Name, parent));
	}

}

#define MENUBAR(x)	static_cast<Gui::MenuBar*>(Gui::Widget::get(#x))

#endif // MENUBAR_H
