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

#include "3ds.h"
#include "joins.h"
#include <vfs/file.h>

//#define DEBUG_3DS

#ifdef DEBUG_3DS
#define PRINT_DEBUG(x)	LOG_DEBUG(LOG_PREFIX_3DS << x)
#else
#define PRINT_DEBUG(x)
#endif

namespace TA3D
{
	REGISTER_MESH_TYPE(Mesh3DS);

	namespace M3DS
	{
		struct ChunkData
		{
			uint16	ID;
			uint32	length;
		};

		struct Material
		{
			QString	name;
			float	ambient[4];
			float	diffuse[4];
			float	specular[4];
			float	shininess;
			float	shin2pct;
			float	shin3pct;
			float	transparency;
			bool	twoSide;
			float	texmap;
			QString	mapname;

			Material() :
					name(), shininess(0.0f),
					shin2pct(0.0f), shin3pct(0.0f),
					transparency(0.0f), twoSide(false),
					texmap(1.0f), mapname()
			{
				ambient[0] = 1.0f;
				ambient[1] = 1.0f;
				ambient[2] = 1.0f;
				ambient[3] = 1.0f;
			}
		};
	}

	using namespace M3DS;

	void read_color_chunk( float color[], File *src )
	{
		ChunkData chunk;
		*src >> chunk.ID;
		*src >> chunk.length;
		switch (chunk.ID)
		{
		case COL_RGB:
		case COL_RGB2:
			src->read( (char*)color, sizeof( float ) * 3 );
			break;
		case COL_TRU:
		case COL_TRU2:
			for( int i = 0 ; i < 3 ;i++ )
			{
				unsigned char c;
				*src >> c;
				color[ i ] = (float)c / 255.0f;
			}
			break;
    default:
			src->seek(src->tell() + chunk.length - 6);
		};
	}

	float read_percent_chunk(File *src)
	{
		ChunkData chunk;
		*src >> chunk.ID;
		*src >> chunk.length;
		switch (chunk.ID)
		{
		case PER_INT:
			{
				uint16	percent;
				*src >> percent;
				return percent;
			}
		case PER_FLOAT:
			{
				float	percent;
				*src >> percent;
				return percent;
			}
		default:
			src->seek(src->tell() + chunk.length - 6);
		};
		return 0.0f;
	}

	QString read_MatMapname_chunk(File *src)
	{
		ChunkData chunk;
		*src >> chunk.ID;
		*src >> chunk.length;
		switch (chunk.ID)
		{
		case MAT_MAPNAME:
			return src->getString();
		default:
			src->seek(src->tell() + chunk.length - 6);
		};
		return QString();
	}

	Mesh3DS::Mesh3DS()
	{
		Color = 0xFFFFFFFF;
	}

	void Mesh3DS::load_texture_id(int id)
	{
		if (id < 0 || id >= (int)tex_cache_name.size())
			return;
		bool useAlpha(false);
		gltex.push_back(gfx->load_texture(tex_cache_name[id], FILTER_TRILINEAR, NULL, NULL, true, 0, &useAlpha, true));
		if (useAlpha)
			Flag |= SURFACE_BLENDED;
	}

	Model *Mesh3DS::load(const QString &filename)
	{
		File *file = VFS::Instance()->readFile(filename);

		if (!file || !file->isOpen())
			return NULL;

		Mesh3DS *firstObj = new Mesh3DS;
		Mesh3DS *cur_obj = NULL;
		Mesh3DS *read_obj = NULL;
		HashMap<Material*>::Dense material;
		Material *currentMat = NULL;
		Vector3D local[4];
		while (!file->eof())
		{
			ChunkData chunk;
			*file >> chunk.ID;
			*file >> chunk.length;
			if (file->eof())
				break;
			switch (chunk.ID)
			{
			case MAIN3DS:
				PRINT_DEBUG("MAIN3DS (" << chunk.ID << ',' << chunk.length << ')');
				break;
			case EDIT3DS:
				PRINT_DEBUG("-EDIT3DS (" << chunk.ID << ',' << chunk.length << ')');
				break;
			case EDIT_MATERIAL:
				PRINT_DEBUG("-EDIT_MATERIAL (" << chunk.ID << ',' << chunk.length << ')');
                if (currentMat && currentMat->name.isEmpty())
					delete currentMat;
				currentMat = new Material;
				break;
			case MAT_NAME:
				PRINT_DEBUG("-MAT_NAME (" << chunk.ID << ',' << chunk.length << ')');
				{
					QString name = file->getString();
					PRINT_DEBUG("name = " << name);
					if (currentMat)
					{
						currentMat->name = name;
                        if (!name.isEmpty())
						{
							if (material.count(name)
								&& material[name] != currentMat
								&& material[name] != NULL)
								delete material[name];
							material[name] = currentMat;
						}
					}
				}
				break;
			case MAT_AMBIENT:
				PRINT_DEBUG("-MAT_AMBIENT (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					read_color_chunk( currentMat->ambient, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_DIFFUSE:
				PRINT_DEBUG("-MAT_DIFFUSE (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					read_color_chunk( currentMat->diffuse, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SPECULAR:
				PRINT_DEBUG("-MAT_SPECULAR (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					read_color_chunk( currentMat->specular, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHININESS:
				PRINT_DEBUG("-MAT_SHININESS (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					currentMat->shininess = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHIN2PCT:
				PRINT_DEBUG("-MAT_SHIN2PCT (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					currentMat->shin2pct = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHIN3PCT:
				PRINT_DEBUG("-MAT_SHIN3PCT (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					currentMat->shin3pct = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TRANSPARENCY:
				PRINT_DEBUG("-MAT_TRANSPARENCY (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					currentMat->transparency = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TWO_SIDE:
				PRINT_DEBUG("-MAT_TWO_SIDE (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
					currentMat->twoSide = true;
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TEXMAP:
				PRINT_DEBUG("-MAT_TEXMAP (" << chunk.ID << ',' << chunk.length << ')');
				if (currentMat)
				{
					uint16 n_id;
					*file >> n_id;
					file->seek(file->tell() - 2);
					if (n_id == MAT_MAPNAME)
					{
						currentMat->mapname = read_MatMapname_chunk( file );
						currentMat->texmap = read_percent_chunk( file );
					}
					else
					{
						currentMat->texmap = read_percent_chunk( file );
						currentMat->mapname = read_MatMapname_chunk( file );
					}
                    currentMat->mapname = "textures/" + currentMat->mapname;
					PRINT_DEBUG("texmap : " << currentMat->mapname);
				}
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_CONFIG1:
				PRINT_DEBUG("-EDIT_CONFIG1 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_CONFIG2:
				PRINT_DEBUG("-EDIT_CONFIG2 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P1:
				PRINT_DEBUG("-EDIT_VIEW_P1 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P2:
				PRINT_DEBUG("-EDIT_VIEW_P2 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P3:
				PRINT_DEBUG("-EDIT_VIEW_P3 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW1:
				PRINT_DEBUG("-EDIT_VIEW1 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_BACKGR:
				PRINT_DEBUG("-EDIT_BACKGR (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case EDIT_AMBIENT:
				PRINT_DEBUG("-EDIT_AMBIENT (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_OBJECT:
				PRINT_DEBUG("-EDIT_OBJECT (" << chunk.ID << ',' << chunk.length << ')');
				if (cur_obj)
				{
					Mesh3DS *n_obj = new Mesh3DS;
					cur_obj->next = n_obj;
					cur_obj = n_obj;
				}
				else
					cur_obj = firstObj;
				cur_obj->type = MESH_TYPE_TRIANGLES;
				cur_obj->name = file->getString();		// Read the object's name
				cur_obj->Flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
				read_obj = cur_obj;
				break;
            case OBJ_LIGHT:
				PRINT_DEBUG("-OBJ_LIGHT (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_CAMERA:
				PRINT_DEBUG("-OBJ_CAMERA (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_UNKNWN01:
				PRINT_DEBUG("-OBJ_UNKNWN01 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_UNKNWN02:
				PRINT_DEBUG("-OBJ_UNKNWN02 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_TRIMESH:
				PRINT_DEBUG("-OBJ_TRIMESH (" << chunk.ID << ',' << chunk.length << ')');
				if (read_obj->nb_vtx > 0)		// Add a sub object
				{
					read_obj->child = new Mesh3DS;
					read_obj = static_cast<Mesh3DS*>(read_obj->child);
					read_obj->type = MESH_TYPE_TRIANGLES;
					read_obj->name = cur_obj->name;
					read_obj->Flag = SURFACE_ADVANCED | SURFACE_GOURAUD | SURFACE_LIGHTED;
				}
				local[0].x = 1.0f;		local[0].y = 0.0f;		local[0].z = 0.0f;
				local[1].x = 0.0f;		local[1].y = 1.0f;		local[1].z = 0.0f;
				local[2].x = 0.0f;		local[2].y = 0.0f;		local[2].z = 1.0f;
				local[3].x = 0.0f;		local[3].y = 0.0f;		local[3].z = 0.0f;
				break;
            case TRI_VERTEXL:
				{
					PRINT_DEBUG("-TRI_VERTEXL (" << chunk.ID << ',' << chunk.length << ')');
					uint16 nb_vtx;
					*file >> nb_vtx;
					read_obj->nb_vtx = nb_vtx;
					read_obj->points = new Vector3D[nb_vtx << 1];
					read_obj->N = new Vector3D[nb_vtx << 1];
					if (read_obj->tcoord == NULL)
					{
						read_obj->tcoord = new float[nb_vtx * 2];
						for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
						{
							read_obj->tcoord[ i << 1 ] = 0.0f;
							read_obj->tcoord[ (i << 1) + 1 ] = 0.0f;
						}
					}
					file->read( (char*)read_obj->points, (int)sizeof( Vector3D ) * nb_vtx );
					for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
					{
						read_obj->points[ i ] = read_obj->points[ i ].x * local[ 0 ] + read_obj->points[ i ].y * local[ 1 ] + read_obj->points[ i ].z * local[ 2 ] + local[ 3 ];
						read_obj->N[ i ].reset();
					}
				}
				break;
            case TRI_FACEL2:
				PRINT_DEBUG("-TRI_FACEL2 (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case TRI_MATERIAL:
				PRINT_DEBUG("-TRI_MATERIAL (" << chunk.ID << ',' << chunk.length << ')');
				{
					QString material_name = file->getString();
					PRINT_DEBUG("material name = " << material_name);

					Material *cur_mat = material[material_name];

					if (cur_mat)
					{
						PRINT_DEBUG("material found");
                        if (!cur_mat->mapname.isEmpty())
						{
							PRINT_DEBUG("loading texture " << cur_mat->mapname);
							read_obj->Flag |= SURFACE_TEXTURED;
                            const QString &name = cur_mat->mapname.trimmed();
							read_obj->tex_cache_name.push_back(name);
						}
						if (cur_mat->transparency > 0.0f)
						{
							read_obj->Flag |= SURFACE_BLENDED;
							read_obj->Color = (read_obj->Color & 0xFFFFFF00) | (uint32)(cur_mat->transparency * 255);
						}
						if (cur_mat->name == "team")			// The magic team material
						{
							read_obj->Flag |= SURFACE_PLAYER_COLOR;
						}
					}
					else
						LOG_WARNING(LOG_PREFIX_3DS << "WARNING: material not found!!");

					uint16	nb_faces;
					*file >> nb_faces;
					for( int i = 0 ; i < nb_faces ; i++ )
					{
						uint16 cur_face;
						*file >> cur_face;
					}
				}
				break;
            case TRI_MAPPING:
				PRINT_DEBUG("-TRI_MAPPING (" << chunk.ID << ',' << chunk.length << ')');
				{
					uint16	nb_vtx;
					*file >> nb_vtx;
					if (read_obj->tcoord == NULL)
						read_obj->tcoord = new float[2 * nb_vtx];
					file->read( (char*)read_obj->tcoord, 2 * (int)sizeof( float ) * nb_vtx );
					for( int i = 0 ; i < nb_vtx ; i++ )
						read_obj->tcoord[ i * 2 + 1 ] = 1.0f - read_obj->tcoord[ i * 2 + 1 ];
				}
				break;
            case TRI_FACEL1:
				PRINT_DEBUG("-TRI_FACEL1 (" << chunk.ID << ',' << chunk.length << ')');
				uint16 nb_index;
				*file >> nb_index;
				read_obj->nb_t_index = short(nb_index * 3);
				read_obj->t_index = new GLushort[nb_index * 3];
				for( int i = 0 ; i < nb_index * 3 ; i += 3 )
				{
					uint16 idx[3];
					*file >> idx[0];
					*file >> idx[1];
					*file >> idx[2];
					read_obj->t_index[i] = idx[0];
					read_obj->t_index[i+1] = idx[1];
					read_obj->t_index[i+2] = idx[2];
					if (read_obj->points)
					{
						Vector3D AB,AC;
						AB = read_obj->points[ read_obj->t_index[ i + 1 ] ] - read_obj->points[ read_obj->t_index[ i ] ];
						AC = read_obj->points[ read_obj->t_index[ i + 2 ] ] - read_obj->points[ read_obj->t_index[ i ] ];
						AB = AB * AC;
						AB.unit();
						read_obj->N[ read_obj->t_index[ i ] ] += AB;
						read_obj->N[ read_obj->t_index[ i + 1 ] ] += AB;
						read_obj->N[ read_obj->t_index[ i + 2 ] ] += AB;
					}
					uint16 face_info;
					*file >> face_info;
				}
				if (read_obj->points)
					for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
						read_obj->N[ i ].unit();
				break;
            case TRI_SMOOTH:
				PRINT_DEBUG("-TRI_SMOOTH (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case TRI_LOCAL:
				PRINT_DEBUG("-TRI_LOCAL (" << chunk.ID << ',' << chunk.length << ')');
				*file >> local[0];		// X
				*file >> local[1];		// Y
				*file >> local[2];		// Z
				*file >> local[3];		// local origin
				break;
            case TRI_VISIBLE:
				PRINT_DEBUG("-TRI_VISIBLE (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
				break;
            default:
				PRINT_DEBUG("unknown chunk (" << chunk.ID << ',' << chunk.length << ')');
				file->seek( file->tell() + chunk.length - 6 );
			}
		}
        if (currentMat && currentMat->name.isEmpty())
			delete currentMat;
		for(HashMap<Material*>::Dense::iterator it = material.begin() ; it != material.end() ; ++it)
			if (*it)
				delete *it;

		delete file;

		Model *model = new Model;
		model->mesh = Joins::computeStructure(firstObj, filename);
		model->postLoadComputations();
		Joins::computeSelection(model);
		return model;
	}

	const char *Mesh3DS::getExt()
	{
		return ".3ds";
	}
}
