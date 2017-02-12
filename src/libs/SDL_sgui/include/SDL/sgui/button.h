#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"
#include "unicode.h"

namespace Gui
{

class Button : public Widget
{
public:
	Button(const ustring &Name, const ustring &caption, CallbackType Callback = NoCallback);
	virtual ~Button();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	virtual void draw(SDL_Surface *target);
	virtual void mousePressEvent(SDL_Event *e);
	virtual void mouseReleaseEvent(SDL_Event *e);
	virtual void mouseLeave();
	virtual void mouseEnter();

private:
	bool state;
	SDL_Surface *buf;
	int ox, oy;
	bool highlight;

	PROPERTY2(ustring, Caption);
	PROPERTY(CallbackType, Callback);
};

inline Button &Button_(const ustring &Name, const ustring &caption = ustring(), CallbackType Callback = NoCallback)
{
	return *(new Button(Name, caption, Callback));
}

}

#define BUTTON(x)	static_cast<Gui::Button*>(Gui::Widget::get(#x))

#endif // BUTTON_H
