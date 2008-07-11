
#include "sidedata.h"
#include "../cTAFileParser.h"
#include "../gfx/gui/skin.h"



namespace TA3D
{

    SideData ta3dSideData;


    SideData::SideData()
        :unit_ext(), unit_dir(), model_dir(), download_dir(), weapon_dir(),
        guis_dir(), gamedata_dir()
    {
        init();
    }

    SideData::~SideData()
    {
        destroy();
    }

    void SideData::init()
    {
        nb_side=0;
        for (int i = 0; i < 10; ++i)
            side_name[i] = side_pref[i] = side_com[i] = side_int[i] = NULL;
        unit_ext = ".fbi";
        unit_dir = "units\\";
        model_dir = "objects3d\\";
        download_dir = "download\\";
        weapon_dir = "weapons\\";
        guis_dir = "guis\\";
        gamedata_dir = "gamedata\\";
    }

    void SideData::destroy()
    {
        for(int i = 0;i < 10; ++i)
        {
            if(side_name[i])	free(side_name[i]);
            if(side_pref[i])	free(side_pref[i]);
            if(side_com[i])		free(side_com[i]);
            if(side_int[i])		free(side_int[i]);
        }
        unit_ext.clear();
        unit_dir.clear();
        model_dir.clear();
        download_dir.clear();
        weapon_dir.clear();
        guis_dir.clear();
        gamedata_dir.clear();
        init();
    }




    void SideData::loadData()
    {
        destroy();

        UTILS::cTAFileParser mod_parser( "ta3d.mod" );
        unit_ext = mod_parser.PullAsString( "MOD.unit_ext", ".fbi" );
        unit_dir = mod_parser.PullAsString( "MOD.unit_dir", "units\\" );
        model_dir = mod_parser.PullAsString( "MOD.model_dir", "objects3d\\" );
        download_dir = mod_parser.PullAsString( "MOD.download_dir", "download\\" );
        weapon_dir = mod_parser.PullAsString( "MOD.weapon_dir", "weapons\\" );
        guis_dir = mod_parser.PullAsString( "MOD.guis_dir", "guis\\" );
        gamedata_dir = mod_parser.PullAsString( "MOD.gamedata_dir", "gamedata\\" );
        if( unit_dir[ unit_dir.length() - 1 ] != '\\' )			unit_dir += "\\";
        if( model_dir[ model_dir.length() - 1 ] != '\\' )		model_dir += "\\";
        if( download_dir[ download_dir.length() - 1 ] != '\\' )	download_dir += "\\";
        if( weapon_dir[ weapon_dir.length() - 1 ] != '\\' )		weapon_dir += "\\";
        if( guis_dir[ guis_dir.length() - 1 ] != '\\' )			guis_dir += "\\";
        if( gamedata_dir[ gamedata_dir.length() - 1 ] != '\\' )	gamedata_dir += "\\";

        UTILS::cTAFileParser sidedata_parser( gamedata_dir + "sidedata.tdf" );

        nb_side = 0;

        while (sidedata_parser.PullAsString( format( "side%d.name", nb_side ), "" ) != "")
        {
            side_name[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.name", nb_side ) ).c_str() );
            side_pref[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.nameprefix", nb_side ) ).c_str() );
            side_com[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.commander", nb_side ) ).c_str() );
            side_int[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.intgaf", nb_side ) ).c_str() );

            int pal_id = sidedata_parser.PullAsInt( format( "side%d.metalcolor", nb_side ) );
            side_int_data[ nb_side ].metal_color = makeacol( pal[ pal_id ].r << 2, pal[ pal_id ].g << 2, pal[ pal_id ].b << 2, 0xFF );
            pal_id = sidedata_parser.PullAsInt( format( "side%d.energycolor", nb_side ) );
            side_int_data[ nb_side ].energy_color = makeacol( pal[ pal_id ].r << 2, pal[ pal_id ].g << 2, pal[ pal_id ].b << 2, 0xFF );

            side_int_data[ nb_side ].EnergyBar = read_gui_element( &sidedata_parser, format( "side%d.energybar", nb_side ) );
            side_int_data[ nb_side ].EnergyNum = read_gui_element( &sidedata_parser, format( "side%d.energynum", nb_side ) );
            side_int_data[ nb_side ].EnergyMax = read_gui_element( &sidedata_parser, format( "side%d.energymax", nb_side ) );
            side_int_data[ nb_side ].Energy0 = read_gui_element( &sidedata_parser, format( "side%d.energy0", nb_side ) );
            side_int_data[ nb_side ].EnergyProduced = read_gui_element( &sidedata_parser, format( "side%d.energyproduced", nb_side ) );
            side_int_data[ nb_side ].EnergyConsumed = read_gui_element( &sidedata_parser, format( "side%d.energyconsumed", nb_side ) );

            side_int_data[ nb_side ].MetalBar = read_gui_element( &sidedata_parser, format( "side%d.metalbar", nb_side ) );
            side_int_data[ nb_side ].MetalNum = read_gui_element( &sidedata_parser, format( "side%d.metalnum", nb_side ) );
            side_int_data[ nb_side ].MetalMax = read_gui_element( &sidedata_parser, format( "side%d.metalmax", nb_side ) );
            side_int_data[ nb_side ].Metal0 = read_gui_element( &sidedata_parser, format( "side%d.metal0", nb_side ) );
            side_int_data[ nb_side ].MetalProduced = read_gui_element( &sidedata_parser, format( "side%d.metalproduced", nb_side ) );
            side_int_data[ nb_side ].MetalConsumed = read_gui_element( &sidedata_parser, format( "side%d.metalconsumed", nb_side ) );

            side_int_data[ nb_side ].UnitName = read_gui_element( &sidedata_parser, format( "side%d.unitname", nb_side ), true );
            side_int_data[ nb_side ].DamageBar = read_gui_element( &sidedata_parser, format( "side%d.damagebar", nb_side ), true );

            side_int_data[ nb_side ].UnitName2 = read_gui_element( &sidedata_parser, format( "side%d.unitname2", nb_side ), true );
            side_int_data[ nb_side ].DamageBar2 = read_gui_element( &sidedata_parser, format( "side%d.damagebar2", nb_side ), true );

            side_int_data[ nb_side ].UnitMetalMake = read_gui_element( &sidedata_parser, format( "side%d.unitmetalmake", nb_side ), true );
            side_int_data[ nb_side ].UnitMetalUse = read_gui_element( &sidedata_parser, format( "side%d.unitmetaluse", nb_side ), true );
            side_int_data[ nb_side ].UnitEnergyMake = read_gui_element( &sidedata_parser, format( "side%d.unitenergymake", nb_side ), true );
            side_int_data[ nb_side ].UnitEnergyUse = read_gui_element( &sidedata_parser, format( "side%d.unitenergyuse", nb_side ), true );

            side_int_data[ nb_side ].Name = read_gui_element( &sidedata_parser, format( "side%d.name", nb_side ), true );
            side_int_data[ nb_side ].Description = read_gui_element( &sidedata_parser, format( "side%d.description", nb_side ), true );

            nb_side++;
        }
    }

    int	SideData::getSideId(const String& side) const
    {
        for(int i = 0; i < nb_side; ++i)
        {
            if(side == side_name[i])
                return i;
        }
        return -1;
    }


    char* SideData::get_line(char *data) const
    {
        int pos = 0;
        while (data[pos] != 0 && data[pos] != 13 && data[pos] != 10)
            ++pos;
        char*d = new char[pos + 1];
        memcpy(d, data, pos);
        d[pos] = 0;
        return d;
    }



    IntrElementCoords read_gui_element(UTILS::cTAFileParser* parser, const String& element, bool bottom)
    {
        IntrElementCoords gui_element;
        gui_element.x1 = parser->PullAsInt(element + ".x1");
        gui_element.y1 = parser->PullAsInt(element + ".y1");
        gui_element.x2 = parser->PullAsInt(element + ".x2");
        gui_element.y2 = parser->PullAsInt(element + ".y2");
        if (bottom)
        {
            gui_element.y1 += SCREEN_H - 480;
            gui_element.y2 += SCREEN_H - 480;
        }
        return gui_element;
    }



} // namespace TA3D
