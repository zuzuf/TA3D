#ifndef SPINBOX_H
#define SPINBOX_H

#include "widget.h"

namespace Gui
{

	class SpinBox : public Widget
	{
	public:
		SpinBox(const ustring &Name, Widget *parent = NULL);
		virtual ~SpinBox();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;
		virtual bool canTakeFocus() const;

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void keyPressEvent(SDL_Event *e);
		virtual void mousePressEvent(SDL_Event *e);
		virtual void mouseMoveEvent(SDL_Event *e);
		virtual void mouseLeave();

	private:
		int highlight;

		PROPERTY2(double, Value);
		PROPERTY2(double, Minimum);
		PROPERTY2(double, Maximum);
		PROPERTY(double, Step);
		PROPERTY(uint32, Precision);

	private:
		static SDL_Surface *arrowup;
		static SDL_Surface *arrowdown;
	};

	inline SpinBox &SpinBox_(const ustring &Name, Widget *parent = NULL)
	{
		return *(new SpinBox(Name, parent));
	}

}

#define SPINBOX(x)	static_cast<Gui::SpinBox*>(Gui::Widget::get(#x))

#endif // SPINBOX_H
