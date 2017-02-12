#ifndef UTILS_H
#define UTILS_H

#include "unicode.h"

namespace Gui
{
	class Receiver;

	class Utils
	{
	public:
		static void message(const ustring &title, const ustring &msg);
		static ustring input(const ustring &title, const ustring &msg);
		static bool ask(const ustring &title, const ustring &msg);

		static Receiver *actionCloseWindow();
		static Receiver *actionSetBool(bool &b);
	};

}

#endif // UTILS_H
