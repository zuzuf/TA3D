
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/scrollarea.h>
#include <SDL/sgui/scrollarea.h>
#include <SDL/sgui/scrollbar.h>
#include <SDL/sgui/frame.h>
#include <limits>

using namespace std;

namespace Gui
{

	ScrollArea::ScrollArea(const ustring &Name, Widget *parent) : Widget(Name, parent)
	{
		ShiftX = 0;
		ShiftY = 0;
		bUpdatingLayout = true;
		frame = new Frame(L"", new Widget(L""), this);
		vscroll = new Scrollbar(L"", Scrollbar::Vertical, this);
		hscroll = new Scrollbar(L"", Scrollbar::Horizontal, this);
		vscroll->setMinimum(0);
		hscroll->setMinimum(0);
		LINK(vscroll, this);
		LINK(hscroll, this);

		bUpdatingLayout = false;
		setCentralWidget(Widget_(L""));
	}

	ScrollArea::~ScrollArea()
	{

	}

	int ScrollArea::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	int ScrollArea::getOptimalHeight() const
	{
		return numeric_limits<int>::min();
	}

	void ScrollArea::setCentralWidget(Widget *widget)
	{
		set<Widget*> ch = frame->getCentralWidget()->childs;
		for(set<Widget*>::iterator i = ch.begin() ; i != ch.end() ; ++i)
			frame->getCentralWidget()->remove(*i);
		frame->getCentralWidget()->addChild(widget);
	}

	Widget *ScrollArea::getCentralWidget()
	{
		if (frame->getCentralWidget()->childs.empty())
			return NULL;
		return *frame->getCentralWidget()->childs.begin();
	}

	void ScrollArea::setLayout(Layout *layout)
	{
		getCentralWidget()->setLayout(layout);
	}

	Layout *ScrollArea::getLayout()
	{
		return getCentralWidget()->getLayout();
	}

	uint32 ScrollArea::getBackgroundColor() const
	{
		return frame->getBackgroundColor();
	}

	void ScrollArea::setBackgroundColor(uint32 c)
	{
		frame->setBackgroundColor(c);
	}

	void ScrollArea::updateLayout()
	{
		if (bUpdatingLayout)
			return;

		bUpdatingLayout = true;

		const int ow = getCentralWidget()->getOptimalWidth() + 27;
		const int oh = getCentralWidget()->getOptimalHeight() + 27;

		if (frame->getWidth() != w - 17 || frame->getHeight() != h - 17)
			frame->resize(w - 17, h - 17);
		else
			frame->updateLayout();
		vscroll->setMaximum(max<int>(0, oh - h));
		hscroll->setMaximum(max<int>(0, ow - w));
		vscroll->setPos(w - 17, 0);
		vscroll->resize(16, h - 16);
		hscroll->setPos(0, h - 17);
		hscroll->resize(w - 16, 16);
		ShiftX = hscroll->getValue();
		ShiftY = vscroll->getValue();
		getCentralWidget()->setPos(-ShiftX, -ShiftY);

		if (parent)
			parent->updateLayout();

		bUpdatingLayout = false;
	}

	void ScrollArea::proc(Widget *widget)
	{
		if (widget == vscroll)
		{
			setShiftY(vscroll->getValue());
		}
		else if (widget == hscroll)
		{
			setShiftX(hscroll->getValue());
		}
	}

	void ScrollArea::onSetShiftX()
	{
		hscroll->setValue(ShiftX);
		ShiftX = hscroll->getValue();
		getCentralWidget()->setPos(-ShiftX, -ShiftY);
	}

	void ScrollArea::onSetShiftY()
	{
		vscroll->setValue(ShiftY);
		ShiftY = vscroll->getValue();
		getCentralWidget()->setPos(-ShiftX, -ShiftY);
	}

	void ScrollArea::mousePressEvent(SDL_Event *e)
	{
		switch(e->button.button)
		{
		case SDL_BUTTON_WHEELDOWN:
		case SDL_BUTTON_WHEELUP:
			const int ow = getCentralWidget()->getOptimalWidth() + 27;
			const int oh = getCentralWidget()->getOptimalHeight() + 27;
			vscroll->setMaximum(max<int>(0, oh - h));
			hscroll->setMaximum(max<int>(0, ow - w));
			vscroll->setPos(w - 17, 0);
			vscroll->resize(16, h - 16);
			hscroll->setPos(0, h - 17);
			hscroll->resize(w - 16, 16);
			vscroll->mousePressEvent(e);
			ShiftX = hscroll->getValue();
			ShiftY = vscroll->getValue();
			getCentralWidget()->setPos(-ShiftX, -ShiftY);
			break;
		};
	}
}
