
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/types.h>
#include <SDL/sgui/button.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>

namespace Gui
{

	Button::Button(const ustring &Name, const ustring &caption, CallbackType Callback) : Widget(Name), Caption(caption), Callback(Callback)
	{
		state = false;
		buf = NULL;
		ox = oy = -1;
		highlight = false;
	}

	Button::~Button()
	{
		if (buf)
			SDL_FreeSurface(buf);
	}

	void Button::draw(SDL_Surface *target)
	{
		int nx, ny;
		getAbsolutePos(nx, ny);
		if (buf && buf->w == w && buf->h == h && nx == ox && ny == oy)
			blit(buf, target, 0, 0);
		else
		{
			if (buf && (buf->w != w || buf->h != h))
			{
				SDL_FreeSurface(buf);
				buf = NULL;
			}
			if (!buf)
				buf = createNativeSurface(w, h);
			blit(target, buf, 0, 0);
		}
		ox = nx;
		oy = ny;
		const int tx = (w - Caption.size() * 8) / 2;
		const int ty = (h - 16) / 2;
		if (state)
		{
			fillroundedbox(target, 2, 2, w - 1, h - 1, 4, verylightgrey);
			roundedgradientbox(target, 3, 3, w - 2, h - 2, 4, 0.0f, 1.0f / h, verylightgrey, darkgrey);
			Font::print(target, tx + 2, ty + 2, Caption, black);
		}
		else
		{
			fillroundedbox(target, 2, 2, w - 1, h - 1, 4, darkgrey);
			fillroundedbox(target, 0, 0, w - 3, h - 3, 4, highlight ? lightblue : verylightgrey);
			roundedgradientbox(target, 1, 1, w - 4, h - 4, 4, 0.0f, -1.0f / h, verylightgrey, darkgrey);
			Font::print(target, tx, ty, Caption, black);
		}
	}

	void Button::mousePressEvent(SDL_Event *e)
	{
		switch(e->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (!state)
			{
				refresh();
				state = true;
			}
			break;
		};
	}

	void Button::mouseReleaseEvent(SDL_Event *e)
	{
		switch(e->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (state)
			{
				refresh();
				state = false;
				emit();
				if (Callback.first)
					Callback.first(Callback.second);
			}
			break;
		};
	}

	int Button::getOptimalWidth() const
	{
		return 8 + Caption.size() * 8;
	}

	int Button::getOptimalHeight() const
	{
		return 24;
	}

	void Button::mouseLeave()
	{
		if (state)
		{
			refresh();
			state = false;
		}
		if (highlight)
		{
			highlight = false;
			refresh();
		}
	}

	void Button::mouseEnter()
	{
		if (!state && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LEFT))
		{
			refresh();
			state = true;
		}
		if (!highlight)
		{
			highlight = true;
			refresh();
		}
	}

	void Button::onSetCaption()
	{
		updateLayout();
	}

}
