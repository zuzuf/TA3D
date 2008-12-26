
#include "statistics.h"
#include "../../ta3dbase.h"
#include "../../EngineClass.h"
#include "../players.h"




namespace TA3D
{
namespace Menus
{

    
    bool Statistics::Execute()
    {
        Statistics m;
        return m.execute();
    }



    Statistics::Statistics()
        :Abstract()
    {}

    Statistics::~Statistics()
    {}


    void Statistics::doUpdateObject(const String& id, const short indx, const uint32 color, const String& value)
    {
        String nameID("statistics.");
        nameID << id << indx;
        pArea->set_caption(nameID, value);
        GUIOBJ* o = pArea->get_object(nameID);
        if (o)
            o->Data = color;
    }

    bool Statistics::doInitialize()
    {
        LOG_DEBUG(LOG_PREFIX_MENU_STATS << "Entering...");
        gfx->set_2D_mode();
        gfx->ReInitTexSys();
        loadAreaFromTDF("statistics", "gui/statistics.area");

        // Statistics for All players
        for (short i = 0; i < players.nb_player; ++i)
        {
            uint32 color = gfx->makeintcol(player_color[3 * player_color_map[i]],
                                           player_color[3 * player_color_map[i] + 1],
                                           player_color[3 * player_color_map[i] + 2]);

            doUpdateObject("player", i, color, players.nom[i]);
            doUpdateObject("side",   i, color, players.side[i]);
            doUpdateObject("losses", i, color, players.losses[i]);
            doUpdateObject("kills",  i, color, players.kills[i]);
            doUpdateObject("energy", i, color, round(players.energy_total[i]));
            doUpdateObject("metal",  i, color, round(players.metal_total[i]));
        }
        return true;
    }


    void Statistics::doFinalize()
    {
        // Do nothing
        LOG_DEBUG(LOG_PREFIX_MENU_STATS << "Done.");
    }


    void Statistics::waitForEvent()
    {
        bool keyIsPressed(false);
        do
        {
            // Get if a key was pressed
            keyIsPressed = keypressed();
            // Grab user events
            pArea->check();
            // Wait to reduce CPU consumption 
            rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);

        } while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
                 && mouse_b == 0
                 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE]
                 && !keyIsPressed && !pArea->scrolling);
    }


    bool Statistics::maySwitchToAnotherMenu()
    {
        // Exit
        return (key[KEY_SPACE] || key[KEY_ENTER] || key[KEY_ESC] || pArea->get_state("statistics.b_ok"));
    }


} // namespace Menus
} // namespace TA3D


