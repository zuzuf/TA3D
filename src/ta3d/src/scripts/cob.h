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
|                                         cob.h                                      |
|  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| et à l'éxecution des fichiers cob du jeu totalannihilation qui sont les scripts    |
| qui permettent d'animer les modèles 3d du jeu.(cela inclus également les classes   |
| de gestion des unités).                                                            |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __COB_H__
# define  __COB_H__

# include <stdafx.h>
# include <misc/string.h>
# include "script.data.h"
# include <engine/unit.defines.h>


namespace TA3D
{

    //! Class responsible for COB scripts data
    class CobScript : public ScriptData
    {
    public:
        CobScript();
        virtual ~CobScript();

        void init();
        void destroy();

        /*!
        ** \brief Get the index of a script according its name
        ** \param name name of the script
        ** \return Index of the script, -1 if not found
        */
        int findFromName(const QString& name);

        /*virtual*/ void load(const QString &filename);

        /*virtual*/ int identify(const QString &name);

    public:
        int             nb_script;      // Nombre de scripts / Number of scripts
        byte            *Data;          // Données du fichier COB / COB data
        int             **script_code;  // Code des scripts / script codes
        QStringList  names;          // Nom des scripts / script names
        int             nb_piece;       // Nombre de pièces / Number of pieces
        QStringList  piece_name;     // Nom des pièces de l'objet 3d concerné / Name of pieces
        int             *dec_offset;
        int             nbStaticVar;    // Number of static variables
        int             codeSize;       // Size of the code chunk

    }; // class SCRIPT

} // namespace TA3D


#endif
