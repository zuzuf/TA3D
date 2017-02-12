#ifndef LABEL_H
#define LABEL_H

#include "widget.h"
#include <vector>

namespace Gui
{

class Label : public Widget
{
public:
	enum HAlign { LEFT = 0, CENTER = 1, RIGHT = 2 };
	enum VAlign { TOP = 0, MIDDLE = 1, BOTTOM = 2 };
public:
	Label(const ustring &Name, const ustring &caption = ustring(), Widget *parent = NULL);
	virtual ~Label();

	inline const std::wstring &getCaption() const {	return caption;	}
	void setCaption(const ustring &caption);

	inline HAlign getAlignment() const {	return side;	}
	inline void setAlignment(HAlign side) {	this->side = side;	}
	inline VAlign getVAlignment() const {	return valign;	}
	inline void setVAlignment(VAlign valign) {	this->valign= valign;	}

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	virtual void draw(SDL_Surface *target);

private:
	std::wstring caption;
	std::vector<std::wstring> lines;
	int maxchars;
	HAlign side;
	VAlign valign;

	PROPERTY(uint32, Color);
};

inline Label &Label_(const ustring &Name, const ustring &caption = ustring(), Widget *parent = NULL)
{
	return *(new Label(Name, caption, parent));
}

}

#define LABEL(x)	static_cast<Gui::Label*>(Gui::Widget::get(#x))

#endif // LABEL_H
