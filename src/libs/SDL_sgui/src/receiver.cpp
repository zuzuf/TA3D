
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/receiver.h>
#include <SDL/sgui/widget.h>

using namespace std;

namespace Gui
{

	Receiver::Receiver()
	{
	}

	Receiver::~Receiver()
	{
		set<Widget*> tmp = emitters;
		for(set<Widget*>::iterator i = tmp.begin() ; i != tmp.end() ; ++i)
			(*i)->removeListener(this);
	}

	void Receiver::proc(const std::wstring &)
	{
	}

	void Receiver::proc(Widget *widget)
	{
		proc(widget->getName());
	}

}
