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

/*-----------------------------------------------------------------------------------\
  |                                       weapons.h                                    |
  |   Ce module contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers tdf du jeu totalannihilation concernant les armes utilisées par les   |
  | unités du jeu.                                                                     |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#ifndef __TA3D_INGAME_WEAPONS_H__
# define __TA3D_INGAME_WEAPONS_H__

# include "../../stdafx.h"
# include "weapons.def.h"
# include "weapons.manager.h"
# include "weapons.single.h"
# include "weapons.ingame.h"


namespace TA3D
{

    /*!
    ** \brief Load all available weapons
    **
    ** Load all TDF files
    **
    ** \param progress Callback to display the progression of the loading
    */
    void load_weapons(void (*progress)(float percent,const String &msg) = NULL);



} // namespace TA3D

#endif // __TA3D_INGAME_WEAPONS_H__
