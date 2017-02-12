#ifndef WIDGET_H
#define WIDGET_H

#include <set>
#include <map>
#include <vector>
#include "types.h"
#include "unicode.h"

struct SDL_Surface;
struct SDL_Cursor;
union SDL_Event;

namespace Gui
{
	class Layout;
	class Receiver;

	class Widget
	{
		friend class Group;
		friend class TabWidget;
		friend class ScrollArea;
		friend class Layout;
		friend class HBoxLayout;
		friend class VBoxLayout;
		friend class UnmanagedLayout;
		friend class Window;
	public:
		enum Event { EVENT_NONE = 0,
					 EVENT_REFRESH,
					 EVENT_CLOSE};
	public:
		Widget(const ustring &Name, Widget *parent = NULL);
		virtual ~Widget();

		virtual void resize(int w, int h);

		virtual void addChild(Widget &widget) { addChild(&widget); }
		virtual void addChild(Widget *widget);
		virtual void remove(Widget *widget);

		virtual void paint(SDL_Surface *target);
		virtual void refresh(const bool chain = false);

		virtual void updateLayout();

		inline int getX() const {	return x;	}
		inline int getY() const {	return y;	}
		void getAbsolutePos(int &x, int &y) const;
		inline int getWidth() const {	return w;	}
		inline int getHeight() const {	return h;	}

		inline void setWidth(int w)	{	resize(w, h);	}
		inline void setHeight(int h)	{	resize(w, h);	}

		inline void setPos(int x, int y) { this->x = x;	this->y = y;	}

		virtual void setLayout(Layout *layout);
		virtual Layout *getLayout();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		virtual bool canTakeFocus() const;

		virtual Widget *getRoot();

		void setName(const ustring &Name);
		const std::wstring &getName() const;

		inline void addListener(Receiver &receiver)	{	addListener(&receiver);	}
		void addListener(Receiver *receiver);
		inline void removeListener(Receiver &receiver)	{	removeListener(&receiver);	}
		void removeListener(Receiver *receiver);

	#define PROPERTY(type, name)\
	private:\
		type name;\
	public:\
		inline const type &get##name() const { return name; }\
		inline void set##name(const type &v) { this->name = v; refresh(); }

	#define PROPERTYP(type, name)\
	private:\
		type name;\
	public:\
		inline type get##name() { return name; }\
		inline void set##name(type v) { this->name = v; refresh(); }

	#define PROPERTY2(type, name)\
	private:\
		type name;\
	public:\
		inline const type &get##name() const { return name; }\
		inline void set##name(const type &v) { this->name = v; onSet##name(); refresh(); }\
		void onSet##name();

	#define PROPERTY2P(type, name)\
	private:\
		type name;\
	public:\
		inline type get##name() { return name; }\
		inline void set##name(type v) { this->name = v; onSet##name(); refresh(); }\
		void onSet##name();

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void event(SDL_Event *e);
		virtual void mouseMoveEvent(SDL_Event *e);
		virtual void mousePressEvent(SDL_Event *e);
		virtual void mouseReleaseEvent(SDL_Event *e);
		virtual void keyPressEvent(SDL_Event *e);
		virtual void keyReleaseEvent(SDL_Event *e);
		virtual void mouseEnter();
		virtual void mouseLeave();
		virtual void gainFocus();
		virtual void loseFocus();
		virtual void resizeEvent();
		void takeFocus();
		Widget *getFocus();
		virtual std::vector<Widget*> getGroup();
		virtual void emit();

	public:
		static void emitEvent(Event e);
		static Widget *get(const ustring &Name);

	protected:
		int x, y;
		int w, h;

		Widget *parent;
		Widget *focusWidget;
		std::set<Widget*> childs;

		bool bRefresh;
		bool bRefreshChain;
		bool bFocus;
		bool bMouseIn;

		Layout *layout;

		std::wstring Name;

		bool bUpdatingLayout;

		std::set<Receiver*> listeners;

	private:
		static std::map<std::wstring, Widget*> wtable;
	};

	inline Widget &Widget_(const ustring &Name, Widget *parent = NULL)
	{
		return *(new Widget(Name, parent));
	}

	uint16 &getMouseX(SDL_Event *e);
	uint16 &getMouseY(SDL_Event *e);
	inline uint16 &getMouseX(SDL_Event &e)	{	return getMouseX(&e);	}
	inline uint16 &getMouseY(SDL_Event &e)	{	return getMouseY(&e);	}
}

#define WIDGET(x)	Gui::Widget::get(#x)

#define LINK(x, y)		x->addListener(y)
#define UNLINK(x, y)	x->removeListener(y)

#endif // WIDGET_H
