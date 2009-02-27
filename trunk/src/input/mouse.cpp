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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"

int TA3D::VARS::mouse_x = 0;
int TA3D::VARS::mouse_y = 0;
int TA3D::VARS::mouse_z = 0;
int TA3D::VARS::mouse_b = 0;
int CURSOR_MOVE;
int CURSOR_GREEN;
int CURSOR_CROSS;
int CURSOR_RED;
int CURSOR_LOAD;
int CURSOR_UNLOAD;
int CURSOR_GUARD;
int CURSOR_PATROL;
int CURSOR_REPAIR;
int CURSOR_ATTACK;
int CURSOR_BLUE;
int CURSOR_AIR_LOAD;
int CURSOR_BOMB_ATTACK;
int CURSOR_BALANCE;
int CURSOR_RECLAIM;
int CURSOR_WAIT;
int CURSOR_CANT_ATTACK;
int CURSOR_CROSS_LINK;
int CURSOR_CAPTURE;
int CURSOR_REVIVE;

TA3D::Gaf::AnimationList cursor;

int cursor_type = CURSOR_DEFAULT;

using namespace TA3D::VARS;

namespace TA3D
{
    void poll_mouse()
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                {
                    set_key_down( event.key.keysym.sym );
                    uint32 c = event.key.keysym.unicode;
                    c |= event.key.keysym.sym << 16;
                    VARS::keybuf.push_back( c );
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                switch(event.button.button)
                {
                case SDL_BUTTON_WHEELDOWN:
                    if (event.button.state == SDL_PRESSED)
                        mouse_z--;
                    break;
                case SDL_BUTTON_WHEELUP:
                    if (event.button.state == SDL_PRESSED)
                        mouse_z++;
                    break;
                };
                break;
            case SDL_KEYUP:
                set_key_up( event.key.keysym.sym );
                break;
            };
        }
        mouse_b = 0;
        uint8 m_b = SDL_GetMouseState( &mouse_x, &mouse_y );
        if (m_b & SDL_BUTTON(1))    mouse_b |= 1;
        if (m_b & SDL_BUTTON(3))    mouse_b |= 2;
        if (m_b & SDL_BUTTON(2))    mouse_b |= 4;
    }

	void position_mouse(int x, int y)
	{
	    mouse_x = x;
	    mouse_y = y;
	    SDL_WarpMouse(x,y);
	    poll_mouse();
        SDL_GetRelativeMouseState(NULL, NULL);
	}

	void get_mouse_mickeys(int *mx, int *my)
	{
	    poll_mouse();
        SDL_GetRelativeMouseState(mx, my);
	}

    uint32 start = 0;

    int anim_cursor(const int type)
    {
        return (type == -1)
            ? ((msec_timer-start) / 100) % cursor[cursor_type].nb_bmp
            : ((msec_timer-start) / 100) % cursor[type].nb_bmp;
    }

    void draw_cursor()
    {
        int curseur = anim_cursor();
        if (curseur < 0 || curseur >= cursor[cursor_type].nb_bmp)
        {
            curseur = 0;
            start = msec_timer;
        }
        float dx = cursor[cursor_type].ofs_x[curseur];
        float dy = cursor[cursor_type].ofs_y[curseur];
        float sx = cursor[cursor_type].bmp[curseur]->w;
        float sy = cursor[cursor_type].bmp[curseur]->h;
        gfx->set_color(0xFFFFFFFF);
        gfx->set_alpha_blending();
        gfx->drawtexture(cursor[cursor_type].glbmp[curseur],
                         mouse_x - dx,
                         mouse_y - dy,
                         mouse_x - dx + sx,
                         mouse_y - dy + sy);
        gfx->unset_alpha_blending();
    }

    void init_mouse()
    {
        mouse_b = 0;
        position_mouse(SCREEN_W >> 1, SCREEN_H >> 1);

        SDL_ShowCursor(SDL_DISABLE);

        // Loading and creating cursors
        byte *data = HPIManager->PullFromHPI("anims\\cursors.gaf");	// Load cursors
        if (data)
        {
            cursor.loadGAFFromRawData(data, true);
            cursor.convert();

            CURSOR_MOVE        = cursor.findByName("cursormove"); // Match cursor variables with cursor anims
            CURSOR_GREEN       = cursor.findByName("cursorgrn");
            CURSOR_CROSS       = cursor.findByName("cursorselect");
            CURSOR_RED         = cursor.findByName("cursorred");
            CURSOR_LOAD        = cursor.findByName("cursorload");
            CURSOR_UNLOAD      = cursor.findByName("cursorunload");
            CURSOR_GUARD       = cursor.findByName("cursordefend");
            CURSOR_PATROL      = cursor.findByName("cursorpatrol");
            CURSOR_REPAIR      = cursor.findByName("cursorrepair");
            CURSOR_ATTACK      = cursor.findByName("cursorattack");
            CURSOR_BLUE        = cursor.findByName("cursornormal");
            CURSOR_AIR_LOAD    = cursor.findByName("cursorpickup");
            CURSOR_BOMB_ATTACK = cursor.findByName("cursorairstrike");
            CURSOR_BALANCE     = cursor.findByName("cursorunload");
            CURSOR_RECLAIM     = cursor.findByName("cursorreclamate");
            CURSOR_WAIT        = cursor.findByName("cursorhourglass");
            CURSOR_CANT_ATTACK = cursor.findByName("cursortoofar");
            CURSOR_CROSS_LINK  = cursor.findByName("pathicon");
            CURSOR_CAPTURE     = cursor.findByName("cursorcapture");
            CURSOR_REVIVE      = cursor.findByName("cursorrevive");
            if (CURSOR_REVIVE == -1) // If you don't have the required cursors, then resurrection won't work
                CURSOR_REVIVE = cursor.findByName("cursorreclamate");
            delete[] data;
        }
        else
        {
            LOG_DEBUG("RESOURCES ERROR : (anims\\cursors.gaf not found)");
            exit(2);
        }
        cursor_type = CURSOR_DEFAULT;
    }
}
