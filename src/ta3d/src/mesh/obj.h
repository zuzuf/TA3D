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

#ifndef __TA3D_MESH_OBJ_H__
#define __TA3D_MESH_OBJ_H__

#include "3dm.h"

#define LOG_PREFIX_OBJ               "[obj] "

namespace TA3D
{
	namespace MOBJ
	{
		struct Material
		{
			QString name;
			QString textureName;
		};
	}

	class MeshOBJ : public Mesh3DM		// Classe pour la gestion des (sous-)objets des modèles 3do
	{
	public:
		MeshOBJ();
		virtual ~MeshOBJ() {}

		virtual void load_texture_id(int id);

        void load(QIODevice *file, const QString &filename);
	private:
		void obj_finalize(const QString &filename, const std::vector<int> &face, const std::vector<Vector3D> &vertex, const std::vector<Vector2D> &tcoord, MOBJ::Material* mtl = NULL);
	public:
		static Model *load(const QString &filename);
		static const char *getExt();
	};
}

#endif // __TA3D_MESH_OBJ_H__
