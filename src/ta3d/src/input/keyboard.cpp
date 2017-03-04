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

#include <stdafx.h>
#include "keyboard.h"
#include "mouse.h"
#include <QHash>

using namespace TA3D::VARS;

namespace TA3D
{
	namespace VARS
	{
        int						ascii_to_scancode[256];
        bool                    key[256];
        bool                    prevkey_down[256];
        bool                    prevkey_up[256];
		std::deque<uint32>      keybuf;
        QHash<int, uint16>      qt2Keycode;
	}


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

	void init_keyboard()
	{
        memset(VARS::key, 0, sizeof(VARS::key));
		VARS::keybuf.clear();

        qt2Keycode[Qt::Key_Enter] = KEY_ENTER;
//        qt2Keycode[Qt::Key_Enter] = KEY_ENTER_PAD;
        qt2Keycode[Qt::Key_Space] = KEY_SPACE;
        qt2Keycode[Qt::Key_Left] = KEY_LEFT;
        qt2Keycode[Qt::Key_Right] = KEY_RIGHT;
        qt2Keycode[Qt::Key_Up] = KEY_UP;
        qt2Keycode[Qt::Key_Down] = KEY_DOWN;
        qt2Keycode[Qt::Key_Tab] = KEY_TAB;
        qt2Keycode[Qt::Key_Shift] = KEY_LSHIFT;
//        qt2Keycode[Qt::Key_Shift] = KEY_RSHIFT;
        qt2Keycode[Qt::Key_Control] = KEY_LCONTROL;
//        qt2Keycode[Qt::Key_Control] = KEY_RCONTROL;
        qt2Keycode[Qt::Key_Escape] = KEY_ESC;
        qt2Keycode[Qt::Key_Backspace] = KEY_BACKSPACE;
        qt2Keycode[Qt::Key_Delete] = KEY_DEL;
        qt2Keycode[Qt::Key_Alt] = KEY_ALT;
        qt2Keycode[Qt::Key_Pause] = KEY_PAUSE;

        qt2Keycode[Qt::Key_F1] = KEY_F1;
        qt2Keycode[Qt::Key_F2] = KEY_F2;
        qt2Keycode[Qt::Key_F3] = KEY_F3;
        qt2Keycode[Qt::Key_F4] = KEY_F4;
        qt2Keycode[Qt::Key_F5] = KEY_F5;
        qt2Keycode[Qt::Key_F6] = KEY_F6;
        qt2Keycode[Qt::Key_F7] = KEY_F7;
        qt2Keycode[Qt::Key_F8] = KEY_F8;
        qt2Keycode[Qt::Key_F9] = KEY_F9;
        qt2Keycode[Qt::Key_F10] = KEY_F10;
        qt2Keycode[Qt::Key_F11] = KEY_F11;
        qt2Keycode[Qt::Key_F12] = KEY_F12;

        qt2Keycode[Qt::Key_0] = KEY_0;
        qt2Keycode[Qt::Key_1] = KEY_1;
        qt2Keycode[Qt::Key_2] = KEY_2;
        qt2Keycode[Qt::Key_3] = KEY_3;
        qt2Keycode[Qt::Key_4] = KEY_4;
        qt2Keycode[Qt::Key_5] = KEY_5;
        qt2Keycode[Qt::Key_6] = KEY_6;
        qt2Keycode[Qt::Key_7] = KEY_7;
        qt2Keycode[Qt::Key_8] = KEY_8;
        qt2Keycode[Qt::Key_9] = KEY_9;

        qt2Keycode[Qt::Key_Plus] = KEY_PLUS;
//        qt2Keycode[Qt::Key_Plus] = KEY_PLUS_PAD;
        qt2Keycode[Qt::Key_Minus] = KEY_MINUS;
//        qt2Keycode[Qt::Key_Minus] = KEY_MINUS_PAD;

        qt2Keycode[Qt::Key_A] = KEY_A;
        qt2Keycode[Qt::Key_B] = KEY_B;
        qt2Keycode[Qt::Key_C] = KEY_C;
        qt2Keycode[Qt::Key_D] = KEY_D;
        qt2Keycode[Qt::Key_E] = KEY_E;
        qt2Keycode[Qt::Key_F] = KEY_F;
        qt2Keycode[Qt::Key_G] = KEY_G;
        qt2Keycode[Qt::Key_H] = KEY_H;
        qt2Keycode[Qt::Key_I] = KEY_I;
        qt2Keycode[Qt::Key_J] = KEY_J;
        qt2Keycode[Qt::Key_K] = KEY_K;
        qt2Keycode[Qt::Key_L] = KEY_L;
        qt2Keycode[Qt::Key_M] = KEY_M;
        qt2Keycode[Qt::Key_N] = KEY_N;
        qt2Keycode[Qt::Key_O] = KEY_O;
        qt2Keycode[Qt::Key_P] = KEY_P;
        qt2Keycode[Qt::Key_Q] = KEY_Q;
        qt2Keycode[Qt::Key_R] = KEY_R;
        qt2Keycode[Qt::Key_S] = KEY_S;
        qt2Keycode[Qt::Key_T] = KEY_T;
        qt2Keycode[Qt::Key_U] = KEY_U;
        qt2Keycode[Qt::Key_V] = KEY_V;
        qt2Keycode[Qt::Key_W] = KEY_W;
        qt2Keycode[Qt::Key_X] = KEY_X;
        qt2Keycode[Qt::Key_Y] = KEY_Y;
        qt2Keycode[Qt::Key_Z] = KEY_Z;
        qt2Keycode[Qt::Key_PageUp] = KEY_PAGEUP;
        qt2Keycode[Qt::Key_PageDown] = KEY_PAGEDOWN;

        // This is used to show/hide the console
        qt2Keycode[Qt::Key_ParenRight] = KEY_TILDE;

        qt2Keycode[Qt::Key_CapsLock] = KEY_CAPSLOCK;

        qt2Keycode[Qt::Key_End] = KEY_END;
        qt2Keycode[Qt::Key_Home] = KEY_HOME;

		// Initializing the ascii to scancode table
        memset(ascii_to_scancode, 0, sizeof(ascii_to_scancode));

		ascii_to_scancode[int('a')] = KEY_A;
		ascii_to_scancode[int('b')] = KEY_B;
		ascii_to_scancode[int('c')] = KEY_C;
		ascii_to_scancode[int('d')] = KEY_D;
		ascii_to_scancode[int('e')] = KEY_E;
		ascii_to_scancode[int('f')] = KEY_F;
		ascii_to_scancode[int('g')] = KEY_G;
		ascii_to_scancode[int('h')] = KEY_H;
		ascii_to_scancode[int('i')] = KEY_I;
		ascii_to_scancode[int('j')] = KEY_J;
		ascii_to_scancode[int('k')] = KEY_K;
		ascii_to_scancode[int('l')] = KEY_L;
		ascii_to_scancode[int('m')] = KEY_M;
		ascii_to_scancode[int('n')] = KEY_N;
		ascii_to_scancode[int('o')] = KEY_O;
		ascii_to_scancode[int('p')] = KEY_P;
		ascii_to_scancode[int('q')] = KEY_Q;
		ascii_to_scancode[int('r')] = KEY_R;
		ascii_to_scancode[int('s')] = KEY_S;
		ascii_to_scancode[int('t')] = KEY_T;
		ascii_to_scancode[int('u')] = KEY_U;
		ascii_to_scancode[int('v')] = KEY_V;
		ascii_to_scancode[int('w')] = KEY_W;
		ascii_to_scancode[int('x')] = KEY_X;
		ascii_to_scancode[int('y')] = KEY_Y;
		ascii_to_scancode[int('z')] = KEY_Z;

		for (int i = 0; i < 26; ++i)
			ascii_to_scancode[int('A' + i) ] = ascii_to_scancode[int('a' + i)];

		ascii_to_scancode[int('0')] = KEY_0;
		ascii_to_scancode[int('1')] = KEY_1;
		ascii_to_scancode[int('2')] = KEY_2;
		ascii_to_scancode[int('3')] = KEY_3;
		ascii_to_scancode[int('4')] = KEY_4;
		ascii_to_scancode[int('5')] = KEY_5;
		ascii_to_scancode[int('6')] = KEY_6;
		ascii_to_scancode[int('7')] = KEY_7;
		ascii_to_scancode[int('8')] = KEY_8;
		ascii_to_scancode[int('9')] = KEY_9;

		ascii_to_scancode[int(' ')] = KEY_SPACE;
		ascii_to_scancode[int('\n')] = KEY_ENTER;
		ascii_to_scancode[27] = KEY_ESC;
	}

    char keycode2char(int keycode)
    {
        switch(keycode)
        {
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            return keycode - KEY_0 + '0';
        case KEY_A:
        case KEY_B:
        case KEY_C:
        case KEY_D:
        case KEY_E:
        case KEY_F:
        case KEY_G:
        case KEY_H:
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_M:
        case KEY_N:
        case KEY_O:
        case KEY_P:
        case KEY_Q:
        case KEY_R:
        case KEY_S:
        case KEY_T:
        case KEY_U:
        case KEY_V:
        case KEY_W:
        case KEY_X:
        case KEY_Y:
        case KEY_Z:
            return keycode - KEY_A + 'a';
        case KEY_PLUS:
            return '+';
        case KEY_MINUS:
            return '-';
        case KEY_SPACE:
            return ' ';
        }
        return 0;
    }

    void set_key_down(int key)
	{
        const auto it = qt2Keycode.find(key);
        if (it == qt2Keycode.end())
            return;

        VARS::key[it.value()] = true;
        keybuf.push_back(it.value());
    }


    void set_key_up(int key)
	{
        const auto it = qt2Keycode.find(key);
        if (it == qt2Keycode.end())
			return;

        VARS::key[it.value()] = false;
	}


	bool key_down_event(uint16 keycode)
	{
        if (keycode >= 256)
			return false;

		if (!prevkey_down[keycode] && key[keycode])
		{
			prevkey_down[keycode] = true;
			return true;
		}
		prevkey_down[keycode] = key[keycode];
		return false;
	}


	bool key_up_event(uint16 keycode)
	{
        if (keycode >= 256)
			return false;

		if (prevkey_up[keycode] && !key[keycode])
		{
			prevkey_up[keycode] = false;
			return true;
		}
		prevkey_up[keycode] = key[keycode];
		return false;
	}

	bool key_event(uint16 keycode)
	{
		return key_down_event(keycode) || key_up_event(keycode);
	}





} // namespace TA3D

