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



namespace TA3D
{

void glbutton(const String &caption,float x1,float y1,float x2,float y2,bool etat=false);

const String msg_box(TA3D::Interfaces::GfxFont fnt,const String &title,const String &msg,bool ask);

//-------------- These are the GUI functions needed by the editors ----------------------------

		// Pour les couleurs standards

#define GrisM	makeacol(128,128,128,255)
#define GrisF	makeacol(64,64,64,255)
#define GrisC	makeacol(192,192,192,255)
#define Noir	makeacol(0,0,0,255)
#define Blanc	makeacol(255,255,255,255)
#define Bleu	makeacol(0,0,255,255)
#define Rouge	makeacol(255,0,0,255)
#define Vert	makeacol(0,255,0,255)
#define Jaune	makeacol(255,255,0,255)
#define GrisCM	makeacol(170,170,170,255)
#define RougeF	makeacol(128,0,0,255)

		// Object types
#define OBJ_BUTTON		0x0		// Button
#define OBJ_FMENU		0x1		// Floating Menu
#define OBJ_OPTIONB		0x2		// Option Button
#define OBJ_PBAR		0x3		// Progress Bar
#define OBJ_TEXTBAR		0x4		// Text Input
#define OBJ_OPTIONC		0x5		// CheckBox
#define OBJ_MENU		0x6		// Menu
#define OBJ_TEXT		0x7		// Print text
#define OBJ_LINE		0x8		// Draw a line
#define OBJ_BOX			0x9		// Draw a box
#define OBJ_IMG			0xA		// Draw a picture
		// Work in progress for objects below
#define OBJ_LIST		0xB		// Draw a list object
#define OBJ_TA_BUTTON	0xC		// TA Button, used for TA interface
#define OBJ_NONE		0xD		// Used for things that are not intended to be drawn

#define TA_ID_0				0x0		// The ID 0 is always used in Cavedog GUI files to represent the first gadget that defines the interface
#define TA_ID_BUTTON		0x1		// Makes the gadget a button
#define TA_ID_LIST_BOX		0x2		// Creates a listbox
#define TA_ID_TEXT_FIELD	0x3		// Creates a textfield, it's doesnt have any borders
#define TA_ID_SCROLL_BAR	0x4		// Creates a Vertical/horizontal Scroll bar
#define TA_ID_LABEL			0x5		// It makes the gadget a label
#define TA_ID_BLANK_IMG		0x6		// This creates a blank surface that will receive a picture at run time
#define TA_ID_FONT			0x7		// It is used to set the default font for labels
#define TA_ID_IMG			0x12	// Used to display a picture box

#define ASW_YESNO		0x0
#define ASW_OKCANCEL	0x1

struct wnd				// Pour la gestion directe de l'interface dans le programme
{
   int		x,y;			// coordinates
   int		width,height;	// size
   String	Title;			// name
};

class SKIN;			// Class SKIN to handle GUI skins for GUIOBJ objects and WND windows

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

void draw_Window( wnd Wnd );
unsigned char WinMov( int AMx, int AMy, int AMb, int Mx, int My, int Mb, wnd *Wnd );
String dirname( String fname );
const String Dialog( const String &Title, String Filter = "*.*" );
bool WndAsk( const String &Title, const String &Msg, int ASW_TYPE=ASW_OKCANCEL );
void Popup( const String &Title, const String &Msg );
const String GetVal( const String &Title );

extern TA3D::Interfaces::GfxFont gui_font;
extern float gui_font_h;

extern bool	use_normal_alpha_function;

#define	FLAG_CAN_BE_CLICKED					0x0001			// If you can click it
#define	FLAG_CAN_GET_FOCUS					0x0002			// If it can be selected
#define	FLAG_HIGHLIGHT						0x0004			// If it's highlighted when selected
#define	FLAG_FILL							0x0008			// If it must be filled (for type BOX)
#define FLAG_SWITCH							0x0010			// If it behaves like a switch
#define FLAG_HIDDEN							0x0020			// If it's not visible
#define FLAG_MULTI_STATE					0x0040			// If it can have more than 2 states
#define FLAG_BUILD_PIC						0x0080			// If it's a build pic, replace the picture with the current unit's one
#define FLAG_DISABLED						0x0100
#define FLAG_CENTERED						0x0200			// Centered
#define FLAG_TEXT_ADJUST					0x0400			// Tell the text renderer to print '\n' correctly and to fit in the given space
#define FLAG_NO_BORDER						0x0800			// Tell the object not to draw its borders so you can see the background instead
#define FLAG_MISSION_MODE					0x1000			// Tell the object not to draw its borders so you can see the background instead

class WND;

class GUIOBJ					// Structure pour les objets contenus dans les fenêtres
{
public:
	byte				Type;			// Type of objet
	bool				Focus;			// Selected??
	bool				Etat;			// State of the object
	float				x1,y1;			// Position(within the window)
	float				x2,y2;
	std::vector< String >	Text;			// Text displayed by the object
	void				(*Func)(int);	// Pointer to linked function
	uint32				Data;			// Additional data
	uint32				Pos;			// Position in a list
	sint32				Value;			// Used by floatting menus
	float				s;				// Size factor (for text)
	uint32				Flag;			// Flags
	bool				MouseOn;		// If the cursor is on it
	bool				activated;		// For buttons/menus/... indicates that it is pressed (while click isn't finished)
	bool				destroy_img;	// For img control, tell to destroy the texture

	std::vector< String >	OnClick;		// Send that signal when clicked
	std::vector< String >	OnHover;		// Send that signal when mouse is over
	std::vector< String >	SendDataTo;		// Send Data to that object on the window
	std::vector< String >	SendPosTo;		// Send Pos to that object on the window
	String				Name;			// name of the object
	String				help_msg;		// Help message displayed when the mouse cursor is over the object

	float				u1,v1,u2,v2;
	bool				wait_a_turn;	// Used to deal with show/hide msg

	byte				current_state;
	std::vector< TA3D::Interfaces::GfxTexture >	gltex_states;
	byte				nb_stages;
	sint16				shortcut_key;

	GUIOBJ()			// Constructor of the object
	{
		Value = -1;
		help_msg.clear();
		shortcut_key = -1;
		nb_stages = 0;
		current_state = 0;
		gltex_states.clear();
		wait_a_turn = false;
		OnClick.clear();
		OnHover.clear();
		SendDataTo.clear();
		SendPosTo.clear();
		Text.clear();
		MouseOn=false;
		Flag=0;
		Focus=false;
		Etat=false;
		Func=NULL;
		Data=0;
		Pos=0;
		s=1.0f;
		u1=0.0f;
		v1=0.0f;
		u2=1.0f;
		v2=1.0f;
		activated = false;
		destroy_img = false;
	}

	~GUIOBJ()
	{
		help_msg.clear();
		Name.clear();
		Text.clear();
		for(unsigned int i = 0 ; i < gltex_states.size() ; ++i)
			gltex_states[ i ].destroy();
		gltex_states.clear();

		if( destroy_img ) {
			GLuint gl_data = (GLuint) Data;
			gfx->destroy_texture( gl_data );
			Data = 0;
			destroy_img = false;
			}
	}

	uint32	num_entries();

	uint32	msg( const String &message, WND *wnd = NULL );			// Reacts to a message transfered from the Interface

	void set_caption( String caption );

				// Creates a GUI_OBJ
	void create_button(float X1,float Y1,float X2,float Y2,const String &Caption,void (*F)(int), float size=1.0f);
	void create_optionc(float X1,float Y1,const String &Caption,bool ETAT,void (*F)(int), SKIN *skin = NULL, float size=1.0f );
	void create_optionb(float X1,float Y1,const String &Caption,bool ETAT,void (*F)(int), SKIN *skin = NULL, float size=1.0f );
	void create_textbar(float X1,float Y1,float X2,float Y2,const String &Caption,int MaxChar, void(*F)(int)=NULL, float size=1.0f);
	void create_menu(float X1,float Y1,const String::Vector &Entry,void (*F)(int), float size=1.0f);
	void create_menu(float X1,float Y1,float X2,float Y2,const String::Vector &Entry,void (*F)(int), float size=1.0f);
	void create_pbar(float X1,float Y1,float X2,float Y2,int PCent, float size=1.0f);
	void create_text(float X1,float Y1,const String &Caption,int Col=Noir, float size = 1.0f);
	void create_line(float X1,float Y1,float X2,float Y2,int Col=Noir);
	void create_box(float X1,float Y1,float X2,float Y2,int Col=Noir);
	void create_img(float X1,float Y1,float X2,float Y2,GLuint img);
	void create_list(float X1,float Y1,float X2,float Y2,const String::Vector &Entry, float size=1.0f);
	void create_ta_button(float X1,float Y1,const std::vector< String > &Caption, const std::vector< GLuint > &states, int nb_st);
};

class WND : public ObjectSync // Class for the window object
{
public:
	int			x,y;            // coordinates
	int			width,height;   // size
	int			title_h;		// title height as it is displayed
	String		Title;	 		// title
	String		Name;			// Name of the window
	GUIOBJ		*Objets;		// Objects within the window
	int			NbObj;			// Number of objects
	cHashTable< int >	obj_hashtable;		// hashtable used to speed up operations on GUIOBJ objects

	GLuint		background;		// Uses a background
	bool		repeat_bkg;		// Repeat or scale background ?
	uint32		bkg_w;
	uint32		bkg_h;

	bool		Lock;			// Moveable window?
	bool		show_title;		// Draw the title?
	bool		draw_borders;	// Draw borders?
	int			u_format;		// Format used by the window
	bool		hidden;			// Is the window visible ?
	bool		was_hidden;		// In order to do some cleaning
	uint32		color;			// Background color of the window ( can use alpha channel )
	bool		background_wnd;	// Background window -> stay in background
	bool		get_focus;		// Has this window priority over the others ?

private:
	bool		delete_gltex;
	float		size_factor;

public:

	inline WND() : obj_hashtable()			// Constructor
	{
		get_focus = false;
		title_h = 0;
		bkg_w = bkg_h = 1;
		repeat_bkg = false;
		color = makeacol( 0x7F, 0x7F, 0x7F, 0xFF );			// Default : grey
		hidden = false;
		was_hidden = false;
		u_format = U_UTF8;
		delete_gltex = false;
		width = SCREEN_W>>1;
		height = SCREEN_H>>1;
		x = SCREEN_W>>2;
		y = SCREEN_H>>2;
		NbObj = 0;
		Objets = NULL;
		Lock = false;
		show_title = true;
		draw_borders = true;
		background = 0;
		background_wnd = false;
		size_factor = 1.0f;
	}

	inline WND( const String &filename ) : obj_hashtable()			// Constructor
	{
		get_focus = false;
		bkg_w = bkg_h = 1;
		repeat_bkg = false;
		color = makeacol( 0x7F, 0x7F, 0x7F, 0xFF );			// Default : grey
		hidden = false;
		u_format = U_UTF8;
		delete_gltex = false;
		width = SCREEN_W>>1;
		height = SCREEN_H>>1;
		x = SCREEN_W>>2;
		y = SCREEN_H>>2;
		NbObj = 0;
		Objets = NULL;
		Lock = false;
		show_title = true;
		draw_borders = true;
		background = 0;
		size_factor = 1.0f;
		load_tdf( filename );
	}

	inline ~WND()
    {
        obj_hashtable.EmptyHashTable();
        destroy();
    }

	void draw( String &help_msg, bool Focus=true,bool Deg=true, SKIN *skin=NULL );						// Draw the window
	byte WinMov(int AMx,int AMy,int AMb,int Mx,int My,int Mb, SKIN *skin=NULL );						// Handle window's moves
	int check(int AMx,int AMy,int AMz,int AMb,bool timetoscroll=true, SKIN *skin = NULL );	// Handle window's events
	void destroy();																	// Every life has an end...

	uint32	msg( const String &message );											// Respond to Interface message
	bool	get_state( const String &message );										// Return the state of specified object
	sint32	get_value( const String &message );										// Return the value of specified object
	String	get_caption( const String &message );									// Return the caption of specified object
	GUIOBJ	*get_object( const String &message );									// Return a pointer to the specified object

	void load_tdf( const String &filename, SKIN *skin = NULL );						// Load a window from a *.TDF file describing the window
	void load_gui( const String &filename, cHashTable< std::vector< TA3D::Interfaces::GfxTexture >* > &gui_hashtable );// Load a window from a TA *.GUI file describing the interface
};

class AREA:	public ObjectSync, // This class is a window handler, so it will manage windows, and signals given to them
			protected IInterface							// This is a global declaration since GUI is everywhere
{
private:
	std::vector< WND* >		vec_wnd;			// This vector stores all the windows the area object deals with
	std::vector< uint16 >	vec_z_order;		// This vector stores data about the z order of windows
	String				name;				// How is that area called ?
	int					amx, amy, amz, amb;	// Remember last cursor position
	SKIN				*skin;				// The skin used by the area
	cHashTable< std::vector< TA3D::Interfaces::GfxTexture >* > gui_hashtable;		// hashtable used to speed up loading of *.gui files and save memory
	cHashTable< int >	wnd_hashtable;		// hashtable used to speed up operations on WND objects
	String				cached_key;
	WND					*cached_wnd;
	uint32				scroll_timer;
public:
	bool				scrolling;
	GLuint				background;			// Of course we need a background, not a single color :-)

	private:
		uint32		InterfaceMsg( const lpcImsg msg );	// Manage signals sent through the interface to GUI

	public:

		AREA( const String area_name = "unnamed_area" );	// Constructor that gives a name to the area
		~AREA();											// Destructor

		void destroy();										// A function that can reset the object

		uint16 load_window( const String &filename );		// Loads a window from a TDF file
		uint16 check();										// Check the user interface, manage events
		void draw();										// Draw all the windows
		void load_tdf( const String &filename );			// Loads a TDF file telling which windows to load and which skin to use

		WND		*get_wnd( const String &message );			// Return the specified window
		bool	get_state( const String &message );			// Return the state of specified object in the specified window
		sint32	get_value( const String &message );			// Return the value of specified object in the specified window
		String	get_caption( const String &message );		// Return the caption of specified object in the specified window
		GUIOBJ	*get_object( const String &message, bool skip_hidden = false );		// Return a pointer to the specified object
		void	set_state( const String &message, const bool &state );			// Set the state of specified object in the specified window
		void	set_value( const String &message, const sint32 &value );		// Set the value of specified object in the specified window
		void	set_data( const String &message, const sint32 &data );			// Set the data of specified object in the specified window
		void	set_enable_flag( const String &message, const bool &enable );	// Set the enabled/disabled state of specified object in the specified window
		void	set_caption( const String &message, const String &caption );	// Set the caption of specified object in the specified window
		int		msg( String message );				// Send that message to the area
};


} // namespace TA3D

#endif
