
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/checkbox.h>
#include <SDL/sgui/font.h>
#include "checkdata.h"
#include <SDL/sgui/renderapi.h>

using namespace std;

namespace Gui
{

	SDL_Surface *CheckBox::check = NULL;

	CheckBox::CheckBox(const ustring &Name, const ustring &Caption, Widget *parent) : Widget(Name, parent), Caption(Caption)
	{
		State = false;
		buf = NULL;
		if (!check)
		{
			check = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			const char *data = header_data;
			for(uint32 y = 0 ; y < height ; ++y)
			{
				for(uint32 x = 0 ; x < width ; ++x)
				{
					int c[3];
					HEADER_PIXEL(data, c);
					((byte*)check->pixels)[y * check->pitch + x] = 0xFF - (c[0] + c[1] + c[2]) / 3;
				}
			}
		}
	}

	CheckBox::~CheckBox()
	{
		if (buf)	SDL_FreeSurface(buf);
	}

	int CheckBox::getOptimalWidth() const
	{
		return 28 + 8 * Caption.size();
	}

	int CheckBox::getOptimalHeight() const
	{
		return 24;
	}

	void CheckBox::draw(SDL_Surface *target)
	{
		fillroundedbox(target, 2, 2, 23, 23, 4, grey);
		fillroundedbox(target, 1, 1, 22, 22, 4, verylightgrey);
		roundedgradientbox(target, 2, 2, 21, 21, 4, 0.0f, -1.0f / 24.0f, verylightgrey, darkgrey);
		if (State)
			Font::drawGlyph(check, target, 0, 0, black);
		if (buf == NULL || uint32(buf->w) != Caption.size() * 8 || compareSurfaces(buf, target, 28, 4))
		{
			if (buf && uint32(buf->w) != Caption.size() * 8)
			{
				SDL_FreeSurface(buf);
				buf = NULL;
			}
			if (!buf)
				buf = createNativeSurface(Caption.size() * 8, 16);
			Font::print(target, 28, 4, Caption, black);
			blit(target, buf, 28, 4, 0, 0, buf->w, buf->h);
		}
	}

	void CheckBox::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.x < 24 && e->button.y < 24)
			{
				State ^= true;
				emit();
				refresh();
				return;
			}
		}
	}

	void CheckBox::onSetCaption()
	{
		updateLayout();
	}

}
