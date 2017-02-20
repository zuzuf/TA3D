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
    
#ifndef __TA3D_XX_RESTORE_H__
# define __TA3D_XX_RESTORE_H__


# include "misc/string.h"
# include "ingame/gamedata.h"


namespace TA3D
{

	void save_game( const QString filename, GameData *game_data );

	bool load_game_data( const QString filename, GameData *game_data, bool loading = false );

	void load_game( GameData *game_data );

} // namespace TA3D

#endif // __TA3D_XX_RESTORE_H__
