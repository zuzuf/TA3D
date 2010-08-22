#include "TA3D_NameSpace.h"
#include <misc/settings.h>
#include <SDL/SDL_sgui.h>

void configWindow()
{
	using namespace Gui;
	using namespace TA3D;

	// Load current settings
	TA3D::Settings::Load();

	// Create the config window
	Window wnd("config");
	wnd.setTitle("TA3D failsafe config tool");

	wnd.addChild(TabWidget_("tabs")
				 / (Spacer_(false) | Button_("ok", "   ok   ") | Spacer_(false) | Button_("cancel", " cancel ") | Spacer_(false)));

	// The video tab
	TABWIDGET(tabs)->addTab("video",
							  (Label_("L1", "width      ") | Spacer_(false) | SpinBox_("width"))
							/ (Label_("L2", "height     ") | Spacer_(false) | SpinBox_("height"))
							/ (Label_("L3", "color depth") | Spacer_(false) | SpinBox_("bpp"))
							/ (Label_("L3", "FSAA       ") | Spacer_(false) | SpinBox_("fsaa"))
							/ (Label_("L3", "Fullscreen ") | Spacer_(false) | CheckBox_("fullscreen", ""))
							/ Spacer_(true));

	SPINBOX(width)->setMinimum(640);
	SPINBOX(width)->setMaximum(3200);
	SPINBOX(width)->setValue(lp_CONFIG->screen_width);

	SPINBOX(height)->setMinimum(480);
	SPINBOX(height)->setMaximum(2400);
	SPINBOX(height)->setValue(lp_CONFIG->screen_height);

	SPINBOX(bpp)->setMinimum(16);
	SPINBOX(bpp)->setMaximum(32);
	SPINBOX(bpp)->setValue(lp_CONFIG->color_depth);

	SPINBOX(fsaa)->setMinimum(0);
	SPINBOX(fsaa)->setMaximum(4);
	SPINBOX(fsaa)->setValue(lp_CONFIG->fsaa);

	CHECKBOX(fullscreen)->setState(lp_CONFIG->fullscreen);

	// The audio tab
	TABWIDGET(tabs)->addTab("audio", Label_("", "not implemented yet"));

	// The game tab
	TABWIDGET(tabs)->addTab("game", Label_("", "not implemented yet"));

	// The advanced tab
	TABWIDGET(tabs)->addTab("advanced", Label_("", "not implemented yet"));

	// Ok and cancel buttons
	bool bOk = false;

	BUTTON(ok)->addListener(Utils::actionCloseWindow());
	BUTTON(ok)->addListener(Utils::actionSetBool(bOk));
	BUTTON(cancel)->addListener(Utils::actionCloseWindow());

	// Run the config window
	wnd();

	if (bOk)
	{
		lp_CONFIG->screen_width = SPINBOX(width)->getValue();
		lp_CONFIG->screen_height = SPINBOX(height)->getValue();
		lp_CONFIG->color_depth = SPINBOX(bpp)->getValue();
		lp_CONFIG->fsaa = SPINBOX(fsaa)->getValue();
		lp_CONFIG->fullscreen = CHECKBOX(fullscreen)->getState();

		// Save modified settings
		TA3D::Settings::Save();
	}

	// Exit
	exit(0);
}
