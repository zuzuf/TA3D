#ifndef LISTBOX_H
#define LISTBOX_H

#include "scrollbar.h"
#include <vector>
#include <string>

namespace Gui
{

class Frame;

class ListBox : public Widget
{
public:
	ListBox(const ustring &Name, Widget *parent = NULL);
	virtual ~ListBox();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

	void addElement(const ustring &elt);
	void insertElement(int i, const ustring &elt);
	void removeElement(int i);
	const std::wstring &getElement(int i) const;
	inline const std::wstring &getSelectedElement() const
	{
		return getElement(getSelection());
	}

protected:
	virtual void draw(SDL_Surface *target);
	virtual void mousePressEvent(SDL_Event *e);
	virtual void mouseMoveEvent(SDL_Event *e);
	virtual void mouseLeave();

private:
	uint32 shift;
	std::vector<std::wstring> elements;
	Frame *frame;
	Scrollbar *scrollbar;

	PROPERTY(int, Selection)
};

inline ListBox &ListBox_(const ustring &Name, Widget *parent = NULL)
{
	return *(new ListBox(Name, parent));
}

}

#define LISTBOX(x)	static_cast<Gui::ListBox*>(Gui::Widget::get(#x))

#endif // LISTBOX_H
