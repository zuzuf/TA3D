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
        extern bool                             key[256];
    }

	/*!
	** \brief return true is there are key codes waiting in the buffer, false otherwise
	*/
	bool keypressed();

	/*!
	** \brief return the next key code in the key buffer
	*/
	uint32 readkey();

    //! \brief convert a key code to a char representation
    char keycode2char(int keycode);

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
    void set_key_up(int key);

	/*!
	** \brief set a key down
	*/
    void set_key_down(int key);

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

enum KeyCode
{
    KEY_NONE = 0,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_ENTER,
    KEY_SPACE,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_TAB,
    KEY_LSHIFT,
    KEY_LCONTROL,
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_DEL,
    KEY_ALT,
    KEY_PAUSE,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_PLUS,
    KEY_MINUS,

    KEY_A = 'a',
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_PAGEUP,
    KEY_PAGEDOWN,

    // This is used to show/hide the console
    KEY_TILDE,

    KEY_CAPSLOCK,

    KEY_END,
    KEY_HOME,

    KEY_RSHIFT = KEY_LSHIFT,
    KEY_RCONTROL = KEY_LCONTROL,
    KEY_PLUS_PAD = KEY_PLUS,
    KEY_MINUS_PAD = KEY_MINUS,
    KEY_ENTER_PAD = KEY_ENTER
};

#endif
