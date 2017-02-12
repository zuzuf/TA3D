#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "widget.h"

namespace Gui
{

class ProgressBar : public Widget
{
public:
	ProgressBar(const ustring &Name, Widget *parent = NULL);
	virtual ~ProgressBar();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	virtual void draw(SDL_Surface *target);

private:
	PROPERTY2(int, Value);
};

inline ProgressBar &ProgressBar_(const ustring &Name, Widget *parent = NULL)
{
	return *(new ProgressBar(Name, parent));
}

}

#define PROGRESSBAR(x)	static_cast<Gui::ProgressBar*>(Gui::Widget::get(#x))

#endif // PROGRESSBAR_H
