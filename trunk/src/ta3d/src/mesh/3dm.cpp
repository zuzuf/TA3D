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
#include <misc/files.h>
#include <logs/logs.h>
#include <gfx/gl.extensions.h>
#include <zlib.h>


namespace TA3D
{

	REGISTER_MESH_TYPE(Mesh3DM);


	void Mesh3DM::init3DM()
	{
		init();
		Color = 0;
		RColor = 0;
		Flag = 0;
		frag_shader_src.clear();
		vert_shader_src.clear();
		s_shader.destroy();
		glColorTexture = 0;
		root = NULL;
	}



	void Mesh3DM::destroy3DM()
	{
		destroy();
		gfx->destroy_texture(glColorTexture);
		Color = 0;
		RColor = 0;
		Flag = 0;
		frag_shader_src.clear();
		vert_shader_src.clear();
		s_shader.destroy();
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
			for(uint32 i = 0 ; i < tex_cache_name.size() ; ++i)
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

			const std::vector<GLuint> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

			hide |= explodes ^ exploding_parts;
			if (chg_col)
				glGetFloatv(GL_CURRENT_COLOR, color_factor);
			int texID = player_color_map[side];
			bool disableDL = ((pTex->size() > 1 && (Flag & SURFACE_TEXTURED)) || Flag & SURFACE_GLSL) && !notex;
			bool animatedTex = false;
			if (script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_ANIMATED_TEXTURE)
				&& !fixed_textures && !pTex->empty())
			{
				texID = ((int)(t * 10.0f)) % pTex->size();
				disableDL = false;
				animatedTex = true;
			}
			if (gl_dlist.size() > texID && gl_dlist[texID] && !hide && !chg_col && !notex && !disableDL)
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
				if (!chg_col && !notex && gl_dlist[texID] == 0 && !disableDL)
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
					if (!chg_col || !notex)
					{
						if (Flag & SURFACE_PLAYER_COLOR)
							glColor4f(player_color[side * 3], player_color[side * 3 + 1], player_color[side * 3 + 2], (Color & 0xFF) / 255.0f);		// Couleur de matière
						else
							glColor4ubv((GLubyte*)&Color);		// Couleur de matière
					}
					else if (chg_col && notex)
					{
						if (Flag & SURFACE_PLAYER_COLOR)
							glColor4f(player_color[player_color_map[side] * 3] * color_factor[0],
									  player_color[player_color_map[side] * 3 + 1] * color_factor[1],
									  player_color[player_color_map[side] * 3 + 2] * color_factor[2],
									  geta32(Color) / 255.0f * color_factor[3]);		// Couleur de matière
						else
							glColor4f(getr32(Color) / 255.0f * color_factor[0],
									  getg32(Color) / 255.0f * color_factor[1],
									  getb32(Color) / 255.0f * color_factor[2],
									  geta32(Color) / 255.0f * color_factor[3]);		// Couleur de matière
					}

					if (Flag & SURFACE_GLSL)			// Using vertex and fragment programs
					{
						s_shader.on();
						for (uint32 j = 0; j < pTex->size() ; ++j)
							s_shader.setvar1i( String("tex") << j, j );
					}

					if (Flag & SURFACE_GOURAUD)			// Type d'éclairage
						glShadeModel (GL_SMOOTH);
					else
						glShadeModel (GL_FLAT);

					if (Flag & SURFACE_LIGHTED)			// Eclairage
						glEnable(GL_LIGHTING);
					else
						glDisable(GL_LIGHTING);

					if (chg_col || !notex)
					{
						if ((Flag & SURFACE_BLENDED) || (chg_col && color_factor[3] != 1.0f)) // La transparence
						{
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
							glEnable(GL_BLEND);
							glAlphaFunc( GL_GREATER, 0.1 );
							glEnable( GL_ALPHA_TEST );
						}
						else
						{
							glDisable(GL_ALPHA_TEST);
							glDisable(GL_BLEND);
						}
					}

					if ((Flag & SURFACE_TEXTURED) && !notex) // Les textures et effets de texture
					{
						activated_tex = true;
						for (uint32 j = 0; j < pTex->size() ; ++j)
						{
							glActiveTextureARB(GL_TEXTURE0_ARB + j);
							glEnable(GL_TEXTURE_2D);
							if (animatedTex)
								glBindTexture(GL_TEXTURE_2D, (*pTex)[texID]);
							else
								glBindTexture(GL_TEXTURE_2D, (*pTex)[j]);
							if (j == pTex->size() - 1 && (Flag & SURFACE_REFLEC))
							{
								glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
								glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
								glEnable(GL_TEXTURE_GEN_S);
								glEnable(GL_TEXTURE_GEN_T);
								glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, TA3D_GL_COMBINE_EXT);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_COMBINE_RGB_EXT,GL_INTERPOLATE);

								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE0_RGB_EXT,GL_TEXTURE);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE1_RGB_EXT,TA3D_GL_PREVIOUS_EXT);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE2_RGB_EXT,TA3D_GL_CONSTANT_EXT);
								glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
								float RColorf[4] = { getr32(RColor) / 255.0f,
									getg32(RColor) / 255.0f,
									getb32(RColor) / 255.0f,
									geta32(RColor) / 255.0f};
								glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, RColorf);
							}
							else
							{
								glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
								glDisable(GL_TEXTURE_GEN_S);
								glDisable(GL_TEXTURE_GEN_T);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
							}
							if (animatedTex)
								break;
						}
						for (uint32 j = 0; j < pTex->size() ; ++j)
						{
							glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
							glEnableClientState(GL_TEXTURE_COORD_ARRAY);
							glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
							if (animatedTex)
								break;
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

					if ((Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD)) == SURFACE_ADVANCED)
						glShadeModel (GL_SMOOTH);
					if ((Flag&SURFACE_GLSL) && (Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
						s_shader.off();
					glDisable(GL_ALPHA_TEST);

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
				}
				if (creating_list)
					glEndList();
			}
			if (sel_primitive && selprim >= 0 && nb_vtx > 0)
			{
				gfx->disable_model_shading();
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_NORMAL_ARRAY);
				glDisable(GL_LIGHTING);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_FOG);
				if (!set)
					glVertexPointer( 3, GL_FLOAT, 0, points);
				glColor3ub(0,0xFF,0);
				glTranslatef( 0.0f, 2.0f, 0.0f );
				glDrawRangeElements(GL_LINE_LOOP, 0, nb_vtx-1, 4,GL_UNSIGNED_SHORT,sel);		// dessine la primitive de sélection
				glTranslatef( 0.0f, -2.0f, 0.0f );
				if (notex)
				{
					int var = abs(0xFF - (msec_timer%1000)*0x200/1000);
					glColor3ub(0,var,0);
				}
				else
					glColor3ub(0xFF,0xFF,0xFF);
				alset = false;
				gfx->enable_model_shading();
				glEnable(GL_FOG);
			}
			if (chg_col)
				glColor4fv(color_factor);
			if (child && !(explodes && !exploding_parts))
				alset = child->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts && !explodes );
			glPopMatrix();
		}
		if (next)
			alset = next->draw(t, data_s, sel_primitive, alset, notex, side, chg_col, exploding_parts);

		return alset;
	}

	bool Mesh3DM::draw_nodl(bool alset)
	{
		glPushMatrix();

		glTranslatef(pos_from_parent.x, pos_from_parent.y, pos_from_parent.z);

		if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
		{
			glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
			glEnableClientState(GL_NORMAL_ARRAY);
			alset = false;

			std::vector<GLuint> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

			if (Flag & SURFACE_GLSL)			// Using vertex and fragment programs
			{
				s_shader.on();
				for (uint32 j = 0; j < pTex->size() ; ++j)
					s_shader.setvar1i( String("tex") << j, j + 1 );
			}

			if (Flag & SURFACE_GOURAUD)			// Type d'éclairage
				glShadeModel (GL_SMOOTH);
			else
				glShadeModel (GL_FLAT);

			if (Flag & SURFACE_LIGHTED)			// Eclairage
				glEnable(GL_LIGHTING);
			else
				glDisable(GL_LIGHTING);

			if (Flag & SURFACE_BLENDED) // La transparence
			{
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
				glAlphaFunc( GL_GREATER, 0.1 );
				glEnable( GL_ALPHA_TEST );
			}
			else
			{
				glDisable(GL_ALPHA_TEST);
				glDisable(GL_BLEND);
			}

			glClientActiveTextureARB(GL_TEXTURE0_ARB + pTex->size());
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glActiveTextureARB(GL_TEXTURE0_ARB + pTex->size());
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, glColorTexture);
			glTexCoordPointer(2, GL_FLOAT, 0, tcoord);

			for (uint32 j = 0; j < pTex->size() ; ++j)
			{
				glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glActiveTextureARB(GL_TEXTURE0_ARB + j);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, (*pTex)[j]);
				glTexCoordPointer(2, GL_FLOAT, 0, tcoord);

				if (j == pTex->size() - 1 && (Flag & SURFACE_REFLEC))
				{
					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
					glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
					glEnable(GL_TEXTURE_GEN_S);
					glEnable(GL_TEXTURE_GEN_T);
					glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, TA3D_GL_COMBINE_EXT);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_COMBINE_RGB_EXT,GL_INTERPOLATE);

					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE0_RGB_EXT,GL_TEXTURE);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE1_RGB_EXT,TA3D_GL_PREVIOUS_EXT);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_SOURCE2_RGB_EXT,TA3D_GL_CONSTANT_EXT);
					glTexEnvi(GL_TEXTURE_ENV,TA3D_GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
					float RColorf[4] = { getr32(RColor) / 255.0f,
						getg32(RColor) / 255.0f,
						getb32(RColor) / 255.0f,
						geta32(RColor) / 255.0f};
					glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, RColorf);
				}
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

			if ((Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD)) == SURFACE_ADVANCED)
				glShadeModel (GL_SMOOTH);
			if ((Flag&SURFACE_GLSL) && (Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
				s_shader.off();

			for (uint32 j = 0; j < pTex->size() + 1 ; ++j)
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

	void Mesh3DM::load(File *file, const String &filename, Mesh3DM *root)
	{
		destroy3DM();
		if (root == NULL)
			root = this;
		this->root = root;

		if (file == NULL)
			return;

		uint8 len = file->getc();
		char tmp[257];
		memset(tmp, 0, 257);
		file->read(tmp, len);
		name = String(tmp, len);

		file->read(pos_from_parent.x);
		if (isNaN(pos_from_parent.x))           // Some error checks
		{
			name.clear();
			file->close();
			return;
		}

		file->read(pos_from_parent.y);
		if (isNaN(pos_from_parent.y))           // Some error checks
		{
			name.clear();
			file->close();
			return;
		}

		file->read(pos_from_parent.z);
		if (isNaN(pos_from_parent.z))
		{
			name.clear();
			file->close();
			return;
		}

		file->read(nb_vtx);
		if (nb_vtx < 0)
		{
			name.clear();
			file->close();
			return;
		}
		if (nb_vtx > 0)
		{
			points = new Vector3D[nb_vtx<<1];
			file->read(points,sizeof(Vector3D)*nb_vtx);
		}
		else
			points = NULL;

		file->read(sel, sizeof(GLushort) * 4);

		file->read(nb_p_index); // Read point data
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
			file->read(p_index, sizeof(GLushort) * nb_p_index);
		}
		else
			p_index = NULL;

		file->read(nb_l_index);	// Read line data
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
			file->read(l_index, sizeof(GLushort) * nb_l_index);
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
			file->read(t_index, sizeof(GLushort) * nb_t_index);
		}
		else
			t_index = NULL;

		tcoord = new float[nb_vtx << 1];
		file->read(tcoord, sizeof(float) * nb_vtx << 1);

		float Colorf[4];
		float RColorf[4];
		file->read(Colorf, sizeof(float) * 4);	// Read surface data
		file->read(RColorf, sizeof(float) * 4);
		Color = makeacol32((int)(Colorf[0] * 255), (int)(Colorf[1] * 255), (int)(Colorf[2] * 255), (int)(Colorf[3] * 255));
		RColor = makeacol32((int)(RColorf[0] * 255), (int)(RColorf[1] * 255), (int)(RColorf[2] * 255), (int)(RColorf[3] * 255));
		file->read(Flag);
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

		glColorTexture = gfx->create_color_texture(Color);

		sint8 NbTex = 0;
		file->read(NbTex);
		bool compressed = NbTex < 0;
		NbTex = abs( NbTex );
		gltex.resize(NbTex);
		for (uint8 i = 0; i < NbTex; ++i)
		{
			SDL_Surface *tex;
			if (!compressed)
			{
				int tex_w;
				int tex_h;
				file->read(tex_w);
				file->read(tex_h);

				tex = gfx->create_surface_ex(32, tex_w, tex_h);
				if (tex == NULL)
				{
					destroy();
					file->close();
					return;
				}
				try
				{
					for (int y = 0; y < tex->h; ++y)
						for (int x = 0; x < tex->w; ++x)
							file->read(SurfaceInt(tex, x, y));
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
				file->read(w);
				file->read(h);
				file->read(bpp);
				file->read(img_size);	// Read RGBA data
				byte *buffer = new byte[ img_size ];

				try
				{
					file->read(buffer, img_size);

					tex = gfx->create_surface_ex( bpp, w, h );
					uLongf len = tex->w * tex->h * tex->format->BytesPerPixel;
					uncompress ( (Bytef*) tex->pixels, &len, (Bytef*) buffer, img_size);
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

			String cache_filename = !filename.empty() ? String(filename) << '-' << (!name.empty() ? name : "none") << '-' << int(i) << ".bin" : String();
			cache_filename.replace('/', 'S');
			cache_filename.replace('\\', 'S');

			gltex[i] = 0;
			if (!gfx->is_texture_in_cache(cache_filename))
			{
				cache_filename = TA3D::Paths::Files::ReplaceExtension( cache_filename, ".tex" );
				if (!TA3D::Paths::Exists( String(TA3D::Paths::Caches) << cache_filename ))
					SaveTex( tex, String(TA3D::Paths::Caches) << cache_filename );
			}
			tex_cache_name.push_back( cache_filename );

			SDL_FreeSurface(tex);
		}

		if (Flag & SURFACE_GLSL) // Fragment & Vertex shaders
		{
			uint32 shader_size;
			file->read(shader_size);
			char *buf = new char[shader_size + 1];
			buf[shader_size] = 0;
			file->read(buf, shader_size);
			vert_shader_src = buf;
			DELETE_ARRAY(buf);

			file->read(shader_size);
			buf = new char[shader_size + 1];
			buf[shader_size] = 0;
			file->read(buf,shader_size);
			frag_shader_src = buf;
			DELETE_ARRAY(buf);
			s_shader.load_memory(frag_shader_src.data(),frag_shader_src.size(),vert_shader_src.data(),vert_shader_src.size());
		}

		N = new Vector3D[nb_vtx << 1]; // Calculate normals
		if (nb_t_index>0 && t_index != NULL)
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

		byte link = file->getc();

		if (link == 2) // Load animation data if present
		{
			animation_data = new Animation;
			file->read( animation_data->type );
			file->read( animation_data->angle_0 );
			file->read( animation_data->angle_1 );
			file->read( animation_data->angle_w );
			file->read( animation_data->translate_0 );
			file->read( animation_data->translate_1 );
			file->read( animation_data->translate_w );

			link = file->getc();
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

		link = file->getc();
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

	Model *Mesh3DM::load(const String &filename)
	{
		File *file = VFS::Instance()->readFile(filename);
		if (!file)
		{
			LOG_ERROR(LOG_PREFIX_3DM << "could not read file '" << filename << "'");
			return NULL;
		}

		if (file->getc() == 0)       // This is a pointer file
		{
			String realFilename = file->data() + 1;
			delete file;
			realFilename.trim();
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
		std::deque<Mesh3DM*> qmesh;
		qmesh.push_back(mesh);
		while(!qmesh.empty())
		{
			Mesh3DM *cur = qmesh.front();
			qmesh.pop_front();
			if (cur->child)
				qmesh.push_back((Mesh3DM*)(cur->child));
			if (cur->next)
				qmesh.push_back((Mesh3DM*)(cur->next));
			const std::vector<GLuint> *pTex = (cur->Flag & SURFACE_ROOT_TEXTURE) ? &(cur->root->gltex) : &(cur->gltex);
			if (pTex->size() > 1)
			{
				model->useDL = false;
				break;
			}
		}
		return model;
	}


	bool Mesh3DM::has_animation_data() const
	{
		const std::vector<GLuint> *pTex = (Flag & SURFACE_ROOT_TEXTURE) ? &(root->gltex) : &gltex;

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

