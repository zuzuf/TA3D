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

#ifndef __TA3D__BASE__H
#define __TA3D__BASE__H

#include "TA3D_Exception.h"
#include "gfx/glfunc.h"
#include "misc/matrix.h"				// Some math routines
#include "gaf.h"				// read pictures/animations from GAF files
#include "gui.h"				// Graphical User Interface
#include "TA3D_hpi.h"			// HPI handler

#include "misc/camera.h"

#define TA3D_SHIFT_PRESSED	( key[KEY_LSHIFT] || key[KEY_RSHIFT] )
#define TA3D_CTRL_PRESSED	( key[KEY_LCONTROL] || key[KEY_RCONTROL] )

using namespace TA3D::UTILS::HPI;

namespace TA3D
{
    namespace VARS
    {
        extern RGB *pal;

        extern cHPIHandler *HPIManager;
    }
}

using namespace TA3D::VARS;
using namespace TA3D::VARS::CONSTANTS;

#define TIME_UNIT				0.04f	// Pour la simulation physique

#define TA3D_LANG_ENGLISH		0x0
#define TA3D_LANG_FRENCH		0x1
#define TA3D_LANG_GERMAN		0x2
#define TA3D_LANG_SPANISH		0x3
#define TA3D_LANG_ITALIAN		0x4
#define TA3D_LANG_JAPANESE		0x5

#define EXIT_NONE		0x0
#define EXIT_VICTORY	0x1
#define EXIT_DEFEAT		0x2



extern float	player_color[30];
extern byte		player_color_map[10];

extern int LANG;				// Variable indiquant la langue

extern int CURSOR_MOVE;
extern int CURSOR_GREEN;
extern int CURSOR_CROSS;
extern int CURSOR_RED;
extern int CURSOR_LOAD;
extern int CURSOR_UNLOAD;
extern int CURSOR_GUARD;
extern int CURSOR_PATROL;
extern int CURSOR_REPAIR;
extern int CURSOR_ATTACK;
extern int CURSOR_BLUE;
#define CURSOR_DEFAULT		CURSOR_BLUE
extern int CURSOR_AIR_LOAD;
extern int CURSOR_BOMB_ATTACK;
extern int CURSOR_BALANCE;
extern int CURSOR_RECLAIM;
extern int CURSOR_WAIT;
extern int CURSOR_CANT_ATTACK;
extern int CURSOR_CROSS_LINK;
extern int CURSOR_CAPTURE;
extern int CURSOR_REVIVE;

extern int expected_players;

#ifndef TA3D_MSEC_TIMER
#define TA3D_MSEC_TIMER
extern volatile uint32	msec_timer;
#endif

extern int start;
extern ANIMS cursor;

extern int fire;
extern int build_part;

extern int cursor_type;

int anim_cursor(int type=-1);
void draw_cursor();

const float DEG2RAD=PI/180.0f;
const float RAD2DEG=180.0f/PI;

const float DEG2TA=65536.0f/360.0f;
const float TA2DEG=360.0f/65536.0f;

const float RAD2TA=RAD2DEG*DEG2TA;
const float TA2RAD=TA2DEG*DEG2RAD;

const float i2pwr16=1.0f/65536.0f;



/*------------------------------------------------------------------------\
  |              Classe pour la gestion de l'éclairage matériel             |
  \------------------------------------------------------------------------*/

class HWLIGHT
{
public:
       VECTOR	Pos;				// Position
       VECTOR	Dir;				// Direction
       GLfloat LightAmbient[4];
           GLfloat LightDiffuse[4];
           GLfloat LightSpecular[4];
           GLuint	HWNb;				// Indice de lumière matérielle
       float	Att;				// Attenuation
       bool	Directionnal;		// Indique si il s'agit d'une lumière directionnelle
       
           void init()
           {
               LightAmbient[0]=0.125f;
                   LightAmbient[1]=0.125f;
                   LightAmbient[2]=0.125f;
                   LightAmbient[3]=1.0f;
                   
                   LightDiffuse[0]=1.0f;
                   LightDiffuse[1]=1.0f;
                   LightDiffuse[2]=1.0f;
                   LightDiffuse[3]=1.0f;
                   
                   LightSpecular[0]=1.0f;
                   LightSpecular[1]=1.0f;
                   LightSpecular[2]=1.0f;
                   LightSpecular[3]=1.0f;
                   
                   Pos.x=0.0f;
                   Pos.y=0.0f;
                   Pos.z=0.0f;
                   Dir=Pos;
                   Dir.z=-1.0f;
                   HWNb=GL_LIGHT0;
                   Att=0.05f;
               Directionnal=true;
           }
       
           HWLIGHT()
           {
               init();
           }
       
           void Enable()
           {
               glEnable (HWNb);
                   //glEnable (GL_LIGHTING);
                   glEnable (GL_COLOR_MATERIAL);
           }
       
           void Disable()
           {
               glDisable(HWNb);
           }
       
           void Set(TA3D::Camera View)
           {
               GLfloat LightPosition[4];
               if(Directionnal) {
                   LightPosition[0]=Dir.x;
                       LightPosition[1]=Dir.y;
                       LightPosition[2]=Dir.z;
                       LightPosition[3]=0.0f;
               }
               else {
                   View.setView();
                       LightPosition[0]=Pos.x;
                       LightPosition[1]=Pos.y;
                       LightPosition[2]=Pos.z;
                       LightPosition[3]=1.0f;
               }
               
                   glLightfv(HWNb, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
               glLightfv(HWNb, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
               glLightfv(HWNb, GL_SPECULAR, LightSpecular);	// Setup The Diffuse Light
               glLightfv(HWNb, GL_POSITION,LightPosition);		// Position The Light
               glLightf(HWNb, GL_LINEAR_ATTENUATION, Att);		// Attenuation
           }
};

struct INT_ELEMENT
{
    uint16	x1, y1;
    uint16	x2, y2;
};

struct INT_DATA
{
    INT_ELEMENT		EnergyBar;
    INT_ELEMENT		EnergyNum;
    INT_ELEMENT		EnergyMax;
    INT_ELEMENT		Energy0;
    INT_ELEMENT		EnergyProduced;
    INT_ELEMENT		EnergyConsumed;

    INT_ELEMENT		MetalBar;
    INT_ELEMENT		MetalNum;
    INT_ELEMENT		MetalMax;
    INT_ELEMENT		Metal0;
    INT_ELEMENT		MetalProduced;
    INT_ELEMENT		MetalConsumed;

    INT_ELEMENT		UnitName;
    INT_ELEMENT		DamageBar;

    INT_ELEMENT		UnitName2;
    INT_ELEMENT		DamageBar2;

    INT_ELEMENT		UnitMetalMake;
    INT_ELEMENT		UnitMetalUse;
    INT_ELEMENT		UnitEnergyMake;
    INT_ELEMENT		UnitEnergyUse;

    INT_ELEMENT		Name;
    INT_ELEMENT		Description;

    uint32			metal_color;
    uint32			energy_color;
};

class SIDEDATA
{
public:
    int			nb_side;
    char		*side_name[10];			// The name
    char		*side_pref[10];			// The prefix
    char		*side_com[10];			// The commander
    char		*side_int[10];			// The interface gaf
    INT_DATA	side_int_data[10];		// The interface data (position of the gui elements)
    String		unit_ext;
    String		unit_dir;
    String		model_dir;
    String		download_dir;
    String		weapon_dir;
    String		guis_dir;
    String		gamedata_dir;

    void init()
    {
        nb_side=0;
        for(int i=0;i<10;i++)
            side_name[i] = side_pref[i] = side_com[i] = side_int[i] = NULL;
        unit_ext = ".fbi";
        unit_dir = "units\\";
        model_dir = "objects3d\\";
        download_dir = "download\\";
        weapon_dir = "weapons\\";
        guis_dir = "guis\\";
        gamedata_dir = "gamedata\\";
    }

    void destroy()
    {
        for(int i=0;i<10;i++) {
            if(side_name[i])	free(side_name[i]);
            if(side_pref[i])	free(side_pref[i]);
            if(side_com[i])		free(side_com[i]);
            if(side_int[i])		free(side_int[i]);
        }
        unit_ext.clear();
        unit_dir.clear();
        model_dir.clear();
        download_dir.clear();
        weapon_dir.clear();
        guis_dir.clear();
        gamedata_dir.clear();
        init();
    }

    SIDEDATA() : unit_ext(), unit_dir(), model_dir(), download_dir(), weapon_dir(), guis_dir(), gamedata_dir()
    {
        init();
    }

    ~SIDEDATA()
    {
        destroy();
    }

private:
    inline char *get_line(char *data)
    {
        int pos=0;
        while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)	pos++;
        char *d=new char[pos+1];
        memcpy(d,data,pos);
        d[pos]=0;
        return d;
    }

public:
    int	get_side_id(const char *side);
    void load_data();
};

#define	FOW_DISABLED	0x0
#define	FOW_GREY		0x1
#define	FOW_BLACK		0x2
#define FOW_ALL			0x3

class GAME_DATA				// Structure used to pass game information at launch time
{
public:
    char				*map_filename;		// Which map to play
    int					nb_players;			// How many players
    Vector< String >	player_names;		// Their names
    Vector< String >	player_sides;		// Their sides
    Vector< byte >		player_control;		// Who control them
    Vector< int >		player_network_id;	// Network ID of players
    Vector< byte >		ai_level;			// What's their level (for AI)
    Vector< uint32 >	energy;				// How much energy do they have when game starts
    Vector< uint32 >	metal;				// Idem with metal
    Vector< byte >		ready;				// Who is ready ?
    char				*game_script;		// Which script to run
    uint8				fog_of_war;			// flags to configure FOW
    bool				campaign;			// Are we in campaign mode ?
    char				*use_only;			// The use only file to read
    String				saved_file;			// If not empty it's the name of the file to load

    inline GAME_DATA()
    {
        saved_file.clear();

        use_only = NULL;
        campaign = false;
        fog_of_war = FOW_DISABLED;
        map_filename=NULL;
        game_script=NULL;
        nb_players=0;
        player_names.clear();
        player_sides.clear();
        player_control.clear();
        ai_level.clear();
        energy.clear();
        metal.clear();
        ready.clear();

        player_names.resize(10);
        player_sides.resize(10);
        player_control.resize(10);
        ai_level.resize(10);
        energy.resize(10);
        metal.resize(10);
        ready.resize(10);
        player_network_id.resize(10);
        for( uint16 i = 0 ; i < 10 ; i++ ) {
            energy[i] = metal[i] = 10000;
            player_network_id[i] = -1;
            ready[i] = false;
        }
    }

    inline int net2id( int id )
    {
        for( int i = 0 ; i < nb_players ; i++ )
            if( player_network_id[ i ] == id )
                return i;
        return -1;
    }

    inline ~GAME_DATA()
    {
        if(use_only)		free(use_only);
        if(map_filename)	free(map_filename);
        if(game_script)		free(game_script);
        map_filename=NULL;
        game_script=NULL;
        nb_players=0;
        player_names.clear();
        player_sides.clear();
        player_control.clear();
        player_network_id.clear();
        ai_level.clear();
        energy.clear();
        metal.clear();
        ready.clear();
        saved_file.clear();
    }
};

extern SIDEDATA ta3d_sidedata;


void reset_mouse();
void reset_keyboard();

void init_rand_table();
const int rand_from_table();

BITMAP *load_memory_pcx(byte *data,RGB *cpal);
void PutTex(GLuint Tex,float x1,float y1,float x2,float y2);
GLuint LoadTex(const char *file);
GLuint LoadMaskedTex(const char *file,const char *filealpha);
BITMAP *LoadMaskedTexBmp(const char *file,const char *filealpha);
int play(GAME_DATA *game_data);



#endif
