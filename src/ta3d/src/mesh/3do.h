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
|                                         3do.h                                      |
|  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers 3do de total annihilation qui sont les fichiers contenant les modèles |
| 3d des objets du jeu.                                                              |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_3DO
#define __CLASSE_3DO

# include <stdafx.h>
# include <misc/string.h>
# include <misc/hash_table.h>
# include <ta3dbase.h>
# include <gaf.h>
# include <vector>
# include <misc/matrix.h>
# include <gfx/glfunc.h>
# include <gfx/shader.h>
# include <scripts/script.data.h>
# include "mesh.h"

namespace TA3D
{
	struct tagObject				// Structure pour l'en-tête du fichier
	{
		tagObject()
		{
			// Set the whole structure to 0
			memset(this, 0, sizeof(tagObject));
		}
		~tagObject() {}

		int		VersionSignature;
		int		NumberOfVertexes;
		int		NumberOfPrimitives;
		int		OffsetToselectionPrimitive;
		int		XFromParent;
		int		YFromParent;
		int		ZFromParent;
		int		OffsetToObjectName;
		int		Always_0;
		int		OffsetToVertexArray;
		int		OffsetToPrimitiveArray;
		int		OffsetToSiblingObject;
		int		OffsetToChildObject;
	};

	struct tagPrimitive
	{
		tagPrimitive()
		{
			memset(this, 0, sizeof(tagPrimitive));
		}
		~tagPrimitive() {}

		int		ColorIndex;
		int		NumberOfVertexIndexes;
		int		Always_0;
		int		OffsetToVertexIndexArray;
		int		OffsetToTextureName;
		int		Unknown_1;
		int		Unknown_2;
		int		IsColored;
	};

	struct tagVertex		// Structure pour lire les coordonnées des points
	{
		tagVertex()
			:x(0), y(0), z(0)
		{}
		~tagVertex() {}
		int	x;
		int	y;
		int	z;
	};

	class Mesh3DO : public Mesh        // Classe pour la gestion des (sous-)objets des modèles 3do
    {
    public:

        int load(QIODevice *file, int dec = 0, const QString &filename = QString());
        void create_from_2d(SDL_Surface *bmp,float w,float h,float max_h);

		virtual bool draw(float t, AnimationData *data_s = NULL, bool sel_primitive = false, bool alset = false, bool notex = false, int side = 0, bool chg_col = true, bool exploding_parts = false);
        virtual bool draw_nodl(bool alset = false);

        void init3DO();

		inline Mesh3DO() {init3DO();}

        void destroy3DO();

		virtual ~Mesh3DO() {destroy3DO();}

    public:
		static Model *load(const QString &filename);
		static const char *getExt();
	};
} // namespace TA3D

#endif
