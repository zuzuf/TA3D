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

void main_menu(void);

char *select_map(String *def_choice = NULL);

void config_menu(void);

void stats_menu(void);

void setup_game(void);		// Setup a game and launch it

void battle_room(void);		// Everything you need to launch a network game

void campaign_main_menu(void);		// The campaign main menu, select the campaign you want to play

#endif
