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

void glbutton(const String &caption,float x1,float y1,float x2,float y2,bool etat=false);

const String msg_box(TA3D::Interfaces::GfxFont fnt,const String &title,const String &msg,bool ask);

//-------------- These are the GUI functions needed by the editors ----------------------------

		// Pour les couleurs standards

struct wnd				// Pour la gestion directe de l'interface dans le programme
{
   int		x,y;			// coordinates
   int		width,height;	// size
   String	Title;			// name
};


/*--------- Functions that can use the skin object -------------------------------------------------*/

void button ( float x, float y, float x2, float y2, const String &Title, bool Etat, float s=1.0f , SKIN *skin=NULL );
void FloatMenu( float x, float y, const String::Vector &Entry, int Index, int StartEntry=0 , SKIN *skin=NULL, float size = 1.0f );
void ListBox( float x1, float y1, float x2, float y2, const String::Vector &Entry, int Index, int Scroll , SKIN *skin=NULL, float size = 1.0f, uint32 flags = 0 );
void OptionButton( float x, float y, const String &Title, bool Etat , SKIN *skin=NULL, float size = 1.0f );
void OptionCase( float x, float y, const String &Title, bool Etat , SKIN *skin=NULL, float size = 1.0f );
void TextBar( float x1, float y1, float x2, float y2, const String &Caption, bool Etat , SKIN *skin=NULL, float size = 1.0f );
void ProgressBar( float x1, float y1, float x2, float y2, int Value , SKIN *skin=NULL, float size = 1.0f );
void PopupMenu( float x1, float y1, const String &msg, SKIN *skin=NULL, float size = 1.0f );
void ScrollBar( float x1, float y1, float x2, float y2, float Value, bool vertical=true, SKIN *skin = NULL, float size = 1.0f );
int draw_text_adjust( float x1, float y1, float x2, float y2, String msg, float size, int pos = 0, bool mission_mode = false );

/*--------------------------------------------------------------------------------------------------*/

void draw_Window(wnd& Wnd );
unsigned char WinMov( int AMx, int AMy, int AMb, int Mx, int My, int Mb, wnd *Wnd );
String dirname( String fname );
const String Dialog( const String &Title, String Filter = "*.*" );
bool WndAsk( const String &Title, const String &Msg, int ASW_TYPE=ASW_OKCANCEL );
void Popup( const String &Title, const String &Msg );
const String GetVal( const String &Title );

extern float gui_font_h;

extern bool	use_normal_alpha_function;


} // namespace TA3D

#endif
