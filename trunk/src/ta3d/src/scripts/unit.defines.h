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
#ifndef __UNIT_DEFINES_H__
# define  __UNIT_DEFINES_H__

# define  ACTIVATION          1   // set or get
# define  STANDINGMOVEORDERS  2   // set or get
# define  STANDINGFIREORDERS  3   // set or get
# define  HEALTH              4   // get (0-100%)
# define  INBUILDSTANCE       5   // set or get
# define  BUSY                6   // set or get (used by misc. special case missions like transport ships)
# define  PIECE_XZ            7   // get
# define  PIECE_Y             8   // get
# define  UNIT_XZ             9   // get
# define  UNIT_Y              10  // get
# define  UNIT_HEIGHT         11  // get
# define  XZ_ATAN             12  // get atan of packed x,z coords
# define  XZ_HYPOT            13  // get hypot of packed x,z coords
# define  ATAN                14  // get ordinary two-parameter atan
# define  HYPOT               15  // get ordinary two-parameter hypot
# define  GROUND_HEIGHT       16  // get
# define  BUILD_PERCENT_LEFT  17  // get 0 = unit is built and ready, 1-100 = How much is left to build
# define  YARD_OPEN           18  // set or get (change which plots we occupy when building opens and closes)
# define  BUGGER_OFF          19  // set or get (ask other units to clear the area)
# define  ARMORED             20  // set or get

# define  MIN_ID                      69      // returns the lowest valid unit ID number
# define  MAX_ID                      70      // returns the highest valid unit ID number
# define  MY_ID                       71      // returns ID of current unit
# define  UNIT_TEAM                   72      // returns team(player ID in TA) of unit given with parameter
# define  UNIT_BUILD_PERCENT_LEFT     73      // basically BUILD_PERCENT_LEFT, but comes with a unit parameter
# define  UNIT_ALLIED                 74      // is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
# define  UNIT_IS_ON_THIS_COMP        75      // indicates if the 1st parameter(a unit ID) is local to this computer
# define  VETERAN_LEVEL               32      // gets kills * 100

#endif
