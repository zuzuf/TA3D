#ifndef UNMANAGEDLAYOUT_H
#define UNMANAGEDLAYOUT_H

#include "layout.h"

namespace Gui
{

	class UnmanagedLayout : public Layout
	{
	public:
		UnmanagedLayout();
		virtual ~UnmanagedLayout();

		virtual int getOptimalWidth() const;
		virtual int getOptimalHeight() const;

		virtual void operator ()();
	};

}

#endif // UNMANAGEDLAYOUT_H
