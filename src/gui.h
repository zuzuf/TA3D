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

/*---------------------------------------------------------------------------------\
|                                        gui.h                                     |
|         Contient les fonctions nécessaires à la gestion de l'interface de ta3D   |
|  comme les boutons, les fenêtres,...                                             |
|                                                                                  |
\---------------------------------------------------------------------------------*/

#ifndef MODULE_GUI
#define MODULE_GUI

#include "gfx/gfx.h"
#include "misc/hash_table.h"
#include "threads/thread.h"
#include "TA3D_NameSpace.h"
#include <vector>
#include "gfx/gui/base.h"
#include "gfx/gui/skin.h"


namespace TA3D
{

//void draw_Window(wnd& Wnd );
//unsigned char WinMov( int AMx, int AMy, int AMb, int Mx, int My, int Mb, wnd *Wnd );
const String Dialogf( const String &Title, String Filter = "*.*" );
bool WndAsk( const String &Title, const String &Msg, int ASW_TYPE=ASW_OKCANCEL );
void Popup( const String &Title, const String &Msg );
const String GetVal( const String &Title );

} // namespace TA3D

#endif
