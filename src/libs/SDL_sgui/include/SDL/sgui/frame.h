#ifndef FRAME_H
#define FRAME_H

#include "widget.h"

namespace Gui
{

class Frame : public Widget
{
	friend class ListBox;
public:
	Frame(const ustring &Name, Widget *centralWidget = NULL, Widget *parent = NULL);
	virtual ~Frame();

	void setCentralWidget(Widget *widget);
	inline void setCentralWidget(Widget &widget)	{	setCentralWidget(&widget);	}
	Widget *getCentralWidget();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

	virtual void updateLayout();

	virtual void setLayout(Layout *layout);
	virtual Layout *getLayout();

protected:
	virtual void draw(SDL_Surface *target);

	PROPERTY(uint32, BackgroundColor);
};

inline Frame &Frame_(const ustring &Name, Widget *centralWidget = NULL, Widget *parent = NULL)
{
	return *(new Frame(Name, centralWidget, parent));
}

inline Frame &Frame_(const ustring &Name, Widget &centralWidget, Widget *parent = NULL)
{
	return *(new Frame(Name, &centralWidget, parent));
}

}

#define FRAME(x)	static_cast<Gui::Frame*>(Gui::Widget::get(L""#x))

#endif // FRAME_H
