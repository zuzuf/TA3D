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
#include <TA3D_NameSpace.h>
#include "mouse.h"
#include "keyboard.h"
#include <misc/math.h>
#include <QApplication>
#include <misc/timer.h>



float TA3D::VARS::fmouse_x = 0.0f;
float TA3D::VARS::fmouse_y = 0.0f;
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

	int old_mx = 0;
	int old_my = 0;

	void poll_inputs()
	{
        qApp->processEvents();

//        mouse_b = 0;
//		int dx(0), dy(0);
//		int rmx(0), rmy(0);
//		uint8 m_b = SDL_GetMouseState( &rmx, &rmy );
//		dx = rmx - old_mx;
//		dy = rmy - old_my;
//		fmouse_x += float(dx) * lp_CONFIG->mouse_sensivity;
//		fmouse_y += float(dy) * lp_CONFIG->mouse_sensivity;
//		if (m_b & SDL_BUTTON(1))    mouse_b |= 1;
//		if (m_b & SDL_BUTTON(3))    mouse_b |= 2;
//		if (m_b & SDL_BUTTON(2))    mouse_b |= 4;
//        fmouse_x = Math::Clamp(fmouse_x, 0.f, (float)(SCREEN_W));
//        fmouse_y = Math::Clamp(fmouse_y, 0.f, (float)SCREEN_H);
//		mouse_x = (int)(fmouse_x + 0.5f);
//		mouse_y = (int)(fmouse_y + 0.5f);
//		if (rmx != mouse_x || rmy != mouse_y)
//			SDL_WarpMouse(uint16(mouse_x), uint16(mouse_y));
//		old_mx = mouse_x;
//		old_my = mouse_y;

//		if (lp_CONFIG->fullscreen && key[KEY_ALT] && key[KEY_TAB]&& (SDL_GetAppState() & SDL_APPACTIVE))
//			SDL_WM_IconifyWindow();
	}

	int mouse_lx = 0;
	int mouse_ly = 0;

	void position_mouse(int x, int y)
	{
		mouse_lx += x - mouse_x;
		mouse_ly += y - mouse_y;
		mouse_x = x;
		mouse_y = y;
        mouse_x = Math::Clamp<int>(mouse_x, 0, SCREEN_W);
        mouse_y = Math::Clamp<int>(mouse_y, 0, SCREEN_H);
		fmouse_x = float(mouse_x);
		fmouse_y = float(mouse_y);
		old_mx = mouse_x;
		old_my = mouse_y;
        QCursor::setPos(gfx->mapToGlobal(QPoint(mouse_x, mouse_y)));
//		SDL_WarpMouse(uint16(mouse_x), uint16(mouse_y));
        poll_inputs();
	}

	void get_mouse_mickeys(int *mx, int *my)
	{
		poll_inputs();
		int dx = mouse_x - mouse_lx;
		int dy = mouse_y - mouse_ly;
		mouse_lx = mouse_x;
		mouse_ly = mouse_y;
		if (mx) *mx = dx;
		if (my) *my = dy;
	}

	static uint32 start = 0;

	int anim_cursor(const int type)
	{
		return (type < 0)
            ? ((msectimer() - start) / 100) % cursor[cursor_type].nb_bmp
            : ((msectimer() - start) / 100) % cursor[type].nb_bmp;
	}

	void draw_cursor()
	{
        int cursor_id = anim_cursor();
        if (cursor_id < 0 || cursor_id >= cursor[cursor_type].nb_bmp)
		{
            cursor_id = 0;
            start = msectimer();
		}
        static int prev_cursor_type = -1;
        static int prev_cursor_id = -1;

        if (prev_cursor_id == cursor_id && prev_cursor_type == cursor_type)
            return;

        prev_cursor_type = cursor_type;
        prev_cursor_id = cursor_id;

        const int dx = cursor[cursor_type].ofs_x[cursor_id];
        const int dy = cursor[cursor_type].ofs_y[cursor_id];

        // Render cursor as system cursor (cursor motion looks smoother that way)
        qApp->setOverrideCursor(QCursor(QPixmap::fromImage(cursor[cursor_type].bmp[cursor_id]),dx,dy));
	}

	void init_mouse()
	{
		grab_mouse(lp_CONFIG->grab_inputs);

		mouse_b = 0;
		position_mouse(SCREEN_W >> 1, SCREEN_H >> 1);

		// Loading and creating cursors
        QIODevice *file = VFS::Instance()->readFile("anims/cursors.gaf");	// Load cursors
		if (file)
		{
			cursor.loadGAFFromRawData(file, false);
			cursor.convert(false, false);

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
			delete file;
		}
		else
		{
            LOG_DEBUG("RESOURCES ERROR : (anims/cursors.gaf not found)");
			exit(2);
		}
		cursor_type = CURSOR_DEFAULT;
	}

	void grab_mouse(bool grab)
	{
        gfx->setMouseGrabEnabled(grab);
	}
}
