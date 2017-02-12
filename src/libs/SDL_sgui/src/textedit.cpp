#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/textedit.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <limits>

using namespace std;

namespace Gui
{

	TextEdit::TextEdit(const ustring &Name, Widget *parent) : Frame(Name, NULL, parent), CursorRow(0), CursorCol(0)
	{
		shiftrow = shiftcol = 0;
		if (lines.empty())
			lines.push_back(wstring());
		setBackgroundColor(white);
	}

	TextEdit::~TextEdit()
	{
	}

	void TextEdit::setLayout(Layout *layout)
	{
		Widget::setLayout(layout);
	}

	Layout *TextEdit::getLayout()
	{
		return Widget::getLayout();
	}

	void TextEdit::keyPressEvent(SDL_Event *e)
	{
		if (lines.empty())
			lines.push_back(wstring());
		if (CursorRow >= lines.size())
		{
			CursorRow = lines.size() - 1;
			refresh();
		}
		if (CursorCol >= lines[CursorRow].size())
		{
			CursorCol = lines[CursorRow].size();
			refresh();
		}
		switch(e->key.keysym.sym)
		{
		case SDLK_LEFT:
			if (CursorCol > 0)
			{
				--CursorCol;
				if (shiftcol > CursorCol)	shiftcol = CursorCol;
				refresh();
			}
			else if (CursorRow > 0)
			{
				--CursorRow;
				if (shiftrow > CursorRow)	shiftrow = CursorRow;
				CursorCol = lines[CursorRow].size();
				while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
				refresh();
			}
			break;
		case SDLK_RIGHT:
			if (CursorCol < lines[CursorRow].size())
			{
				++CursorCol;
				while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
				refresh();
			}
			else if (CursorRow + 1 < lines.size())
			{
				++CursorRow;
				CursorCol = 0;
				shiftcol = 0;
				while (CursorRow - shiftrow > uint32(h - 26) / 16)	++shiftrow;
				refresh();
			}
			break;
		case SDLK_UP:
			if (CursorRow > 0)
			{
				--CursorRow;
				if (shiftrow > CursorRow)	shiftrow = CursorRow;
				CursorCol = min<uint32>(CursorCol, lines[CursorRow].size());
				if (CursorCol < shiftcol)	shiftcol = CursorCol;
				refresh();
			}
			break;
		case SDLK_DOWN:
			if (CursorRow + 1 < lines.size())
			{
				++CursorRow;
				while (CursorRow - shiftrow > uint32(h - 26) / 16)	++shiftrow;
				CursorCol = min<uint32>(CursorCol, lines[CursorRow].size());
				if (CursorCol < shiftcol)	shiftcol = CursorCol;
				refresh();
			}
			break;
		case SDLK_BACKSPACE:
			if (CursorCol > 0)
			{
				lines[CursorRow].erase(--CursorCol, 1);
				if (shiftcol > CursorCol)	shiftcol = CursorCol;
				refresh();
			}
			else if (CursorRow > 0)
			{
				--CursorRow;
				if (shiftrow > CursorRow)	shiftrow = CursorRow;
				CursorCol = lines[CursorRow].size();
				lines[CursorRow] += lines[CursorRow + 1];
				lines.erase(lines.begin() + CursorRow + 1);
				while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
				refresh();
			}
			break;
		case SDLK_DELETE:
			if (CursorCol < lines[CursorRow].size())
			{
				lines[CursorRow].erase(CursorCol, 1);
				while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
				refresh();
			}
			else if (CursorRow + 1 < lines.size())
			{
				lines[CursorRow] += lines[CursorRow + 1];
				lines.erase(lines.begin() + CursorRow + 1);
				while (CursorRow - shiftrow > uint32(h - 26) / 16)	++shiftrow;
				refresh();
			}
			break;
		case SDLK_END:
			CursorCol = lines[CursorRow].size();
			while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
			refresh();
			break;
		case SDLK_HOME:
			CursorCol = 0;
			shiftcol = 0;
			refresh();
			break;
		case SDLK_RETURN:
			lines.insert(lines.begin() + CursorRow + 1, lines[CursorRow].substr(CursorCol));
			lines[CursorRow] = lines[CursorRow].substr(0, CursorCol);
			CursorCol = 0;
			shiftcol = 0;
			++CursorRow;
			while (CursorRow - shiftrow > uint32(h - 26) / 16)	++shiftrow;
			refresh();
			break;
		case SDLK_ESCAPE:
			break;
		default:
			{
				wchar_t c = e->key.keysym.unicode;
				if (Font::hasGlyph(c))
				{
					lines[CursorRow].insert(CursorCol++, 1, c);
					while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
					refresh();
				}
			}
			break;
		};
	}

	void TextEdit::draw(SDL_Surface *target)
	{
		Frame::draw(target);
		if (lines.empty())
			return;

		uint32 CursorCol = this->CursorCol;
		uint32 CursorRow = this->CursorRow;
		if (CursorRow >= lines.size())
			CursorRow = lines.size() - 1;
		if (CursorCol > lines[CursorRow].size())
			CursorCol = lines[CursorRow].size();

		SDL_Surface *sub = createSubSurface(target, 5, 5, w - 10, h - 10);

		const int maxrows = (h - 10) / 16;
		for(int i = 0 ; i < maxrows ; ++i)
		{
			const uint32 row = i + shiftrow;
			if (row >= lines.size())
				break;

			Font::print(sub, -shiftcol * 8, i * 16, lines[row], black);
			if (bFocus && row == CursorRow)
				Font::print(sub, CursorCol * 8 - shiftcol * 8, i * 16, L"_", black);
		}

		SDL_FreeSurface(sub);
	}

	void TextEdit::gainFocus()
	{
		refresh();
	}

	void TextEdit::loseFocus()
	{
		refresh();
	}

	bool TextEdit::canTakeFocus() const
	{
		return true;
	}

	int TextEdit::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int TextEdit::getOptimalHeight() const
	{
		return numeric_limits<int>::min();
	}

	void TextEdit::resizeEvent()
	{
		if (lines.empty())
			lines.push_back(wstring());
		if (CursorRow >= lines.size())	CursorRow = lines.size() - 1;
		if (CursorCol > lines[CursorRow].size())	CursorCol = lines[CursorRow].size();
		const uint32 wc = (w - 18) / 8;
		const uint32 hc = (h - 26) / 16;
		bool needRefresh = false;
		if (CursorCol - shiftcol > wc)
		{
			shiftcol = CursorCol - wc;
			needRefresh = true;
		}
		if (CursorCol < shiftcol)
		{
			shiftcol = CursorCol;
			needRefresh = true;
		}
		if (CursorRow - shiftrow > hc)
		{
			shiftrow = CursorRow - hc;
			needRefresh = true;
		}
		if (CursorRow < shiftrow)
		{
			shiftrow = CursorRow;
			needRefresh = true;
		}
		if (needRefresh)
			refresh();
	}

	void TextEdit::mousePressEvent(SDL_Event *e)
	{
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.y >= 5 && e->button.y < h - 5
				&& e->button.x >= 5 && e->button.x < w - 5)
			{
				const uint32 rowidx = min<uint32>(lines.size() - 1, (e->button.y - 5) / 16 + shiftrow);
				const uint32 colidx = min<uint32>(lines[rowidx].size(), (e->button.x - 5) / 8 + shiftcol);
				if (colidx != CursorCol || rowidx != CursorRow)
				{
					CursorCol = colidx;
					if (shiftcol > CursorCol)	shiftcol = CursorCol;
					while (CursorCol - shiftcol > uint32(w - 18) / 8)	++shiftcol;
					CursorRow = rowidx;
					if (shiftrow > CursorRow)	shiftrow = CursorRow;
					refresh();
					while (CursorRow - shiftrow > uint32(h - 26) / 16)	++shiftrow;
				}
			}
		}
	}

	void TextEdit::setText(const ustring &text)
	{
		lines.clear();
		wstring buf;
		for(wstring::const_iterator i = text.begin() ; i != text.end() ; ++i)
		{
			if (*i == L'\n')
			{
				lines.push_back(buf);
				buf.clear();
			}
			else
				buf += *i;
		}
		if (!buf.empty())
			lines.push_back(buf);
		if (lines.empty())
			lines.push_back(wstring());
		CursorCol = 0;
		CursorRow = 0;
		shiftcol = shiftrow = 0;
		refresh();
	}

	wstring TextEdit::getText() const
	{
		wstring text;
		for(vector<wstring>::const_iterator i = lines.begin() ; i != lines.end() ; ++i)
		{
			if (!text.empty())
				text += L'\n';
			text += *i;
		}
		return text;
	}

	void TextEdit::mouseEnter()
	{
		SDL_SetCursor(cursor_edit);
	}

	void TextEdit::mouseLeave()
	{
		SDL_SetCursor(cursor_arrow);
	}

}
