#ifndef HBOXLAYOUT_H
#define HBOXLAYOUT_H

#include "layout.h"

namespace Gui
{

	class HBoxLayout : public Layout
{
public:
    HBoxLayout();
	virtual ~HBoxLayout();

	virtual int getOptimalWidth() const;
	virtual int getOptimalHeight() const;

protected:
	virtual void operator ()();
};

}

#endif // HBOXLAYOUT_H
