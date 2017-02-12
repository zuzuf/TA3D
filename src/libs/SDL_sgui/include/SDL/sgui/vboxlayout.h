#ifndef VBOXLAYOUT_H
#define VBOXLAYOUT_H

#include "layout.h"

namespace Gui
{

class VBoxLayout : public Layout
{
public:
	VBoxLayout();
	virtual ~VBoxLayout();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	virtual void operator ()();
};

}

#endif // VBOXLAYOUT_H
