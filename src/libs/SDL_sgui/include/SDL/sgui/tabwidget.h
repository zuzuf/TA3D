#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "widget.h"
#include <vector>
#include <utility>

namespace Gui
{

	class TabWidget : public Widget
	{
	public:
		TabWidget(const ustring &Name, Widget *parent = NULL);
		virtual ~TabWidget();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		int addTab(const ustring &title, Widget *widget);
		inline int addTab(const ustring &title, Widget &widget)	{	return addTab(title, &widget);	}
		void removeTab(int idx);
		const std::wstring &getTabName(int idx);
		Widget *getTabWidget(int idx);
		void setTabName(int idx, const ustring &Name);
		void setTabWidget(int idx, Widget *widget);

		virtual void paint(SDL_Surface *target);

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void updateLayout();
		virtual void event(SDL_Event *e);
		virtual void mousePressEvent(SDL_Event *e);
		virtual void mouseEnter();

	private:
		inline void addChild(Widget &)	{}
		inline void addChild(Widget *) {}
		virtual void remove(Widget *);

	private:
		std::vector< std::pair<std::wstring, Widget*> > tabs;

		PROPERTY2(int, CurrentTab);
		PROPERTY2(bool, FrameLess);
	};

	inline TabWidget &TabWidget_(const ustring &Name, Widget *parent = NULL)
	{
		return *(new TabWidget(Name, parent));
	}

}

#define TABWIDGET(x)	static_cast<Gui::TabWidget*>(Gui::Widget::get(#x))

#endif // TABWIDGET_H
