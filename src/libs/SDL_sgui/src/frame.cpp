
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/frame.h>
#include <SDL/sgui/renderapi.h>
#include <limits>

using namespace std;


namespace Gui
{

	Frame::Frame(const ustring &Name, Widget *centralWidget, Widget *parent) : Widget(Name, parent)
	{
		if (centralWidget)
			addChild(centralWidget);
		else
			addChild(new Widget(L""));
		setBackgroundColor(lightgrey);
	}

	Frame::~Frame()
	{
	}

	void Frame::setCentralWidget(Widget *widget)
	{
		set<Widget*> ch = childs;
		for(set<Widget*>::iterator i = ch.begin() ; i != ch.end() ; ++i)
			remove(*i);
		addChild(widget);
	}

	int Frame::getOptimalWidth() const
	{
		if (childs.empty())
			return 10;
		const int ow = (*childs.begin())->getOptimalWidth();
		return ow <= -1 ? numeric_limits<int>::min() : 10 + ow;
	}

	int Frame::getOptimalHeight() const
	{
		if (childs.empty())
			return 10;
		const int oh = (*childs.begin())->getOptimalHeight();
		return oh <= -1 ? numeric_limits<int>::min() : 10 + oh;
	}

	void Frame::updateLayout()
	{
		if (childs.empty())
			return;
		getCentralWidget()->setPos(5, 5);
		if (getCentralWidget()->getWidth() != w - 10 || getCentralWidget()->getHeight() != h - 10)
			getCentralWidget()->resize(w - 10, h - 10);
		else
			getCentralWidget()->updateLayout();
	}

	void Frame::draw(SDL_Surface *target)
	{
		const int r = 4;
		fillroundedbox(target, 0, 0, w - 1, h - 1, r, BackgroundColor);
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
	}

	Widget *Frame::getCentralWidget()
	{
		if (childs.empty())
			return NULL;
		return *childs.begin();
	}

	void Frame::setLayout(Layout *layout)
	{
		getCentralWidget()->setLayout(layout);
	}

	Layout *Frame::getLayout()
	{
		return getCentralWidget()->getLayout();
	}

}
