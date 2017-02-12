#ifndef PICTURE_H
#define PICTURE_H

#include "widget.h"

namespace Gui
{

class Picture : public Widget
{
	enum { LEFT = 0, CENTER = 1, RIGHT = 2, TOP = 3, BOTTOM = 4 };
public:
	Picture(const ustring &Name, SDL_Surface *pic = NULL, Widget *parent = NULL);
	virtual ~Picture();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

	void setPicture(SDL_Surface *pic);

protected:
	virtual void draw(SDL_Surface *target);

private:
	SDL_Surface *pic;

	PROPERTY(int, Alignment);
	PROPERTY(int, VAlignment);
};

inline Picture &Picture_(const ustring &Name, SDL_Surface *pic = NULL, Widget *parent = NULL)
{
	return *(new Picture(Name, pic, parent));
}

}

#define PICTURE(x)	static_cast<Gui::Picture*>(Gui::Widget::get(#x))

#endif // PICTURE_H
