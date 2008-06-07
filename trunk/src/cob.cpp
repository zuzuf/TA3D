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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"				// Some math routines
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "cob.h"

	void SCRIPT::load_cob(byte *data)
	{
		destroy();				// Au cas où

		COBHeader header;
		header.VersionSignature=*((int*)data);
		header.NumberOfScripts=*((int*)(data+4));
		header.NumberOfPieces=*((int*)(data+8));
		header.Unknown_0=*((int*)(data+12));
		header.Unknown_1=*((int*)(data+16));
		header.Unknown_2=*((int*)(data+20));
		header.OffsetToScriptCodeIndexArray=*((int*)(data+24));
		header.OffsetToScriptNameOffsetArray=*((int*)(data+28));
		header.OffsetToPieceNameOffsetArray=*((int*)(data+32));
		header.OffsetToScriptCode=*((int*)(data+36));
		header.Unknown_3=*((int*)(data+40));

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
		name=(char**) malloc(sizeof(char*)*nb_script);
		piece_name=(char**) malloc(sizeof(char*)*nb_piece);

		int f_pos=header.OffsetToScriptNameOffsetArray;
		int i;
		for(i=0;i<nb_script;i++)
			name[i]=strdup((char*)(data+(*((int*)(data+f_pos+4*i)))));
		f_pos=header.OffsetToPieceNameOffsetArray;
		for(i=0;i<nb_piece;i++)
			piece_name[i]=strdup((char*)(data+(*((int*)(data+f_pos+4*i)))));
		Data=data;
		script_code=(int**) malloc(sizeof(int*)*nb_script);
		dec_offset=(int*) malloc(sizeof(int)*nb_script);
		for(i=0;i<nb_script;i++) {
			dec_offset[i]=(*((int*)(data+header.OffsetToScriptCodeIndexArray+4*i)));
			script_code[i]=(int*)(data+header.OffsetToScriptCode+4*(*((int*)(data+header.OffsetToScriptCodeIndexArray+4*i))));
			}
	}
