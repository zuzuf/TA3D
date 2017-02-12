
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/scrollbar.h>
#include "arrowdata.h"
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <limits>

using namespace std;

namespace Gui
{

	SDL_Surface *Scrollbar::arrowleft = NULL;
	SDL_Surface *Scrollbar::arrowright = NULL;
	SDL_Surface *Scrollbar::arrowup = NULL;
	SDL_Surface *Scrollbar::arrowdown = NULL;

	Scrollbar::Scrollbar(const ustring &Name, bool orientation, Widget *parent) : Widget(Name, parent), Orientation(orientation)
	{
		Value = 0;
		Minimum = 0;
		Maximum = 10;
		highlight = 0;
		if (!arrowleft)
		{
			arrowleft = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			arrowright = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			arrowup = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			arrowdown = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			const char *data = header_data;
			for(uint32 y = 0 ; y < height ; ++y)
			{
				for(uint32 x = 0 ; x < width ; ++x)
				{
					int c[3];
					HEADER_PIXEL(data, c);
					((byte*)arrowleft->pixels)[y * arrowleft->pitch + x] = (c[0] + c[1] + c[2]) / 3;
					((byte*)arrowright->pixels)[y * arrowright->pitch + arrowright->w - x - 1] = (c[0] + c[1] + c[2]) / 3;
					((byte*)arrowup->pixels)[x * arrowup->pitch + y] = (c[0] + c[1] + c[2]) / 3;
					((byte*)arrowdown->pixels)[(arrowdown->h - x - 1) * arrowdown->pitch + y] = (c[0] + c[1] + c[2]) / 3;
				}
			}
		}
	}

	Scrollbar::~Scrollbar()
	{
	}

	int Scrollbar::getOptimalWidth() const
	{
		return Orientation == Vertical ? 16 : numeric_limits<int>::min();
	}

	int Scrollbar::getOptimalHeight() const
	{
		return Orientation == Horizontal ? 16 : numeric_limits<int>::min();
	}

	void Scrollbar::draw(SDL_Surface *target)
	{
		fill(target, lightgrey);
		if (Orientation == Vertical)
		{
			const int mx = w / 2;
			const int y0 = 0;
			const int y1 = h - 16;
			Font::drawGlyph(arrowup, target, mx - arrowup->w / 2, y0, highlight == 1 ? lightblue : black);
			Font::drawGlyph(arrowdown, target, mx - arrowdown->w / 2, y1, highlight == 2 ? lightblue : black);

			roundedgradientbox(target, 0, 16, w - 1, h - 17, 4, 1.0f / w, 0.0f, grey, darkgrey);
			if (Maximum > Minimum)
			{
				const float pos = float(Value - Minimum) / (Maximum - Minimum);
				const int bh = min<int>(h - 32, max<int>(16, (h - 32) / (Maximum - Minimum + 1)));
				const int rh = h - 32 - bh;
				const int p = int(rh * pos + 0.5f);

				roundedgradientbox(target, 0, 16 + p, w - 1, 16 + p + bh, 4, -1.0f / w, -1.0f / h, grey, verylightgrey);
				roundedgradientbox(target, 1, 17 + p, w - 2, 15 + p + bh, 4, -1.0f / w, 0.0f, lightgrey, grey);
			}
		}
		else
		{
			const int my = h / 2;
			const int x0 = 0;
			const int x1 = w - 16;
			Font::drawGlyph(arrowleft, target, x0, my - arrowleft->h / 2, highlight == 1 ? lightblue : black);
			Font::drawGlyph(arrowright, target, x1, my - arrowright->h / 2, highlight == 2 ? lightblue : black);

			roundedgradientbox(target, 16, 0, w - 17, h - 1, 4, 0.0f, 1.0f / h, grey, darkgrey);
			if (Maximum > Minimum)
			{
				const float pos = float(Value - Minimum) / (Maximum - Minimum);
				const int bw = min<int>(w - 32, max<int>(16, (w - 32) / (Maximum - Minimum + 1)));
				const int rw = w - 32 - bw;
				const int p = int(rw * pos + 0.5f);

				roundedgradientbox(target, 16 + p, 0, 16 + p + bw, h - 1, 4, -1.0f / w, -1.0f / h, grey, verylightgrey);
				roundedgradientbox(target, 17 + p, 1, 15 + p + bw, h - 2, 4, 0.0f, -1.0f / h, lightgrey, grey);
			}
		}
	}

	void Scrollbar::mousePressEvent(SDL_Event *e)
	{
		switch(e->button.button)
		{
		case SDL_BUTTON_WHEELDOWN:
			if (Value < Maximum)
			{
				Value = min<int>(Value + 10, Maximum);
				emit();
				refresh();
			}
			break;
		case SDL_BUTTON_WHEELUP:
			if (Value > Minimum)
			{
				Value = max<int>(Value - 10, Minimum);
				emit();
				refresh();
			}
			break;
		case SDL_BUTTON_LEFT:
			if (Orientation == Vertical)
			{
				if (e->button.y < 16)
				{
					if (highlight != 1)
					{
						highlight = 1;
						refresh();
					}
					if (Value > Minimum)
					{
						--Value;
						emit();
						refresh();
					}
					return;
				}
				if (e->button.y >= h - 16)
				{
					if (highlight != 2)
					{
						highlight = 2;
						refresh();
					}
					if (Value < Maximum)
					{
						++Value;
						emit();
						refresh();
					}
					return;
				}
				if (highlight)
				{
					highlight = 0;
					refresh();
				}
				const float pos = float(Value - Minimum) / (Maximum - Minimum);
				const int bh = min<int>(h - 32, max<int>(16, (h - 32) / (Maximum - Minimum + 1)));
				const int rh = h - 32 - bh;
				const int p = int(rh * pos + 0.5f);
				if (e->button.y < p + 16 || e->button.y > p + 16 + bh)
				{
					if (h - 32 - bh == 0)
						Value = Minimum;
					else
					{
						Value = (e->button.y - 16) * (Maximum - Minimum) / (h - 32 - bh);
						Value = clamp(Value, Minimum, Maximum);
					}
					emit();
					refresh();
				}
			}
			else
			{
				if (e->button.x < 16)
				{
					if (highlight != 1)
					{
						highlight = 1;
						refresh();
					}
					if (Value > Minimum)
					{
						--Value;
						emit();
						refresh();
					}
					return;
				}
				if (e->button.x >= w - 16)
				{
					if (highlight != 2)
					{
						highlight = 2;
						refresh();
					}
					if (Value < Maximum)
					{
						++Value;
						emit();
						refresh();
					}
					return;
				}
				if (highlight)
				{
					highlight = 0;
					refresh();
				}
				const float pos = float(Value - Minimum) / (Maximum - Minimum);
				const int bw = min<int>(w - 32, max<int>(16, (w - 32) / (Maximum - Minimum + 1)));
				const int rw = w - 32 - bw;
				const int p = int(rw * pos + 0.5f);
				if (e->button.x < p + 16 || e->button.x > p + 16 + bw)
				{
					if (w - 32 - bw == 0)
						Value = Minimum;
					else
					{
						Value = (e->button.x - 16) * (Maximum - Minimum) / (w - 32 - bw);
						Value = clamp(Value, Minimum, Maximum);
					}
					emit();
					refresh();
				}
			}
			break;
		};

	}

	void Scrollbar::mouseMoveEvent(SDL_Event *e)
	{
		if (Orientation == Vertical)
		{
			if (e->motion.y < 16 || e->motion.y >= h - 16)
			{
				if (e->motion.y < 16 && highlight != 1)
				{
					highlight = 1;
					refresh();
				}
				else if (e->motion.y >= h - 16 && highlight != 2)
				{
					highlight = 2;
					refresh();
				}
				return;
			}
			if (highlight)
			{
				highlight = 0;
				refresh();
			}
		}
		else
		{
			if (e->motion.x < 16 || e->motion.x >= w - 16)
			{
				if (e->motion.x < 16 && highlight != 1)
				{
					highlight = 1;
					refresh();
				}
				else if (e->motion.x >= w - 16 && highlight != 2)
				{
					highlight = 2;
					refresh();
				}
				return;
			}
			if (highlight)
			{
				highlight = 0;
				refresh();
			}
		}

		if (!(e->motion.state & SDL_BUTTON_LEFT))
			return;
		if (Orientation == Vertical)
		{
			const int bh = min<int>(h - 32, max<int>(16, (h - 32) / (Maximum - Minimum + 1)));
			const int prev = Value;
			if (h - 32 - bh == 0)
				Value = Minimum;
			else
			{
				Value = (e->motion.y - 16 - bh / 2) * (Maximum - Minimum) / (h - 32 - bh);
				Value = clamp(Value, Minimum, Maximum);
			}
			if (Value != prev)
			{
				emit();
				refresh();
			}
		}
		else
		{
			const int bw = min<int>(w - 32, max<int>(16, (w - 32) / (Maximum - Minimum + 1)));
			const int prev = Value;
			if (w - 32 - bw == 0)
				Value = Minimum;
			else
			{
				Value = (e->motion.x - 16 - bw / 2) * (Maximum - Minimum) / (w - 32 - bw);
				Value = clamp(Value, Minimum, Maximum);
			}
			if (Value != prev)
			{
				emit();
				refresh();
			}
		}
	}

	void Scrollbar::mouseLeave()
	{
		if (highlight > 0)
		{
			highlight = 0;
			refresh();
		}
	}

	void Scrollbar::onSetValue()
	{
		Value = clamp<int>(Value, Minimum, Maximum);
	}

	void Scrollbar::onSetMaximum()
	{
		Value = clamp<int>(Value, Minimum, Maximum);
	}

	void Scrollbar::onSetMinimum()
	{
		Value = clamp<int>(Value, Minimum, Maximum);
	}

}
