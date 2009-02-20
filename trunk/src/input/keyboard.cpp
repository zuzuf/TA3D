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
#include "keyboard.h"
#include "mouse.h"

int						TA3D::VARS::ascii_to_scancode[ 256 ];
int                     TA3D::VARS::key[0x1000];
std::list<uint32>       TA3D::VARS::keybuf;
int                     TA3D::VARS::remap[256];

using namespace TA3D::VARS;

namespace TA3D
{
	uint32 readkey()
	{
	    if (VARS::keybuf.empty())
            return 0;

	    uint32 res = VARS::keybuf.front();
	    VARS::keybuf.pop_front();
	    return res;
    }

	bool keypressed()
	{
	    return !VARS::keybuf.empty();
    }

	void clear_keybuf()
	{
	    VARS::keybuf.clear();
	}

    void poll_keyboard()
    {
        poll_mouse();
    }

    void init_keyboard()
    {
        SDL_EnableUNICODE(1);

        SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

        // We need some remapping hack to support some keyboards (french keyboards don't access KEY_0..9)
        for(int i = 0 ; i < 256 ; i++)
            remap[i] = 0;

        remap[38] = KEY_1;
        remap[233] = KEY_2;
        remap[34] = KEY_3;
        remap[39] = KEY_4;
        remap[40] = KEY_5;
        remap[45] = KEY_6;
        remap[232] = KEY_7;
        remap[95] = KEY_8;
        remap[231] = KEY_9;
        remap[224] = KEY_0;

        for(int i = 0 ; i < 0x1000 ; i++)
            VARS::key[i] = 0;
	    VARS::keybuf.clear();

        // Initializing the ascii to scancode table
        for (int i = 0; i < 256; ++i)
            ascii_to_scancode[i] = 0;

        ascii_to_scancode[ 'a' ] = KEY_A;
        ascii_to_scancode[ 'b' ] = KEY_B;
        ascii_to_scancode[ 'c' ] = KEY_C;
        ascii_to_scancode[ 'd' ] = KEY_D;
        ascii_to_scancode[ 'e' ] = KEY_E;
        ascii_to_scancode[ 'f' ] = KEY_F;
        ascii_to_scancode[ 'g' ] = KEY_G;
        ascii_to_scancode[ 'h' ] = KEY_H;
        ascii_to_scancode[ 'i' ] = KEY_I;
        ascii_to_scancode[ 'j' ] = KEY_J;
        ascii_to_scancode[ 'k' ] = KEY_K;
        ascii_to_scancode[ 'l' ] = KEY_L;
        ascii_to_scancode[ 'm' ] = KEY_M;
        ascii_to_scancode[ 'n' ] = KEY_N;
        ascii_to_scancode[ 'o' ] = KEY_O;
        ascii_to_scancode[ 'p' ] = KEY_P;
        ascii_to_scancode[ 'q' ] = KEY_Q;
        ascii_to_scancode[ 'r' ] = KEY_R;
        ascii_to_scancode[ 's' ] = KEY_S;
        ascii_to_scancode[ 't' ] = KEY_T;
        ascii_to_scancode[ 'u' ] = KEY_U;
        ascii_to_scancode[ 'v' ] = KEY_V;
        ascii_to_scancode[ 'w' ] = KEY_W;
        ascii_to_scancode[ 'x' ] = KEY_X;
        ascii_to_scancode[ 'y' ] = KEY_Y;
        ascii_to_scancode[ 'z' ] = KEY_Z;

        for (int i = 0; i < 26; ++i)
            ascii_to_scancode[ 'A' + i ] = ascii_to_scancode[ 'a' + i ];

        ascii_to_scancode[ '0' ] = KEY_0;
        ascii_to_scancode[ '1' ] = KEY_1;
        ascii_to_scancode[ '2' ] = KEY_2;
        ascii_to_scancode[ '3' ] = KEY_3;
        ascii_to_scancode[ '4' ] = KEY_4;
        ascii_to_scancode[ '5' ] = KEY_5;
        ascii_to_scancode[ '6' ] = KEY_6;
        ascii_to_scancode[ '7' ] = KEY_7;
        ascii_to_scancode[ '8' ] = KEY_8;
        ascii_to_scancode[ '9' ] = KEY_9;

        ascii_to_scancode[ ' ' ] = KEY_SPACE;
        ascii_to_scancode[ '\n' ] = KEY_ENTER;
        ascii_to_scancode[ 27 ] = KEY_ESC;
    }

    void set_key_down(uint16 keycode)
    {
        if (keycode < 256 && remap[keycode])
            VARS::key[remap[keycode]] = 1;
        VARS::key[keycode] = 1;
    }

    void set_key_up(uint16 keycode)
    {
        if (keycode < 256 && remap[keycode])
            VARS::key[remap[keycode]] = 0;
        VARS::key[keycode] = 0;
    }
}
