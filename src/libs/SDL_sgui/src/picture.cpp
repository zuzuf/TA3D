
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/picture.h>
#include <SDL/sgui/renderapi.h>

namespace Gui
{

	Picture::Picture(const ustring &Name, SDL_Surface *pic, Widget *parent) : Widget(Name, parent), pic(pic)
	{
		Alignment = CENTER;
		VAlignment = CENTER;
	}

	Picture::~Picture()
	{
		if (pic)
			SDL_FreeSurface(pic);
		pic = NULL;
	}

	void Picture::draw(SDL_Surface *target)
	{
		if (pic)
		{
			int x = 0;
			switch(Alignment)
			{
			case LEFT:		x = 0;	break;
			case CENTER:	x = (w - pic->w - 1) / 2;	break;
			case RIGHT:		x = w - pic->w - 1;	break;
			};
			int y = 0;
			switch(VAlignment)
			{
			case TOP:		y = 0;	break;
			case CENTER:	y = (h - pic->h - 1) / 2;	break;
			case BOTTOM:	y = h - pic->h - 1;	break;
			};

			blit(pic, target, x, y);
		}
	}

	void Picture::setPicture(SDL_Surface *pic)
	{
		if (this->pic)
			SDL_FreeSurface(this->pic);
		this->pic = pic;
		refresh();
	}

	int Picture::getOptimalWidth() const
	{
		if (pic)
			return pic->w;
		return 0;
	}

	int Picture::getOptimalHeight() const
	{
		if (pic)
			return pic->h;
		return 0;
	}

}
