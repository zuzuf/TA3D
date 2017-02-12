
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/lineinput.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <limits>

using namespace std;

namespace Gui
{

	LineInput::LineInput(const ustring &Name, Widget *parent) : Frame(Name, NULL, parent), CursorPos(0)
	{
		Password = false;
		shift = 0;
		setBackgroundColor(white);
	}

	LineInput::~LineInput()
	{
	}

	void LineInput::setLayout(Layout *layout)
	{
		Widget::setLayout(layout);
	}

	Layout *LineInput::getLayout()
	{
		return Widget::getLayout();
	}

	void LineInput::keyPressEvent(SDL_Event *e)
	{
		if (CursorPos > Text.size())
		{
			CursorPos = Text.size();
			refresh();
		}
		switch(e->key.keysym.sym)
		{
		case SDLK_LEFT:
			if (CursorPos > 0)
			{
				CursorPos--;
				if (shift > CursorPos)
					shift = CursorPos;
				refresh();
			}
			break;
		case SDLK_RIGHT:
			if (CursorPos < Text.size())
			{
				++CursorPos;
				while (CursorPos - shift > uint32(w - 18) / 8)
					++shift;
				refresh();
			}
			break;
		case SDLK_BACKSPACE:
			if (CursorPos > 0)
			{
				Text.erase(--CursorPos, 1);
				if (shift > CursorPos)
					shift = CursorPos;
				refresh();
			}
			break;
		case SDLK_DELETE:
			if (CursorPos < Text.size())
			{
				Text.erase(CursorPos, 1);
				refresh();
			}
			break;
		case SDLK_END:
			CursorPos = Text.size();
			while (CursorPos - shift > uint32(w - 18) / 8)
				++shift;
			refresh();
			break;
		case SDLK_HOME:
			CursorPos = 0;
			shift = 0;
			refresh();
			break;
		case SDLK_ESCAPE:
		case SDLK_UP:
		case SDLK_DOWN:
			break;
		case SDLK_RETURN:
			emit();
			break;
		default:
			{
				wchar_t c = e->key.keysym.unicode;
				if (Font::hasGlyph(c))
				{
					Text.insert(CursorPos++, 1, c);
					while (CursorPos - shift > uint32(w - 18) / 8)
						++shift;
					refresh();
				}
			}
			break;
		};
	}

	void LineInput::draw(SDL_Surface *target)
	{
		Frame::draw(target);
		uint32 CursorPos = this->CursorPos;
		if (CursorPos > Text.size())
			CursorPos = Text.size();

		SDL_Surface *sub = createSubSurface(target, 5, 5, w - 10, h - 10);

		if (Password)
			Font::print(sub, -shift * 8, 0, string(Text.size(), '*'), black);
		else
			Font::print(sub, -shift * 8, 0, Text, black);
		if (bFocus)
			Font::print(sub, CursorPos * 8 - shift * 8, 0, "_", black);

		SDL_FreeSurface(sub);
	}

	void LineInput::gainFocus()
	{
		refresh();
	}

	void LineInput::loseFocus()
	{
		refresh();
	}

	bool LineInput::canTakeFocus() const
	{
		return true;
	}

	int LineInput::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int LineInput::getOptimalHeight() const
	{
		return 26;
	}

	void LineInput::resizeEvent()
	{
		if (CursorPos > Text.size())
			CursorPos = Text.size();
		const uint32 wc = (w - 18) / 8;
		if (CursorPos - shift > wc)
		{
			shift = CursorPos - wc;
			refresh();
			return;
		}
		if (CursorPos < shift)
		{
			shift = CursorPos;
			refresh();
			return;
		}
	}

	void LineInput::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.y >= 5 && e->button.y < h - 5
				&& e->button.x >= 5 && e->button.x < w - 5)
			{
				const uint32 idx = min<uint32>(Text.size(), (e->button.x - 5) / 8 + shift);
				if (idx != CursorPos)
				{
					CursorPos = idx;
					refresh();
				}
			}
		}
	}

	void LineInput::mouseEnter()
	{
		SDL_SetCursor(cursor_edit);
	}

	void LineInput::mouseLeave()
	{
		SDL_SetCursor(cursor_arrow);
	}

}
