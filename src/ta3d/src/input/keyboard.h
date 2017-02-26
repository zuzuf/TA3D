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

#ifndef __TA3D_KEYBOARD_H__
# define __TA3D_KEYBOARD_H__

# include <deque>
# include <stdafx.h>
# include <Qt>


namespace TA3D
{
	namespace VARS
	{
		extern int								ascii_to_scancode[256];
		extern bool                             key[0x1000];
		extern bool                             prevkey_down[0x1000];
		extern bool                             prevkey_up[0x1000];
		extern std::deque<uint32>               keybuf;
	}

	/*!
	** \brief return true is there are key codes waiting in the buffer, false otherwise
	*/
	bool keypressed();

	/*!
	** \brief return the next key code in the key buffer
	*/
	uint32 readkey();

	/*!
	** \brief clears the key code buffer
	*/
	void clear_keybuf();

	/*!
	** \brief initialize keyboard handler
	*/
	void init_keyboard();

	/*!
	** \brief set a key up
	*/
	void set_key_up(uint16 keycode);

	/*!
	** \brief set a key down
	*/
	void set_key_down(uint16 keycode);

    /*!
    ** \brief returns true if the given key state has changed to down since last call with the same key
    */
    bool key_down_event(uint16 keycode);

    /*!
    ** \brief returns true if the given key state has changed to up since last call with the same key
    */
    bool key_up_event(uint16 keycode);

    /*!
    ** \brief returns true if the given key state has changed since last call with the same key
    */
    bool key_event(uint16 keycode);
}

#define KEY_ENTER       Qt::Key_Enter
#define KEY_SPACE       Qt::Key_Space
#define KEY_LEFT        Qt::Key_Left
#define KEY_RIGHT       Qt::Key_Right
#define KEY_UP          Qt::Key_Up
#define KEY_DOWN        Qt::Key_Down
#define KEY_TAB         Qt::Key_Tab
#define KEY_LSHIFT      Qt::Key_Shift
#define KEY_RSHIFT      Qt::Key_Shift
#define KEY_LCONTROL    Qt::Key_Control
#define KEY_RCONTROL    Qt::Key_Control
#define KEY_ESC         Qt::Key_Escape
#define KEY_BACKSPACE   Qt::Key_Backspace
#define KEY_DEL         Qt::Key_Delete
#define KEY_ALT         Qt::Key_Alt
#define KEY_PAUSE       Qt::Key_Pause

#define KEY_F1          Qt::Key_F1
#define KEY_F2          Qt::Key_F2
#define KEY_F3          Qt::Key_F3
#define KEY_F4          Qt::Key_F4
#define KEY_F5          Qt::Key_F5
#define KEY_F6          Qt::Key_F6
#define KEY_F7          Qt::Key_F7
#define KEY_F8          Qt::Key_F8
#define KEY_F9          Qt::Key_F9
#define KEY_F10         Qt::Key_F10
#define KEY_F11         Qt::Key_F11
#define KEY_F12         Qt::Key_F12

#define KEY_0           Qt::Key_0
#define KEY_1           Qt::Key_1
#define KEY_2           Qt::Key_2
#define KEY_3           Qt::Key_3
#define KEY_4           Qt::Key_4
#define KEY_5           Qt::Key_5
#define KEY_6           Qt::Key_6
#define KEY_7           Qt::Key_7
#define KEY_8           Qt::Key_8
#define KEY_9           Qt::Key_9

#define KEY_PLUS        Qt::Key_Plus
#define KEY_MINUS       Qt::Key_Minus
#define KEY_PLUS_PAD    Qt::Key_Plus
#define KEY_MINUS_PAD   Qt::Key_Minus

#define KEY_A           Qt::Key_A
#define KEY_B           Qt::Key_B
#define KEY_C           Qt::Key_C
#define KEY_D           Qt::Key_D
#define KEY_E           Qt::Key_E
#define KEY_F           Qt::Key_F
#define KEY_G           Qt::Key_G
#define KEY_H           Qt::Key_H
#define KEY_I           Qt::Key_I
#define KEY_J           Qt::Key_J
#define KEY_K           Qt::Key_K
#define KEY_L           Qt::Key_L
#define KEY_M           Qt::Key_M
#define KEY_N           Qt::Key_N
#define KEY_O           Qt::Key_O
#define KEY_P           Qt::Key_P
#define KEY_Q           Qt::Key_Q
#define KEY_R           Qt::Key_R
#define KEY_S           Qt::Key_S
#define KEY_T           Qt::Key_T
#define KEY_U           Qt::Key_U
#define KEY_V           Qt::Key_V
#define KEY_W           Qt::Key_W
#define KEY_X           Qt::Key_X
#define KEY_Y           Qt::Key_Y
#define KEY_Z           Qt::Key_Z
#define KEY_PAGEUP      Qt::Key_PageUp
#define KEY_PAGEDOWN    Qt::Key_PageDown

// This is used to show/hide the console
#define KEY_TILDE       Qt::Key_ParenRight

#define KEY_CAPSLOCK    Qt::Key_CapsLock

#define KEY_END         Qt::Key_End
#define KEY_HOME        Qt::Key_Home
#define KEY_ENTER_PAD	Qt::Key_Enter

#endif
