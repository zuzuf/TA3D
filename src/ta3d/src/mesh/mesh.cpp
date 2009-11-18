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
|                                       mesh.cpp                                     |
|  This module contains the base class for all meshes. The base mesh class is a      |
| virtual class so that a mesh type can be implemented separately                    |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#include <stdafx.h>
#include <misc/paths.h>
#include <misc/files.h>
#include <gfx/particles/particles.h>
#include <languages/i18n.h>
#include <ingame/sidedata.h>
#include "mesh.h"
#include "3do.h"
#include "3dm.h"
#include "s3o.h"

namespace TA3D
{



	MODEL_MANAGER	model_manager;



	void ANIMATION_DATA::init()
	{
		is_moving = false;
		nb_piece = 0;
		axe[0] = axe[1] = axe[2] = NULL;
		flag = NULL;
		explosion_flag = NULL;
		pos = NULL;
		dir = NULL;
		matrix = NULL;
		explode = false;
		explode_time = 0.0f;
	}

	void ANIMATION_DATA::destroy()
	{
		for (int i = 0; i < 3; ++i)
			DELETE_ARRAY(axe[i]);
		DELETE_ARRAY(matrix);
		DELETE_ARRAY(dir);
		DELETE_ARRAY(pos);
		DELETE_ARRAY(flag);
		DELETE_ARRAY(explosion_flag);
		init();
	}


	void ANIMATION_DATA::load(const int nb)
	{
		destroy();		// Au cas où
		nb_piece = nb;
		flag = new short[nb_piece];
		explosion_flag = new short[nb_piece];
		pos = new Vector3D[nb_piece];
		dir = new Vector3D[nb_piece];
		matrix = new Matrix[nb_piece];
		for (int i = 0; i < nb_piece; ++i)
		{
			flag[i] = 0;
			explosion_flag[i] = 0;
			pos[i].x = pos[i].y = pos[i].z = 0.0f;
			dir[i] = pos[i];
			matrix[i] = Scale(1.0f);
		}
		for (int i = 0; i < 3; ++i)
		{
			DELETE_ARRAY(axe[i]);
			axe[i] = new AXE[nb_piece];
			for (int e = 0; e < nb_piece; ++e)
			{
				axe[i][e].move_speed = 0.0f;
				axe[i][e].move_distance = 0.0f;
				axe[i][e].pos = 0.0f;
				axe[i][e].rot_angle = 0.0f;
				axe[i][e].rot_speed = 0.0f;
				axe[i][e].rot_accel = 0.0f;
				axe[i][e].angle = 0.0f;
				axe[i][e].rot_limit = true;
				axe[i][e].rot_speed_limit = false;
				axe[i][e].rot_target_speed = 0.0f;
				axe[i][e].is_moving = false;
			}
		}
	}



	void ANIMATION_DATA::move(const float dt, const float g)
	{
		if (!is_moving)
			return;
		is_moving = false;

		if (explode_time > 0.0f)
			explode_time -= dt;
		explode = explode_time > 0.0f;

        for (int e = 0; e < nb_piece; ++e)
		{
			if (flag[e] & FLAG_EXPLODE)// && (explosion_flag[e]&EXPLODE_SDL_SurfaceONLY)!=EXPLODE_SDL_SurfaceONLY)		// This piece is exploding
			{
                for (int i = 0; i < 3; ++i)
				{
					if (i == 1 && explosion_flag[e]&EXPLODE_FALL)
						axe[i][e].move_speed-=g;
					axe[i][e].pos += axe[i][e].move_speed * dt;
					axe[i][e].angle += axe[i][e].rot_speed * dt;
					is_moving = true;
				}
			}
			else
			{
                for (int i = 0; i < 3; ++i)
				{
					if (!axe[i][e].is_moving)
						continue;
					axe[i][e].is_moving = false;
					float a = axe[i][e].move_distance;
					if (!Yuni::Math::Zero(a))
					{
						axe[i][e].is_moving = true;
						is_moving = true;
						float c = axe[i][e].move_speed*dt;
						axe[i][e].move_distance -= c;
						axe[i][e].pos += c;
						if ((a>0.0f && axe[i][e].move_distance<0.0f) || (a<0.0f && axe[i][e].move_distance>0.0f))
						{
							axe[i][e].pos+=axe[i][e].move_distance;
							axe[i][e].move_distance=0.0f;
						}
					}

					while (axe[i][e].angle>180.0f)
						axe[i][e].angle-=360.0f;		// Maintient l'angle dans les limites
					while (axe[i][e].angle<-180.0f)
						axe[i][e].angle+=360.0f;

					a = axe[i][e].rot_angle;
					if ((!Yuni::Math::Zero(axe[i][e].rot_speed) || !Yuni::Math::Zero(axe[i][e].rot_accel)) && ((!Yuni::Math::Zero(a) && axe[i][e].rot_limit) || !axe[i][e].rot_limit))
					{
						axe[i][e].is_moving = true;
						is_moving = true;

						float b=axe[i][e].rot_speed;
						if (b<-7200.0f)
							b=axe[i][e].rot_speed=-7200.0f;
						else if (b>7200.0f)
							b=axe[i][e].rot_speed=7200.0f;

						axe[i][e].rot_speed += axe[i][e].rot_accel * dt;
						if (axe[i][e].rot_speed_limit)
						{
							if ((b <= axe[i][e].rot_target_speed && axe[i][e].rot_speed >= axe[i][e].rot_target_speed)
								|| (b >= axe[i][e].rot_target_speed && axe[i][e].rot_speed <= axe[i][e].rot_target_speed))
							{
								axe[i][e].rot_accel = 0.0f;
								axe[i][e].rot_speed = axe[i][e].rot_target_speed;
								axe[i][e].rot_speed_limit = false;
							}
						}
						float c = axe[i][e].rot_speed * dt;
						axe[i][e].angle += c;
						if (axe[i][e].rot_limit)
						{
							axe[i][e].rot_angle-=c;
							if ((a>=0.0f && axe[i][e].rot_angle<=0.0f) || (a<=0.0f && axe[i][e].rot_angle>=0.0f))
							{
								axe[i][e].angle+=axe[i][e].rot_angle;
								axe[i][e].rot_angle=0.0f;
								axe[i][e].rot_speed=0.0f;
								axe[i][e].rot_accel=0.0f;
							}
						}
					}
				}
			}
		}
	}

	void ANIMATION::animate( float &t, Vector3D &R, Vector3D& T)
	{
		if (type & ROTATION)
		{
			if (type & ROTATION_PERIODIC)
			{
				float coef;
				if( type & ROTATION_COSINE )
					coef = 0.5f + 0.5f * cosf( t * angle_w );
				else {
					coef = t * angle_w;
					int i = (int) coef;
					coef = coef - i;
					coef = (i&1) ? (1.0f - coef) : coef;
				}
				R = coef * angle_0 + (1.0f - coef) * angle_1;
			}
			else
				R = t * angle_0;
		}
		if (type & TRANSLATION)
		{
			if (type & TRANSLATION_PERIODIC)
			{
				float coef;
				if (type & TRANSLATION_COSINE)
					coef = 0.5f + 0.5f * cosf( t * translate_w );
				else
				{
					coef = t * translate_w;
					int i = (int) coef;
					coef = coef - i;
					coef = (i&1) ? (1.0f - coef) : coef;
				}
				T = coef * translate_0 + (1.0f - coef) * translate_1;
			}
			else
				T = t * translate_0;
		}
	}

	void MESH::init()
	{
		selprim = -1;

		fixed_textures = false;

		animation_data = NULL;

		compute_min_max = true;

		last_nb_idx = 0;
		last_dir.x = last_dir.y = last_dir.z = 0.0f;

		type = MESH_TYPE_TRIANGLES;

		line_on = NULL;
		emitter_point = false;
		emitter = false;
		t_line = NULL;
		line_v_idx[0] = NULL;
		line_v_idx[1] = NULL;
		nb_line = 0;
		face_reverse = NULL;
		shadow_index = NULL;
		tcoord = NULL;
		pos_from_parent = Vector3D(0.0f, 0.0f, 0.0f);
		nb_vtx = 0;
		nb_prim = 0;
		name.clear();
		next = child = NULL;
		points = NULL;
		p_index = NULL;
		l_index = NULL;
		t_index = NULL;
		nb_p_index = 0;
		nb_l_index = 0;
		nb_t_index = 0;
		F_N = NULL;
		N = NULL;
		nb_index = NULL;
		script_index = -1;
		gltex.clear();
		gl_dlist.clear();
	}

	void MESH::destroy()
	{
		animation_data = NULL;
		DELETE_ARRAY(line_on);
		DELETE_ARRAY(t_line);
		DELETE_ARRAY(line_v_idx[0]);
		DELETE_ARRAY(line_v_idx[1]);
		DELETE_ARRAY(shadow_index);
		DELETE_ARRAY(tcoord);
		for(unsigned int i = 0; i < gltex.size() ; ++i)
			if (gltex.size() > i)
				gfx->destroy_texture(gltex[i]);
		gltex.clear();
		for (unsigned int i = 0; i < gl_dlist.size(); i++)
			if (gl_dlist[i])
				glDeleteLists(gl_dlist[i], 1);
		gl_dlist.clear();
		DELETE_ARRAY(nb_index);
		DELETE_ARRAY(face_reverse);
		DELETE_ARRAY(F_N);
		DELETE_ARRAY(N);
		DELETE_ARRAY(points);
		DELETE_ARRAY(p_index);
		DELETE_ARRAY(l_index);
		DELETE_ARRAY(t_index);
		name.clear();
		next = NULL;
		child = NULL;
		init();
	}

	void MESH::load_texture_id(int id)
	{
		if (id < 0 || id >= 10)
			return;
		if (id < (int)tex_cache_name.size() && !tex_cache_name[id].empty() && TA3D::Paths::ExtractFileExt(tex_cache_name[id]) == ".tex")
		{
			if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
				gfx->set_texture_format(GL_COMPRESSED_RGBA_ARB);
			else
				gfx->set_texture_format(GL_RGBA8);

			SDL_Surface *bmp = LoadTex( TA3D::Paths::Caches + tex_cache_name[id] );
			GLuint texid = gfx->make_texture(bmp, FILTER_TRILINEAR, true);
			gltex[id] = texid;

			if (bmp)
			{
				gfx->save_texture_to_cache( TA3D::Paths::Files::ReplaceExtension(tex_cache_name[id],".bin"), texid, bmp->w, bmp->h);
				SDL_FreeSurface( bmp );
			}
			else
				LOG_WARNING(LOG_PREFIX_3DO << "could not load texture : " << tex_cache_name[id]);

			tex_cache_name[id].clear();
		}
		else
		{
			if (id < (int)tex_cache_name.size() && !tex_cache_name[id].empty() && gfx->is_texture_in_cache(tex_cache_name[id]))
			{
				gltex[id] = gfx->load_texture_from_cache(tex_cache_name[id]);
				tex_cache_name[id].clear();
			}
		}
	}

	void MESH::check_textures()
	{
		if (!tex_cache_name.empty())
		{
			for (unsigned int i = 0 ; i < tex_cache_name.size() ; ++i)
				load_texture_id(i);
			tex_cache_name.clear();
		}
		if (child)
			child->check_textures();
		if (next)
			next->check_textures();
	}

	bool MESH::compute_emitter()
	{
		emitter = ((nb_t_index == 0 || nb_vtx == 0) && child == NULL && next == NULL);
		if (child)
			emitter |= child->compute_emitter();
		if (next)
			emitter |= next->compute_emitter();
		return emitter;
	}


	bool MESH::compute_emitter_point(int &obj_idx)
	{
		emitter_point |= ( script_index == obj_idx);
		emitter |= emitter_point;
		if (child)
			emitter |= child->compute_emitter_point(obj_idx);
		if (next)
			emitter |= next->compute_emitter_point(obj_idx);
		return emitter;
	}


	void MESH::Identify(ScriptData *script)			// Identifie les pièces utilisées par le script
	{
		script_index = -1;				// Pièce non utilisée / Unused piece
		if (!name.empty())
			script_index = script->identify(name);
		if (next)
			next->Identify(script);
		if (child)
			child->Identify(script);
	}


	void MESH::compute_center(Vector3D *center,Vector3D dec, int *coef)		// Calcule les coordonnées du centre de l'objet, objets liés compris
	{
		for (int i = 0; i < nb_vtx; ++i)
		{
			++(*coef);
			center->x += points[i].x + dec.x + pos_from_parent.x;
			center->y += points[i].y + dec.y + pos_from_parent.y;
			center->z += points[i].z + dec.z + pos_from_parent.z;
		}
		if (next)
			next->compute_center(center, dec, coef);
		if (child)
			child->compute_center(center, dec + pos_from_parent, coef);
	}


	float MESH::compute_size_sq(Vector3D center)		// Carré de la taille(on fera une racine après)
	{
		float size = 0.0f;
		for (int i = 0; i < nb_vtx; ++i)
		{
			float dist = (points[i] - center).sq();
			if(size < dist)
				size = dist;
		}
		if (next)
		{
			float size_next=next->compute_size_sq(center);
			if(size<size_next)
				size=size_next;
		}
		if (child)
		{
			float size_child = child->compute_size_sq(center);
			if(size < size_child)
				size = size_child;
		}
		return size;
	}


	float MESH::compute_top(float top, Vector3D dec)
	{
		for(int i = 0;i < nb_vtx; ++i)
			top = Math::Max(top, points[i].y + dec.y + pos_from_parent.y);
		if (next)
			top = next->compute_top(top, dec);
		if (child)
			top = child->compute_top(top, dec + pos_from_parent );
		return top;
	}


	float MESH::compute_bottom(float bottom, Vector3D dec)
	{
		for (int i = 0; i < nb_vtx; ++i)
			bottom = Math::Min(bottom, points[i].y + dec.y + pos_from_parent.y);
		if (next)
			bottom = next->compute_bottom(bottom, dec);
		if (child)
			bottom = child->compute_bottom(bottom, dec + pos_from_parent);
		return bottom;
	}

	bool MESH::has_animation_data()
	{
		if (animation_data)
			return true;
		if (next)
			return next->has_animation_data();
		if (child)
			return child->has_animation_data();
		return false;
	}

	uint16 MESH::set_obj_id(uint16 id)
	{
		nb_sub_obj = id;
		if (next)
			id = next->set_obj_id(id);
		obj_id = id++;
		if (child)
			id = child->set_obj_id(id);
		nb_sub_obj = id - nb_sub_obj;
		return id;
	}




	int MESH::random_pos(ANIMATION_DATA *data_s, int id, Vector3D* vec)
	{
		if (id == obj_id)
		{
			if (nb_t_index > 2 && (data_s == NULL || script_index < 0 || !(data_s->flag[script_index] & FLAG_HIDE)) )
			{
				int rnd_idx = (Math::RandomTable() % (nb_t_index / 3)) * 3;
				float a = (Math::RandomTable() & 0xFF) / 255.0f;
				float b = (1.0f - a) * (Math::RandomTable() & 0xFF) / 255.0f;
				float c = 1.0f - a - b;
				vec->x = a * points[ t_index[rnd_idx]].x + b * points[t_index[rnd_idx + 1]].x + c * points[t_index[rnd_idx + 2]].x;
				vec->y = a * points[ t_index[rnd_idx]].y + b * points[t_index[rnd_idx + 1]].y + c * points[t_index[rnd_idx + 2]].y;
				vec->z = a * points[ t_index[rnd_idx]].z + b * points[t_index[rnd_idx + 1]].z + c * points[t_index[rnd_idx + 2]].z;
				if (data_s && script_index >= 0)
					*vec = data_s->pos[script_index] + *vec * data_s->matrix[script_index];
			}
			else
				return 0;
			return (data_s && script_index >= 0) ? 2 : 1;
		}
		if (id > obj_id)
		{
			if (child != NULL)
			{
				int r = child->random_pos( data_s, id, vec );
				if (r)
				{
					if (r == 1)
						*vec = *vec + pos_from_parent;
					return r;
				}
				return 0;
			}
			return 0;
		}
		if (next != NULL)
			return next->random_pos(data_s, id, vec);
		return 0;
	}




	void MESH::compute_coord(ANIMATION_DATA* data_s, Vector3D *pos, bool c_part, int p_tex, Vector3D *target,
							 Vector3D* upos, Matrix* M, float size, Vector3D* center, bool reverse,
							 MESH* src, ANIMATION_DATA* src_data)
	{
		Vector3D opos = *pos;
		Matrix OM;
		if (M)
			OM = *M;
		if (script_index >= 0 && data_s)
		{
			if (M)
			{
				Vector3D ipos;
				ipos.x = data_s->axe[0][script_index].pos;
				ipos.y = data_s->axe[1][script_index].pos;
				ipos.z = data_s->axe[2][script_index].pos;
				*pos = *pos + (pos_from_parent + ipos) * (*M);
				//                *M = RotateZ(data_s->axe[2][script_index].angle * DEG2RAD)
				//                    * RotateY(data_s->axe[1][script_index].angle * DEG2RAD)
				//                    * RotateX(data_s->axe[0][script_index].angle * DEG2RAD)
				//                    * (*M);
				*M = RotateZYX( data_s->axe[2][script_index].angle * DEG2RAD,
								data_s->axe[1][script_index].angle * DEG2RAD,
								data_s->axe[0][script_index].angle * DEG2RAD)
					* (*M);
				data_s->matrix[script_index] = *M;
				if (nb_l_index == 2)
				{
					data_s->dir[script_index] = (points[l_index[1]] - points[l_index[0]]) * (*M);
					data_s->dir[script_index].unit();
				}
				else
					data_s->dir[script_index].reset();
				if (child)
					data_s->pos[script_index] = *pos + child->pos_from_parent * (*M);
				else
					data_s->pos[script_index] = *pos;
			}
		}
		else if (M)
			*pos = *pos + pos_from_parent * (*M);
		else
			*pos = *pos + pos_from_parent;

		if (c_part && emitter_point ) // Emit a  particle
		{
			Vector3D Dir;
			Vector3D t_mod;
			float life = 1.0f;
			byte nb = (Math::RandomTable() % 60) + 1;
			ParticlesSystem* system = NULL;
			for (int i = 0; i < nb; ++i)
			{
				bool random_vector = true;
				if (src != NULL)
				{
					for (int base_n = Math::RandomTable(), n = 0; random_vector && n < src->nb_sub_obj; ++n)
						random_vector = !src->random_pos( src_data, (base_n + n) % src->nb_sub_obj, &t_mod );
				}
				if (random_vector)
				{
					t_mod.x = (((int)(Math::RandomTable()%2001))-1000)*0.001f;
					t_mod.y = (((int)(Math::RandomTable()%2001))-1000)*0.001f;
					t_mod.z = (((int)(Math::RandomTable()%2001))-1000)*0.001f;
					t_mod.unit();
					t_mod = (Math::RandomTable() % 1001) * 0.001f * size * t_mod;
					if (center)
						t_mod=t_mod+(*center);
				}
				float speed=1.718281828f;			// expf(1.0f) - 1.0f because of speed law: S(t) = So * expf( -t / tref ) and a lifetime of 1 sec
				if (reverse)
				{
					Dir = *pos - (t_mod + *target);
					Dir.x += upos->x;
					Dir.y += upos->y;
					Dir.z += upos->z;
					system = particle_engine.emit_part_fast( system, t_mod+*target,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
				}
				else
				{
					Dir = t_mod + *target - *pos;
					Dir.x -= upos->x;
					Dir.y -= upos->y;
					Dir.z -= upos->z;
					system = particle_engine.emit_part_fast( system, *upos+*pos,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
				}
			}
		}
		if (child)
			child->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
		*pos = opos;
		if (M)
			*M = OM;
		if (next)
			next->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
	}




	bool MESH::draw_shadow(Vector3D Dir, float t, ANIMATION_DATA *data_s, bool alset, bool exploding_parts)
	{
		bool explodes = script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
		bool hide = false;
		Vector3D ODir = Dir;
		glPushMatrix();
		if (!(explodes && !exploding_parts))
		{
			glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);

			if (script_index >= 0 && data_s)
			{
				if (!explodes ^ exploding_parts)
				{
					glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
					glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
					glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
					glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
					Dir = Dir * RotateXYZ(-data_s->axe[0][script_index].angle * DEG2RAD,
										  -data_s->axe[1][script_index].angle * DEG2RAD,
										  -data_s->axe[2][script_index].angle * DEG2RAD);
				}
				hide = data_s->flag[script_index] & FLAG_HIDE;
			}
			else
			{
				if (animation_data)
				{
					Vector3D R,T;
					animation_data->animate(t, R, T);
					glTranslatef(T.x, T.y, T.z);
					glRotatef(R.x, 1.0f, 0.0f, 0.0f);
					glRotatef(R.y, 0.0f, 1.0f, 0.0f);
					glRotatef(R.z, 0.0f, 0.0f, 1.0f);
					Dir = Dir * RotateXYZ(-R.x * DEG2RAD, -R.y * DEG2RAD, -R.z * DEG2RAD);
				}
			}
			hide |= explodes ^ exploding_parts;
			if (nb_t_index > 0 && !hide)
			{
				if (!alset)
				{
					glDisable(GL_LIGHTING);
					glDisable(GL_TEXTURE_2D);
					glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
					glDisableClientState(GL_NORMAL_ARRAY);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					alset=true;
				}
				/*-----------------Code de calcul du cone d'ombre-------------------------*/
				if (shadow_index == NULL)
					shadow_index = new GLushort[nb_t_index * 12];
				uint16 nb_idx = 0;

				if (t_line == NULL) // Repère les arêtes
				{
					t_line = new short[nb_t_index];
					line_v_idx[0] = new short[nb_t_index];
					line_v_idx[1] = new short[nb_t_index];
					face_reverse = new byte[ nb_t_index ];
					nb_line=0;
					for (short i = 0; i < nb_t_index; i += 3)
					{
						for (byte a = 0; a < 3; ++a)
						{
							short idx = -1;
							face_reverse[i + a] = 0;
							for (short e = 0; e < nb_line; ++e)
							{
								if (line_v_idx[0][e] == t_index[i + a] && line_v_idx[1][e] == t_index[i + ((a + 1) % 3)])
								{
									idx = e;
									break;
								}
								else if (line_v_idx[0][e]==t_index[i+((a+1)%3)] && line_v_idx[1][e]==t_index[i+a]) {
									idx=e;
									face_reverse[ i + a ] = 2;
									break;
								}
							}
							if (idx == -1)
							{
								line_v_idx[0][nb_line] = t_index[i + a];
								line_v_idx[1][nb_line] = t_index[i + ((a + 1) % 3)];
								idx = nb_line;
								++nb_line;
							}
							t_line[i + a] = idx;
						}
					}
					line_on = new byte[nb_line];
				}

				if (Dir != last_dir) // Don't need to compute things twice
				{
					::memset((byte*)line_on, 0, nb_line);
					uint16 e = 0;
					for (int i = 0; i < nb_t_index; i += 3)
					{
						if ((F_N[e++] % Dir) >= 0.0f)
							continue;	// Use face normal
						line_on[t_line[i]]   = ((line_on[t_line[i]]   ^ 1) & 1) | face_reverse[i];
						line_on[t_line[i+1]] = ((line_on[t_line[i+1]] ^ 1) & 1) | face_reverse[i+1];
						line_on[t_line[i+2]] = ((line_on[t_line[i+2]] ^ 1) & 1) | face_reverse[i+2];
					}
					for (short i = 0; i < nb_line; ++i)
					{
						if (!(line_on[i] & 1))
							continue;
						points[line_v_idx[0][i]+nb_vtx]=points[line_v_idx[0][i]]+Dir;		// Projection
						points[line_v_idx[1][i]+nb_vtx]=points[line_v_idx[1][i]]+Dir;

						if (line_on[i] & 2)
						{
							shadow_index[nb_idx++] = line_v_idx[1][i];
							shadow_index[nb_idx++] = line_v_idx[0][i];
							shadow_index[nb_idx++] = line_v_idx[0][i]+nb_vtx;
							shadow_index[nb_idx++] = line_v_idx[1][i]+nb_vtx;
						}
						else
						{
							shadow_index[nb_idx++] = line_v_idx[0][i];
							shadow_index[nb_idx++] = line_v_idx[1][i];
							shadow_index[nb_idx++] = line_v_idx[1][i] + nb_vtx;
							shadow_index[nb_idx++] = line_v_idx[0][i] + nb_vtx;
						}
					}
					last_nb_idx = nb_idx;
					last_dir = Dir;
				}
				else
					nb_idx = last_nb_idx;
				if (nb_idx)
				{
					glVertexPointer(3, GL_FLOAT, 0, points);
					glDrawRangeElements(GL_QUADS, 0, (nb_vtx<<1)-1, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout
				}
			}
			if (child && !(explodes && !exploding_parts))
			{
				glPushMatrix();
				alset=child->draw_shadow(Dir,t,data_s,alset,exploding_parts & !explodes);
				glPopMatrix();
			}
		}
		if (next)
		{
			glPopMatrix();
			glPushMatrix();
			alset=next->draw_shadow(ODir,t,data_s,alset,exploding_parts);
			glPopMatrix();
		}
		else
			glPopMatrix();
		return alset;
	}




	bool MESH::draw_shadow_basic(Vector3D Dir,float t,ANIMATION_DATA *data_s,bool alset,bool exploding_parts)
	{
		bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
		bool hide=false;
		Vector3D ODir=Dir;
		glPushMatrix();
		if (!(explodes && !exploding_parts))
		{

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
				//            Dir=((Dir*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD))*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD))*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
				Dir = Dir*RotateXYZ(-data_s->axe[0][script_index].angle*DEG2RAD, -data_s->axe[1][script_index].angle*DEG2RAD, -data_s->axe[2][script_index].angle*DEG2RAD);
				hide=data_s->flag[script_index]&FLAG_HIDE;
			}
			else
				if (animation_data )
				{
					Vector3D R,T;
					animation_data->animate( t, R, T );
					glTranslatef( T.x, T.y, T.z );
					glRotatef( R.x, 1.0f, 0.0f, 0.0f );
					glRotatef( R.y, 0.0f, 1.0f, 0.0f );
					glRotatef( R.z, 0.0f, 0.0f, 1.0f );
					//                Dir=((Dir*RotateX(-R.x*DEG2RAD))*RotateY(-R.y*DEG2RAD))*RotateZ(-R.z*DEG2RAD);
					Dir = Dir*RotateXYZ(-R.x*DEG2RAD, -R.y*DEG2RAD, -R.z*DEG2RAD);
				}
			hide |= explodes ^ exploding_parts;
			if (nb_t_index>0 && !hide)
			{
				if (!alset)
				{
					glDisable(GL_LIGHTING);
					glDisable(GL_TEXTURE_2D);
					glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
					glDisableClientState(GL_NORMAL_ARRAY);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					alset=true;
				}
				/*-----------------Code de calcul du cone d'ombre-------------------------*/
				if (shadow_index == NULL)
					shadow_index = new GLushort[nb_t_index * 12];
				uint16 nb_idx = 0;

				if (t_line == NULL) // Repère les arêtes
				{
					t_line = new short[nb_t_index];
					line_v_idx[0] = new short[nb_t_index];
					line_v_idx[1] = new short[nb_t_index];
					face_reverse = new byte[nb_t_index];
					nb_line = 0;
					for (short i = 0; i < nb_t_index; i += 3)
						for (byte a = 0; a < 3; ++a)
						{
							short idx=-1;
							face_reverse[ i + a ] = 0;
							for (short e = 0;e < nb_line; ++e)
								if (line_v_idx[0][e] == t_index[i+a] && line_v_idx[1][e] == t_index[i + ((a + 1) % 3)])
								{
									idx=e;
									break;
								}
								else
									if (line_v_idx[0][e] == t_index[i + ((a + 1) % 3)] && line_v_idx[1][e] == t_index[i+a])
									{
										idx=e;
										face_reverse[ i + a ] = 2;
										break;
									}
							if (idx == -1)
							{
								line_v_idx[0][nb_line]=t_index[i+a];
								line_v_idx[1][nb_line]=t_index[i+((a+1)%3)];
								idx = nb_line;
								++nb_line;
							}
							t_line[i+a]=idx;
						}
					line_on = new byte[nb_line];
				}

				if (Dir != last_dir) // Don't need to compute things twice
				{
					::memset((byte*)line_on, 0, nb_line);

					uint16 e = 0;
					for (int i = 0; i < nb_t_index; i += 3)
					{
						if ((F_N[e++] % Dir ) >= 0.0f)
							continue;	// Use face normal
						line_on[t_line[i]] = ((line_on[t_line[i]] ^ 1) & 1) | face_reverse[i];
						line_on[t_line[i+1]] = ((line_on[t_line[i+1]] ^ 1) & 1) | face_reverse[i+1];
						line_on[t_line[i+2]] = ((line_on[t_line[i+2]] ^ 1) & 1) | face_reverse[i+2];
					}
					for (short i = 0; i < nb_line; ++i)
					{
						if (!(line_on[i]&1))
							continue;
						points[line_v_idx[0][i] + nb_vtx] = points[line_v_idx[0][i]] + Dir; // Projection calculations
						points[line_v_idx[1][i] + nb_vtx] = points[line_v_idx[1][i]] + Dir;

						if (line_on[i] & 2)
						{
							shadow_index[nb_idx++] = line_v_idx[1][i];
							shadow_index[nb_idx++] = line_v_idx[0][i];
							shadow_index[nb_idx++] = line_v_idx[0][i] + nb_vtx;
							shadow_index[nb_idx++] = line_v_idx[1][i] + nb_vtx;
						}
						else
						{
							shadow_index[nb_idx++] = line_v_idx[0][i];
							shadow_index[nb_idx++] = line_v_idx[1][i];
							shadow_index[nb_idx++] = line_v_idx[1][i] + nb_vtx;
							shadow_index[nb_idx++] = line_v_idx[0][i] + nb_vtx;
						}
					}
					last_nb_idx = nb_idx;
					last_dir = Dir;
				}
				else
					nb_idx = last_nb_idx;
				glVertexPointer( 3, GL_FLOAT, 0, points);
				glFrontFace(GL_CW);						// 1ère passe
				glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
				glDrawRangeElements(GL_QUADS, 0, (nb_vtx<<1)-1, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout

				glFrontFace(GL_CCW);  						// 2nd passe
				glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
				glDrawRangeElements(GL_QUADS, 0, (nb_vtx<<1)-1, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout
			}

			if (child && !(explodes && !exploding_parts))
			{
				glPushMatrix();
				alset=child->draw_shadow_basic(Dir,t,data_s,alset, exploding_parts & !explodes);
				glPopMatrix();
			}
		}
		if (next)
		{
			glPopMatrix();
			glPushMatrix();
			alset=next->draw_shadow_basic(ODir,t,data_s,alset, exploding_parts);
			glPopMatrix();
		}
		else
			glPopMatrix();
		return alset;
	}

	int MESH::hit(Vector3D Pos,Vector3D Dir,ANIMATION_DATA *data_s,Vector3D *I,Matrix M)
	{
		Matrix OM = M;
		Matrix AM = Scale(1.0f);
		Matrix M_Dir = M;
		bool hide = false;
		Vector3D ODir = Dir;
		Vector3D OPos = Pos;
		bool is_hit = false;
		int hit_idx = -2;

		Vector3D T = pos_from_parent;
		Vector3D MP;
		if (!(script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE)))	 // We can't select that
		{
			if (script_index>=0 && data_s)
			{
				T.x += data_s->axe[0][script_index].pos;
				T.y += data_s->axe[1][script_index].pos;
				T.z += data_s->axe[2][script_index].pos;
				Matrix l_M = RotateXYZ(-data_s->axe[0][script_index].angle*DEG2RAD, -data_s->axe[1][script_index].angle*DEG2RAD, -data_s->axe[2][script_index].angle*DEG2RAD);
				M_Dir = M * l_M;
				M = l_M;

				AM = RotateZYX(data_s->axe[2][script_index].angle*DEG2RAD, data_s->axe[1][script_index].angle*DEG2RAD, data_s->axe[0][script_index].angle*DEG2RAD);
				hide=data_s->flag[script_index]&FLAG_HIDE;
			}
			else
				M = Scale(1.0f);
			Pos = (Pos - T) * M;

			if ((nb_t_index>0 || selprim >= 0 ) && !hide)
			{
				Vector3D A;
				Vector3D B;
				Vector3D C;
				Dir = Dir * M_Dir;
				Dir.unit();
				//-----------------Code de calcul d'intersection--------------------------
				for (int i = 0; i < nb_t_index; i += 3)
				{
					A = points[t_index[i]];
					B = points[t_index[i + 1]];
					C = points[t_index[i + 2]];
					Vector3D AB = B - A;
					Vector3D AC = C - A;
					Vector3D N  = AB * AC;
					if (Yuni::Math::Zero(N % Dir))
						continue;
					float dist = -((Pos - A) % N) / (N % Dir);
					if (dist < 0.0f)
						continue;
					Vector3D P_p = Pos + dist * Dir;

					//					if (is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
					if (is_hit && (MP - P_p) % Dir < 0.0f)
						continue;

					float a;
					float b;
					float c;		// Coefficients pour que P soit le barycentre de A,B,C
					Vector3D AP = P_p - A;
					float pre_cal = AB.x * AC.y - AB.y * AC.x;
					if (!Yuni::Math::Zero(AC.y) && !Yuni::Math::Zero(pre_cal))
					{
						b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
						a = (AP.y - b * AB.y) / AC.y;
					}
					else
					{
						if (!Yuni::Math::Zero(AB.x) && !Yuni::Math::Zero(pre_cal))
						{
							a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
							b = (AP.x - a * AC.x) / AB.x;
						}
						else
						{
							pre_cal = AB.x * AC.z - AB.z * AC.x;
							if (!Yuni::Math::Zero(AC.z) && !Yuni::Math::Zero(pre_cal))
							{
								b=(AP.x*AC.z-AP.z*AC.x)/pre_cal;
								a=(AP.z-b*AB.z)/AC.z;
							}
							else
							{
								pre_cal=-pre_cal;
								if (!Yuni::Math::Zero(AB.z) && !Yuni::Math::Zero(pre_cal))
								{
									a =(AP.x*AB.z-AP.z*AB.x)/pre_cal;
									b=(AP.z-a*AC.z)/AB.z;
								}
								else
								{
									pre_cal = AB.y*AC.x-AB.x*AC.y;
									if (!Yuni::Math::Zero(AC.x) && !Yuni::Math::Zero(pre_cal))
									{
										b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
										a=(AP.x-b*AB.x)/AC.x;
									}
									else
									{
										if (!Yuni::Math::Zero(AB.y) && !Yuni::Math::Zero(pre_cal))
										{
											a=(AP.x*AB.y-AP.y*AB.x)/pre_cal;
											b=(AP.y-a*AC.y)/AB.y;
										}
										else
											continue;		// Saute le point s'il n'est pas positionnable
									}
								}
							}
						}
					}
					c = 1.0f - a - b;
					if (a < 0.0f || b < 0.0f || c < 0.0f)
						continue; // Le point n'appartient pas au triangle
					MP = P_p;
					is_hit = true;
					hit_idx = script_index;
				}

				if (selprim >= 0)
				{
					for (int i = 0 ; i < 2 ; ++i) // Selection primitive ( used to allow selecting naval factories easily )
					{
						A = points[sel[i]];
						B = points[sel[i+1]];
						C = points[sel[3]];
						Vector3D AB = B - A;
						Vector3D AC = C - A;
						Vector3D N  = AB * AC;
						if (Yuni::Math::Zero(N % Dir))
							continue;
						float dist = -((Pos - A) % N) / (N % Dir);
						if (dist < 0.0f)
							continue;
						Vector3D P_p=Pos+dist*Dir;

						//						if (is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
						if (is_hit && (MP - P_p) % Dir < 0.0f)
							continue;

						float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
						Vector3D AP = P_p - A;
						float pre_cal = AB.x * AC.y - AB.y * AC.x;
						if (!Yuni::Math::Zero(AC.y) && !Yuni::Math::Zero(pre_cal))
						{
							b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
							a = (AP.y - b * AB.y) / AC.y;
						}
						else
						{
							if (!Yuni::Math::Zero(AB.x) && !Yuni::Math::Zero(pre_cal))
							{
								a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
								b = (AP.x - a * AC.x) / AB.x;
							}
							else
							{
								pre_cal = AB.x * AC.z - AB.z * AC.x;
								if (!Yuni::Math::Zero(AC.z) && !Yuni::Math::Zero(pre_cal))
								{
									b = (AP.x * AC.z - AP.z * AC.x) / pre_cal;
									a = (AP.z - b * AB.z) / AC.z;
								}
								else
								{
									pre_cal = -pre_cal;
									if (!Yuni::Math::Zero(AB.z) && !Yuni::Math::Zero(pre_cal))
									{
										a = (AP.x * AB.z - AP.z * AB.x) / pre_cal;
										b = (AP.z - a * AC.z) / AB.z;
									}
									else
									{
										pre_cal = AB.y*AC.x-AB.x*AC.y;
										if (!Yuni::Math::Zero(AC.x) && !Yuni::Math::Zero(pre_cal))
										{
											b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
											a=(AP.x-b*AB.x)/AC.x;
										}
										else
										{
											if (!Yuni::Math::Zero(AB.y) && !Yuni::Math::Zero(pre_cal))
											{
												a = (AP.x * AB.y - AP.y * AB.x) / pre_cal;
												b = (AP.y - a * AC.y) / AB.y;
											}
											else
												continue;		// Saute le point s'il n'est pas positionnable
										}
									}
								}
							}
						}
						c = 1.0f - a - b;
						if (a<0.0f || b<0.0f || c<0.0f)
							continue;		// Le point n'appartient pas au triangle
						MP = P_p;
						is_hit = true;
						hit_idx = script_index;
					}
				}
			}

			if (child)
			{
				Vector3D MP2;
				int nhit = child->hit(Pos, ODir, data_s, &MP2, M_Dir);
				if (nhit >= -1 && !is_hit)
				{
					MP = MP2;
					hit_idx = nhit;
				}
				else
				{
					if (nhit >= -1 && is_hit)
					{
						if ((MP2 -MP) % Dir < 0.0f)
						{
							MP = MP2;
							hit_idx = nhit;
						}
					}
				}
				is_hit |= nhit;
			}
			if (is_hit)
				MP = (MP * AM) + T;
		}
		if (next)
		{
			Vector3D MP2;
			int nhit = next->hit(OPos, ODir, data_s, &MP2, OM);
			Dir = ODir * OM;
			if (nhit >= -1 && !is_hit)
			{
				MP = MP2;
				hit_idx = nhit;
			}
			else
			{
				if (nhit >= -1 && is_hit)
					if ((MP2 - MP) % Dir < 0.0f)
					{
						MP = MP2;
						hit_idx = nhit;
					}
			}
			is_hit |= nhit;
		}
		if (is_hit)
			*I = MP;
		return hit_idx;
	}



	// hit_fast is a faster version of hit but less precise, designed for use in weapon code
	bool MESH::hit_fast(Vector3D Pos,Vector3D Dir,ANIMATION_DATA *data_s,Vector3D *I)
	{
		bool hide = false;
		Vector3D ODir = Dir;
		Vector3D OPos = Pos;
		Matrix AM;
		bool is_hit = false;


		Vector3D T = pos_from_parent;
		Vector3D MP;
		if (!(script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE)))
		{
			if (script_index >= 0 && data_s)
			{
				T.x += data_s->axe[0][script_index].pos;
				T.y += data_s->axe[1][script_index].pos;
				T.z += data_s->axe[2][script_index].pos;
				Matrix l_M = RotateXYZ(-data_s->axe[0][script_index].angle * DEG2RAD, -data_s->axe[1][script_index].angle * DEG2RAD, -data_s->axe[2][script_index].angle * DEG2RAD);
				Dir = Dir * l_M;
				Pos = (Pos - T) * l_M;
				AM = RotateZYX(data_s->axe[2][script_index].angle * DEG2RAD, data_s->axe[1][script_index].angle * DEG2RAD, data_s->axe[0][script_index].angle * DEG2RAD);
				hide = data_s->flag[script_index]&FLAG_HIDE;
			}
			else
				AM = Scale(1.0f);
			if (nb_t_index > 0 && nb_vtx > 0 && !hide)
			{
				if (compute_min_max ) // Required pre-calculations
				{
					compute_min_max = false;
					min_x = max_x = points[0].x;
					min_y = max_y = points[0].y;
					min_z = max_z = points[0].z;
					for (short i = 1; i < nb_vtx ; ++i)
					{
						min_x = Math::Min(min_x, points[i].x);
						max_x = Math::Max(max_x, points[i].x);
						min_y = Math::Min(min_y, points[i].y);
						max_y = Math::Max(max_y, points[i].y);
						min_z = Math::Min(min_z, points[i].z);
						max_z = Math::Max(max_z, points[i].z);
					}
					min_x -= 0.1f;
					max_x += 0.1f;
					min_y -= 0.1f;
					max_y += 0.1f;
					min_z -= 0.1f;
					max_z += 0.1f;
				}

				// Collision detector using boxes
				if (Pos.x >= min_x && Pos.x <= max_x
					&& Pos.y >= min_y && Pos.y <= max_y
					&& Pos.z >= min_z && Pos.z <= max_z ) // The ray starts from inside
				{
					MP = Pos;
					is_hit = true;
				}
				else
				{
					if (!Yuni::Math::Zero(Dir.x)) // 2 x planes
					{
						MP = Pos + ( (min_x - Pos.x) / Dir.x) * Dir;
						if (MP.y >= min_y && MP.y <= max_y && MP.z >= min_z && MP.z <= max_z )
							is_hit = true;
						else
						{
							MP = Pos + ( (max_x - Pos.x) / Dir.x) * Dir;
							if (MP.y >= min_y && MP.y <= max_y && MP.z >= min_z && MP.z <= max_z )
								is_hit = true;
						}
					}
					if (!is_hit && !Yuni::Math::Zero(Dir.y))// 2 y planes
					{
						MP = Pos + ( (min_y - Pos.y) / Dir.y) * Dir;
						if (MP.x >= min_x && MP.x <= max_x && MP.z >= min_z && MP.z <= max_z )
							is_hit = true;
						else
						{
							MP = Pos + ( (max_y - Pos.y) / Dir.y) * Dir;
							if (MP.x >= min_x && MP.x <= max_x && MP.z >= min_z && MP.z <= max_z )
								is_hit = true;
						}
					}
					if (!is_hit && !Yuni::Math::Zero(Dir.z))// 2 z planes
					{
						MP = Pos + ( (min_z - Pos.z) / Dir.z) * Dir;
						if (MP.y >= min_y && MP.y <= max_y && MP.x >= min_x && MP.x <= max_x )
							is_hit = true;
						else
						{
							MP = Pos + ( (max_z - Pos.z) / Dir.z) * Dir;
							if (MP.y >= min_y && MP.y <= max_y && MP.x >= min_x && MP.x <= max_x )
								is_hit = true;
						}
					}
				}
			}
			if (child && !is_hit)
			{
				Vector3D MP2;
				bool nhit = child->hit_fast(Pos,Dir,data_s,&MP2);
				if (nhit)
				{
					if (!is_hit || (MP2 - MP) % Dir < 0.0f)
						MP = MP2;
					is_hit = true;
				}
			}
			if (is_hit)
				MP = (MP * AM) + T;
		}
		if (next && !is_hit)
		{
			Vector3D MP2;
			bool nhit = next->hit_fast( OPos, ODir, data_s, &MP2);
			if (nhit)
			{
				if (!is_hit || (MP2 - MP) % ODir < 0.0f)
					MP = MP2;
				is_hit = true;
			}
		}
		if (is_hit)
			*I = MP;
		return is_hit;
	}






	float MESH::print_struct(float Y, float X, TA3D::Font *fnt)
	{
		gfx->print(fnt, X, Y, 0.0f,      0xFFFFFFFF, String::Format("%s [%d]", name.c_str(),script_index));
		gfx->print(fnt, 320.0f, Y, 0.0f, 0xFFFFFFFF, String::Format("(v:%d",   nb_vtx));
		gfx->print(fnt, 368.0f, Y, 0.0f, 0xFFFFFFFF, String::Format(",p:%d",   nb_prim));
		gfx->print(fnt, 416.0f, Y, 0.0f, 0xFFFFFFFF, String::Format(",t:%d",   nb_t_index));
		gfx->print(fnt, 464.0f, Y, 0.0f, 0xFFFFFFFF, String::Format(",l:%d",   nb_l_index));
		gfx->print(fnt, 512.0f, Y, 0.0f, 0xFFFFFFFF, String::Format(",p:%d)",  nb_p_index));
		float nwY = Y + 8.0f;
		if (child)
			nwY = child->print_struct(nwY, X + 8.0f, fnt);
		if (next)
			nwY = next->print_struct(nwY, X, fnt);
		return nwY;
	}




	MODEL* MODEL_MANAGER::get_model(const String& name)
	{
		if (name.empty())
			return NULL;

		const String l = String::ToLower(name);
		int e = model_hashtable.find("objects3d\\" + l + ".3do") - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find("objects3d\\" + l + ".3dm") - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find("objects3d\\" + l + ".s3o") - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find(l) - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find(l + ".3do") - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find(l + ".3dm") - 1;
		if (e >= 0)
			return model[e];

		e = model_hashtable.find(l + ".s3o") - 1;
		if (e >= 0)
			return model[e];
		return NULL;
	}


	MODEL_MANAGER::~MODEL_MANAGER()
	{
		for (unsigned int i = 0 ; i < model.size() ; ++i)
			delete model[i];
		model.clear();
		model_hashtable.emptyHashTable();
	}

	void MODEL_MANAGER::init()
	{
		model.clear();
		name.clear();
	}


	void MODEL_MANAGER::destroy()
	{
		for (unsigned int i = 0 ; i < model.size(); ++i)
			delete model[i];
		model.clear();
		name.clear();
		model_hashtable.emptyHashTable();
		model_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
		init();
	}


	void MODEL_MANAGER::compute_ids()
	{
		for (unsigned int i = 0; i < model.size(); ++i)
			model[i]->id = i;
	}


	void MODEL_MANAGER::create_from_2d(SDL_Surface *bmp,float w,float h,float max_h,const String& filename)
	{
		name.push_back(filename);
		model_hashtable.insert(String::ToLower(filename), nb_models);

		MODEL *pModel = new MODEL;
		pModel->create_from_2d(bmp,w,h,max_h);
		model.push_back(pModel);
	}


	int MODEL_MANAGER::load_all(void (*progress)(float percent,const String &msg))
	{
		const String loading3DModelsText = I18N::Translate("Loading 3D Models");

		String::List file_list;
		VFS::Instance()->getFilelist(ta3dSideData.model_dir + "*.3dm", file_list);
		VFS::Instance()->getFilelist(ta3dSideData.model_dir + "*.s3o", file_list);
		VFS::Instance()->getFilelist(ta3dSideData.model_dir + "*.3do", file_list);

		if (!file_list.empty())
		{
			int n = 0;
			int progressIncrement = 0;
			const String::List::const_iterator end = file_list.end();
			for (String::List::const_iterator e = file_list.begin(); e != end; ++e)
			{
				LOG_DEBUG("[Mesh] Loading `" << *e << "`");
				if (++progressIncrement == 25 && progress != NULL)
				{
					// Update the progress bar
					progress((100.0f + n * 50.0f / (file_list.size() + 1)) / 7.0f, loading3DModelsText);
					// Reset the increment
					progressIncrement = 0;
				}
				++n;
				name.push_back(*e);

				if (get_model(e->substr(0, e->size() - 4)) == NULL) 	// Vérifie si le modèle n'est pas déjà chargé
				{
					MODEL *pModel = MODEL::load(*e);
					if (pModel)
					{
						model.push_back(pModel);
						model_hashtable.insert(String::ToLower(*e), model.size());
					}
					else
					{
						name.pop_back();
						LOG_ERROR("could not load model " << *e);
					}
				}
				else
					name.pop_back();
			}
		}
		return 0;
	}


	void MODEL::init()
	{
		nb_obj = 0;
		mesh = NULL;
		center = Vector3D(0.0f, 0.0f, 0.0f);
		size = 0.0f;
		size2 = 0.0f;
		dlist = 0;
		animated = false;
		top = bottom = 0.0f;
		from_2d = false;
	}



	void MODEL::destroy()
	{
		if (dlist)
			glDeleteLists(dlist, 1);
		init();
	}


	void MODEL::postLoadComputations()
	{
		if (!mesh)   return;
		nb_obj = mesh->set_obj_id( 0 );

		animated = mesh->has_animation_data();

		Vector3D O;
		int coef(0);
		center = Vector3D(0.0f, 0.0f, 0.0f);
		mesh->compute_center(&center,O,&coef);
		center = (1.0f / coef) * center;
		size = 2.0f * mesh->compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
		size2 = sqrtf(0.5f * size);
		mesh->compute_emitter();
		compute_topbottom();
	}

	void MODEL::create_from_2d(SDL_Surface *bmp,float w,float h,float max_h)
	{
		MESH_3DO *pMesh = new MESH_3DO;
		pMesh->create_from_2d(bmp,w,h,max_h);
		mesh = pMesh;

		postLoadComputations();
		from_2d = true;
	}

	void MODEL::draw(float t,ANIMATION_DATA *data_s,bool sel,bool notex,bool c_part,int p_tex,Vector3D *target,Vector3D *upos,Matrix *M,float Size,Vector3D* Center,bool reverse,int side,bool chg_col,MESH *src,ANIMATION_DATA *src_data)
	{
		if (!mesh)  return;
		gfx->enable_model_shading();

		sel &= !gfx->getShadowMapMode();        // Don't render selection primitive while in shadow map mode !! otherwise it would cast a shadow :/

		if (notex)
			glDisable(GL_TEXTURE_2D);
		if (chg_col && notex)
		{
			byte var = abs(255 - (((int)(t * 256) & 0xFF)<<1));
			glColor3ub(0,var,0);
		}

		if (data_s == NULL && animated)
			mesh->draw(t, NULL, sel, false, notex, side, chg_col);
		else
		{
			if (data_s == NULL && dlist == 0 && !sel && !notex && !chg_col)
			{
				check_textures();
				dlist = glGenLists (1);
				glNewList (dlist, GL_COMPILE_AND_EXECUTE);
				mesh->draw_nodl();
				glEndList();
			}
			else
			{
				if (data_s == NULL && !sel && !notex && !chg_col)
					glCallList( dlist );
				else
				{
					mesh->draw(t, data_s, sel, false, notex, side, chg_col);
					if( data_s && data_s->explode )
						mesh->draw(t, data_s, sel, false, notex, side, chg_col, true);
				}
			}
		}

		gfx->disable_model_shading();

		if (c_part)                 // It is safe to do this even in shadow map mode because this is done only once in a while
		{
			Vector3D pos;
			mesh->compute_coord(data_s,&pos,c_part,p_tex,target,upos,M,Size,Center,reverse,src,src_data);
		}
		if (chg_col && notex)
			glColor3ub(0xFF,0xFF,0xFF);
	}


	void MODEL::draw_shadow(const Vector3D& Dir, float t, ANIMATION_DATA* data_s)
	{
		if (!mesh)  return;
		glDisable(GL_TEXTURE_2D);
		mesh->draw_shadow(Dir,t,data_s,false);
		if( data_s && data_s->explode )
			mesh->draw_shadow(Dir,t,data_s,false,true);
	}

	void MODEL::draw_shadow_basic(const Vector3D& Dir, float t, ANIMATION_DATA *data_s)
	{
		if (!mesh) return;
		glDisable(GL_TEXTURE_2D);
		mesh->draw_shadow_basic(Dir, t, data_s, false);
		if (data_s && data_s->explode)
			mesh->draw_shadow_basic(Dir,t,data_s,false,true);
	}


	void MODEL::compute_coord(ANIMATION_DATA* data_s, Matrix* M)
	{
		if (!mesh)  return;
		Vector3D pos;
		mesh->compute_coord(data_s, &pos, false, 0, NULL, NULL, M);
	}


	void MODEL::compute_topbottom()
	{
		Vector3D O;
		O.x = O.y = O.z = 0.0f;
		top = mesh->compute_top(-99999.0f, O);
		bottom = mesh->compute_bottom(99999.0f, O);
	}

	MODEL *MODEL::load(const String &filename)
	{
		String ext = Paths::ExtractFileExt(filename).toLower();
		if (ext == ".3do")
			return MESH_3DO::load(filename);
		if (ext == ".3dm")
			return MESH_3DM::load(filename);
		if (ext == ".s3o")
			return MESH_S3O::load(filename);
		LOG_WARNING(LOG_PREFIX_MODEL << "model could not be loaded : file extension unknown");
		return NULL;
	}
}
