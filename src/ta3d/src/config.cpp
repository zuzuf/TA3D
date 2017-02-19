#include "configdialog.h"
#include <cstdlib>

void configWindow()
{
	// Create the config window
    ConfigDialog wnd;
    wnd.do_config();

	// Exit
	exit(0);
}
