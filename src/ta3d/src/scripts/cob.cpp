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
#include <misc/matrix.h>				// Some math routines
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>


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
		int CodeLength;
		//!
		int StaticVariableCount ;
		//! Always seems to be 0
		int Unknown_0;

		//!
		int OffsetToScriptCodeIndexArray;
		//!
		int OffsetToScriptNameOffsetArray;
		//!
		int OffsetToPieceNameOffsetArray;
		//!
		int OffsetToScriptCode;

		//! Always seems to point to first script name
		int Unknown_1;

	}; // class COBHeader


	CobScript::CobScript()
	{
		init();
	}

	CobScript::~CobScript()
	{
		destroy();
	}

    void CobScript::load(const QString &filename)
	{
		destroy();				// Au cas où

        QIODevice *file = VFS::Instance()->readFile(filename);

		if (file == NULL)
			return;

		COBHeader header;
#define READ(X) file->read((char*)&X, sizeof(X))
        READ(header);

#ifdef DEBUG_MODE
		/*		printf("header.NumberOfScripts=%d\n",header.NumberOfScripts);
				printf("header.NumberOfPieces=%d\n",header.NumberOfPieces);
				printf("header.OffsetToScriptCodeIndexArray=%d\n",header.OffsetToScriptCodeIndexArray);
				printf("header.header.OffsetToScriptNameOffsetArray=%d\n",header.OffsetToScriptNameOffsetArray);
				printf("header.OffsetToPieceNameOffsetArray=%d\n",header.OffsetToPieceNameOffsetArray);
				printf("header.OffsetToScriptCode=%d\n",header.OffsetToScriptCode);*/
#endif

		nbStaticVar = header.StaticVariableCount;
		nb_script = header.NumberOfScripts;
		nb_piece = header.NumberOfPieces;
        names.clear();
        piece_name.clear();
        names.reserve(nb_script);
        piece_name.reserve(nb_piece);

		int i;
		for (i = 0; i < nb_script; ++i)
		{
			file->seek(header.OffsetToScriptNameOffsetArray + 4 * i);
			int ofs;
            READ(ofs);
			file->seek(ofs);
            names.push_back(QString::fromLatin1(getString(file)).toUpper());
		}
		for(i = 0; i < nb_piece; ++i)
		{
			file->seek(header.OffsetToPieceNameOffsetArray + 4 * i);
			int ofs;
            READ(ofs);
			file->seek(ofs);
            piece_name.push_back(QString::fromLatin1(getString(file)));
		}
		codeSize = header.CodeLength * 4;
		Data = new byte[codeSize];
		file->seek(header.OffsetToScriptCode);
        file->read((char*)Data, codeSize);
		script_code = new int*[nb_script];
		dec_offset = new int[nb_script];
		file->seek(header.OffsetToScriptCodeIndexArray);
        file->read((char*)dec_offset, (int)sizeof(int) * nb_script);
		for (i = 0; i < nb_script; ++i)
		{
			file->seek(4 * dec_offset[i]);
			script_code[i] = (int*)(Data + 4 * dec_offset[i]);
		}

		delete file;
	}

	void CobScript::destroy()
	{
		names.clear();
		DELETE_ARRAY(script_code);
		DELETE_ARRAY(dec_offset);
		DELETE_ARRAY(Data);
		piece_name.clear();
		init();
	}

	void CobScript::init()
	{
		names.clear();
		Data = NULL;
		nb_script = 0;
		nb_piece = 0;
		script_code = NULL;
		piece_name.clear();
		dec_offset = NULL;
		nbStaticVar = 0;
		codeSize = 0;
	}


    int CobScript::findFromName(const QString& name)
	{
        const QString &nameUpper = name.toUpper();
        int indx(-1);
        for (const QString &i : names)
		{
            ++indx;
            if (i == nameUpper)
				return indx;
		}
		return -1;
	}


    int CobScript::identify(const QString &name)
	{
        const QString &nameLower = name.toLower();
		for (int i = 0; i < nb_piece; ++i)
		{
            if (nameLower == piece_name[i].toLower()) // Pièce identifiée / Identified :)
				return i;
		}
		return -1;
	}



} // namespace TA3D

