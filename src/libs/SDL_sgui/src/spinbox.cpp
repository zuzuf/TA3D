
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/spinbox.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <sstream>
#include <iomanip>
#include "arrowdata.h"
#include <limits>

using namespace std;

namespace Gui
{

	SDL_Surface *SpinBox::arrowup = NULL;
	SDL_Surface *SpinBox::arrowdown = NULL;

	SpinBox::SpinBox(const ustring &Name, Widget *parent) : Widget(Name, parent)
	{
		Value = 1.0;
		Minimum = 0.0;
		Maximum = 100.0;
		Step = 1.0;
		Precision = 0;
		highlight = 0;

		if (!arrowup)
		{
			arrowup = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			arrowdown = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0xFF, 0);
			const char *data = header_data;
			for(uint32 y = 0 ; y < height ; ++y)
			{
				for(uint32 x = 0 ; x < width ; ++x)
				{
					int c[3];
					HEADER_PIXEL(data, c);
					((byte*)arrowup->pixels)[x * arrowup->pitch + y] = (c[0] + c[1] + c[2]) / 3;
					((byte*)arrowdown->pixels)[(arrowdown->h - x - 1) * arrowdown->pitch + y] = (c[0] + c[1] + c[2]) / 3;
				}
			}
		}
	}

	SpinBox::~SpinBox()
	{
	}

	int SpinBox::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int SpinBox::getOptimalHeight() const
	{
		return 24;
	}

	void SpinBox::draw(SDL_Surface *target)
	{
		const int r = 4;
		fillroundedbox(target, 0, 0, w - 1, h - 1, r, white);
		roundedbox(target, 0, 0, w - 1, h - 1, r, grey);
		arc(target, r, r + 1, r, 180, 270, grey);
		arc(target, w - r - 1, r + 1, r, 270, 360, grey);
		arc(target, r, h - r - 2, r, 90, 180, grey);
		hline(target, h - 1, r + 1, w - 1 - r, verylightgrey);
		vline(target, w - 1, r + 1, h - 1 - r, verylightgrey);
		arc(target, w - 1 - r, h - 1 - r, r, 0, 90, verylightgrey);

		roundedbox(target, 1, 1, w - 2, h - 2, r, darkgrey);
		hline(target, h - 2, 2 + r, w - 2 - r, white);
		vline(target, w - 2, 2 + r, h - 2 - r, white);
		arc(target, w - 2 - r, h - 2 - r, r, 0, 90, white);

		Font::drawGlyph(arrowup, target, w - 18, -3, highlight == 1 ? lightblue : black);
		Font::drawGlyph(arrowdown, target, w - 18, h - 14, highlight == 2 ? lightblue : black);

		wstringstream str;
		str << std::setprecision(Precision) << std::setiosflags(ios_base::fixed) << Value;
		Font::print(target, 8, (h - 16) / 2, str.str(), black);
	}

	void SpinBox::keyPressEvent(SDL_Event *e)
	{
		switch(e->key.keysym.sym)
		{
		case SDLK_UP:
			if (Value < Maximum)
			{
				Value = clamp(Value + Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		case SDLK_DOWN:
			if (Value > Minimum)
			{
				Value = clamp(Value - Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		case SDLK_PAGEUP:
			if (Value < Maximum)
			{
				Value = clamp(Value + 10.0 * Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		case SDLK_PAGEDOWN:
			if (Value > Minimum)
			{
				Value = clamp(Value - 10.0 * Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		default:
			break;
		}
	}

	void SpinBox::mousePressEvent(SDL_Event *e)
	{
		if (e->button.x >= w - 16 && e->button.x <= w - 4)
		{
			if (e->button.y >= 2 && e->button.y <= 12)
			{
				if (highlight != 1)
				{
					highlight = 1;
					refresh();
				}
			}
			else if (e->button.y < h - 2 && e->button.y >= h - 11)
			{
				if (highlight != 2)
				{
					highlight = 2;
					refresh();
				}
			}
			else if (highlight)
			{
				highlight = 0;
				refresh();
			}
		}
		else if (highlight)
		{
			highlight = 0;
			refresh();
		}
		switch (e->button.button)
		{
		case SDL_BUTTON_LEFT:
			switch(highlight)
			{
			case 1:
				Value = clamp(Value + Step, Minimum, Maximum);
				emit();
				refresh();
				break;
			case 2:
				Value = clamp(Value - Step, Minimum, Maximum);
				emit();
				refresh();
				break;
			};
			break;
		case SDL_BUTTON_WHEELDOWN:
			if (e->button.x < w - 16 && e->button.x >= 2
				&& e->button.y >= 2 && e->button.y <= h - 2
				&& Value > Minimum)
			{
				Value = clamp(Value - Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		case SDL_BUTTON_WHEELUP:
			if (e->button.x < w - 16 && e->button.x >= 2
				&& e->button.y >= 2 && e->button.y <= h - 2
				&& Value < Maximum)
			{
				Value = clamp(Value + Step, Minimum, Maximum);
				emit();
				refresh();
			}
			break;
		};
	}

	void SpinBox::mouseMoveEvent(SDL_Event *e)
	{
		if (e->motion.x >= w - 16 && e->motion.x <= w - 4)
		{
			if (e->motion.y >= 2 && e->motion.y <= 12)
			{
				if (highlight != 1)
				{
					highlight = 1;
					refresh();
				}
				return;
			}
			if (e->motion.y < h - 2 && e->motion.y >= h - 11)
			{
				if (highlight != 2)
				{
					highlight = 2;
					refresh();
				}
				return;
			}
		}
		if (highlight)
		{
			highlight = 0;
			refresh();
		}
	}

	void SpinBox::mouseLeave()
	{
		if (highlight)
		{
			highlight = 0;
			refresh();
		}
	}

	void SpinBox::onSetMaximum()
	{
		Value = clamp(Value, Minimum, Maximum);
	}

	void SpinBox::onSetMinimum()
	{
		Value = clamp(Value, Minimum, Maximum);
	}

	void SpinBox::onSetValue()
	{
		Value = clamp(Value, Minimum, Maximum);
	}

	bool SpinBox::canTakeFocus() const
	{
		return true;
	}

}
