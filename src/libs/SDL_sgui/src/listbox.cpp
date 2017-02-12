
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/listbox.h>
#include <SDL/sgui/frame.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <limits>

using namespace std;

namespace Gui
{

	ListBox::ListBox(const ustring &Name, Widget *parent) : Widget(Name, parent)
	{
		Selection = -1;
		shift = 0;
		scrollbar = new Scrollbar(L"", Scrollbar::Vertical);
		frame = new Frame(L"");
		frame->setBackgroundColor(white);
	}

	ListBox::~ListBox()
	{
		delete frame;
		delete scrollbar;
	}

	void ListBox::draw(SDL_Surface *target)
	{
		const uint32 maxrows = (h - 10) / 16;

		frame->resize(w - 17, h);
		scrollbar->resize(16, h);

		scrollbar->setMinimum(0);
		scrollbar->setMaximum(max<int>(elements.size() - maxrows, 0));
		scrollbar->setValue(shift);

		SDL_Surface *tframe = createSubSurface(target, 0, 0, w - 17, h);
		frame->draw(tframe);
		for(uint32 i = 0 ; i < maxrows ; ++i)
		{
			const uint32 idx = i + shift;
			if (idx >= elements.size())
				continue;
			if (int(idx) == Selection)
			{
				fillroundedbox(tframe, 5, 5 + i * 16, w - 22, 21 + i * 16, 4, blue);
				Font::print(tframe, 10, 5 + i * 16, elements[idx], white);
			}
			else
				Font::print(tframe, 10, 5 + i * 16, elements[idx], black);
		}
		SDL_FreeSurface(tframe);

		SDL_Surface *tscroll = createSubSurface(target, w - 16, 0, 16, h);
		scrollbar->draw(tscroll);
		SDL_FreeSurface(tscroll);
	}

	void ListBox::addElement(const ustring &elt)
	{
		elements.push_back(elt);
		refresh();
	}

	void ListBox::insertElement(int i, const ustring &elt)
	{
		if (i < 0 || i > (int)elements.size())
			return;
		if (Selection >= i)
		{
			++Selection;
			while (Selection - shift > uint32((h - 26) / 16))	--shift;
		}
		elements.insert(elements.begin() + i, elt);
		refresh();
	}

	void ListBox::removeElement(int i)
	{
		if (i < 0 || i >= int(elements.size()))
			return;
		if (Selection == i)
			Selection = -1;
		else if (Selection > i)
		{
			--Selection;
			if (Selection < int(shift))
				shift = Selection;
		}
		elements.erase(elements.begin() + i);
		refresh();
	}

	const wstring &ListBox::getElement(int i) const
	{
		if (i < 0 || i >= int(elements.size()))
		{
			static const wstring null;
			return null;
		}
		return elements[i];
	}

	void ListBox::mousePressEvent(SDL_Event *e)
	{
		frame->resize(w - 17, h);
		scrollbar->resize(16, h);
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.x >= 5 && e->button.x < w - 16 && e->button.y >= 5 && e->button.y < h - 5)
			{
				const int idx = min<int>((e->button.y - 5) / 16 + shift, elements.size() - 1);
				if (idx != Selection)
				{
					Selection = idx;
					emit();
					refresh();
					return;
				}
			}
			else if (e->button.x >= w - 16)
			{
				const uint32 maxrows = (h - 10) / 16;
				scrollbar->setMinimum(0);
				scrollbar->setMaximum(max<int>(elements.size() - maxrows, 0));
				scrollbar->setValue(shift);
				SDL_Event ev = *e;
				ev.button.x -= w - 16;
				scrollbar->mousePressEvent(&ev);
				if (int(shift) != scrollbar->getValue())
				{
					shift = scrollbar->getValue();
					refresh();
				}
			}
			else
			{
				scrollbar->mouseLeave();
				if (scrollbar->bRefresh)
					refresh();
			}
		}
		else if (e->button.x >= w - 16)
		{
			SDL_Event ev = *e;
			ev.button.x -= w - 16;
			scrollbar->mousePressEvent(&ev);
			if (scrollbar->bRefresh)
				refresh();
		}
		else
		{
			scrollbar->mouseLeave();
			if (scrollbar->bRefresh)
				refresh();
		}
	}

	void ListBox::mouseMoveEvent(SDL_Event *e)
	{
		scrollbar->resize(16, h);
		if (e->button.button == SDL_BUTTON_LEFT)
		{
			if (e->button.x >= w - 16)
			{
				const uint32 maxrows = (h - 10) / 16;
				scrollbar->setMinimum(0);
				scrollbar->setMaximum(max<int>(elements.size() - maxrows, 0));
				scrollbar->setValue(shift);
				SDL_Event ev = *e;
				ev.motion.x -= w - 16;
				scrollbar->mouseMoveEvent(&ev);
				if (int(shift) != scrollbar->getValue())
				{
					shift = scrollbar->getValue();
					refresh();
				}
			}
			else
			{
				scrollbar->mouseLeave();
				if (scrollbar->bRefresh)
					refresh();
			}
		}
		else if (e->button.x >= w - 16)
		{
			SDL_Event ev = *e;
			ev.motion.x -= w - 16;
			scrollbar->mouseMoveEvent(&ev);
			if (scrollbar->bRefresh)
				refresh();
		}
		else
		{
			scrollbar->mouseLeave();
			if (scrollbar->bRefresh)
				refresh();
		}
	}

	int ListBox::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int ListBox::getOptimalHeight() const
	{
		return numeric_limits<int>::min();
	}

	void ListBox::mouseLeave()
	{
		scrollbar->mouseLeave();
	}

}
