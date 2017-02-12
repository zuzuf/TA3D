
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/widget.h>
#include <SDL/sgui/layout.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/receiver.h>
#include <SDL/sgui/renderapi.h>
#include <SDL/sgui/sdl-headers.h>
#include <queue>
#include <cmath>

using namespace std;

namespace Gui
{

	std::map<std::wstring, Widget*> Widget::wtable;

	Widget::Widget(const ustring &Name, Widget *parent)
		: x(0), y(0), w(320), h(240),
		parent(parent), bRefresh(true), bRefreshChain(false), bFocus(false), bMouseIn(false),
		Name(Name)
	{
		bUpdatingLayout = false;
		setName(Name);
		initCursors();
		focusWidget = NULL;
		layout = new Layout;
		layout->parent = this;
		if (parent)
		{
			parent->addChild(this);
			return;
		}
		resize(w, h);
	}

	Widget::~Widget()
	{
		if (parent)
			parent->remove(this);

		set<Widget*> ch = childs;
		for(set<Widget*>::const_iterator i = ch.begin() ; i != ch.end() ; ++i)
		{
			delete *i;
		}
		childs.clear();
		wtable.erase(Name);

		set<Receiver*> recv = listeners;
		for(set<Receiver*>::iterator i = recv.begin() ; i != recv.end() ; ++i)
		{
			(*i)->emitters.erase(this);
			if ((*i)->emitters.empty())
				delete (*i);
		}
		listeners.clear();

		if (layout)
			delete layout;
	}

	void Widget::addChild(Widget *widget)
	{
		if (!widget)
			return;
		if (childs.count(widget) == 0)
		{
			if (widget->parent)
				widget->parent->remove(widget);
			widget->parent = this;
			childs.insert(widget);
			layout->addWidget(widget);
			updateLayout();
		}
	}

	void Widget::remove(Widget *widget)
	{
		if (childs.count(widget))
		{
			childs.erase(widget);
			widget->parent = NULL;
			layout->remove(widget);
			updateLayout();
		}
	}

	void Widget::resize(int w, int h)
	{
		if ((w != this->w && w >= 0) || (h != this->h && h >= 0))
		{
			if (w >= 0)
				this->w = w;
			if (h >= 0)
				this->h = h;
			updateLayout();
			resizeEvent();
		}
	}

	void Widget::paint(SDL_Surface *target)
	{
		if (bRefresh)
		{
			draw(target);
			for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			{
				(*i)->bRefresh = true;
				(*i)->bRefreshChain = true;
			}
			bRefreshChain = true;
		}
		if (bRefreshChain)
		{
			for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			{
				if (!(*i)->bRefresh && !(*i)->bRefreshChain)
					continue;
				if ((*i)->x >= target->clip_rect.x + target->clip_rect.w
					|| (*i)->y >= target->clip_rect.y + target->clip_rect.h
					|| (*i)->x + (*i)->w <= target->clip_rect.x
					|| (*i)->y + (*i)->h <= target->clip_rect.y)
					continue;
				SDL_Surface sub = SubSurface(target, (*i)->x, (*i)->y, (*i)->w, (*i)->h);
				(*i)->paint(&sub);
			}
		}
		bRefresh = false;
		bRefreshChain = false;
	}

	void Widget::refresh(const bool chain)
	{
		if (!chain)
			bRefresh = true;
		else
			bRefreshChain = true;
		if (parent)
			parent->refresh(true);
	}

	void Widget::event(SDL_Event *e)
	{
		switch(e->type)
		{
		case SDL_KEYDOWN:
			if (bFocus)
				keyPressEvent(e);
			break;
		case SDL_KEYUP:
			if (bFocus)
				keyReleaseEvent(e);
			break;
		case SDL_MOUSEBUTTONDOWN:
			mousePressEvent(e);
			break;
		case SDL_MOUSEBUTTONUP:
			mouseReleaseEvent(e);
			break;
		case SDL_MOUSEMOTION:
			mouseMoveEvent(e);
			break;
		};

		switch(e->type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
			if (!bMouseIn)
			{
				bMouseIn = true;
				mouseEnter();
			}
		};

		bool bFocusOnChild = false;
		bool bTakeFocus = false;
		switch(e->type)
		{
		case SDL_KEYDOWN:			/**< Keys pressed */
		case SDL_KEYUP:				/**< Keys released */
			for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
				(*i)->event(e);
			break;

		case SDL_MOUSEBUTTONDOWN:	/**< Mouse button pressed */
			if (e->button.x < this->w && e->button.y < this->h && canTakeFocus())
				bTakeFocus = true;
		case SDL_MOUSEBUTTONUP:		/**< Mouse button released */
			for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
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
				if (!(*i)->bMouseIn)
				{
					(*i)->bMouseIn = true;
					(*i)->mouseEnter();
				}
				if ((*i)->canTakeFocus())
					bFocusOnChild = true;
				SDL_Event mb = *e;
				mb.button.x -= (*i)->x;
				mb.button.y -= (*i)->y;
				(*i)->event(&mb);
			}
			if (bTakeFocus && !bFocusOnChild)
				takeFocus();
			break;
		case SDL_MOUSEMOTION:			/**< Mouse moved */
			for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
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
				if (!(*i)->bMouseIn)
				{
					(*i)->bMouseIn = true;
					(*i)->mouseEnter();
				}
				SDL_Event mm = *e;
				mm.motion.x -= (*i)->x;
				mm.motion.y -= (*i)->y;
				(*i)->event(&mm);
			}
			break;

		case SDL_ACTIVEEVENT:	/**< Application loses/gains visibility */
		case SDL_JOYAXISMOTION:	/**< Joystick axis motion */
		case SDL_JOYBALLMOTION:	/**< Joystick trackball motion */
		case SDL_JOYHATMOTION:	/**< Joystick hat position change */
		case SDL_JOYBUTTONDOWN:	/**< Joystick button pressed */
		case SDL_JOYBUTTONUP:	/**< Joystick button released */
		case SDL_VIDEORESIZE:	/**< User resized video mode */
		case SDL_VIDEOEXPOSE:	/**< Screen needs to be redrawn */
		case SDL_QUIT:			/**< User-requested quit */
		case SDL_SYSWMEVENT:	/**< System specific event */

		default:
			return;
		};
	}

	void Widget::takeFocus()
	{
		Widget *pFocus = getFocus();
		if (pFocus == this)
			return;
		if (pFocus)
		{
			pFocus->bFocus = false;
			pFocus->loseFocus();
		}
		Widget *root = this;
		while(root->parent)
			root = root->parent;
		root->focusWidget = this;
		bFocus = true;
		gainFocus();
	}

	Widget *Widget::getFocus()
	{
		Widget *root = this;
		while(root->parent)
			root = root->parent;
		return root->focusWidget;
	}

	void Widget::emitEvent(Event e)
	{
		SDL_Event _event;
		_event.type = SDL_USEREVENT;
		_event.user.code = e;
		SDL_PushEvent(&_event);
	}

	void Widget::updateLayout()
	{
		if (bUpdatingLayout)
			return;

		bUpdatingLayout = true;

		(*layout)();

		if (parent)
			parent->updateLayout();

		refresh();

		bUpdatingLayout = false;
	}

	void Widget::setLayout(Layout *layout)
	{
		if (this->layout == layout)
			return;

		layout->parent = this;
		layout->clear();
		layout->nextID = this->layout->nextID;
		layout->wmap = this->layout->wmap;

		delete this->layout;
		this->layout = layout;

		updateLayout();
	}

	Layout *Widget::getLayout()
	{
		return layout;
	}

	void Widget::draw(SDL_Surface *)
	{
	}

	void Widget::mouseMoveEvent(SDL_Event *)	{}
	void Widget::mousePressEvent(SDL_Event *)	{}
	void Widget::mouseReleaseEvent(SDL_Event *)	{}
	void Widget::keyPressEvent(SDL_Event *)		{}
	void Widget::keyReleaseEvent(SDL_Event *)	{}
	void Widget::gainFocus()					{}
	void Widget::loseFocus()					{}

	int Widget::getOptimalWidth() const
	{
		if (layout)
			return layout->getOptimalWidth();
		if (childs.empty())
			return this->w;
		int w = 0;
		for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			w = max(w, (*i)->getX() + max(0, (*i)->getOptimalWidth()));
		return w;
	}

	int Widget::getOptimalHeight() const
	{
		if (layout)
			return layout->getOptimalHeight();
		if (childs.empty())
			return this->h;
		int h = 0;
		for(set<Widget*>::const_iterator i = childs.begin() ; i != childs.end() ; ++i)
			h = max(h, (*i)->getY() + max(0, (*i)->getOptimalHeight()));
		return h;
	}

	void Widget::mouseEnter()
	{
	}

	void Widget::mouseLeave()
	{
		for(set<Widget*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
			if ((*i)->bMouseIn)
			{
				(*i)->bMouseIn = false;
				(*i)->mouseLeave();
			}
	}

	bool Widget::canTakeFocus() const
	{
		return false;
	}

	void Widget::resizeEvent()
	{
	}

	vector<Widget*> Widget::getGroup()
	{
		if (parent)
			return parent->getGroup();
		vector<Widget*> group;
		queue<Widget*> wqueue;
		wqueue.push(this);
		while(!wqueue.empty())
		{
			Widget *cur = wqueue.front();
			wqueue.pop();
			group.push_back(cur);
			for(set<Widget*>::iterator i = cur->childs.begin() ; i != cur->childs.end() ; ++i)
				wqueue.push(*i);
		}
		return group;
	}

	const std::wstring &Widget::getName() const
	{
		return Name;
	}

	void Widget::setName(const ustring &Name)
	{
		wtable.erase(this->Name);
		this->Name = Name;
		wtable[Name] = this;
	}

	Widget *Widget::get(const ustring &Name)
	{
		map<wstring, Widget*>::iterator it = wtable.find(Name);
		if (it == wtable.end())
			return NULL;
		return it->second;
	}

	void Widget::getAbsolutePos(int &x, int &y) const
	{
		x = 0;
		y = 0;
		if (parent)
			parent->getAbsolutePos(x, y);
		x += this->x;
		y += this->y;
	}

	void Widget::addListener(Receiver *receiver)
	{
		if (listeners.count(receiver) == 0)
		{
			listeners.insert(receiver);
			receiver->emitters.insert(this);
		}
	}

	void Widget::removeListener(Receiver *receiver)
	{
		if (listeners.count(receiver))
		{
			listeners.erase(receiver);
			receiver->emitters.erase(this);
		}
	}

	void Widget::emit()
	{
		for(set<Receiver*>::iterator i = listeners.begin() ; i != listeners.end() ; ++i)
			(*i)->proc(this);
	}

	uint16 &getMouseX(SDL_Event *e)
	{
		switch(e->type)
		{
		case SDL_MOUSEMOTION:
			return e->motion.x;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			return e->button.x;
		};
		static uint16 zero = 0;
		return zero;
	}

	uint16 &getMouseY(SDL_Event *e)
	{
		switch(e->type)
		{
		case SDL_MOUSEMOTION:
			return e->motion.y;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			return e->button.y;
		};
		static uint16 zero = 0;
		return zero;
	}

	Widget *Widget::getRoot()
	{
		if (parent)
			return parent->getRoot();
		return this;
	}
}
