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
|                                          s3o.h                                     |
|  This module loads and display Spring models.                                      |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_S3O
#define __CLASSE_S3O

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
    /// Structure in .s3o files representing draw primitives
    struct Piece
    {
        int name;		///< offset in file to char* name of this piece
        int numChilds;		///< number of sub pieces this piece has
        int childs;		///< file offset to table of dwords containing offsets to child pieces
        int numVertices;	///< number of vertices in this piece
        int vertices;		///< file offset to vertices in this piece
        int vertexType;	///< 0 for now
        int primitiveType;	///< type of primitives for this piece, 0=triangles,1 triangle strips,2=quads
        int vertexTableSize;	///< number of indexes in vertice table
        int vertexTable;	///< file offset to vertice table, vertice table is made up of dwords indicating vertices for this piece, to indicate end of a triangle strip use 0xffffffff
        int collisionData;	///< offset in file to collision data, must be 0 for now (no collision data)
        float xoffset;		///< offset from parent piece
        float yoffset;
        float zoffset;
    };

    /// Header structure for .s3o files
    struct S3OHeader
    {
        char magic[12];		///< "Spring unit\0"
        int version;		///< 0 for this version
        float radius;		///< radius of collision sphere
        float height;		///< height of whole object
        float midx;		///< these give the offset from origin(which is supposed to lay in the ground plane) to the middle of the unit collision sphere
        float midy;
        float midz;
        int rootPiece;		///< offset in file to root piece
        int collisionData;	///< offset in file to collision data, must be 0 for now (no collision data)
        int texture1;		///< offset in file to char* filename of first texture
        int texture2;		///< offset in file to char* filename of second texture
    };

    struct SS3OVertex
    {
        Vector3D pos;
        Vector3D normal;
        float textureX;
        float textureY;
    };

    enum {S3O_PRIMTYPE_TRIANGLES, S3O_PRIMTYPE_TRIANGLE_STRIP, S3O_PRIMTYPE_QUADS};

	class MeshS3O : public Mesh        // Classe pour la gestion des (sous-)objets des modèles 3do
    {
	private:
		static Shader s3oShader;
		static Shader s3oShader_woShadows;
	public:
		MeshS3O *root;
    public:

		void load(File *file, const QString &filename);
		MeshS3O* LoadPiece(File* file, MeshS3O* model, MeshS3O *root);

		virtual bool draw(float t, AnimationData *data_s = NULL, bool sel_primitive = false, bool alset = false, bool notex = false, int side = 0, bool chg_col = true, bool exploding_parts = false);
        virtual bool draw_nodl(bool alset = false);

        void initS3O();

		inline MeshS3O() {initS3O();}

        void destroyS3O();

		virtual ~MeshS3O() {destroyS3O();}

    public:
		static Model *load(const QString &filename);
		static const char *getExt();
		static void init_shaders();
	};
} // namespace TA3D

#endif
