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
  |                                         cob.cpp                                    |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | et à l'éxecution des fichiers cob du jeu totalannihilation qui sont les scripts    |
  | qui permettent d'animer les modèles 3d du jeu.(cela inclus également les classes   |
  | de gestion des unités).                                                            |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "cob.h"
#include "../misc/matrix.h"				// Some math routines
#include "../TA3D_NameSpace.h"
#include "../ta3dbase.h"


namespace TA3D
{

    /*!
     * \brief Definition of a Header of a COB file
     */
    struct COBHeader
    {
        //!
        int VersionSignature;
        //!
        int NumberOfScripts;
        //!
        int NumberOfPieces;

        //!
        int Unknown_0;
        //!
        int Unknown_1;
        //! Always seems to be 0
        int Unknown_2;

        //!
        int OffsetToScriptCodeIndexArray;
        //!
        int OffsetToScriptNameOffsetArray;
        //!
        int OffsetToPieceNameOffsetArray;
        //!
        int OffsetToScriptCode;

        //! Always seems to point to first script name
        int Unknown_3;

    }; // class COBHeader




    void SCRIPT::load_cob(byte *data)
    {
        destroy();				// Au cas où

        COBHeader header;
        header.VersionSignature = *((int*)data);
        header.NumberOfScripts = *((int*)(data + 4));
        header.NumberOfPieces = *((int*)(data + 8));
        header.Unknown_0 = *((int*)(data + 12));
        header.Unknown_1 = *((int*)(data + 16));
        header.Unknown_2 = *((int*)(data + 20));
        header.OffsetToScriptCodeIndexArray  = *((int*)(data + 24));
        header.OffsetToScriptNameOffsetArray = *((int*)(data + 28));
        header.OffsetToPieceNameOffsetArray = *((int*)(data + 32));
        header.OffsetToScriptCode = *((int*)(data + 36));
        header.Unknown_3 = *((int*)(data + 40));

#ifdef DEBUG_MODE
        /*		printf("header.NumberOfScripts=%d\n",header.NumberOfScripts);
                printf("header.NumberOfPieces=%d\n",header.NumberOfPieces);
                printf("header.OffsetToScriptCodeIndexArray=%d\n",header.OffsetToScriptCodeIndexArray);
                printf("header.header.OffsetToScriptNameOffsetArray=%d\n",header.OffsetToScriptNameOffsetArray);
                printf("header.OffsetToPieceNameOffsetArray=%d\n",header.OffsetToPieceNameOffsetArray);
                printf("header.OffsetToScriptCode=%d\n",header.OffsetToScriptCode);*/
#endif

        nb_script=header.NumberOfScripts;
        nb_piece=header.NumberOfPieces;
        names.resize(nb_script);
        piece_name=(char**) malloc(sizeof(char*)*nb_piece);

        int f_pos = header.OffsetToScriptNameOffsetArray;
        int i;
        for (i = 0; i < nb_script; ++i)
            names[i] = String( (char*)(data + (*((int*)(data + f_pos + 4 * i)))) ).toUpper();
        f_pos=header.OffsetToPieceNameOffsetArray;
        for(i = 0; i < nb_piece; ++i)
            piece_name[i] = strdup((char*)(data+(*((int*)(data+f_pos+4*i)))));
        Data=data;
        script_code=(int**) malloc(sizeof(int*)*nb_script);
        dec_offset=(int*) malloc(sizeof(int)*nb_script);
        for (i = 0; i < nb_script; ++i)
        {
            dec_offset[i]  = (*((int*)(data + header.OffsetToScriptCodeIndexArray + 4 * i)));
            script_code[i] = (int*)(data + header.OffsetToScriptCode
                                    + 4 * (*((int*)(data + header.OffsetToScriptCodeIndexArray + 4 * i))));
        }
    }




    void SCRIPT::destroy()
    {
        names.clear();
        if (script_code)
            free(script_code);
        if (dec_offset)
            free(dec_offset);
        if (Data)
            delete[] Data;
        if (piece_name)
        {
            for (int i = 0; i < nb_piece; ++i)
                free(piece_name[i]);
            free(piece_name);
        }
        init();
    }

    void SCRIPT::init()
    {
        names.clear();
        Data = NULL;
        nb_script = 0;
        nb_piece = 0;
        script_code = NULL;
        piece_name = NULL;
        dec_offset = NULL;
    }


    int SCRIPT::findFromName(const String& name)
    {
        String nameUpper = name;
        nameUpper.toUpper();
        int indx(0);
        for (String::Vector::const_iterator i = names.begin(); i != names.end(); ++i, ++indx)
        {
            if (*i == nameUpper)
                return indx;
        }
        return -1;
    }

} // namespace TA3D
