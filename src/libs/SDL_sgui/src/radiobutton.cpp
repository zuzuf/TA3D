
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/radiobutton.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>


using namespace std;


namespace Gui
{

	RadioButton::RadioButton(const ustring &Name, const ustring &Caption, Widget *parent) : Widget(Name, parent), Caption(Caption)
	{
		State = false;
		buf = NULL;
	}

	RadioButton::~RadioButton()
	{
		if (buf)
			SDL_FreeSurface(buf);
	}

	int RadioButton::getOptimalWidth() const
	{
		return 28 + 8 * Caption.size();
	}

	int RadioButton::getOptimalHeight() const
	{
		return 24;
	}

	void RadioButton::draw(SDL_Surface *target)
	{
		fillcircle(target, 13, 13, 10, grey);
		fillcircle(target, 12, 12, 10, verylightgrey);
		gradientcircle(target, 12, 12, 9, 1.0f / 48.0f, -1.0f / 48.0f, verylightgrey, darkgrey);
		if (State)
		{
			fillcircle(target, 12, 12, 4, grey);
			fillcircle(target, 12, 12, 3, black);
		}
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

	void RadioButton::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.x < 24 && e->button.y < 24)
			{
				State ^= true;
				if (State)
				{
					vector<Widget*> group = getGroup();
					for(vector<Widget*>::iterator i = group.begin() ; i != group.end() ; ++i)
					{
						RadioButton *p = dynamic_cast<RadioButton*>(*i);
						if (p && p != this && p->getState())
							p->setState(false);
					}
				}
				emit();
				refresh();
				return;
			}
		}
	}

	void RadioButton::onSetCaption()
	{
		updateLayout();
	}

}
