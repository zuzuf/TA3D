#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "widget.h"
#include "unicode.h"

namespace Gui
{

	class CheckBox : public Widget
	{
	public:
		CheckBox(const ustring &Name, const ustring &Caption, Widget *parent = NULL);
		virtual ~CheckBox();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void mousePressEvent(SDL_Event *e);

	private:
		SDL_Surface *buf;
		static SDL_Surface *check;

		PROPERTY2(ustring, Caption);
		PROPERTY(bool, State);
	};

	inline CheckBox &CheckBox_(const ustring &Name, const ustring &Caption, Widget *parent = NULL)
	{
		return *(new CheckBox(Name, Caption, parent));
	}
}

#define CHECKBOX(x)	static_cast<Gui::CheckBox*>(Gui::Widget::get(#x))

#endif // CHECKBOX_H
