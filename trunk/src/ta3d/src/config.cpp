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
							  (Label_("", "width              ") | Spacer_(false) | SpinBox_("width"))
							/ (Label_("", "height             ") | Spacer_(false) | SpinBox_("height"))
							/ (Label_("", "color depth        ") | Spacer_(false) | SpinBox_("bpp"))
							/ (Label_("", "FSAA               ") | Spacer_(false) | SpinBox_("fsaa"))
							/ (Label_("", "Fullscreen         ") | Spacer_(false) | CheckBox_("fullscreen", ""))
							/ (Label_("", "Anisotropy         ") | Spacer_(false) | SpinBox_("anisotropy"))
							/ (Label_("", "Shadows quality    ") | Spacer_(false) | SpinBox_("shadows"))
							/ (Label_("", "Water quality      ") | Spacer_(false) | SpinBox_("water"))
							/ (Label_("", "Shadowmap size     ") | Spacer_(false) | SpinBox_("shadowmap"))
							/ (Label_("", "texture cache      ") | Spacer_(false) | CheckBox_("texturecache", ""))
							/ (Label_("", "texture compression") | Spacer_(false) | CheckBox_("texturecompression", ""))
							/ (Label_("", "far sight          ") | Spacer_(false) | CheckBox_("farsight", ""))
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

	SPINBOX(anisotropy)->setMinimum(1);
	SPINBOX(anisotropy)->setMaximum(16);
	SPINBOX(anisotropy)->setValue(lp_CONFIG->anisotropy);

	SPINBOX(shadows)->setMinimum(0);
	SPINBOX(shadows)->setMaximum(3);
	SPINBOX(shadows)->setValue(lp_CONFIG->shadow_quality);

	SPINBOX(shadowmap)->setMinimum(0);
	SPINBOX(shadowmap)->setMaximum(3);
	SPINBOX(shadowmap)->setValue(lp_CONFIG->shadowmap_size);

	SPINBOX(water)->setMinimum(0);
	SPINBOX(water)->setMaximum(5);
	SPINBOX(water)->setValue(lp_CONFIG->water_quality);

	CHECKBOX(texturecache)->setState(lp_CONFIG->use_texture_cache);
	CHECKBOX(texturecompression)->setState(lp_CONFIG->use_texture_compression);
	CHECKBOX(fullscreen)->setState(lp_CONFIG->fullscreen);
	CHECKBOX(farsight)->setState(lp_CONFIG->far_sight);

	// The audio tab
	TABWIDGET(tabs)->addTab("audio", (Label_("", "sound volume       ") | Spacer_(false) | SpinBox_("soundvolume"))
								   / (Label_("", "music volume       ") | Spacer_(false) | SpinBox_("musicvolume"))
								   / Spacer_(true));
	SPINBOX(soundvolume)->setMinimum(0);
	SPINBOX(soundvolume)->setMaximum(128);
	SPINBOX(soundvolume)->setValue(lp_CONFIG->sound_volume);

	SPINBOX(musicvolume)->setMinimum(0);
	SPINBOX(musicvolume)->setMaximum(128);
	SPINBOX(musicvolume)->setValue(lp_CONFIG->music_volume);

	// The game tab
	TABWIDGET(tabs)->addTab("game", (Label_("", "player name        ") | Spacer_(false) | LineInput_("playername"))
								  / (Label_("", "language           ") | Spacer_(false) | LineInput_("language"))
								  / (Label_("", "mod                ") | Spacer_(false) | LineInput_("mod"))
								  / Spacer_(true));

	LINEINPUT(playername)->setText(lp_CONFIG->player_name.c_str());
	LINEINPUT(language)->setText(lp_CONFIG->Lang.c_str());
	LINEINPUT(mod)->setText(lp_CONFIG->last_MOD.c_str());

	// The advanced tab
	TABWIDGET(tabs)->addTab("advanced", (Label_("", "developer mode        ") | Spacer_(false) | CheckBox_("devmode", ""))
									  / (Label_("", "7z command            ") | Spacer_(false) | LineInput_("system7zCommand"))
									  / (Label_("", "NetServer             ") | Spacer_(false) | LineInput_("netserver"))
									  / Spacer_(true));
	CHECKBOX(devmode)->setState(lp_CONFIG->developerMode);
	LINEINPUT(system7zCommand)->setText(lp_CONFIG->system7zCommand.c_str());
	LINEINPUT(netserver)->setText(lp_CONFIG->net_server.c_str());

	// Ok and cancel buttons
	bool bOk = false;

	BUTTON(ok)->addListener(Utils::actionCloseWindow());
	BUTTON(ok)->addListener(Utils::actionSetBool(bOk));
	BUTTON(cancel)->addListener(Utils::actionCloseWindow());

	// Run the config window
	wnd();

	if (bOk)
	{
		lp_CONFIG->screen_width = static_cast<TA3D::uint16>(SPINBOX(width)->getValue());
		lp_CONFIG->screen_height = static_cast<TA3D::uint16>(SPINBOX(height)->getValue());
		lp_CONFIG->color_depth = static_cast<TA3D::uint8>(SPINBOX(bpp)->getValue());
		lp_CONFIG->fsaa = static_cast<TA3D::sint16>(SPINBOX(fsaa)->getValue());
		lp_CONFIG->fullscreen = CHECKBOX(fullscreen)->getState();
		lp_CONFIG->anisotropy = static_cast<TA3D::sint16>(SPINBOX(anisotropy)->getValue());
		lp_CONFIG->shadow_quality = static_cast<TA3D::sint16>(SPINBOX(shadows)->getValue());
		lp_CONFIG->water_quality = static_cast<TA3D::sint16>(SPINBOX(water)->getValue());
		lp_CONFIG->shadowmap_size = static_cast<TA3D::uint8>(SPINBOX(shadowmap)->getValue());
		lp_CONFIG->use_texture_cache = CHECKBOX(texturecache)->getState();
		lp_CONFIG->use_texture_compression = CHECKBOX(texturecompression)->getState();
		lp_CONFIG->far_sight = CHECKBOX(farsight)->getState();

		lp_CONFIG->sound_volume = static_cast<int>(SPINBOX(soundvolume)->getValue());
		lp_CONFIG->music_volume = static_cast<int>(SPINBOX(musicvolume)->getValue());

		lp_CONFIG->player_name = std::string(LINEINPUT(playername)->getText());
		lp_CONFIG->Lang = std::string(LINEINPUT(language)->getText());
		lp_CONFIG->last_MOD = std::string(LINEINPUT(mod)->getText());
		TA3D_CURRENT_MOD = lp_CONFIG->last_MOD;

		lp_CONFIG->developerMode = CHECKBOX(devmode)->getState();

		lp_CONFIG->system7zCommand = std::string(LINEINPUT(system7zCommand)->getText());
		lp_CONFIG->net_server = std::string(LINEINPUT(netserver)->getText());

		// Save modified settings
		TA3D::Settings::Save();
	}

	// Exit
	exit(0);
}
