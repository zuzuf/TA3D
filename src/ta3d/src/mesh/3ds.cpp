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
#include <vfs/file.h>

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
			String	name;
			float	ambient[4];
			float	diffuse[4];
			float	specular[4];
			float	shininess;
			float	shin2pct;
			float	shin3pct;
			float	transparency;
			bool	twoSide;
			float	texmap;
			String	mapname;
		};
	}

	using namespace M3DS;

	void read_color_chunk( float color[], File *src )
	{
		ChunkData chunk;
		*src >> chunk.ID;
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
				color[ i ] = c / 255.0f;
			}
			break;
    default:
			src->seek(src->tell() + chunk.length - 6);
		};
	}

	float read_percent_chunk(File *src)
	{
		ChunkData chunk;
		*src >> chunk;
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

	String read_MatMapname_chunk(File *src)
	{
		ChunkData chunk;
		*src >> chunk;
		switch (chunk.ID)
		{
		case MAT_MAPNAME:
			return src->getString();
		default:
			src->seek(src->tell() + chunk.length - 6);
		};
		return String();
	}

	MODEL *Mesh3DS::load(const String &filename)
	{
		File *file = VFS::Instance()->readFile(filename);

		if (!file || !file->isOpen())
			return NULL;

		Mesh3DS *cur_obj = NULL;
		Mesh3DS *read_obj = NULL;
		HashMap<Material*>::Dense material;
		Material *currentMat = NULL;
		material.set_empty_key(String());
		Vector3D local[4];
		while (!file->eof())
		{
			ChunkData chunk;
			*file >> chunk;
			if (file->eof())
				break;
			switch (chunk.ID)
			{
			case MAIN3DS:
				//				printf("MAIN3DS (%d,%d)\n", chunk.ID, chunk.length);
				break;
			case EDIT3DS:
				//					printf("-EDIT3DS (%d,%d)\n", chunk.ID, chunk.length);
				break;
			case EDIT_MATERIAL:
				//						printf("--EDIT_Material (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat && currentMat->name.empty())
					delete currentMat;
				currentMat = new Material;
				break;
			case MAT_NAME:
				//							printf("---MAT_name (%d,%d)\n", chunk.ID, chunk.length);
				{
					String name = file->getString();
					//								printf( "name = %s\n", name );
					if (currentMat)
					{
						currentMat->name = name;
						if (!name.empty())
						{
							if (material[name])
								delete material[name];
							material[name] = currentMat;
						}
					}
				}
				break;
			case MAT_AMBIENT:
				//							printf("---MAT_ambient (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					read_color_chunk( currentMat->ambient, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_DIFFUSE:
				//							printf("---MAT_diffuse (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					read_color_chunk( currentMat->diffuse, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SPECULAR:
				//							printf("---MAT_specular (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					read_color_chunk( currentMat->specular, file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHININESS:
				//							printf("---MAT_shininess (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					currentMat->shininess = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHIN2PCT:
				//							printf("---MAT_shin2pct (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					currentMat->shin2pct = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_SHIN3PCT:
				//							printf("---MAT_shin3pct (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					currentMat->shin3pct = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TRANSPARENCY:
				//							printf("---MAT_transparency (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					currentMat->transparency = read_percent_chunk( file );
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TWO_SIDE:
				//							printf("---MAT_twoSide (%d,%d)\n", chunk.ID, chunk.length);
				if (currentMat)
					currentMat->twoSide = true;
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case MAT_TEXMAP:
				//							printf("---MAT_texmap (%d,%d)\n", chunk.ID, chunk.length);
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
					currentMat->mapname = String("textures/") + currentMat->mapname;
				}
				else
					file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_CONFIG1:
				//						printf("--EDIT_CONFIG1 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_CONFIG2:
				//						printf("--EDIT_CONFIG2 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P1:
				//						printf("--EDIT_VIEW_P1 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P2:
				//						printf("--EDIT_VIEW_P2 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW_P3:
				//						printf("--EDIT_VIEW_P2 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_VIEW1:
				//						printf("--EDIT_VIEW1 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_BACKGR:
				//						printf("--EDIT_BACKGR (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case EDIT_AMBIENT:
				//						printf("--EDIT_ambient (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case EDIT_OBJECT:
				//						printf("--EDIT_OBJECT (%d,%d)\n", chunk.ID, chunk.length);
				if (cur_obj)
				{
					Mesh3DS *n_obj = new Mesh3DS;
					cur_obj->next = n_obj;
					cur_obj = n_obj;
				}
				else
					cur_obj = new Mesh3DS;
				cur_obj->type = MESH_TYPE_TRIANGLES;
				cur_obj->name = file->getString();		// Read the object's name
				read_obj = cur_obj;
				break;
            case OBJ_LIGHT:
				//							printf("---OBJ_LIGHT (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_CAMERA:
				//							printf("---OBJ_CAMERA (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_UNKNWN01:
				//							printf("---OBJ_UNKNWN01 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_UNKNWN02:
				//							printf("---OBJ_UNKNWN02 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case OBJ_TRIMESH:
				//							printf("---OBJ_TRIMESH (%d,%d)\n", chunk.ID, chunk.length);
				if (read_obj->nb_vtx > 0)		// Add a sub object
				{
					read_obj->child = new Mesh3DS;
					read_obj = static_cast<Mesh3DS*>(MESH::Ptr::WeakPointer(read_obj->child));
					read_obj->type = MESH_TYPE_TRIANGLES;
					read_obj->name = cur_obj->name;
				}
				local[0].x = 1.0f;		local[0].y = 0.0f;		local[0].z = 0.0f;
				local[1].x = 0.0f;		local[1].y = 1.0f;		local[1].z = 0.0f;
				local[2].x = 0.0f;		local[2].y = 0.0f;		local[2].z = 1.0f;
				local[3].x = 0.0f;		local[3].y = 0.0f;		local[3].z = 0.0f;
				break;
            case TRI_VERTEXL:
				{
					//								printf("----TRI_VERTEXL (%d,%d)\n", chunk.ID, chunk.length);
					uint16 nb_vtx;
					*file >> nb_vtx;
					read_obj->nb_vtx = nb_vtx;
					read_obj->points = new Vector3D[nb_vtx];
					read_obj->N = new Vector3D[nb_vtx];
					if (read_obj->tcoord == NULL)
					{
						read_obj->tcoord = new float[nb_vtx * 2];
						for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
						{
							read_obj->tcoord[ i << 1 ] = 0.0f;
							read_obj->tcoord[ (i << 1) + 1 ] = 0.0f;
						}
					}
					file->read( (char*)read_obj->points, sizeof( Vector3D ) * nb_vtx );
					for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
					{
						read_obj->points[ i ] = read_obj->points[ i ].x * local[ 0 ] + read_obj->points[ i ].y * local[ 1 ] + read_obj->points[ i ].z * local[ 2 ] + local[ 3 ];
						read_obj->N[ i ].reset();
					}
				}
				break;
            case TRI_FACEL2:
				//								printf("----TRI_FACEL2 (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
			case TRI_MATERIAL:
				//								printf("----TRI_Material (%d,%d)\n", chunk.ID, chunk.length);
				{
					String material_name = file->getString();
					//									printf("material name = %s\n", material_name );

					Material *cur_mat = material[material_name];

					if (cur_mat)
					{
						//										printf("material found\n");
						if (!cur_mat->mapname.empty())
						{
							//											printf("loading texture %s\n", cur_mat->mapname );
							read_obj->Flag |= SURFACE_TEXTURED;
							read_obj->gltex.resize(1);
							String name = cur_mat->mapname;
							name.trim();
							read_obj->gltex[0] = gfx->load_texture(name);
						}
						if (cur_mat->transparency > 0.0f)
						{
							read_obj->Flag |= SURFACE_BLENDED;
							read_obj->Color = (read_obj->Color & 0xFFFFFF00) | (uint32)(cur_mat->transparency * 255);
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
				//								printf("----TRI_MAPPING (%d,%d)\n", chunk.ID, chunk.length);
				{
					uint16	nb_vtx;
					*file >> nb_vtx;
					read_obj->tcoord = new float[2 * nb_vtx];
					file->read( (char*)read_obj->tcoord, 2 * sizeof( float ) * nb_vtx );
					for( int i = 0 ; i < nb_vtx ; i++ )
						read_obj->tcoord[ i * 2 + 1 ] = 1.0f - read_obj->tcoord[ i * 2 + 1 ];
				}
				break;
            case TRI_FACEL1:
				//								printf("----TRI_FACEL1 (%d,%d)\n", chunk.ID, chunk.length);
				uint16 nb_index;
				*file >> nb_index;
				read_obj->nb_t_index = nb_index;
				read_obj->t_index = new GLushort(nb_index * 3);
				for( int i = 0 ; i < nb_index * 3 ; i += 3 )
				{
					uint16 idx[3];
					*file >> idx[0];
					*file >> idx[1];
					*file >> idx[2];
					read_obj->t_index[i] = idx[0];
					read_obj->t_index[i+1] = idx[1];
					read_obj->t_index[i+2] = idx[2];
					if (!read_obj->points)
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
				if (!read_obj->points)
					for (int i = 0 ; i < read_obj->nb_vtx ; ++i)
						read_obj->N[ i ].unit();
				break;
            case TRI_SMOOTH:
				//								printf("----TRI_SMOOTH (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            case TRI_LOCAL:
				//								printf("----TRI_LOCAL (%d,%d)\n", chunk.ID, chunk.length);
				*file >> local[0];		// X
				*file >> local[1];		// Y
				*file >> local[2];		// Z
				*file >> local[3];		// local origin
				break;
            case TRI_VISIBLE:
				//								printf("----TRI_VISIBLE (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
				break;
            default:
				//				printf("unknown (%d,%d)\n", chunk.ID, chunk.length);
				file->seek( file->tell() + chunk.length - 6 );
			}
		}
		if (currentMat && currentMat->name.empty())
			delete currentMat;
		for(HashMap<Material*>::Dense::iterator it = material.begin() ; it != material.end() ; ++it)
			if (it->second)
				delete it->second;

		delete file;

		MODEL *model = new MODEL;
		model->mesh = read_obj;
		model->postLoadComputations();
		return model;
	}

	const char *Mesh3DS::getExt()
	{
		return ".3ds";
	}
}
