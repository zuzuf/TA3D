#ifndef SPACER_H
#define SPACER_H

#include "widget.h"

namespace Gui
{

	class Spacer : public Widget
	{
	public:
		Spacer(bool vert);
		virtual ~Spacer();

		virtual int getOptimalWidth() const;
		virtual	int getOptimalHeight() const;

	private:
		bool vert;
	};

	inline Spacer &Spacer_(bool vert)
	{
		return *(new Spacer(vert));
	}

}

#endif // SPACER_H
