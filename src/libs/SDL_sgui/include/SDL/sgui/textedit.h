#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "frame.h"
#include <string>
#include <vector>

namespace Gui
{

class TextEdit : public Frame
{
public:
	TextEdit(const ustring &Name, Widget *parent = NULL);
	virtual ~TextEdit();

	virtual void setLayout(Layout *layout);
	virtual Layout *getLayout();

	virtual bool canTakeFocus() const;
	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

	void setText(const ustring &text);
	std::wstring getText() const;

protected:
	virtual void draw(SDL_Surface *target);
	virtual void keyPressEvent(SDL_Event *e);
	virtual void mousePressEvent(SDL_Event *e);
	virtual void gainFocus();
	virtual void loseFocus();
	virtual void resizeEvent();
	virtual void mouseEnter();
	virtual void mouseLeave();

private:
	sint32 shiftrow, shiftcol;
	std::vector<std::wstring> lines;

	PROPERTY(uint32, CursorRow);
	PROPERTY(uint32, CursorCol);
};

inline TextEdit &TextEdit_(const ustring &Name, Widget *parent = NULL)
{
	return *(new TextEdit(Name, parent));
}

}

#define TEXTEDIT(x)	static_cast<Gui::TextEdit*>(Gui::Widget::get(#x))

#endif // TEXTEDIT_H
