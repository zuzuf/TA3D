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
  |                                         3dm.cpp                                    |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers 3dm de TA3D qui sont les fichiers contenant les modèles 3D des objets |
  | du jeu.                                                                            |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include <stdafx.h>
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include "3dm.h"
#include "joins.h"
#include <gfx/particles/particles.h>
#include <ingame/sidedata.h>
#include <languages/i18n.h>
#include <misc/math.h>
#include <misc/paths.h>
#include <misc/timer.h>
#include <misc/files.h>
#include <logs/logs.h>
#include <gfx/gl.extensions.h>
#include <zlib.h>


namespace TA3D
{

    REGISTER_MESH_TYPE(Mesh3DM)


	void Mesh3DM::init3DM()
	{
		init();
		Color = 0;
		RColor = 0;
		Flag = 0;
		frag_shader_src.clear();
		vert_shader_src.clear();
        s_shader = nullptr;
        glColorTexture = nullptr;
		root = NULL;
	}



	void Mesh3DM::destroy3DM()
	{
		destroy();
        glColorTexture = nullptr;
		Color = 0;
		RColor = 0;
		Flag = 0;
		frag_shader_src.clear();
		vert_shader_src.clear();
        s_shader = nullptr;
		root = NULL;
	}

	bool Mesh3DM::draw(float t, AnimationData *data_s, bool sel_primitive, bool alset, bool notex, int side, bool chg_col, bool exploding_parts)
	{
		bool explodes = script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_EXPLODE);
		bool hide = false;
		bool set = false;
		float color_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (!tex_cache_name.empty())
		{
			for (unsigned int i = 0 ; i < tex_cache_name.size(); ++i)
				load_texture_id(i);
			tex_cache_name.clear();
		}
		if (!glColorTexture)
			glColorTexture = gfx->create_color_texture(Color);

		if (!(explodes && !exploding_parts))
		{
			glPushMatrix();
            CHECK_GL();

			glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);
            CHECK_GL();
            if (script_index >= 0 && data_s)
			{
				if (!explodes ^ exploding_parts)
				{
					glTranslatef(data_s->data[script_index].axe[0].pos, data_s->data[script_index].axe[1].pos, data_s->data[script_index].axe[2].pos);
                    CHECK_GL();
                    glRotatef(data_s->data[script_index].axe[0].angle, 1.0f, 0.0f, 0.0f);
                    CHECK_GL();
                    glRotatef(data_s->data[script_index].axe[1].angle, 0.0f, 1.0f, 0.0f);
                    CHECK_GL();
                    glRotatef(data_s->data[script_index].axe[2].angle, 0.0f, 0.0f, 1.0f);
                    CHECK_GL();
                }
				hide = data_s->data[script_index].flag & FLAG_HIDE;
			}
			else if (animation_data && data_s == NULL)
			{
				Vector3D R;
				Vector3D T;
				animation_data->animate(t, R, T);
				glTranslatef(T.x, T.y, T.z);
                CHECK_GL();
                glRotatef(R.x, 1.0f, 0.0f, 0.0f);
                CHECK_GL();
                glRotatef(R.y, 0.0f, 1.0f, 0.0f);
                CHECK_GL();
                glRotatef(R.z, 0.0f, 0.0f, 1.0f);
                CHECK_GL();
            }

            const std::vector<GfxTexture::Ptr> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

			hide |= explodes ^ exploding_parts;
			if (chg_col)
            {
                gfx->glGetFloatv(GL_CURRENT_COLOR, color_factor);
                CHECK_GL();
            }
			int texID = player_color_map[side];
			bool animatedTex = false;
			if (script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_ANIMATED_TEXTURE)
				&& !fixed_textures && !pTex->empty())
			{
				texID = ((int)(t * 10.0f)) % (int)pTex->size();
				animatedTex = true;
			}
            if (!hide)
			{
				if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
				{
					bool activated_tex = false;
					glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                    CHECK_GL();
                    glEnableClientState(GL_NORMAL_ARRAY);
                    CHECK_GL();
                    alset = false;
					set = false;
					if (!chg_col || !notex)
					{
						if (Flag & SURFACE_PLAYER_COLOR)
                        {
							glColor4f(player_color[side * 3], player_color[side * 3 + 1], player_color[side * 3 + 2], float(Color & 0xFF) / 255.0f);		// Couleur de matière
                            CHECK_GL();
                        }
                        else
                        {
							glColor4ubv((GLubyte*)&Color);		// Couleur de matière
                            CHECK_GL();
                        }
                    }
					else if (chg_col && notex)
					{
						if (Flag & SURFACE_PLAYER_COLOR)
						{
							glColor4f(player_color[player_color_map[side] * 3] * color_factor[0],
									  player_color[player_color_map[side] * 3 + 1] * color_factor[1],
									  player_color[player_color_map[side] * 3 + 2] * color_factor[2],
									  (float)geta32(Color) / 255.0f * color_factor[3]);		// Couleur de matière
                            CHECK_GL();
                        }
						else
						{
							glColor4f((float)getr32(Color) / 255.0f * color_factor[0],
									  (float)getg32(Color) / 255.0f * color_factor[1],
									  (float)getb32(Color) / 255.0f * color_factor[2],
									  (float)geta32(Color) / 255.0f * color_factor[3]);		// Couleur de matière
                            CHECK_GL();
                        }
					}

					if (Flag & SURFACE_GLSL)			// Using vertex and fragment programs
					{
                        s_shader->bind();
                        for (size_t j = 0; j < pTex->size() ; ++j)
                            s_shader->setvar1i( QString("tex%1").arg(j).toLatin1().data(), j );
					}

					if (Flag & SURFACE_GOURAUD)			// Type d'éclairage
                    {
                        glShadeModel (GL_SMOOTH);
                        CHECK_GL();
                    }
                    else
                    {
						glShadeModel (GL_FLAT);
                        CHECK_GL();
                    }

					if (Flag & SURFACE_LIGHTED)			// Eclairage
                    {
                        gfx->glEnable(GL_LIGHTING);
                        CHECK_GL();
                    }
                    else
                    {
                        gfx->glDisable(GL_LIGHTING);
                        CHECK_GL();
                    }

					if (chg_col || !notex)
					{
						if ((Flag & SURFACE_BLENDED) || (chg_col && color_factor[3] < 1.0f)) // La transparence
						{
                            gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                            CHECK_GL();
                            gfx->glEnable(GL_BLEND);
                            CHECK_GL();
                            glAlphaFunc(GL_GREATER, 0.1f);
                            CHECK_GL();
                            gfx->glEnable(GL_ALPHA_TEST);
                            CHECK_GL();
                        }
						else
						{
                            gfx->glDisable(GL_ALPHA_TEST);
                            CHECK_GL();
                            gfx->glDisable(GL_BLEND);
                            CHECK_GL();
                        }
					}

					if ((Flag & SURFACE_TEXTURED) && !notex) // Les textures et effets de texture
					{
						activated_tex = true;
						for (unsigned int j = 0; j < pTex->size() ; ++j)
						{
                            gfx->glActiveTexture(GL_TEXTURE0 + j);
                            CHECK_GL();
                            gfx->glEnable(GL_TEXTURE_2D);
                            CHECK_GL();
                            if (animatedTex)
                                (*pTex)[texID]->bind();
							else
                                (*pTex)[j]->bind();
							if (j == pTex->size() - 1 && (Flag & SURFACE_REFLEC))
							{
								glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                                CHECK_GL();
                                glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                                CHECK_GL();
                                gfx->glEnable(GL_TEXTURE_GEN_S);
                                CHECK_GL();
                                gfx->glEnable(GL_TEXTURE_GEN_T);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, TA3D_GL_COMBINE_EXT);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_COMBINE_RGB_EXT,GL_INTERPOLATE);
                                CHECK_GL();

								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE0_RGB_EXT,GL_TEXTURE);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE1_RGB_EXT,TA3D_GL_PREVIOUS_EXT);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE2_RGB_EXT,TA3D_GL_CONSTANT_EXT);
                                CHECK_GL();
                                glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
                                CHECK_GL();
                                float RColorf[4] = { (float)getr32(RColor) / 255.0f,
													 (float)getg32(RColor) / 255.0f,
													 (float)getb32(RColor) / 255.0f,
													 (float)geta32(RColor) / 255.0f};
								glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, RColorf);
                                CHECK_GL();
                            }
							else
							{
								glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                                CHECK_GL();
                                gfx->glDisable(GL_TEXTURE_GEN_S);
                                CHECK_GL();
                                gfx->glDisable(GL_TEXTURE_GEN_T);
                                CHECK_GL();
                                gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                CHECK_GL();
                                gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                                CHECK_GL();
                            }
							if (animatedTex)
								break;
						}
						for (unsigned int j = 0; j < pTex->size() ; ++j)
						{
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                            CHECK_GL();
                            glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                            CHECK_GL();
                            if (animatedTex)
								break;
						}
					}
					else
					{
						for (int j = 6; j >= 0; --j)
						{
                            gfx->glActiveTexture(GL_TEXTURE0 + j);
                            CHECK_GL();
                            gfx->glDisable(GL_TEXTURE_2D);
                            CHECK_GL();
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            CHECK_GL();
                        }
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        CHECK_GL();
                        gfx->glEnable(GL_TEXTURE_2D);
                        CHECK_GL();
                        gfx->default_texture->bind();
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
                            gfx->glDisable( GL_CULL_FACE );
                            CHECK_GL();
                            glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
                            CHECK_GL();
                            gfx->glEnable( GL_CULL_FACE );
                            CHECK_GL();
                            break;
					};

					if ((Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD)) == SURFACE_ADVANCED)
                    {
						glShadeModel (GL_SMOOTH);
                        CHECK_GL();
                    }
                    if ((Flag&SURFACE_GLSL) && (Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
                        s_shader->release();
                    gfx->glDisable(GL_ALPHA_TEST);
                    CHECK_GL();

					if (activated_tex)
					{
						for (unsigned int j = 0; j < pTex->size() ; ++j)
						{
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            CHECK_GL();
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            CHECK_GL();

                            gfx->glActiveTexture(GL_TEXTURE0 + j);
                            CHECK_GL();
                            gfx->glDisable(GL_TEXTURE_2D);
                            CHECK_GL();
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                            CHECK_GL();
                            gfx->glDisable(GL_TEXTURE_GEN_S);
                            CHECK_GL();
                            gfx->glDisable(GL_TEXTURE_GEN_T);
                            CHECK_GL();
                            gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            CHECK_GL();
                            gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            CHECK_GL();
                        }
						glClientActiveTextureARB(GL_TEXTURE0_ARB);
                        CHECK_GL();
                        gfx->glActiveTexture(GL_TEXTURE0);
                        CHECK_GL();
                        gfx->glEnable(GL_TEXTURE_2D);
                        CHECK_GL();
                    }
				}
			}
			if (sel_primitive && selprim >= 0 && nb_vtx > 0)
			{
				gfx->disable_model_shading();
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();
                glDisableClientState(GL_NORMAL_ARRAY);
                CHECK_GL();
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                gfx->glDisable(GL_FOG);
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
					const byte var = (byte)std::abs(int(0xFF - (msectimer() % 1000) * 0x200 / 1000));
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
                gfx->glEnable(GL_FOG);
                CHECK_GL();
            }
			if (chg_col)
            {
				glColor4fv(color_factor);
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

	bool Mesh3DM::draw_nodl(bool alset)
	{
		if (!glColorTexture)
			glColorTexture = gfx->create_color_texture(Color);

		glPushMatrix();
        CHECK_GL();

		glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);
        CHECK_GL();

		if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
		{
			glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
            CHECK_GL();
            glEnableClientState(GL_NORMAL_ARRAY);
            CHECK_GL();
            alset = false;

            std::vector<GfxTexture::Ptr> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

			if (Flag & SURFACE_GLSL)			// Using vertex and fragment programs
			{
                s_shader->bind();
				for (unsigned int j = 0; j < pTex->size() ; ++j)
                    s_shader->setvar1i( QString("tex%1").arg(j).toLatin1().data(), j + 1 );
			}

			if (Flag & SURFACE_GOURAUD)			// Type d'éclairage
            {
				glShadeModel (GL_SMOOTH);
                CHECK_GL();
            }
            else
            {
				glShadeModel (GL_FLAT);
                CHECK_GL();
            }

			if (Flag & SURFACE_LIGHTED)			// Eclairage
            {
                gfx->glEnable(GL_LIGHTING);
                CHECK_GL();
            }
            else
            {
                gfx->glDisable(GL_LIGHTING);
                CHECK_GL();
            }

			if (Flag & SURFACE_BLENDED) // La transparence
			{
                gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                CHECK_GL();
                gfx->glEnable(GL_BLEND);
                CHECK_GL();
                glAlphaFunc( GL_GREATER, 0.1f );
                CHECK_GL();
                gfx->glEnable( GL_ALPHA_TEST );
			}
			else
			{
                gfx->glDisable(GL_ALPHA_TEST);
                CHECK_GL();
                gfx->glDisable(GL_BLEND);
                CHECK_GL();
            }

			glClientActiveTextureARB(GL_TEXTURE0_ARB + (int)pTex->size());
            CHECK_GL();
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            CHECK_GL();
            gfx->glActiveTexture(GL_TEXTURE0 + (int)pTex->size());
            CHECK_GL();
            gfx->glEnable(GL_TEXTURE_2D);
            CHECK_GL();
            glColorTexture->bind();
			glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
            CHECK_GL();

			for (unsigned int j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                CHECK_GL();
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();
                gfx->glActiveTexture(GL_TEXTURE0 + j);
                CHECK_GL();
                gfx->glEnable(GL_TEXTURE_2D);
                CHECK_GL();
                (*pTex)[j]->bind();
				glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                CHECK_GL();

				if (j == pTex->size() - 1 && (Flag & SURFACE_REFLEC))
				{
					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                    CHECK_GL();
                    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_GEN_S);
                    CHECK_GL();
                    gfx->glEnable(GL_TEXTURE_GEN_T);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, TA3D_GL_COMBINE_EXT);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_COMBINE_RGB_EXT,GL_INTERPOLATE);
                    CHECK_GL();

					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE0_RGB_EXT,GL_TEXTURE);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE1_RGB_EXT,TA3D_GL_PREVIOUS_EXT);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE2_RGB_EXT,TA3D_GL_CONSTANT_EXT);
                    CHECK_GL();
                    glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
                    CHECK_GL();
                    float RColorf[4] = { (float)getr32(RColor) / 255.0f,
										 (float)getg32(RColor) / 255.0f,
										 (float)getb32(RColor) / 255.0f,
										 (float)geta32(RColor) / 255.0f};
					glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, RColorf);
                    CHECK_GL();
                }
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
                    gfx->glDisable( GL_CULL_FACE );
                    CHECK_GL();
                    glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx - 1, nb_t_index, GL_UNSIGNED_SHORT, t_index);		// draw everything
                    CHECK_GL();
                    gfx->glEnable( GL_CULL_FACE );
                    CHECK_GL();
                    break;
			};

			if ((Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD)) == SURFACE_ADVANCED)
            {
				glShadeModel (GL_SMOOTH);
                CHECK_GL();
            }
            if ((Flag&SURFACE_GLSL) && (Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
                s_shader->release();

			for (unsigned int j = 0; j < pTex->size() + 1 ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                CHECK_GL();
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();

                gfx->glActiveTexture(GL_TEXTURE0 + j);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_2D);
                CHECK_GL();
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_GEN_S);
                CHECK_GL();
                gfx->glDisable(GL_TEXTURE_GEN_T);
                CHECK_GL();
                gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                CHECK_GL();
                gfx->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                CHECK_GL();
            }
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
            CHECK_GL();
            gfx->glActiveTexture(GL_TEXTURE0);
            CHECK_GL();
            gfx->glEnable(GL_TEXTURE_2D);
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

    void Mesh3DM::load(QIODevice *file, const QString &filename, Mesh3DM *root)
	{
		destroy3DM();
		if (root == NULL)
			root = this;
		this->root = root;

		if (file == NULL)
			return;

        const uint8 len = (uint8)readChar(file);
		char tmp[257];
		memset(tmp, 0, 257);
		file->read(tmp, len);
        name = QString::fromLatin1(tmp, len);

#define READ(X) file->read((char*)&(X), sizeof(X))
        READ(pos_from_parent.x);
		if (isNaN(pos_from_parent.x))           // Some error checks
		{
			name.clear();
			file->close();
			return;
		}

        READ(pos_from_parent.y);
		if (isNaN(pos_from_parent.y))           // Some error checks
		{
			name.clear();
			file->close();
			return;
		}

        READ(pos_from_parent.z);
		if (isNaN(pos_from_parent.z))
		{
			name.clear();
			file->close();
			return;
		}

        READ(nb_vtx);
		if (nb_vtx < 0)
		{
			name.clear();
			file->close();
			return;
		}
		if (nb_vtx > 0)
		{
			points = new Vector3D[nb_vtx<<1];
            file->read((char*)points, (int)sizeof(Vector3D) * nb_vtx);
		}
		else
			points = NULL;

        file->read((char*)sel, (int)sizeof(GLushort) * 4);

        READ(nb_p_index); // Read point data
		if (nb_p_index < 0)
		{
			DELETE_ARRAY(points);
			name.clear();
			init();
			file->close();
			return;
		}
		if (nb_p_index > 0)
		{
			p_index = new GLushort[nb_p_index];
            file->read((char*)p_index, (int)sizeof(GLushort) * nb_p_index);
		}
		else
			p_index = NULL;

        READ(nb_l_index);	// Read line data
		if (nb_l_index < 0)
		{
			DELETE_ARRAY(points);
			DELETE_ARRAY(p_index);
			name.clear();
			init();
			file->close();
			return;
		}
		if (nb_l_index > 0)
		{
			l_index = new GLushort[nb_l_index];
            file->read((char*)l_index, (int)sizeof(GLushort) * nb_l_index);
		}
		else
			l_index = NULL;

		file->read(nb_t_index); // Read triangle data
		if (nb_t_index < 0)
		{
			DELETE_ARRAY(points);
			DELETE_ARRAY(p_index);
			DELETE_ARRAY(l_index);
			name.clear();
			init();
			file->close();
			return;
		}
		if (nb_t_index > 0)
		{
			t_index = new GLushort[nb_t_index];
            file->read((char*)t_index, (int)sizeof(GLushort) * nb_t_index);
		}
		else
			t_index = NULL;

		tcoord = new float[nb_vtx << 1];
        file->read((char*)tcoord, (int)sizeof(float) * nb_vtx << 1);

		float Colorf[4];
		float RColorf[4];
        file->read((char*)Colorf, (int)sizeof(float) * 4);	// Read surface data
        file->read((char*)RColorf, (int)sizeof(float) * 4);
		Color = makeacol32((int)(Colorf[0] * 255), (int)(Colorf[1] * 255), (int)(Colorf[2] * 255), (int)(Colorf[3] * 255));
		RColor = makeacol32((int)(RColorf[0] * 255), (int)(RColorf[1] * 255), (int)(RColorf[2] * 255), (int)(RColorf[3] * 255));
        READ(Flag);
		Flag |= SURFACE_ADVANCED;           // This is default flag ... not very useful now that 3DM and 3DO codes have been separated

		if (Flag >= 0x200)
		{
			DELETE_ARRAY(points);
			DELETE_ARRAY(p_index);
			DELETE_ARRAY(l_index);
			DELETE_ARRAY(tcoord);
			name.clear();
			init();
			file->close();
			return;
		}

		sint8 NbTex = 0;
        READ(NbTex);
		bool compressed = NbTex < 0;
		NbTex = (sint8)abs( NbTex );
		gltex.resize(NbTex);
		for (int i = 0; i < NbTex; ++i)
		{
            QImage tex;
			if (!compressed)
			{
				int tex_w;
				int tex_h;
                READ(tex_w);
                READ(tex_h);

				tex = gfx->create_surface_ex(32, tex_w, tex_h);
                if (tex.isNull())
				{
					destroy();
					file->close();
					return;
				}
				try
				{
                    for (int y = 0 ; y < tex.height() ; ++y)
                        for (int x = 0 ; x < tex.width() ; ++x)
                            READ(SurfaceInt(tex, x, y));
				}
				catch(...)
				{
					destroy();
					file->close();
					return;
				}
			}
			else
			{
				int img_size = 0;
				uint8 bpp;
				int w, h;
                READ(w);
                READ(h);
                READ(bpp);
                READ(img_size);	// Read RGBA data
				byte *buffer = new byte[ img_size ];

				try
				{
                    file->read((char*)buffer, img_size);

					tex = gfx->create_surface_ex( bpp, w, h );
                    uLongf len = tex.byteCount();
                    uncompress ( (Bytef*) tex.bits(), &len, (Bytef*) buffer, img_size);
				}
				catch( ... )
				{
					DELETE_ARRAY(buffer);
					destroy();
					file->close();
					return;
				}

				DELETE_ARRAY(buffer);
			}

            QString cache_filename = !filename.isEmpty() ? filename + '-' + (!name.isEmpty() ? name : "none") + QString("-%1.bin").arg(i) : QString();
			cache_filename.replace('/', 'S');

			gltex[i] = 0;
			if (!gfx->is_texture_in_cache(cache_filename))
			{
				cache_filename = TA3D::Paths::Files::ReplaceExtension( cache_filename, ".tex" );
                if (!TA3D::Paths::Exists( TA3D::Paths::Caches + cache_filename ))
                    SaveTex( tex, TA3D::Paths::Caches + cache_filename );
			}
			tex_cache_name.push_back( cache_filename );
		}

		if (Flag & SURFACE_GLSL) // Fragment & Vertex shaders
		{
			uint32 shader_size;
            READ(shader_size);
			char *buf = new char[shader_size + 1];
			buf[shader_size] = 0;
			file->read(buf, shader_size);
			vert_shader_src = buf;
			DELETE_ARRAY(buf);

            READ(shader_size);
			buf = new char[shader_size + 1];
			buf[shader_size] = 0;
			file->read(buf,shader_size);
			frag_shader_src = buf;
			DELETE_ARRAY(buf);
            s_shader->load_memory(frag_shader_src, vert_shader_src);
		}

		N = new Vector3D[nb_vtx << 1]; // Calculate normals
        if (nb_t_index > 0 && t_index != NULL)
		{
			F_N = new Vector3D[nb_t_index / 3];
			for (int i = 0; i < nb_vtx; ++i)
				N[i].reset();
			int e = 0;
			for (int i = 0 ; i < nb_t_index ; i += 3)
			{
				Vector3D AB,AC,Normal;
				AB = points[t_index[i+1]] - points[t_index[i]];
				AC = points[t_index[i+2]] - points[t_index[i]];
				Normal = AB * AC;
				Normal.unit();
				F_N[e++] = Normal;
				for (byte e = 0; e < 3; ++e)
					N[t_index[i + e]] = N[t_index[i + e]] + Normal;
			}
			for (int i = 0; i < nb_vtx; ++i)
				N[i].unit();
		}

        byte link = (byte)readChar(file);

		if (link == 2) // Load animation data if present
		{
			animation_data = new Animation;
            READ( animation_data->type );
            READ( animation_data->angle_0 );
            READ( animation_data->angle_1 );
            READ( animation_data->angle_w );
            READ( animation_data->translate_0 );
            READ( animation_data->translate_1 );
            READ( animation_data->translate_w );

            link = (byte)readChar(file);
		}

		if (link)
		{
			Mesh3DM *pChild = new Mesh3DM;
			child = pChild;
			pChild->load(file, filename, root);
			if (!file->isOpen())
			{
				destroy();
				return;
			}
		}
		else
			child = NULL;

        link = (byte)readChar(file);
		if (link)
		{
			Mesh3DM *pNext = new Mesh3DM;
			next = pNext;
			pNext->load(file, filename, root);
			if (!file->isOpen())
			{
				destroy();
				return;
			}
		}
		else
			next = NULL;
	}

	Model *Mesh3DM::load(const QString &filename)
	{
        QIODevice *file = VFS::Instance()->readFile(filename);
		if (!file)
		{
			LOG_ERROR(LOG_PREFIX_3DM << "could not read file '" << filename << "'");
			return NULL;
		}

        if (readChar(file) == 0)       // This is a pointer file
		{
            const QString realFilename = QString::fromUtf8(file->readAll()).trimmed().replace('\\', '/');
			delete file;
			LOG_INFO(LOG_PREFIX_3DM << "file '" << filename << "' points to '" << realFilename << "'");
			file = VFS::Instance()->readFile(realFilename);
			if (!file)
			{
				LOG_ERROR(LOG_PREFIX_3DM << "could not read file '" << realFilename << "'");
				return NULL;
			}
		}

		Mesh3DM *mesh = new Mesh3DM;
		file->seek(0);
		mesh->load(file, filename);
		delete file;

		Model *model = new Model;
		model->mesh = mesh;
		model->postLoadComputations();
		Joins::computeSelection(model);
		return model;
	}


	bool Mesh3DM::has_animation_data() const
	{
        const std::vector<GfxTexture::Ptr> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

		if (animation_data || (Flag & SURFACE_GLSL) || ((Flag & SURFACE_TEXTURED) && pTex->size() > 1))
			return true;
		if (next)
			return next->has_animation_data();
		if (child)
			return child->has_animation_data();
		return false;
	}

	const char *Mesh3DM::getExt()
	{
		return ".3dm";
	}
} // namespace TA3D

