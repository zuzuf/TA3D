#ifndef MENU_H
#define MENU_H

#include "floatting.h"

namespace Gui
{
	class Menu : public Floatting
	{
	public:
		Menu(const ustring &Name, const ustring &Caption = ustring(), Widget *parent = NULL);
		virtual ~Menu();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		virtual void setLayout(Layout *layout);

		void addEntry(const ustring &Name, const ustring &Caption);
		void addEntry(const ustring &Name, Menu *menu);
		void addEntry(const ustring &Name, Menu &menu)	{	addEntry(Name, &menu);	}

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void mouseEnter();
		virtual void mouseLeave();
		virtual void onHide();

	private:
		bool bCanBeHidden;

		PROPERTY(ustring, Caption)
	};

	inline Menu &Menu_(const ustring &Name, const ustring &Caption = ustring(), Widget *parent = NULL)
	{
		return *(new Menu(Name, Caption, parent));
	}
}

#define MENU(x)	static_cast<Gui::Menu*>(Gui::Widget::get(#x))

#endif // MENU_H
