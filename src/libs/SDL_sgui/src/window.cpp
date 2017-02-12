
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/window.h>
#include <SDL/sgui/renderapi.h>
#include <SDL/sgui/menubar.h>
#include <SDL/sgui/menu.h>
#include <SDL/sgui/vboxlayout.h>
#include <SDL/sgui/floatting.h>
#include <SDL/sgui/sdl-headers.h>
# if defined(__APPLE__) || defined(__MACH__)
#	include <SDL_getenv.h>
# else
#	include <SDL/SDL_getenv.h>
# endif


using namespace std;

namespace Gui
{

	Window::Window(const ustring &Name, int w, int h, uint32 flags) : Widget(Name), flags(flags)
	{
		middle = new Widget("");
		refreshInProgress = false;
		layoutUpdateInProgress = false;
		Widget::setLayout(new VBoxLayout);
		menubar = NULL;
		takeFocus();
		this->w = -1;
		this->h = -1;
		resize(w, h);
		Widget::addChild(middle);
		setLayout(new VBoxLayout);
	}

	Window::~Window()
	{
		SDL_SetVideoMode(0, 0, 0, 0);
		if (menubar)
			delete menubar;
		childs.clear();
		layout->clear();
		delete middle;
	}

	void Window::resize(int w, int h)
	{
		if (w != this->w || h != this->h)
		{
			this->w = w;
			this->h = h;
			setSDLVideo();
			updateLayout();
		}
	}

	void Window::draw(SDL_Surface *target)
	{
#ifdef __WIN32__
		fill(target, lightgrey);
#else
		gradientbox(target, 0, 0, w - 1, h - 1, 0.0f, -0.46f / h, SDL_MapRGBA(target->format, 0xDC, 0xDC, 0xDC, 0xDC), darkgrey);
#endif
	}

	void Window::setTitle(const ustring &title)
	{
		this->title = title;
		SDL_WM_SetCaption(this->title.c_str(), NULL);
	}

	void Window::operator()()
	{
		SDL_Event e;
		while(SDL_WaitEvent(&e))
		{
			switch(e.type)
			{
			case SDL_KEYDOWN:			/**< Keys pressed */
			case SDL_KEYUP:				/**< Keys released */
			case SDL_MOUSEBUTTONDOWN:	/**< Mouse button pressed */
			case SDL_MOUSEBUTTONUP:		/**< Mouse button released */
			case SDL_MOUSEMOTION:		/**< Mouse moved */
				event(&e);
				break;

			case SDL_VIDEORESIZE:		/**< User resized video mode */
				if (e.resize.w != w || e.resize.h != h)
				{
					SDL_Surface *screen = SDL_GetVideoSurface();
					w = e.resize.w;
					h = e.resize.h;
					if (screen->w < e.resize.w
						|| screen->h < e.resize.h
						|| (screen->w >> 1) > e.resize.w
						|| (screen->h >> 1) > e.resize.h)
						setSDLVideo();
					else
						updateLayout();
					mouseLeave();
				}
				break;

			case SDL_VIDEOEXPOSE:		/**< Screen needs to be redrawn */
				flip();
				break;
			case SDL_ACTIVEEVENT:		/**< Application loses/gains visibility */
				if (e.active.state == SDL_APPMOUSEFOCUS)
				{
					if (!e.active.gain)
					{
						takeFocus();
						mouseLeave();
					}
				}
				if (e.active.state == SDL_APPACTIVE && e.active.gain)
					flip();
				break;

			case SDL_QUIT:				/**< User-requested quit */
				return;

			case SDL_SYSWMEVENT:		/**< System specific event */
				break;

			case SDL_USEREVENT:
				switch(e.user.code)
				{
				case EVENT_REFRESH:
					paint(SDL_GetVideoSurface());
					flip();
					refreshInProgress = false;
					break;
				case EVENT_CLOSE:
					SDL_SetVideoMode(0,0,0,0);
					mouseLeave();
					return;
				};
				break;

			case SDL_JOYAXISMOTION:		/**< Joystick axis motion */
			case SDL_JOYBALLMOTION:		/**< Joystick trackball motion */
			case SDL_JOYHATMOTION:		/**< Joystick hat position change */
			case SDL_JOYBUTTONDOWN:		/**< Joystick button pressed */
			case SDL_JOYBUTTONUP:		/**< Joystick button released */
			default:
				break;
			};
		}
	}

	void Window::refresh(const bool chain)
	{
		if (chain)
			bRefreshChain = true;
		else
			bRefresh = true;
		if (!refreshInProgress)
			emitEvent(EVENT_REFRESH);
		refreshInProgress = true;
	}

	void Window::setResizeable(bool resizeable)
	{
		if (resizeable == isResizeable())
			return;
		flags ^= RESIZEABLE;
		setSDLVideo();
	}

	void Window::setMoveable(bool moveable)
	{
		if (moveable == isMoveable())
			return;
		flags ^= MOVEABLE;
	}

	void Window::setNoFrame(bool noframe)
	{
		if (noframe == hasNoFrame())
			return;
		flags ^= NOFRAME;
		setSDLVideo();
	}

	void Window::setFullscreen(bool fullscreen)
	{
		if (fullscreen == isFullscreen())
			return;
		flags ^= FULLSCREEN;
		setSDLVideo();
	}

	void Window::flip()
	{
		SDL_Flip(SDL_GetVideoSurface());
	}

	void Window::setSDLVideo()
	{
		Uint32 flags = SDL_SWSURFACE | SDL_ANYFORMAT;
		if (this->flags & RESIZEABLE)
			flags |= SDL_RESIZABLE;
		if (this->flags & NOFRAME)
			flags |= SDL_NOFRAME;
		if (this->flags & FULLSCREEN)
			flags |= SDL_FULLSCREEN;
		SDL_SetVideoMode(w, h, 32, flags);
		SDL_EnableUNICODE(SDL_ENABLE);
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		SDL_SetCursor(cursor_arrow);
		SDL_ShowCursor(SDL_ENABLE);
		w = SDL_GetVideoSurface()->w;
		h = SDL_GetVideoSurface()->h;
		updateGUIColors();
		updateLayout();
	}

	void Window::setMenuBar(MenuBar *menubar)
	{
		if (this->menubar == menubar)
			return;
		if (this->menubar)
			delete menubar;
		this->menubar = menubar;

		layout->clear();
		childs.clear();

		if (menubar)
			Widget::addChild(menubar);
		layout->addWidget(middle);
		childs.insert(middle);
		refresh();
	}

	void Window::setLayout(Layout *layout)
	{
		middle->setLayout(layout);
	}

	Layout *Window::getLayout()
	{
		return middle->getLayout();
	}

	void Window::addChild(Widget *widget)
	{
		if (dynamic_cast<MenuBar*>(widget))
			setMenuBar(static_cast<MenuBar*>(widget));
		else
			middle->addChild(widget);
	}

	void Window::remove(Widget *widget)
	{
		if (dynamic_cast<MenuBar*>(widget) && widget == menubar)
			setMenuBar(NULL);
		else
			middle->remove(widget);
	}

	void Window::addFloatting(Floatting *widget)
	{
		if (floatting.count(widget) == 0)
		{
			floatting.insert(widget);
			widget->setWindow(this);
			widget->resize(widget->getOptimalWidth(), widget->getOptimalHeight());
			refresh();
		}
	}

	void Window::removeFloatting(Floatting *widget)
	{
		if (floatting.count(widget))
		{
			floatting.erase(widget);
			widget->setWindow(NULL);
			widget->mouseLeave();
			refresh();
		}
	}

	void Window::paint(SDL_Surface *target)
	{
		Widget::paint(target);
		for(set<Floatting*>::const_iterator i = floatting.begin() ; i != floatting.end() ; ++i)
		{
			if ((*i)->getX() >= target->clip_rect.x + target->clip_rect.w
				|| (*i)->getY() >= target->clip_rect.y + target->clip_rect.h
				|| (*i)->getX() + (*i)->getWidth() <= target->clip_rect.x
				|| (*i)->getY() + (*i)->getHeight() <= target->clip_rect.y)
				continue;
			SDL_Surface sub = SubSurface(target, (*i)->getX(), (*i)->getY(), (*i)->getWidth(), (*i)->getHeight());
			(*i)->bRefresh = true;
			(*i)->paint(&sub);
		}
	}

	void Window::event(SDL_Event *e)
	{
		bool mouseOnFloatting = false;
		set<Floatting*> fobj = floatting;
		switch(e->type)
		{
		case SDL_KEYDOWN:			/**< Keys pressed */
		case SDL_KEYUP:				/**< Keys released */
			for(set<Floatting*>::iterator i = fobj.begin() ; i != fobj.end() ; ++i)
				(*i)->event(e);
			break;

		case SDL_MOUSEBUTTONDOWN:	/**< Mouse button pressed */
		case SDL_MOUSEBUTTONUP:		/**< Mouse button released */
			for(set<Floatting*>::iterator i = fobj.begin() ; i != fobj.end() ; ++i)
			{
				if (e->button.x < (*i)->x
					|| e->button.x >= (*i)->x + (*i)->w
					|| e->button.y < (*i)->y
					|| e->button.y >= (*i)->y + (*i)->h)
				{
					if ((*i)->bMouseIn)
					{
						(*i)->bMouseIn = false;
						(*i)->mouseLeave();
					}
					continue;
				}
				else
					mouseOnFloatting = true;
				SDL_Event mb = *e;
				mb.button.x -= (*i)->x;
				mb.button.y -= (*i)->y;
				(*i)->event(&mb);
			}
			break;
		case SDL_MOUSEMOTION:			/**< Mouse moved */
			for(set<Floatting*>::iterator i = fobj.begin() ; i != fobj.end() ; ++i)
			{
				if (e->motion.x < (*i)->x
					|| e->motion.x >= (*i)->x + (*i)->w
					|| e->motion.y < (*i)->y
					|| e->motion.y >= (*i)->y + (*i)->h)
				{
					if ((*i)->bMouseIn)
					{
						(*i)->bMouseIn = false;
						(*i)->mouseLeave();
					}
					continue;
				}
				else
					mouseOnFloatting = true;
				SDL_Event mm = *e;
				mm.motion.x -= (*i)->x;
				mm.motion.y -= (*i)->y;
				(*i)->event(&mm);
			}
			break;

		default:
			return;
		};
		if (!mouseOnFloatting)
		{
			fobj = floatting;
			if (e->type == SDL_MOUSEBUTTONDOWN)
			{
				for(set<Floatting*>::iterator i = fobj.begin() ; i != fobj.end() ; ++i)
					if (dynamic_cast<Menu*>(*i))
						static_cast<Menu*>(*i)->hide();
			}
			Widget::event(e);
		}
		else
		{
			for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
			{
				if ((*i)->bMouseIn)
				{
					(*i)->bMouseIn = false;
					(*i)->mouseLeave();
				}
			}
		}
	}
}
