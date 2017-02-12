#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include "widget.h"
#include "receiver.h"

namespace Gui
{
	class Scrollbar;
	class Frame;

	class ScrollArea : public Widget, public Receiver
	{
	public:
		ScrollArea(const ustring &Name, Widget *parent = NULL);
		virtual ~ScrollArea();

		void setCentralWidget(Widget *widget);
		inline void setCentralWidget(Widget &widget)	{	setCentralWidget(&widget);	}
		Widget *getCentralWidget();

		uint32 getBackgroundColor() const;
		void setBackgroundColor(uint32 c);

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		virtual void updateLayout();

		virtual void setLayout(Layout *layout);
		virtual Layout *getLayout();

	protected:
		virtual void proc(Widget *widget);
		virtual void mousePressEvent(SDL_Event *e);

	protected:
		Scrollbar *vscroll;
		Scrollbar *hscroll;
		Frame *frame;

		PROPERTY2(int, ShiftX);
		PROPERTY2(int, ShiftY);
	};

	inline ScrollArea &ScrollArea_(const ustring &Name, Widget *parent = NULL)
	{
		return *(new ScrollArea(Name, parent));
	}
}

#define SCROLLAREA(x)	static_cast<Gui::ScrollArea*>(Gui::Widget::get(#x))

#endif // SCROLLAREA_H
