#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>
#include <set>

namespace Gui
{

	class Widget;

	class Receiver
	{
		friend class Widget;
	public:
		Receiver();
		virtual ~Receiver();

	protected:
		virtual void proc(const std::wstring &name);
		virtual void proc(Widget *widget);

	private:
		std::set<Widget*>	emitters;
	};

}

#endif // RECEIVER_H
