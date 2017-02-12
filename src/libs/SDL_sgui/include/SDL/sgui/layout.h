#ifndef LAYOUT_H
#define LAYOUT_H

#include <map>
#include "types.h"

namespace Gui
{

class Widget;

bool comp(const std::pair<Widget*, uint32> &a, const std::pair<Widget*, uint32> &b);

class Layout
{
	friend class Widget;
	friend class Window;
public:
    Layout();
	virtual ~Layout();

protected:
	void addWidget(Widget *widget);
	void remove(Widget *widget);
	void clear();
	virtual void operator()();
	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	std::map<Widget*, uint32> wmap;
	uint32 nextID;
	Widget *parent;
};

Widget &operator+(Widget *w1, Widget &w2);
Widget &operator|(Widget *w1, Widget &w2);
Widget &operator/(Widget *w1, Widget &w2);

inline Widget &operator+(Widget &w1, Widget *w2)
{
	return &w1 + *w2;
}

inline Widget &operator|(Widget &w1, Widget *w2)
{
	return &w1 | *w2;
}

inline Widget &operator/(Widget &w1, Widget *w2)
{
	return &w1 / *w2;
}

inline Widget &operator+(Widget &w1, Widget &w2)
{
	return &w1 + w2;
}

inline Widget &operator|(Widget &w1, Widget &w2)
{
	return &w1 | w2;
}

inline Widget &operator/(Widget &w1, Widget &w2)
{
	return &w1 / w2;
}

}
#endif // LAYOUT_H
