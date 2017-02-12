#ifndef FLOATTING_H
#define FLOATTING_H

#include "widget.h"

namespace Gui
{
	class Window;

	class Floatting : public Widget
	{
	public:
		Floatting(const ustring &Name, Widget *parent = NULL);

		void setWindow(Window *wnd)	{	this->wnd = wnd;	}
		Window *getWindow()	{	return wnd;	}

		virtual void refresh(const bool chain = false);

		virtual Widget *getRoot();

		void hide();

	protected:
		virtual void onHide() = 0;

	private:
		Window* wnd;
	};

}

#endif // FLOATTING_H
