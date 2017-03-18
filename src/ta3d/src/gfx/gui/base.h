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
#ifndef __TA3D_GFX_GUI_BASE_H__
# define __TA3D_GFX_GUI_BASE_H__

# include <gfx/texture.h>
# include <gfx/gfx.h>


//! \name Standard colors
//@{
//!
# define MGray	    makeacol(128,128,128,255)
//!
# define DGray	    makeacol(64,64,64,255)
//!
# define LGray	    makeacol(192,192,192,255)
//!
# define Black	    makeacol(0,0,0,255)
//!
# define White	    makeacol(255,255,255,255)
//!
# define Blue	    makeacol(0,0,255,255)
//!
# define Red	    makeacol(255,0,0,255)
//!
# define Green	    makeacol(0,255,0,255)
//!
# define Yellow	    makeacol(255,255,0,255)
//!
# define LMGrey	    makeacol(170,170,170,255)
//!
# define DRed       makeacol(128,0,0,255)
//@}





//! \name Object types
//@{
//! Button
# define OBJ_BUTTON		    0x0
//! Floatting menu
# define OBJ_FMENU		    0x1
//! Option Button
# define OBJ_OPTIONB		0x2
//! Progress bar
# define OBJ_PBAR		    0x3
//! Text input
# define OBJ_TEXTBAR        0x4
//! Checkbox
# define OBJ_OPTIONC        0x5
//! Menu
# define OBJ_MENU           0x6
//! Print text
# define OBJ_TEXT           0x7
//! Draw a line
# define OBJ_LINE           0x8
//! Draw a boc
# define OBJ_BOX            0x9
//! Draw a picture
# define OBJ_IMG            0xA
//! Draw a list box
# define OBJ_LIST           0xB
//! TA Button, used for TA interface
# define OBJ_TA_BUTTON      0xC
//! Used for things that are not intended to be drawn
# define OBJ_NONE           0xD
//! Large text input
# define OBJ_TEXTEDITOR     0xE
//! Horizontal slider
# define OBJ_HSLIDER        0xF
//! Vertical slider
# define OBJ_VSLIDER        0x10
//@} // Object types





//! \name IDs
//@{
//! The ID 0 is always used in Cavedog GUI files to represent the first gadget
//! that defines the interface
# define TA_ID_0	                    0x0
//! Makes the gadget a button
# define TA_ID_BUTTON                   0x1
//! Creates a listbox
# define TA_ID_LIST_BOX	                0x2
//! Creates a textfield, it's doesnt have any borders
# define TA_ID_TEXT_FIELD	            0x3
//! Creates a Vertical/horizontal Scroll bar
# define TA_ID_SCROLL_BAR	            0x4
//! It makes the gadget a label
# define TA_ID_LABEL                    0x5
//! This creates a blank surface that will receive a picture at run time
# define TA_ID_BLANK_IMG                0x6
//! It is used to set the default font for labels
# define TA_ID_FONT	                    0x7
//! Used to display a picture box
# define TA_ID_IMG                      0x12
//@}




# define ASW_YESNO          0x0
# define ASW_OKCANCEL       0x1



//! \name Flags
//@{
//! If you can click it
#define	FLAG_CAN_BE_CLICKED                 0x0001
//! If it can be selected
#define	FLAG_CAN_GET_FOCUS                  0x0002
//! If it's highlighted when selected
#define	FLAG_HIGHLIGHT                      0x0004
//! If it must be filled (for type BOX)
#define	FLAG_FILL                           0x0008
//! If it behaves like a switch
#define FLAG_SWITCH	                        0x0010
//! If it's not visible
#define FLAG_HIDDEN	                        0x0020
//! If it can have more than 2 states
#define FLAG_MULTI_STATE                    0x0040
//! If it's a build pic, replace the picture with the current unit's one
#define FLAG_BUILD_PIC	                    0x0080
//! Disabled
#define FLAG_DISABLED                       0x0100
//! Centered
#define FLAG_CENTERED                       0x0200
//! Tell the text renderer to print '\n' correctly and to fit in the given space
#define FLAG_TEXT_ADJUST                    0x0400
//! Tell the object not to draw its borders so you can see the background instead
#define FLAG_NO_BORDER                      0x0800
//! Tell the object not to draw its borders so you can see the background instead
#define FLAG_MISSION_MODE                   0x1000
//@}



namespace TA3D
{
namespace Gui
{

	extern TA3D::Font *gui_font;

} // namespace Gui
} // namespace TA3D



#endif // __TA3D_GFX_GUI_BASE_H__
