
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/progressbar.h>
#include <SDL/sgui/font.h>
#include <SDL/sgui/renderapi.h>
#include <sstream>
#include <limits>

using namespace std;

namespace Gui
{

	ProgressBar::ProgressBar(const ustring &Name, Widget *parent) : Widget(Name, parent)
	{
		setValue(0);
	}

	ProgressBar::~ProgressBar()
	{
	}

	int ProgressBar::getOptimalHeight() const
	{
		return 26;
	}

	int ProgressBar::getOptimalWidth() const
	{
		return numeric_limits<int>::min();
	}

	void ProgressBar::onSetValue()
	{
		Value = clamp(Value, 0, 100);
	}

	void ProgressBar::draw(SDL_Surface *target)
	{
		const int r = 4;
		const int p = 2 + (w - 5) * Value / 100;

		roundedgradientbox(target, 1, 1, w - 2, h - 2, 4, 0.0f, 1.0f / h, grey, darkgrey);
		roundedgradientbox(target, 2, 2, p, h - 3, r, 0.0f, -2.0f / h, blue, grey);
		roundedbox(target, 2, 2, p, h - 3, r, darkblue);

		wstringstream sstr;
		sstr << Value << " %";
		wstring str = sstr.str();
		Font::print(target, (w - str.size() * 8) / 2, (h - 16) / 2, str, white);
	}

}
