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

#ifndef __CLASSE_COB
# define  __CLASSE_COB

# include "../stdafx.h"


# define  SCRIPT_MOVE_OBJECT              0x10001000
# define  SCRIPT_WAIT_FOR_TURN            0x10011000
# define  SCRIPT_RANDOM_NUMBER            0x10041000
# define  SCRIPT_LESS                     0x10051000
# define  SCRIPT_GREATER_EQUAL            0x10054000
# define  SCRIPT_GREATER                  0x10053000
# define  SCRIPT_START_SCRIPT             0x10061000
# define  SCRIPT_EXPLODE                  0x10071000
# define  SCRIPT_TURN_OBJECT              0x10002000
# define  SCRIPT_WAIT_FOR_MOVE            0x10012000
# define  SCRIPT_CREATE_LOCAL_VARIABLE    0x10022000
# define  SCRIPT_SUBTRACT                 0x10032000
# define  SCRIPT_GET_VALUE_FROM_PORT      0x10042000
# define  SCRIPT_LESS_EQUAL               0x10052000
# define  SCRIPT_SPIN_OBJECT              0x10003000
# define  SCRIPT_SLEEP                    0x10013000
# define  SCRIPT_MULTIPLY                 0x10033000
# define  SCRIPT_CALL_SCRIPT              0x10063000
# define  SCRIPT_JUMP                     0x10064000
# define  SCRIPT_SHOW_OBJECT              0x10005000
# define  SCRIPT_EQUAL                    0x10055000
# define  SCRIPT_RETURN                   0x10065000
# define  SCRIPT_NOT_EQUAL                0x10056000
# define  SCRIPT_IF                       0x10066000
# define  SCRIPT_HIDE_OBJECT              0x10006000
# define  SCRIPT_SIGNAL                   0x10067000
# define  SCRIPT_DONT_CACHE               0x10008000
# define  SCRIPT_SET_SIGNAL_MASK          0x10068000
# define  SCRIPT_NOT                      0x1005A000
# define  SCRIPT_DONT_SHADE               0x1000E000
# define  SCRIPT_EMIT_SFX                 0x1000F000
# define  SCRIPT_PUSH_CONST               0x10021001
# define  SCRIPT_PUSH_VAR                 0x10021002
# define  SCRIPT_SET_VAR                  0x10023002
# define  SCRIPT_PUSH_STATIC_VAR          0x10021004
# define  SCRIPT_SET_STATIC_VAR           0x10023004
# define  SCRIPT_OR                       0x10036000
# define  SCRIPT_ADD                      0x10031000  //added
# define  SCRIPT_STOP_SPIN                0x10004000  //added
# define  SCRIPT_DIVIDE                   0x10034000  //added
# define  SCRIPT_MOVE_PIECE_NOW           0x1000B000  //added
# define  SCRIPT_TURN_PIECE_NOW           0x1000C000  //added
# define  SCRIPT_CACHE                    0x10007000  //added
# define  SCRIPT_COMPARE_AND              0x10057000  //added
# define  SCRIPT_COMPARE_OR               0x10058000  //added
# define  SCRIPT_CALL_FUNCTION            0x10062000  //added
# define  SCRIPT_GET                      0x10043000  //added
# define  SCRIPT_SET_VALUE                0x10082000  //added
# define  SCRIPT_ATTACH_UNIT              0x10083000  //added
# define  SCRIPT_DROP_UNIT                0x10084000  //added    

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



namespace TA3D
{

    class SCRIPT            // Classe gérant le stockage et l'éxecution des scripts
    {
    public:
        SCRIPT() {init();}
        ~SCRIPT() {destroy();}

        void init();
        void destroy();

        /*!
        ** \brief Get the index of a script according its name
        ** \param name name of the script
        ** \return Index of the script, -1 if not found
        */
        int findFromName(const String& name);

        void load_cob(byte *data);

    public:
        int     nb_script;      // Nombre de scripts
        byte    *Data;          // Données du fichier COB
        int     **script_code;  // Code des scripts
        String::Vector names;         // Nom des scripts
        int     nb_piece;       // Nombre de pièces
        char    **piece_name;   // Nom des pièces de l'objet 3d concerné
        int     *dec_offset;

    }; // class SCRIPT


    /*!
    ** \todo This class must be removed. A std::deque<int> or std::slits<int> will be good enough...
    */
    class SCRIPT_STACK          // Structure pour gérer la liste de la pile permettant l'éxecution des scripts
    {
    public:
        SCRIPT_STACK() :val(0), next(NULL) {}

        int                 val;            // Entier
        SCRIPT_STACK        *next;          // pointeur vers l'élément suivant

    }; // class SCRIPT_STACK


} // namespace TA3D


#endif
