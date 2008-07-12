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

/*-----------------------------------------------------------------------------\
|                                     menu.h                                   |
|       Ce module contient les routines du menu de TA3D                        |
|                                                                              |
\-----------------------------------------------------------------------------*/

#ifndef MODULE_MENU
#define MODULE_MENU


void config_menu(void);

void setup_game(bool client = false, const char *host = NULL);		// Setup a game and launch it

void network_room(void);		// Everything you need to host/join a network game

void wait_room(void *p_game_data);			// Wait until everyone has loaded the game

void campaign_main_menu(void);		// The campaign main menu, select the campaign you want to play

int brief_screen( String campaign_name, int mission_id );			// The brief screen where you are told about your mission objectives

#endif
