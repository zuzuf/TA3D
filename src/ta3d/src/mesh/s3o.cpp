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
#include "joins.h"

#define READ(X) file->read((char*)&X, sizeof(X))

namespace TA3D
{
	Shader MeshS3O::s3oShader;
	Shader MeshS3O::s3oShader_woShadows;

    REGISTER_MESH_TYPE(MeshS3O)

	const char *MeshS3O::getExt()
	{
		return ".s3o";
	}

	void MeshS3O::initS3O()
	{
		init();
		root = NULL;
	}



	void MeshS3O::destroyS3O()
	{
		destroy();
		root = NULL;
	}

	bool MeshS3O::draw(float t, AnimationData *data_s, bool sel_primitive, bool alset, bool notex, int side, bool chg_col, bool exploding_parts)
	{
		init_shaders();
		bool explodes = script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_EXPLODE);
		bool hide = false;
		bool set = false;
		if ( !tex_cache_name.empty() )
		{
            if (!tex_cache_name[0].isEmpty())
			{
				const QString &textureName = tex_cache_name[0];
				if (!lp_CONFIG->disable_GLSL && g_useProgram)			// GLSL-enabled code
				{
					GLuint tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA_compressed(), NULL, true);
					if (tex)
						gltex.push_back(tex);
				}
				else		// Fixed pipeline
				{
					GLuint tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGB_compressed(), NULL, true);
					if (tex)
					{
						gltex.push_back(tex);
						tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, GL_ALPHA8, NULL, true);
						if (tex)
							gltex.push_back(tex);
					}
				}
			}
			if (tex_cache_name.size() == 2)
			{
				const QString &textureName = tex_cache_name[1];
				if (!lp_CONFIG->disable_GLSL && g_useProgram)			// GLSL-enabled code
				{
					GLuint tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA_compressed(), NULL, true);
					if (tex)
						gltex.push_back(tex);
				}
				else		// Fixed pipeline
				{
					// Get the RED channel
					SDL_Surface *bmp = gfx->load_image(textureName);
					if (bmp)
					{
						if (std::max(bmp->w, bmp->h) > lp_CONFIG->getMaxTextureSizeAllowed())
						{
							const int maxTextureSizeAllowed = lp_CONFIG->getMaxTextureSizeAllowed();
							SDL_Surface *tmp = shrink(bmp, std::min(bmp->w, maxTextureSizeAllowed), std::min(bmp->h, maxTextureSizeAllowed));
							SDL_FreeSurface(bmp);
							bmp = tmp;
						}

						SDL_Surface *red = gfx->create_surface_ex(8, bmp->w, bmp->h);

						for(int y = 0 ; y < bmp->h ; ++y)
							for(int x = 0 ; x < bmp->w ; ++x)
								SurfaceByte(red, x, y) = getr32(getpixel(bmp, x, y));

						gfx->set_texture_format(GL_LUMINANCE8);
						GLuint tex = gfx->make_texture(red, FILTER_TRILINEAR, true);
						if (tex)
							gltex.push_back(tex);
						gfx->set_texture_format(0);

						SDL_FreeSurface(red);
						SDL_FreeSurface(bmp);
					}
				}
			}
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
					glTranslatef(data_s->data[script_index].axe[0].pos, data_s->data[script_index].axe[1].pos, data_s->data[script_index].axe[2].pos);
					glRotatef(data_s->data[script_index].axe[0].angle, 1.0f, 0.0f, 0.0f);
					glRotatef(data_s->data[script_index].axe[1].angle, 0.0f, 1.0f, 0.0f);
					glRotatef(data_s->data[script_index].axe[2].angle, 0.0f, 0.0f, 1.0f);
				}
				hide = data_s->data[script_index].flag & FLAG_HIDE;
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
			if (!hide)
			{
				if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
				{
					if ((!lp_CONFIG->disable_GLSL && g_useProgram) || notex)					// GLSL-enabled code/texture-less option (because it's single pass)
					{
						bool activated_tex = false;
						if (!alset)
						{
							glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
							glEnableClientState(GL_NORMAL_ARRAY);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glAlphaFunc( GL_GREATER, 0.1f );
							glEnable(GL_LIGHTING);
							alset = true;
							set = true;
						}
						glEnable(GL_BLEND);
						glEnable( GL_ALPHA_TEST );

						if (!notex) // Les textures et effets de texture
						{
							if (lp_CONFIG->shadow_quality >= 2)
							{
								s3oShader.on();
								s3oShader.setvar1i( "tex0", 0 );
								s3oShader.setvar1i( "tex1", 1 );
								s3oShader.setvar1i( "shadowMap", 7 );
								s3oShader.setvar1f( "t", 0.5f - 0.5f * cosf(t * PI) );
								s3oShader.setvar4f( "team", player_color[player_color_map[side] * 3], player_color[player_color_map[side] * 3 + 1], player_color[player_color_map[side] * 3 + 2], 1.0f);
								s3oShader.setmat4f("light_Projection", gfx->shadowMapProjectionMatrix);
							}
							else
							{
								s3oShader_woShadows.on();
								s3oShader_woShadows.setvar1i( "tex0", 0 );
								s3oShader_woShadows.setvar1i( "tex1", 1 );
								s3oShader_woShadows.setvar1f( "t", 0.5f - 0.5f * cosf(t * PI) );
								s3oShader_woShadows.setvar4f( "team", player_color[player_color_map[side] * 3], player_color[player_color_map[side] * 3 + 1], player_color[player_color_map[side] * 3 + 2], 1.0f);
							}

							activated_tex = true;
							for (uint32 j = 0; j < pTex->size() ; ++j)
							{
								glActiveTextureARB(GL_TEXTURE0_ARB + j);
								glEnable(GL_TEXTURE_2D);
								glBindTexture(GL_TEXTURE_2D, (*pTex)[j]);
							}
							glEnableClientState(GL_TEXTURE_COORD_ARRAY);
							for (uint32 j = 0; j < pTex->size() ; ++j)
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
							for (uint32 j = 0; j < pTex->size() ; ++j)
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
						if (!notex && !lp_CONFIG->disable_GLSL)
							s3oShader.off();
						glDisable(GL_BLEND);
						glDisable(GL_ALPHA_TEST);
					}
					else															// GLSL-disabled code
					{
						if (!alset)
						{
							glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
							glEnableClientState(GL_NORMAL_ARRAY);
							glEnable(GL_LIGHTING);
							alset = true;
							set = true;
						}

						glVertexPointer(3, GL_FLOAT, 0, points);
						glNormalPointer(GL_FLOAT, 0, N);

						// First pass : RGB color only
						glDisable(GL_BLEND);
						glDisable(GL_ALPHA_TEST);
						if (pTex->size() > 0)
						{
							// Enable texturing
							glActiveTextureARB(GL_TEXTURE0_ARB);
							glEnable(GL_TEXTURE_2D);
							glBindTexture(GL_TEXTURE_2D, (*pTex)[0]);
							// Set UV array
							glEnableClientState(GL_TEXTURE_COORD_ARRAY);
							glClientActiveTextureARB(GL_TEXTURE0_ARB);
							glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
						}
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

						if (pTex->size() > 1)		// Second and third passes needed only with textures
						{
							// Second pass : team color
							glBindTexture(GL_TEXTURE_2D, (*pTex)[1]);		// Alpha texture
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glAlphaFunc( GL_GREATER, 0.0 );
							glEnable( GL_ALPHA_TEST );
							glDepthFunc(GL_LEQUAL);

							GLfloat col[4];
							glGetFloatv(GL_CURRENT_COLOR, col);

							glColor4f(player_color[player_color_map[side] * 3],
									  player_color[player_color_map[side] * 3 + 1],
									  player_color[player_color_map[side] * 3 + 2],
									  1.0f);

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

							if (pTex->size() > 2)		// Third pass : light emission
							{
								glBindTexture(GL_TEXTURE_2D, (*pTex)[2]);		// Alpha texture
								const float c = 0.5f - 0.5f * cosf(t * PI);
								glColor4f(c, c, c, 1.0f);
								glBlendFunc(GL_SRC_COLOR, GL_ONE);
								glDisable(GL_LIGHTING);

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

								glEnable(GL_LIGHTING);
							}

							glDisable(GL_BLEND);
							glDisable(GL_ALPHA_TEST);
							glColor4fv(col);
							glDepthFunc(GL_LESS);
						}

						glClientActiveTextureARB(GL_TEXTURE0_ARB);
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					}
				}
			}
			if (sel_primitive && selprim >= 0 && nb_vtx > 0)
			{
				gfx->disable_model_shading();
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_NORMAL_ARRAY);
				glDisable(GL_LIGHTING);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_FOG);
				glDisable(GL_BLEND);
				if (!set)
					glVertexPointer( 3, GL_FLOAT, 0, points);
				glColor3ub(0,0xFF,0);
				glTranslatef( 0.0f, 2.0f, 0.0f );
				glDrawRangeElements(GL_LINE_LOOP, 0, nb_vtx-1, 4,GL_UNSIGNED_SHORT,sel);		// dessine la primitive de sélection
				glTranslatef( 0.0f, -2.0f, 0.0f );
				if (notex)
				{
					byte var = (byte)abs(0xFF - (msec_timer % 1000) * 0x200 / 1000);
					glColor3ub(0, var, 0);
				}
				else
					glColor3ub(0xFF, 0xFF, 0xFF);
				alset = false;
				gfx->enable_model_shading();
				glEnable(GL_FOG);
			}
			if (child && !(explodes && !exploding_parts))
				alset = child->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts && !explodes );
			glPopMatrix();
		}
		if (next)
			alset = next->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts);

		return alset;
	}

	bool MeshS3O::draw_nodl(bool alset)
	{
		init_shaders();
		glPushMatrix();

		glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);

		if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
		{
			if (!alset)
			{
				glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
				glEnableClientState(GL_NORMAL_ARRAY);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glAlphaFunc( GL_GREATER, 0.1f );
				glEnable( GL_ALPHA_TEST );
				glEnable(GL_LIGHTING);
				alset = true;
			}

			std::vector<GLuint> *pTex = &(root->gltex);

			for (uint32 j = 0; j < pTex->size() ; ++j)
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

			for (uint32 j = 0; j < pTex->size() ; ++j)
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

    MeshS3O* MeshS3O::LoadPiece(QIODevice *file, MeshS3O* model, MeshS3O *root)
	{
		MeshS3O* piece = model ? model : new MeshS3O;
		piece->type = MESH_TYPE_TRIANGLES;
		piece->root = root;

		Piece fp;
        READ(fp);

		piece->pos_from_parent.x = 0.5f * fp.xoffset;
		piece->pos_from_parent.y = 0.5f * fp.yoffset;
		piece->pos_from_parent.z = 0.5f * fp.zoffset;
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
        piece->name = QString::fromUtf8(getString(file));

		// retrieve each vertex

		piece->nb_vtx = (short)fp.numVertices;
		piece->points = new Vector3D[piece->nb_vtx];
		piece->N = new Vector3D[piece->nb_vtx];
		piece->tcoord = new float[2 * piece->nb_vtx];

		file->seek(fp.vertices);
		for (int a = 0; a < fp.numVertices; ++a)
		{
			SS3OVertex v;
            READ(v);
			piece->points[a] = 0.5f * v.pos;
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
            READ(vertexDrawIdx);

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

                int pos = file->pos();
                READ(vertexDrawIdx);
				file->seek(pos);
				index.push_back(vertexDrawIdx);
			}
		}

		piece->nb_t_index = (short)index.size();
		piece->t_index = new GLushort[piece->nb_t_index];
		for(size_t i = 0 ; i < index.size() ; ++i)
			piece->t_index[i] = GLushort(index[i]);

		for (int a = 0; a < fp.numChilds; ++a)
		{
			file->seek(fp.childs + a * (int)sizeof(int));
			int childOffset;
            READ(childOffset);

			file->seek(childOffset);
			MeshS3O* childPiece = LoadPiece(file, NULL, root);
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

    void MeshS3O::load(QIODevice *file, const QString &filename)
	{
		destroyS3O();

		S3OHeader header;
        READ(header);
		if (memcmp(header.magic, "Spring unit\0", 12))      // File corrupt or wrong format
		{
			LOG_ERROR(LOG_PREFIX_S3O << "Spring Model Loader error : File is corrupt or in wrong format");
			return;
		}

		MeshS3O* model = this;
		model->type = MESH_TYPE_TRIANGLES;
		model->name = filename;
		if (header.texture1 > 0)
		{
			file->seek(header.texture1);
            const QString textureName = "textures/" + QString::fromUtf8(getString(file));
			model->tex_cache_name.push_back(textureName);
		}
		if (header.texture2 > 0)
		{
			file->seek(header.texture2);
            const QString textureName = "textures/" + QString::fromUtf8(getString(file));
            if (model->tex_cache_name.isEmpty())
				model->tex_cache_name.push_back(QString());
			model->tex_cache_name.push_back(textureName);
		}

		file->seek(header.rootPiece);
		LoadPiece(file, model, this);
	}

	Model *MeshS3O::load(const QString &filename)
	{
        QIODevice *file = VFS::Instance()->readFile(filename);
		if (!file)
		{
			LOG_ERROR(LOG_PREFIX_S3O << "could not read file '" << filename << "'");
			return NULL;
		}

		MeshS3O *mesh = new MeshS3O;
		mesh->load(file, filename);
		delete file;

		Model *model = new Model;
		model->mesh = mesh;
		model->postLoadComputations();
		model->useDL = false;
		Joins::computeSelection(model);
		return model;
	}


	void MeshS3O::init_shaders()
	{
		if (!s3oShader.isLoaded())
			s3oShader.load("shaders/s3o.frag", "shaders/s3o.vert");
		if (!s3oShader_woShadows.isLoaded())
			s3oShader_woShadows.load("shaders/s3o_wos.frag", "shaders/s3o_wos.vert");
	}
} // namespace TA3D

