#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"

namespace Gui
{
	class MenuBar;
	class Floatting;

	class Window : public Widget
	{
		friend class MenuBar;
	public:
		enum { RESIZEABLE = 1, MOVEABLE = 2, NOFRAME = 4, FULLSCREEN = 8 };
	public:
		Window(const ustring &Name, int w = 640, int h = 480, uint32 flags = RESIZEABLE);
		virtual ~Window();

		virtual void resize(int w, int h);
		virtual void refresh(const bool chain = false);

		inline const std::string &getTitle() const {	return title;	}
		void setTitle(const ustring &title);

		void setResizeable(bool resizeable);
		inline bool isResizeable() const {	return flags & RESIZEABLE;	}

		void setMoveable(bool moveable);
		inline bool isMoveable() const {	return flags & MOVEABLE;	}

		void setNoFrame(bool noframe);
		inline bool hasNoFrame() const {	return flags & NOFRAME;	}

		void setFullscreen(bool fullscreen);
		inline bool isFullscreen() const {	return flags & FULLSCREEN;	}

		void setMenuBar(MenuBar *menubar);
		void setMenuBar(MenuBar &menubar)	{	setMenuBar(&menubar);	}

		virtual void setLayout(Layout *layout);
		virtual Layout *getLayout();

		virtual void addChild(Widget *widget);
		virtual void addChild(Widget &widget)	{	addChild(&widget);	}
		virtual void remove(Widget *widget);

		void addFloatting(Floatting *widget);
		void removeFloatting(Floatting *widget);

		virtual void paint(SDL_Surface *target);

	private:
		void setSDLVideo();
		void flip();

	public:
		void operator()();

	protected:
		virtual void draw(SDL_Surface *target);
		virtual void event(SDL_Event *e);

	private:
		std::string title;
		uint32 flags;
		bool refreshInProgress;
		bool layoutUpdateInProgress;
		MenuBar *menubar;
		Widget *middle;
		std::set<Floatting*> floatting;
	};

	inline Window &Window_(const ustring &Name, int w = 640, int h = 480)
	{
		return *(new Window(Name, w, h));
	}

}

#define WINDOW(x)	static_cast<Gui::Window*>(Gui::Widget::get(#x))

#endif // WINDOW_H
