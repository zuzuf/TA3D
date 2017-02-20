/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "sidedata.h"
#include <gfx/gui/skin.h>


#define TA3D_SIDEDATA_DEFAULTUNIT_EXT      ".fbi"
#define TA3D_SIDEDATA_DEFAULTUNIT_DIR      "units\\"
#define TA3D_SIDEDATA_DEFAULTUNIT_MODEL    "objects3d\\"
#define TA3D_SIDEDATA_DEFAULTUNIT_DOWNLOAD "download\\"
#define TA3D_SIDEDATA_DEFAULTUNIT_WEAPON   "weapons\\"
#define TA3D_SIDEDATA_DEFAULTUNIT_GUIS     "guis\\"
#define TA3D_SIDEDATA_DEFAULTUNIT_GAMEDATA "gamedata\\"



namespace TA3D
{

	SideData ta3dSideData;


	SideData::SideData()
		:nb_side(0), unit_ext(TA3D_SIDEDATA_DEFAULTUNIT_EXT),
		unit_dir(TA3D_SIDEDATA_DEFAULTUNIT_DIR),
		model_dir(TA3D_SIDEDATA_DEFAULTUNIT_MODEL),
		download_dir(TA3D_SIDEDATA_DEFAULTUNIT_DOWNLOAD),
		weapon_dir(TA3D_SIDEDATA_DEFAULTUNIT_WEAPON),
		guis_dir(TA3D_SIDEDATA_DEFAULTUNIT_GUIS),
		gamedata_dir(TA3D_SIDEDATA_DEFAULTUNIT_GAMEDATA)
	{
	}

	SideData::~SideData()
	{
	}

	void SideData::init()
	{
		nb_side=0;
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
			side_name[i].clear();
			side_pref[i].clear();
			side_com[i].clear();
			side_int[i].clear();
		}
		unit_ext     = TA3D_SIDEDATA_DEFAULTUNIT_EXT;
		unit_dir     = TA3D_SIDEDATA_DEFAULTUNIT_DIR;
		model_dir    = TA3D_SIDEDATA_DEFAULTUNIT_MODEL;
		download_dir = TA3D_SIDEDATA_DEFAULTUNIT_DOWNLOAD;
		weapon_dir   = TA3D_SIDEDATA_DEFAULTUNIT_WEAPON;
		guis_dir     = TA3D_SIDEDATA_DEFAULTUNIT_GUIS;
		gamedata_dir = TA3D_SIDEDATA_DEFAULTUNIT_GAMEDATA;
	}

	void SideData::destroy()
	{
		init();
	}




	void SideData::loadData()
	{
		destroy();

		TDFParser mod_parser( "ta3d.mod", false, false, false, false);
		unit_ext = mod_parser.pullAsString( "MOD.unit_ext", ".fbi");
		unit_dir = mod_parser.pullAsString( "MOD.unit_dir", "units\\");
		model_dir = mod_parser.pullAsString( "MOD.model_dir", "objects3d\\");
		download_dir = mod_parser.pullAsString( "MOD.download_dir", "download\\");
		weapon_dir = mod_parser.pullAsString( "MOD.weapon_dir", "weapons\\");
		guis_dir = mod_parser.pullAsString( "MOD.guis_dir", "guis\\");
		gamedata_dir = mod_parser.pullAsString( "MOD.gamedata_dir", "gamedata\\");
        if (!unit_dir.endsWith('/'))      unit_dir += "/";
        if (!model_dir.endsWith('/'))     model_dir += "/";
        if (!download_dir.endsWith('/'))  download_dir += "/";
        if (!weapon_dir.endsWith('/'))    weapon_dir += "/";
        if (!guis_dir.endsWith('/'))      guis_dir += "/";
        if (!gamedata_dir.endsWith('/'))  gamedata_dir += "/";

        TDFParser sidedata_parser( gamedata_dir + "sidedata.tdf", false, false, false, false);

		nb_side = 0;

		QString sideName;
        while (!(sideName = sidedata_parser.pullAsString(QString("side%1.name").arg(nb_side), QString())).isEmpty())
		{
			LOG_DEBUG("[sidedata] " << sideName);

			side_name[ nb_side ] = sideName;
            side_pref[ nb_side ] = sidedata_parser.pullAsString( QString("side%1").arg(nb_side) + ".nameprefix" );
            side_com[ nb_side ] = sidedata_parser.pullAsString( QString("side%1").arg(nb_side) + ".commander" );
            side_int[ nb_side ] = sidedata_parser.pullAsString( QString("side%1").arg(nb_side) + ".intgaf" );

            int pal_id = sidedata_parser.pullAsInt( QString("side%1").arg(nb_side) + ".metalcolor" );
			side_int_data[ nb_side ].metal_color = makeacol( pal[ pal_id ].r, pal[ pal_id ].g, pal[ pal_id ].b, 0xFF);
            pal_id = sidedata_parser.pullAsInt( QString("side%1").arg(nb_side) + ".energycolor" );
			side_int_data[ nb_side ].energy_color = makeacol( pal[ pal_id ].r, pal[ pal_id ].g, pal[ pal_id ].b, 0xFF);

            side_int_data[ nb_side ].EnergyBar = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energybar" );
            side_int_data[ nb_side ].EnergyNum = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energynum" );
            side_int_data[ nb_side ].EnergyMax = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energymax" );
            side_int_data[ nb_side ].Energy0 = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energy0" );
            side_int_data[ nb_side ].EnergyProduced = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energyproduced" );
            side_int_data[ nb_side ].EnergyConsumed = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".energyconsumed" );

            side_int_data[ nb_side ].MetalBar = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metalbar" );
            side_int_data[ nb_side ].MetalNum = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metalnum" );
            side_int_data[ nb_side ].MetalMax = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metalmax" );
            side_int_data[ nb_side ].Metal0 = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metal0" );
            side_int_data[ nb_side ].MetalProduced = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metalproduced" );
            side_int_data[ nb_side ].MetalConsumed = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".metalconsumed" );

            side_int_data[ nb_side ].UnitName = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitname", true);
            side_int_data[ nb_side ].DamageBar = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".damagebar", true);

            side_int_data[ nb_side ].UnitName2 = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitname2", true);
            side_int_data[ nb_side ].DamageBar2 = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".damagebar2", true);

            side_int_data[ nb_side ].UnitMetalMake = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitmetalmake", true);
            side_int_data[ nb_side ].UnitMetalUse = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitmetaluse", true);
            side_int_data[ nb_side ].UnitEnergyMake = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitenergymake", true);
            side_int_data[ nb_side ].UnitEnergyUse = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".unitenergyuse", true);

            side_int_data[ nb_side ].Name = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".name", true);
            side_int_data[ nb_side ].Description = read_gui_element( &sidedata_parser, QString("side%1").arg(nb_side) + ".description", true);

			++nb_side;
		}
	}



	int	SideData::getSideId(const QString& side) const
	{
		for (int i = 0; i < nb_side; ++i)
		{
			if (ToLower(side) == ToLower(side_name[i]))
				return i;
		}
		return -1;
	}


	IntrElementCoords read_gui_element(TDFParser* parser, const QString& element, bool bottom)
	{
		IntrElementCoords gui_element;
        gui_element.x1 = parser->pullAsInt(element + ".x1");
        gui_element.y1 = parser->pullAsInt(element + ".y1");
        gui_element.x2 = parser->pullAsInt(element + ".x2");
        gui_element.y2 = parser->pullAsInt(element + ".y2");
		if (bottom)
		{
			gui_element.y1 += SCREEN_H - 480;
			gui_element.y2 += SCREEN_H - 480;
		}
		return gui_element;
	}




} // namespace TA3D
