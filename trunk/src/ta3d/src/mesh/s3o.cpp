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
  |                                         s3o.cpp                                    |
  |  This module loads and display Spring models.                                      |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include <stdafx.h>
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include "s3o.h"
#include <gfx/particles/particles.h>
#include <ingame/sidedata.h>
#include <languages/i18n.h>
#include <misc/math.h>
#include <misc/paths.h>
#include <misc/files.h>
#include <logs/logs.h>
#include <gfx/gl.extensions.h>
#include <zlib.h>


namespace TA3D
{
	void MESH_S3O::initS3O()
	{
		init();
		root = NULL;
	}



	void MESH_S3O::destroyS3O()
	{
		destroy();
		root = NULL;
	}

	bool MESH_S3O::draw(float t, ANIMATION_DATA *data_s, bool sel_primitive, bool alset, bool notex, int side, bool chg_col, bool exploding_parts)
	{
		bool explodes = script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
		bool hide = false;
		bool set = false;
		if ( !tex_cache_name.empty() )
		{
			for(int i = 0 ; i < tex_cache_name.size() ; ++i)
				load_texture_id(i);
			tex_cache_name.clear();
		}

		if (!(explodes && !exploding_parts))
		{
			glPushMatrix();

			glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);
			if (script_index >= 0 && data_s)
			{
				if (!explodes ^ exploding_parts)
				{
					glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
					glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
					glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
					glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
				}
				hide = data_s->flag[script_index] & FLAG_HIDE;
			}
			else if (animation_data && data_s == NULL)
			{
				Vector3D R;
				Vector3D T;
				animation_data->animate(t, R, T);
				glTranslatef(T.x, T.y, T.z);
				glRotatef(R.x, 1.0f, 0.0f, 0.0f);
				glRotatef(R.y, 0.0f, 1.0f, 0.0f);
				glRotatef(R.z, 0.0f, 0.0f, 1.0f);
			}

			std::vector<GLuint> *pTex = &(root->gltex);

			hide |= explodes ^ exploding_parts;
			int texID = player_color_map[side];
			if (script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_ANIMATED_TEXTURE)
				&& !fixed_textures && !pTex->empty())
				texID = ((int)(t * 10.0f)) % pTex->size();
			if (gl_dlist.size() > texID && gl_dlist[texID] && !hide && !chg_col && !notex && false)
			{
				glCallList( gl_dlist[ texID ] );
				alset = false;
				set = false;
			}
			else if (!hide)
			{
				bool creating_list = false;
				if (gl_dlist.size() <= texID)
					gl_dlist.resize(texID + 1);
				if (!chg_col && !notex && gl_dlist[texID] == 0 && false)
				{
					gl_dlist[texID] = glGenLists(1);
					glNewList(gl_dlist[texID], GL_COMPILE_AND_EXECUTE);
					alset = false;
					set = false;
					creating_list = true;
				}
				if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
				{
					bool activated_tex = false;
					glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
					glEnableClientState(GL_NORMAL_ARRAY);
					alset = false;
					set = false;

					glEnable(GL_LIGHTING);

					if (chg_col || !notex)
					{
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						glEnable(GL_BLEND);
						glAlphaFunc( GL_GREATER, 0.1 );
						glEnable( GL_ALPHA_TEST );
					}

					if (!notex) // Les textures et effets de texture
					{
						activated_tex = true;
						for (int j = 0; j < pTex->size() ; ++j)
						{
							glActiveTextureARB(GL_TEXTURE0_ARB + j);
							glEnable(GL_TEXTURE_2D);
							glBindTexture(GL_TEXTURE_2D, (*pTex)[j]);
						}
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						for (int j = 0; j < pTex->size() ; ++j)
						{
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
							glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
						}
					}
					else
					{
						for (int j = 6; j >= 0; --j)
						{
							glActiveTextureARB(GL_TEXTURE0_ARB + j);
							glDisable(GL_TEXTURE_2D);
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
							glDisableClientState(GL_TEXTURE_COORD_ARRAY);
						}
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, gfx->default_texture);
					}
					glVertexPointer(3, GL_FLOAT, 0, points);
					glNormalPointer(GL_FLOAT, 0, N);
					switch(type)
					{
						case MESH_TYPE_TRIANGLES:
							glDrawRangeElements(GL_TRIANGLES, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);				// draw everything
							break;
						case MESH_TYPE_TRIANGLE_STRIP:
							glDisable( GL_CULL_FACE );
							glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
							glEnable( GL_CULL_FACE );
							break;
					};

					if (activated_tex)
					{
						for (int j = 0; j < pTex->size() ; ++j)
						{
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
							glDisableClientState(GL_TEXTURE_COORD_ARRAY);

							glActiveTextureARB(GL_TEXTURE0_ARB + j);
							glDisable(GL_TEXTURE_2D);
							glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
							glDisable(GL_TEXTURE_GEN_S);
							glDisable(GL_TEXTURE_GEN_T);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						}
						glClientActiveTextureARB(GL_TEXTURE0_ARB);
						glActiveTextureARB(GL_TEXTURE0_ARB);
						glEnable(GL_TEXTURE_2D);
					}
				}
				if (creating_list)
					glEndList();
			}
			if (child && !(explodes && !exploding_parts))
				alset = child->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts && !explodes );
			glPopMatrix();
		}
		if (next)
			alset = next->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts);

		return alset;
	}

	bool MESH_S3O::draw_nodl(bool alset)
	{
		glPushMatrix();

		glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);

		if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
		{
			glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
			glEnableClientState(GL_NORMAL_ARRAY);
			alset = false;

			std::vector<GLuint> *pTex = &(root->gltex);

			glEnable(GL_LIGHTING);

			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glAlphaFunc( GL_GREATER, 0.1 );
			glEnable( GL_ALPHA_TEST );

			for (int j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, (*pTex)[j]);
				glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
			}
			glVertexPointer(3, GL_FLOAT, 0, points);
			glNormalPointer(GL_FLOAT, 0, N);
			switch(type)
			{
				case MESH_TYPE_TRIANGLES:
					glDrawRangeElements(GL_TRIANGLES, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);				// draw everything
					break;
				case MESH_TYPE_TRIANGLE_STRIP:
					glDisable( GL_CULL_FACE );
					glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
					glEnable( GL_CULL_FACE );
					break;
			};

			for (int j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				glActiveTextureARB(GL_TEXTURE0_ARB + j);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
		}
		if (child)
			alset = child->draw_nodl(alset);
		glPopMatrix();
		if (next)
			alset = next->draw_nodl(alset);

		return alset;
	}

	MESH_S3O* MESH_S3O::LoadPiece(File* file, MESH_S3O* model, MESH_S3O *root)
	{
		MESH_S3O* piece = model ? model : new MESH_S3O;
		piece->type = MESH_TYPE_TRIANGLES;
		piece->root = root;

		Piece fp;
		*file >> fp;

		piece->pos_from_parent.x = fp.xoffset;
		piece->pos_from_parent.y = fp.yoffset;
		piece->pos_from_parent.z = fp.zoffset;
		switch(fp.primitiveType)
		{
			case S3O_PRIMTYPE_QUADS:
			case S3O_PRIMTYPE_TRIANGLES:
				piece->type = MESH_TYPE_TRIANGLES;
				break;
			case S3O_PRIMTYPE_TRIANGLE_STRIP:
				piece->type = MESH_TYPE_TRIANGLE_STRIP;
				break;
		};
		file->seek(fp.name);
		piece->name = file->getString();

		// retrieve each vertex

		piece->nb_vtx = fp.numVertices;
		piece->points = new Vector3D[piece->nb_vtx];
		piece->N = new Vector3D[piece->nb_vtx];
		piece->tcoord = new float[2 * piece->nb_vtx];

		file->seek(fp.vertices);
		for (int a = 0; a < fp.numVertices; ++a)
		{
			SS3OVertex v;
			*file >> v;
			piece->points[a] = v.pos;
			piece->N[a] = v.normal;
			piece->tcoord[a * 2] = v.textureX;
			piece->tcoord[a * 2 + 1] = v.textureY;
		}


		// retrieve the draw order for the vertices

		std::vector<int> index;

		file->seek(fp.vertexTable);
		for (int a = 0; a < fp.vertexTableSize ; ++a)
		{
			int vertexDrawIdx;
			*file >> vertexDrawIdx;

			index.push_back(vertexDrawIdx);
			if (fp.primitiveType == S3O_PRIMTYPE_QUADS && (a % 4) == 2)        // QUADS need to be split into triangles (this would be done internally by OpenGL anyway since quads are rendered as 2 triangles)
			{
				index.push_back(index[index.size() - 3]);
				index.push_back(vertexDrawIdx);
			}

			// -1 == 0xFFFFFFFF (U)
			if (vertexDrawIdx == -1 && a != fp.vertexTableSize - 1)
			{
				// for triangle strips
				index.push_back(vertexDrawIdx);

				int pos = file->tell();
				*file >> vertexDrawIdx;
				file->seek(pos);
				index.push_back(vertexDrawIdx);
			}
		}

		piece->nb_t_index = index.size();
		piece->t_index = new GLushort[piece->nb_t_index];
		for(int i = 0 ; i < index.size() ; ++i)
			piece->t_index[i] = index[i];

		for (int a = 0; a < fp.numChilds; ++a)
		{
			file->seek(fp.childs + a * sizeof(int));
			int childOffset;
			*file >> childOffset;

			file->seek(childOffset);
			MESH_S3O* childPiece = LoadPiece(file, NULL, root);
			if (piece->child)
			{
				childPiece->next = piece->child;
				piece->child = childPiece;
			}
			else
				piece->child = childPiece;
		}

		return piece;
	}

	void MESH_S3O::load(File *file, const String &filename)
	{
		destroyS3O();

		S3OHeader header;
		*file >> header;
		if (memcmp(header.magic, "Spring unit\0", 12))      // File corrupt or wrong format
		{
			LOG_ERROR(LOG_PREFIX_S3O << "Spring Model Loader error : File is corrupt or in wrong format");
			return;
		}

		MESH_S3O* model = this;
		model->type = MESH_TYPE_TRIANGLES;
		model->name = filename;
		if (header.texture1 > 0)
		{
			file->seek(header.texture1);
			String textureName = String("textures/") << file->getString();
			GLuint tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA());
			if (tex)
				model->gltex.push_back(tex);
		}
		if (header.texture2 > 0)
		{
			file->seek(header.texture2);
			String textureName = String("textures/") << file->getString();
			GLuint tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA());
			if (tex)
				model->gltex.push_back(tex);
		}

		file->seek(header.rootPiece);
		LoadPiece(file, model, this);
	}

	MODEL *MESH_S3O::load(const String &filename)
	{
		File *file = VFS::Instance()->readFile(filename);
		if (!file)
		{
			LOG_ERROR(LOG_PREFIX_S3O << "could not read file '" << filename << "'");
			return NULL;
		}

		MESH_S3O *mesh = new MESH_S3O;
		mesh->load(file, filename);
		delete file;

		MODEL *model = new MODEL;
		model->mesh = mesh;
		model->postLoadComputations();
		return model;
	}



} // namespace TA3D

