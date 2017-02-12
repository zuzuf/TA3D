
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/label.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>

using namespace std;

namespace Gui
{

	Label::Label(const ustring &Name, const ustring &caption, Widget *parent)
		: Widget(Name, parent), caption(caption), side(LEFT), valign(TOP)
	{
		this->h = 16;
		this->w = caption.size() * 8;
		setCaption(caption);
		setColor(black);
	}

	Label::~Label()
	{

	}

	void Label::draw(SDL_Surface *target)
	{
		int y = 0;
		switch(valign)
		{
		case TOP:
			y = 0;
			break;
		case MIDDLE:
			y = (h - 16 * lines.size()) / 2;
			break;
		case BOTTOM:
			y = h - 16 * lines.size();
			break;
		};
		for(uint32 i = 0 ; i < lines.size() ; ++i)
		{
			int x = 0;
			switch(side)
			{
			case LEFT:
				x = 0;
				break;
			case CENTER:
				x = (w - lines[i].size() * 8) / 2;
				break;
			case RIGHT:
				x = w - lines[i].size() * 8;
				break;
			};
			Font::print(target, x, y, lines[i], Color);
			y += 16;
		}
	}

	int Label::getOptimalWidth() const
	{
		return maxchars * 8 + 4;
	}

	int Label::getOptimalHeight() const
	{
		return max<int>(16, 16 * lines.size());
	}

	void Label::setCaption(const ustring &caption)
	{
		this->caption = caption;
		lines.clear();
		wstring buf;
		buf.reserve(caption.size());
		for(uint32 i = 0 ; i < caption.size() ; ++i)
		{
			if (caption[i] ==  L'\n')
			{
				lines.push_back(buf);
				buf.clear();
			}
			else
				buf += caption[i];
		}
		if (!buf.empty())
			lines.push_back(buf);
		maxchars = 0;
		for(uint32 i = 0 ; i < lines.size() ; ++i)
			maxchars = max(maxchars, int(lines[i].size()));
		updateLayout();
	}

}
