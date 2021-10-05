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
#include <misc/timer.h>
#include <cmath>

#define READ(X) file->read((char*)&X, sizeof(X))

namespace TA3D
{
    Shader::Ptr MeshS3O::s3oShader;
    Shader::Ptr MeshS3O::s3oShader_woShadows;

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
                GfxTexture::Ptr tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA_compressed(), NULL, true);
                if (tex)
                    gltex.push_back(tex);
			}
			if (tex_cache_name.size() == 2)
			{
				const QString &textureName = tex_cache_name[1];
                GfxTexture::Ptr tex = gfx->load_texture( textureName, FILTER_TRILINEAR, NULL, NULL, true, gfx->defaultTextureFormat_RGBA_compressed(), NULL, true);
                if (tex)
                    gltex.push_back(tex);
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

            std::vector<GfxTexture::Ptr> *pTex = &(root->gltex);

			hide |= explodes ^ exploding_parts;
			if (!hide)
			{
				if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
				{
                    bool activated_tex = false;
                    if (!alset)
                    {
                        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                        CHECK_GL();
                        glEnableClientState(GL_NORMAL_ARRAY);
                        CHECK_GL();
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        CHECK_GL();
                        glAlphaFunc( GL_GREATER, 0.1f );
                        CHECK_GL();
                        glEnable(GL_LIGHTING);
                        CHECK_GL();
                        alset = true;
                        set = true;
                    }
                    glEnable(GL_BLEND);
                    CHECK_GL();
                    glEnable( GL_ALPHA_TEST );
                    CHECK_GL();

                    if (!notex) // Les textures et effets de texture
                    {
                        if (lp_CONFIG->shadow_quality >= 2)
                        {
                            s3oShader->bind();
                            CHECK_GL();
                            s3oShader->setUniformValue( "tex0", 0 );
                            CHECK_GL();
                            s3oShader->setUniformValue( "tex1", 1 );
                            CHECK_GL();
                            s3oShader->setUniformValue( "shadowMap", 7 );
                            CHECK_GL();
                            s3oShader->setUniformValue( "t", 0.5f - 0.5f * cosf(t * PI) );
                            CHECK_GL();
                            s3oShader->setUniformValue( "team", player_color[player_color_map[side] * 3], player_color[player_color_map[side] * 3 + 1], player_color[player_color_map[side] * 3 + 2], 1.0f);
                            CHECK_GL();
                            s3oShader->setmat4f("light_Projection", gfx->shadowMapProjectionMatrix);
                            CHECK_GL();
                        }
                        else
                        {
                            s3oShader_woShadows->bind();
                            CHECK_GL();
                            s3oShader_woShadows->setUniformValue( "tex0", 0 );
                            CHECK_GL();
                            s3oShader_woShadows->setUniformValue( "tex1", 1 );
                            CHECK_GL();
                            s3oShader_woShadows->setUniformValue( "t", 0.5f - 0.5f * cosf(t * PI) );
                            CHECK_GL();
                            s3oShader_woShadows->setUniformValue( "team", player_color[player_color_map[side] * 3], player_color[player_color_map[side] * 3 + 1], player_color[player_color_map[side] * 3 + 2], 1.0f);
                            CHECK_GL();
                        }

                        activated_tex = true;
                        for (uint32 j = 0; j < pTex->size() ; ++j)
                        {
                            glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glEnable(GL_TEXTURE_2D);
                            CHECK_GL();
                            (*pTex)[j]->bind();
                            CHECK_GL();
                        }
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        CHECK_GL();
                        for (uint32 j = 0; j < pTex->size() ; ++j)
                        {
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                            CHECK_GL();
                        }
                    }
                    else
                    {
                        for (int j = 6; j >= 0; --j)
                        {
                            glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisable(GL_TEXTURE_2D);
                            CHECK_GL();
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            CHECK_GL();
                        }
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        CHECK_GL();
                        glEnable(GL_TEXTURE_2D);
                        CHECK_GL();
                        gfx->default_texture->bind();
                        CHECK_GL();
                    }
                    glVertexPointer(3, GL_FLOAT, 0, points);
                    CHECK_GL();
                    glNormalPointer(GL_FLOAT, 0, N);
                    CHECK_GL();
                    switch(type)
                    {
                    case MESH_TYPE_TRIANGLES:
                        glDrawRangeElements(GL_TRIANGLES, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);				// draw everything
                        CHECK_GL();
                        break;
                    case MESH_TYPE_TRIANGLE_STRIP:
                        glDisable( GL_CULL_FACE );
                        CHECK_GL();
                        glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
                        CHECK_GL();
                        glEnable( GL_CULL_FACE );
                        CHECK_GL();
                        break;
                    };

                    if (activated_tex)
                    {
                        for (uint32 j = 0; j < pTex->size() ; ++j)
                        {
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            CHECK_GL();

                            glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisable(GL_TEXTURE_2D);
                            CHECK_GL();
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                            CHECK_GL();
                            glDisable(GL_TEXTURE_GEN_S);
                            CHECK_GL();
                            glDisable(GL_TEXTURE_GEN_T);
                            CHECK_GL();
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            CHECK_GL();
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            CHECK_GL();
                        }
                        glClientActiveTextureARB(GL_TEXTURE0_ARB);
                        CHECK_GL();
                        glActiveTextureARB(GL_TEXTURE0_ARB);
                        CHECK_GL();
                        glEnable(GL_TEXTURE_2D);
                        CHECK_GL();
                    }
                    if (!notex)
                        s3oShader->release();
                    glDisable(GL_BLEND);
                    CHECK_GL();
                    glDisable(GL_ALPHA_TEST);
                    CHECK_GL();
                }
			}
			if (sel_primitive && selprim >= 0 && nb_vtx > 0)
			{
				gfx->disable_model_shading();
                CHECK_GL();
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();
                glDisableClientState(GL_NORMAL_ARRAY);
                CHECK_GL();
                glDisable(GL_LIGHTING);
                CHECK_GL();
                glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                glDisable(GL_FOG);
                CHECK_GL();
                glDisable(GL_BLEND);
                CHECK_GL();
                if (!set)
                {
					glVertexPointer( 3, GL_FLOAT, 0, points);
                    CHECK_GL();
                }
                glColor3ub(0,0xFF,0);
                CHECK_GL();
                glTranslatef( 0.0f, 2.0f, 0.0f );
                CHECK_GL();
                glDrawRangeElements(GL_LINE_LOOP, 0, nb_vtx-1, 4,GL_UNSIGNED_SHORT,sel);		// dessine la primitive de sélection
                CHECK_GL();
                glTranslatef( 0.0f, -2.0f, 0.0f );
                CHECK_GL();
                if (notex)
				{
					byte var = (byte)std::abs(int(0xFF - (msectimer() % 1000) * 0x200 / 1000));
					glColor3ub(0, var, 0);
                    CHECK_GL();
                }
				else
                {
					glColor3ub(0xFF, 0xFF, 0xFF);
                    CHECK_GL();
                }
                alset = false;
				gfx->enable_model_shading();
                CHECK_GL();
                glEnable(GL_FOG);
                CHECK_GL();
            }
			if (child && !(explodes && !exploding_parts))
				alset = child->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts && !explodes );
			glPopMatrix();
            CHECK_GL();
        }
		if (next)
			alset = next->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts);

		return alset;
	}

	bool MeshS3O::draw_nodl(bool alset)
	{
		init_shaders();
		glPushMatrix();
        CHECK_GL();

		glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);
        CHECK_GL();

		if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
		{
			if (!alset)
			{
				glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                CHECK_GL();
                glEnableClientState(GL_NORMAL_ARRAY);
                CHECK_GL();
                glEnable(GL_BLEND);
                CHECK_GL();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                CHECK_GL();
                glAlphaFunc( GL_GREATER, 0.1f );
                CHECK_GL();
                glEnable( GL_ALPHA_TEST );
                CHECK_GL();
                glEnable(GL_LIGHTING);
                CHECK_GL();
                alset = true;
			}

            std::vector<GfxTexture::Ptr> *pTex = &(root->gltex);

			for (uint32 j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB);
                CHECK_GL();
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();
                glActiveTextureARB(GL_TEXTURE0_ARB);
                CHECK_GL();
                glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                (*pTex)[j]->bind();
                CHECK_GL();
                glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                CHECK_GL();
            }
			glVertexPointer(3, GL_FLOAT, 0, points);
            CHECK_GL();
            glNormalPointer(GL_FLOAT, 0, N);
            CHECK_GL();
            switch(type)
			{
            case MESH_TYPE_TRIANGLES:
                glDrawRangeElements(GL_TRIANGLES, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);				// draw everything
                CHECK_GL();
                break;
            case MESH_TYPE_TRIANGLE_STRIP:
                glDisable( GL_CULL_FACE );
                CHECK_GL();
                glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
                CHECK_GL();
                glEnable( GL_CULL_FACE );
                CHECK_GL();
                break;
            };

			for (uint32 j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                CHECK_GL();
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();

				glActiveTextureARB(GL_TEXTURE0_ARB + j);
                CHECK_GL();
                glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                CHECK_GL();
                glDisable(GL_TEXTURE_GEN_S);
                CHECK_GL();
                glDisable(GL_TEXTURE_GEN_T);
                CHECK_GL();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                CHECK_GL();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                CHECK_GL();
            }
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
            CHECK_GL();
            glActiveTextureARB(GL_TEXTURE0_ARB);
            CHECK_GL();
            glEnable(GL_TEXTURE_2D);
            CHECK_GL();
        }
		if (child)
			alset = child->draw_nodl(alset);
		glPopMatrix();
        CHECK_GL();
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
		Joins::computeSelection(model);
		return model;
	}


	void MeshS3O::init_shaders()
	{
        if (!s3oShader)
            s3oShader = new Shader("shaders/s3o.frag", "shaders/s3o.vert");
        if (!s3oShader_woShadows)
            s3oShader_woShadows = new Shader("shaders/s3o_wos.frag", "shaders/s3o_wos.vert");
	}
} // namespace TA3D

