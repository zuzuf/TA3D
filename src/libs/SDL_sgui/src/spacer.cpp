
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/spacer.h>
#include <limits>

namespace Gui
{

	Spacer::Spacer(bool vert) : Widget(L""), vert(vert)
	{
	}

	Spacer::~Spacer()
	{
	}

	int Spacer::getOptimalWidth() const
	{
		return vert ? 0 : std::numeric_limits<int>::min();
	}

	int Spacer::getOptimalHeight() const
	{
		return vert ? std::numeric_limits<int>::min() : 0;
	}

}
