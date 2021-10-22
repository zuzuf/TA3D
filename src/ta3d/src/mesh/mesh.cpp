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

namespace TA3D
{
	std::vector<MeshTypeManager::MeshLoader> *MeshTypeManager::lMeshLoader = NULL;
    QStringList *MeshTypeManager::lMeshExtension = NULL;

	void MeshTypeManager::registerMeshLoader(MeshLoader loader)
	{
		if (!lMeshLoader)
			lMeshLoader = new std::vector<MeshTypeManager::MeshLoader>;
		MeshTypeManager::lMeshLoader->push_back(loader);
	}

	void MeshTypeManager::registerMeshExtension(const QString &ext)
	{
		if (!lMeshExtension)
            lMeshExtension = new QStringList;
		MeshTypeManager::lMeshExtension->push_back(ext);
	}

	ModelManager	model_manager;



	void AnimationData::init()
	{
		is_moving = false;
		nb_piece = 0;
		data.clear();
		explode = false;
		explode_time = 0.0f;
	}

	void AnimationData::destroy()
	{
		data.clear();
		init();
	}


	void AnimationData::load(const int nb)
	{
		destroy();		// Au cas oÃ¹
		nb_piece = nb;
		data.resize(nb);
		for (DataVector::iterator i = data.begin() ; i != data.end() ; ++i)
		{
			i->flag = 0;
			i->explosion_flag = 0;
			i->pos.reset();
			i->tpos.reset();
			i->dir.reset();
			i->matrix = Scale(1.0f);
			for (int e = 0; e < 3; ++e)
			{
				i->axe[e].move_speed = 0.0f;
				i->axe[e].move_distance = 0.0f;
				i->axe[e].pos = 0.0f;
				i->axe[e].rot_angle = 0.0f;
				i->axe[e].rot_speed = 0.0f;
				i->axe[e].rot_accel = 0.0f;
				i->axe[e].angle = 0.0f;
				i->axe[e].rot_limit = true;
				i->axe[e].rot_speed_limit = false;
				i->axe[e].rot_target_speed = 0.0f;
				i->axe[e].is_moving = false;
			}
		}
	}



	void AnimationData::move(const float dt, const float g)
	{
		if (!is_moving)
			return;
		is_moving = false;

		if (explode_time > 0.0f)
			explode_time -= dt;
		explode = explode_time > 0.0f;

		for (DataVector::iterator e = data.begin() ; e != data.end() ; ++e)
		{
			if ((e->flag & FLAG_EXPLODE) && !(e->explosion_flag & EXPLODE_BITMAPONLY))		// This piece is exploding
			{
                for (int i = 0; i < 3; ++i)
				{
					Axe &axis = e->axe[i];
					if (i == 1 && (e->explosion_flag & EXPLODE_FALL))
						axis.move_speed -= g;
					axis.pos += axis.move_speed * dt;
					axis.angle += axis.rot_speed * dt;
					is_moving = true;
				}
			}
			else
			{
                for (int i = 0; i < 3; ++i)
				{
					Axe &axis = e->axe[i];
					if (!axis.is_moving)
						continue;
					axis.is_moving = false;
					float a = axis.move_distance;
					if (!Math::Zero(a))
					{
						axis.is_moving = true;
						is_moving = true;
						const float c = axis.move_speed * dt;
						axis.move_distance -= c;
						axis.pos += c;
						if ((a > 0.0f && axis.move_distance < 0.0f)
							|| (a < 0.0f && axis.move_distance > 0.0f))
						{
							axis.pos += axis.move_distance;
							axis.move_distance = 0.0f;
						}
					}

					while (axis.angle > 180.0f)
						axis.angle -= 360.0f;		// Maintient l'angle dans les limites
					while (axis.angle < -180.0f)
						axis.angle += 360.0f;

					a = axis.rot_angle;
					if ((!Math::Zero(axis.rot_speed) || !Math::Zero(axis.rot_accel)) && ((!Math::Zero(a) && axis.rot_limit) || !axis.rot_limit))
					{
						axis.is_moving = true;
						is_moving = true;

						float b = axis.rot_speed;
						if (b < -7200.0f)
							b = axis.rot_speed = -7200.0f;
						else if (b > 7200.0f)
							b = axis.rot_speed = 7200.0f;

						axis.rot_speed += axis.rot_accel * dt;
						if (axis.rot_speed_limit)
						{
							if ((b <= axis.rot_target_speed && axis.rot_speed >= axis.rot_target_speed)
								|| (b >= axis.rot_target_speed && axis.rot_speed <= axis.rot_target_speed))
							{
								axis.rot_accel = 0.0f;
								axis.rot_speed = axis.rot_target_speed;
								axis.rot_speed_limit = false;
							}
						}
						const float c = axis.rot_speed * dt;
						axis.angle += c;
						if (axis.rot_limit)
						{
							axis.rot_angle -= c;
							if ((a >= 0.0f && axis.rot_angle <= 0.0f) || (a <= 0.0f && axis.rot_angle >= 0.0f))
							{
								axis.angle += axis.rot_angle;
								axis.rot_angle = 0.0f;
								axis.rot_speed = 0.0f;
								axis.rot_accel = 0.0f;
							}
						}
					}
				}
			}
		}
	}

	void Animation::animate(const float t, Vector3D &R, Vector3D& T) const
	{
		if (type & ROTATION)
		{
			if (type & ROTATION_PERIODIC)
			{
				float coef;
				if (type & ROTATION_COSINE)
					coef = 0.5f + 0.5f * cosf(t * angle_w);
				else
				{
					coef = t * angle_w;
					const int i = (int) coef;
					coef = coef - float(i);
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
					const int i = (int) coef;
					coef = coef - float(i);
					coef = (i&1) ? (1.0f - coef) : coef;
				}
				T = coef * translate_0 + (1.0f - coef) * translate_1;
			}
			else
				T = t * translate_0;
		}
	}

	void Mesh::init()
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
	}

	void Mesh::destroy()
	{
		animation_data = NULL;
		DELETE_ARRAY(line_on);
		DELETE_ARRAY(t_line);
		DELETE_ARRAY(line_v_idx[0]);
		DELETE_ARRAY(line_v_idx[1]);
		DELETE_ARRAY(shadow_index);
		DELETE_ARRAY(tcoord);
		gltex.clear();
		DELETE_ARRAY(nb_index);
		DELETE_ARRAY(face_reverse);
		DELETE_ARRAY(F_N);
		DELETE_ARRAY(N);
		DELETE_ARRAY(points);
		DELETE_ARRAY(p_index);
		DELETE_ARRAY(l_index);
		DELETE_ARRAY(t_index);
		name.clear();
		if (next)
			delete next;
		next = NULL;
		if (child)
			delete child;
		child = NULL;
		init();
	}

	void Mesh::load_texture_id(int id)
	{
		if (id < 0 || id >= 10)
			return;
        if (id < (int)tex_cache_name.size() && !tex_cache_name[id].isEmpty() && TA3D::Paths::ExtractFileExt(tex_cache_name[id]) == ".tex")
		{
			if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
				gfx->set_texture_format(GL_COMPRESSED_RGBA_ARB);
			else
				gfx->set_texture_format(gfx->defaultTextureFormat_RGBA());

            const QString &filename = TA3D::Paths::Caches + tex_cache_name[id];
            const QImage &bmp = LoadTex( filename );

            if (!bmp.isNull())
			{
                GfxTexture::Ptr texid = gfx->make_texture(bmp, FILTER_TRILINEAR, true);
				gltex[id] = texid;
                gfx->save_texture_to_cache( TA3D::Paths::Files::ReplaceExtension(tex_cache_name[id],".bin"), texid, bmp.width(), bmp.height(), true);
			}
			else
			{
                gltex[id] = nullptr;
				LOG_WARNING(LOG_PREFIX_3DO << "could not load texture : " << filename);
			}

			tex_cache_name[id].clear();
		}
		else
		{
            if (id < (int)tex_cache_name.size() && !tex_cache_name[id].isEmpty() && gfx->is_texture_in_cache(tex_cache_name[id]))
			{
				gltex[id] = gfx->load_texture_from_cache(tex_cache_name[id]);
				tex_cache_name[id].clear();
			}
		}
	}

	void Mesh::check_textures()
	{
        if (!tex_cache_name.isEmpty())
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

	bool Mesh::compute_emitter()
	{
		emitter = ((nb_t_index == 0 || nb_vtx == 0) && child == NULL && next == NULL);
		if (child)
			emitter |= child->compute_emitter();
		if (next)
			emitter |= next->compute_emitter();
		return emitter;
	}


	bool Mesh::compute_emitter_point(const int obj_idx)
	{
		emitter_point |= ( script_index == obj_idx);
		emitter |= emitter_point;
		if (child)
			emitter |= child->compute_emitter_point(obj_idx);
		if (next)
			emitter |= next->compute_emitter_point(obj_idx);
		return emitter;
	}


	void Mesh::Identify(ScriptData *script)			// Identifie les pièces utilisées par le script
	{
		script_index = -1;				// Pièce non utilisée / Unused piece
        if (!name.isEmpty())
			script_index = script->identify(name);
		if (next)
			next->Identify(script);
		if (child)
			child->Identify(script);
	}


	void Mesh::compute_center(Vector3D *center, const Vector3D &dec, int *coef) const		// Calcule les coordonnÃ©es du centre de l'objet, objets liÃ©s compris
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


	float Mesh::compute_size_sq(const Vector3D &center) const		// CarrÃ© de la taille(on fera une racine aprÃ¨s)
	{
		float size = 0.0f;
		for (int i = 0; i < nb_vtx; ++i)
		{
			const float dist = (points[i] - center).sq();
			if(size < dist)
				size = dist;
		}
		if (next)
		{
			const float size_next = next->compute_size_sq(center);
			if(size < size_next)
				size = size_next;
		}
		if (child)
		{
			const float size_child = child->compute_size_sq(center);
			if(size < size_child)
				size = size_child;
		}
		return size;
	}


	float Mesh::compute_top(float top, const Vector3D &dec) const
	{
		for(int i = 0;i < nb_vtx; ++i)
			top = Math::Max(top, points[i].y + dec.y + pos_from_parent.y);
		if (next)
			top = next->compute_top(top, dec);
		if (child)
			top = child->compute_top(top, dec + pos_from_parent );
		return top;
	}


	float Mesh::compute_bottom(float bottom, const Vector3D &dec) const
	{
		for (int i = 0; i < nb_vtx; ++i)
			bottom = Math::Min(bottom, points[i].y + dec.y + pos_from_parent.y);
		if (next)
			bottom = next->compute_bottom(bottom, dec);
		if (child)
			bottom = child->compute_bottom(bottom, dec + pos_from_parent);
		return bottom;
	}

	bool Mesh::has_animation_data() const
	{
		if (animation_data)
			return true;
		if (next)
			return next->has_animation_data();
		if (child)
			return child->has_animation_data();
		return false;
	}

	sint32 Mesh::set_obj_id(sint32 id)
	{
		nb_sub_obj = id;
		if (next)
			id = next->set_obj_id(id);
		obj_id = id++;
		if (child)
			id = child->set_obj_id(id);
		nb_sub_obj = uint32(id - nb_sub_obj);
		return id;
	}




	int Mesh::random_pos(const AnimationData *data_s, const int id, Vector3D* vec) const
	{
		if (id == obj_id)
		{
			if (nb_t_index > 2 && (data_s == NULL || script_index < 0 || !(data_s->data[script_index].flag & FLAG_HIDE)) )
			{
				const int rnd_idx = (Math::RandomTable() % (nb_t_index / 3)) * 3;
				const float a = float(Math::RandomTable() & 0xFF) / 255.0f;
				const float b = (1.0f - a) * float(Math::RandomTable() & 0xFF) / 255.0f;
				const float c = 1.0f - a - b;
				*vec = a * points[ t_index[rnd_idx]] + b * points[t_index[rnd_idx + 1]] + c * points[t_index[rnd_idx + 2]];
				if (data_s && script_index >= 0)
					*vec = data_s->data[script_index].pos + *vec * data_s->data[script_index].matrix;
			}
			else
				return 0;
			return (data_s && script_index >= 0) ? 2 : 1;
		}
		if (id > obj_id)
		{
			if (child != NULL)
			{
				const int r = child->random_pos( data_s, id, vec );
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




	void Mesh::compute_coord(AnimationData* data_s, Vector3D *pos, const bool c_part, const int p_tex, const Vector3D *target,
							 Vector3D *upos, Matrix *M, const float size, const Vector3D *center, bool reverse,
							 const Mesh *src, const AnimationData *src_data) const
	{
		const Vector3D opos = *pos;
		const Matrix OM = M ? *M : Matrix();
		if (script_index >= 0 && data_s)
		{
			if (M)
			{
				Vector3D ipos;
				AnimationData::Data &data = data_s->data[script_index];
				ipos.x = data.axe[0].pos;
				ipos.y = data.axe[1].pos;
				ipos.z = data.axe[2].pos;
				*pos = *pos + (pos_from_parent + ipos) * (*M);
				*M = RotateZYX( data.axe[2].angle * DEG2RAD,
								data.axe[1].angle * DEG2RAD,
								data.axe[0].angle * DEG2RAD)
					* (*M);
				data.matrix = *M;
				if (nb_l_index == 2)
				{
					data.dir = (points[l_index[1]] - points[l_index[0]]) * (*M);
					data.dir.unit();
				}
				else
					data.dir.reset();
				if (nb_l_index == 2 && nb_prim == 1)
					data.pos = *pos + points[l_index[0]] * (*M);
				else
					data.pos = *pos;
				if (child)
					data.tpos = *pos + child->pos_from_parent * (*M);
				else
					data.tpos = *pos;
			}
		}
		else if (M)
			*pos = *pos + pos_from_parent * (*M);
		else
			*pos = *pos + pos_from_parent;

		if (c_part && emitter_point) // Emit a  particle
		{
			Vector3D Dir;
			Vector3D t_mod;
			const float life = 1.0f;
			const int nb = (Math::RandomTable() % 60) + 1;
			ParticlesSystem* system = NULL;
			for (int i = 0; i < nb; ++i)
			{
				bool random_vector = true;
				if (src != NULL)
				{
					for (uint32 base_n = Math::RandomTable(), n = 0; random_vector && n < src->nb_sub_obj; ++n)
						random_vector = !src->random_pos( src_data, (base_n + n) % src->nb_sub_obj, &t_mod );
				}
				if (random_vector)
				{
					t_mod.x = float(((int)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
					t_mod.y = float(((int)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
					t_mod.z = float(((int)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
					t_mod.unit();
					t_mod = float(Math::RandomTable() % 1001) * 0.001f * size * t_mod;
					if (center)
						t_mod = t_mod + (*center);
				}
				const float speed = 1.718281828f;			// expf(1.0f) - 1.0f because of speed law: S(t) = So * expf( -t / tref ) and a lifetime of 1 sec
				if (reverse)
				{
					Dir = *pos - (t_mod + *target);
					Dir.x += upos->x;
					Dir.y += upos->y;
					Dir.z += upos->z;
					system = particle_engine.emit_part_fast( system, t_mod + *target, Dir, p_tex, i == 0 ? -nb : 1, speed, life, 2.0f, true);
				}
				else
				{
					Dir = t_mod + *target - *pos;
					Dir.x -= upos->x;
					Dir.y -= upos->y;
					Dir.z -= upos->z;
					system = particle_engine.emit_part_fast( system, *upos + *pos, Dir, p_tex, i == 0 ? -nb : 1, speed, life, 2.0f, true);
				}
			}
		}
		if (child)
			child->compute_coord(data_s, pos, c_part, p_tex, target, upos, M, size, center, reverse, src, src_data);
		*pos = opos;
		if (M)
			*M = OM;
		if (next)
			next->compute_coord(data_s, pos, c_part, p_tex, target, upos, M, size, center, reverse, src, src_data);
	}

	int Mesh::hit(Vector3D Pos,Vector3D Dir,AnimationData *data_s,Vector3D *I,Matrix M) const
	{
		const Matrix OM = M;
		Matrix AM = Scale(1.0f);
		Matrix M_Dir = M;
		bool hide = false;
		const Vector3D ODir = Dir;
		const Vector3D OPos = Pos;
		bool is_hit = false;
		int hit_idx = -2;

		Vector3D T = pos_from_parent;
		Vector3D MP;
		if (!(script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_EXPLODE)))	 // We can't select that
		{
			if (script_index>=0 && data_s)
			{
				T.x += data_s->data[script_index].axe[0].pos;
				T.y += data_s->data[script_index].axe[1].pos;
				T.z += data_s->data[script_index].axe[2].pos;
				Matrix l_M = RotateXYZ(-data_s->data[script_index].axe[0].angle*DEG2RAD, -data_s->data[script_index].axe[1].angle*DEG2RAD, -data_s->data[script_index].axe[2].angle*DEG2RAD);
				M_Dir = M * l_M;
				M = l_M;

				AM = RotateZYX(data_s->data[script_index].axe[2].angle*DEG2RAD, data_s->data[script_index].axe[1].angle*DEG2RAD, data_s->data[script_index].axe[0].angle*DEG2RAD);
				hide=data_s->data[script_index].flag&FLAG_HIDE;
			}
			else
				M = Scale(1.0f);
			Pos = (Pos - T) * M;

			if ((nb_t_index>0 || selprim >= 0 ) && !hide)
			{
				Dir = Dir * M_Dir;
				Dir.unit();
				//-----------------Code de calcul d'intersection--------------------------
				for (int i = 0; i < nb_t_index; i += 3)
				{
					const Vector3D &A = points[t_index[i]];
					const Vector3D &B = points[t_index[i + 1]];
					const Vector3D &C = points[t_index[i + 2]];
					const Vector3D AB = B - A;
					const Vector3D AC = C - A;
					const Vector3D N  = AB * AC;
					if (Math::Zero(N % Dir))
						continue;
					const float dist = -((Pos - A) % N) / (N % Dir);
					if (dist < 0.0f)
						continue;
					const Vector3D P_p = Pos + dist * Dir;

					if (is_hit && (MP - P_p) % Dir < 0.0f)
						continue;

					float a;
					float b;
					float c;		// Coefficients pour que P soit le barycentre de A,B,C
					const Vector3D AP = P_p - A;
					float pre_cal = AB.x * AC.y - AB.y * AC.x;
					if (!Math::Zero(AC.y) && !Math::Zero(pre_cal))
					{
						b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
						a = (AP.y - b * AB.y) / AC.y;
					}
					else
					{
						if (!Math::Zero(AB.x) && !Math::Zero(pre_cal))
						{
							a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
							b = (AP.x - a * AC.x) / AB.x;
						}
						else
						{
							pre_cal = AB.x * AC.z - AB.z * AC.x;
							if (!Math::Zero(AC.z) && !Math::Zero(pre_cal))
							{
								b=(AP.x*AC.z-AP.z*AC.x)/pre_cal;
								a=(AP.z-b*AB.z)/AC.z;
							}
							else
							{
								pre_cal=-pre_cal;
								if (!Math::Zero(AB.z) && !Math::Zero(pre_cal))
								{
									a =(AP.x*AB.z-AP.z*AB.x)/pre_cal;
									b=(AP.z-a*AC.z)/AB.z;
								}
								else
								{
									pre_cal = AB.y*AC.x-AB.x*AC.y;
									if (!Math::Zero(AC.x) && !Math::Zero(pre_cal))
									{
										b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
										a=(AP.x-b*AB.x)/AC.x;
									}
									else
									{
										if (!Math::Zero(AB.y) && !Math::Zero(pre_cal))
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
						const Vector3D &A = points[sel[i]];
						const Vector3D &B = points[sel[i+1]];
						const Vector3D &C = points[sel[3]];
						const Vector3D AB = B - A;
						const Vector3D AC = C - A;
						const Vector3D N  = AB * AC;
						if (Math::Zero(N % Dir))
							continue;
						const float dist = -((Pos - A) % N) / (N % Dir);
						if (dist < 0.0f)
							continue;
						const Vector3D P_p = Pos + dist * Dir;

						if (is_hit && (MP - P_p) % Dir < 0.0f)
							continue;

						float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
						const Vector3D AP = P_p - A;
						float pre_cal = AB.x * AC.y - AB.y * AC.x;
						if (!Math::Zero(AC.y) && !Math::Zero(pre_cal))
						{
							b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
							a = (AP.y - b * AB.y) / AC.y;
						}
						else
						{
							if (!Math::Zero(AB.x) && !Math::Zero(pre_cal))
							{
								a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
								b = (AP.x - a * AC.x) / AB.x;
							}
							else
							{
								pre_cal = AB.x * AC.z - AB.z * AC.x;
								if (!Math::Zero(AC.z) && !Math::Zero(pre_cal))
								{
									b = (AP.x * AC.z - AP.z * AC.x) / pre_cal;
									a = (AP.z - b * AB.z) / AC.z;
								}
								else
								{
									pre_cal = -pre_cal;
									if (!Math::Zero(AB.z) && !Math::Zero(pre_cal))
									{
										a = (AP.x * AB.z - AP.z * AB.x) / pre_cal;
										b = (AP.z - a * AC.z) / AB.z;
									}
									else
									{
										pre_cal = AB.y*AC.x-AB.x*AC.y;
										if (!Math::Zero(AC.x) && !Math::Zero(pre_cal))
										{
											b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
											a=(AP.x-b*AB.x)/AC.x;
										}
										else
										{
											if (!Math::Zero(AB.y) && !Math::Zero(pre_cal))
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
				const int nhit = child->hit(Pos, ODir, data_s, &MP2, M_Dir);
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
			const int nhit = next->hit(OPos, ODir, data_s, &MP2, OM);
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
	bool Mesh::hit_fast(Vector3D Pos, Vector3D Dir, AnimationData *data_s, Vector3D *I)
	{
		bool hide = false;
		const Vector3D ODir = Dir;
		const Vector3D OPos = Pos;
		Matrix AM;
		bool is_hit = false;


		Vector3D T = pos_from_parent;
		Vector3D MP;
		if (!(script_index >= 0 && data_s && (data_s->data[script_index].flag & FLAG_EXPLODE)))
		{
			if (script_index >= 0 && data_s)
			{
				T.x += data_s->data[script_index].axe[0].pos;
				T.y += data_s->data[script_index].axe[1].pos;
				T.z += data_s->data[script_index].axe[2].pos;
				Matrix l_M = RotateXYZ(-data_s->data[script_index].axe[0].angle * DEG2RAD, -data_s->data[script_index].axe[1].angle * DEG2RAD, -data_s->data[script_index].axe[2].angle * DEG2RAD);
				Dir = Dir * l_M;
				Pos = (Pos - T) * l_M;
				AM = RotateZYX(data_s->data[script_index].axe[2].angle * DEG2RAD, data_s->data[script_index].axe[1].angle * DEG2RAD, data_s->data[script_index].axe[0].angle * DEG2RAD);
				hide = data_s->data[script_index].flag&FLAG_HIDE;
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
					for (int i = 1; i < nb_vtx ; ++i)
					{
						min_x = Math::Min(min_x, points[i].x);
						max_x = Math::Max(max_x, points[i].x);
						min_y = Math::Min(min_y, points[i].y);
						max_y = Math::Max(max_y, points[i].y);
						min_z = Math::Min(min_z, points[i].z);
						max_z = Math::Max(max_z, points[i].z);
					}
					min_x -= 1.0f;
					max_x += 1.0f;
					min_y -= 1.0f;
					max_y += 1.0f;
					min_z -= 1.0f;
					max_z += 1.0f;
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
					if ((min_x - Pos.x) * Dir.x > 0.0f) // 2 x planes
					{
						const Vector3D IP = Pos + ((min_x - Pos.x) / Dir.x) * Dir;
						if (IP.y >= min_y && IP.y <= max_y && IP.z >= min_z && IP.z <= max_z)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
					if ((max_x - Pos.x) * Dir.x > 0.0f) // 2 x planes
					{
						const Vector3D IP = Pos + ((max_x - Pos.x) / Dir.x) * Dir;
						if (IP.y >= min_y && IP.y <= max_y && IP.z >= min_z && IP.z <= max_z)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
					if ((min_y - Pos.y) * Dir.y > 0.0f)// 2 y planes
					{
						const Vector3D IP = Pos + ((min_y - Pos.y) / Dir.y) * Dir;
						if (IP.x >= min_x && IP.x <= max_x && IP.z >= min_z && IP.z <= max_z)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
					if ((max_y - Pos.y) * Dir.y > 0.0f)// 2 y planes
					{
						const Vector3D IP = Pos + ((max_y - Pos.y) / Dir.y) * Dir;
						if (IP.x >= min_x && IP.x <= max_x && IP.z >= min_z && IP.z <= max_z)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
					if ((min_z - Pos.z) * Dir.z > 0.0f)// 2 z planes
					{
						const Vector3D IP = Pos + ((min_z - Pos.z) / Dir.z) * Dir;
						if (IP.y >= min_y && IP.y <= max_y && IP.x >= min_x && IP.x <= max_x)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
					if ((max_z - Pos.z) * Dir.z > 0.0f)// 2 z planes
					{
						const Vector3D IP = Pos + ((max_z - Pos.z) / Dir.z) * Dir;
						if (IP.y >= min_y && IP.y <= max_y && IP.x >= min_x && IP.x <= max_x)
						{
							if (!is_hit || (IP - MP) % Dir < 0.0f)
								MP = IP;
							is_hit = true;
						}
					}
				}
			}
			if (child)
			{
				Vector3D MP2;
				const bool nhit = child->hit_fast(Pos,Dir,data_s,&MP2);
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
		if (next)
		{
			Vector3D MP2;
			const bool nhit = next->hit_fast( OPos, ODir, data_s, &MP2);
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






	float Mesh::print_struct(float Y, float X, TA3D::Font *fnt)
	{
        fnt->print(X, Y,      0xFFFFFFFF, name + QString(" [%1]").arg(script_index));
        fnt->print(320.0f, Y, 0xFFFFFFFF, QString("(v:%1").arg(nb_vtx));
        fnt->print(368.0f, Y, 0xFFFFFFFF, QString(",p:%1").arg(nb_prim));
        fnt->print(416.0f, Y, 0xFFFFFFFF, QString(",t:%1").arg(nb_t_index));
        fnt->print(464.0f, Y, 0xFFFFFFFF, QString(",l:%1").arg(nb_l_index));
        fnt->print(512.0f, Y, 0xFFFFFFFF, QString(",p:%1").arg(nb_p_index));
		float nwY = Y + 8.0f;
		if (child)
			nwY = child->print_struct(nwY, X + 8.0f, fnt);
		if (next)
			nwY = next->print_struct(nwY, X, fnt);
		return nwY;
	}


	void Mesh::hideFlares()
	{
        const QString &lower_name = name.toLower();
        if ((lower_name.contains("flare")
            || lower_name.contains("flash")
            || lower_name.contains("fire")) && !child)
			nb_t_index = 0;
		if (child)
			child->hideFlares();
		if (next)
			next->hideFlares();
	}


	Model* ModelManager::get_model(const QString& name)
	{
        if (name.isEmpty())
			return NULL;

        const QString &l = name.toLower();

		HashMap<int>::Dense::iterator e = model_hashtable.find(l);
		if (e != model_hashtable.end())
			return model[*e];

		if (MeshTypeManager::lMeshExtension)
            for(QStringList::iterator ext = MeshTypeManager::lMeshExtension->begin() ; ext != MeshTypeManager::lMeshExtension->end() ; ++ext)
			{
                e = model_hashtable.find("objects3d/" + l + *ext);
				if (e != model_hashtable.end())
					return model[*e];

                e = model_hashtable.find(l + *ext);
				if (e != model_hashtable.end())
					return model[*e];
			}

		return NULL;
	}


	ModelManager::~ModelManager()
	{
		for (unsigned int i = 0 ; i < model.size() ; ++i)
			delete model[i];
		model.clear();
		model_hashtable.clear();
	}

	void ModelManager::init()
	{
		model.clear();
		name.clear();
	}


	void ModelManager::destroy()
	{
		for (unsigned int i = 0 ; i < model.size(); ++i)
			delete model[i];
		model.clear();
		name.clear();
		model_hashtable.clear();
		init();
	}


	void ModelManager::compute_ids()
	{
		for (unsigned int i = 0; i < model.size(); ++i)
			model[i]->id = i;
	}


    void ModelManager::create_from_2d(QImage bmp,float w,float h,float max_h,const QString& filename)
	{
		mInternals.lock();

        model_hashtable[filename.toLower()] = nb_models;
		name.push_back(filename);

		Model *pModel = new Model;
		model.push_back(pModel);
		mInternals.unlock();

		pModel->create_from_2d(bmp,w,h,max_h);
	}

    void MeshTypeManager::getMeshList(QStringList &filelist)
	{
		if (lMeshExtension == NULL)
			return;
        for(const QString &ext : *lMeshExtension)
            VFS::Instance()->getFilelist(ta3dSideData.model_dir + '*' + ext, filelist);
	}

	int ModelManager::load_all(ProgressNotifier *progress)
	{
		const QString loading3DModelsText = I18N::Translate("Loading 3D Models");

        QStringList file_list;
		MeshTypeManager::getMeshList(file_list);

		if (!file_list.empty())
		{
            QStringList final_file_list;
			HashSet<QString>::Dense files;
            for(const QString &it : file_list)
			{
                const QString filename = Substr(it, 0, it.size() - 4).toLower();
                const QString ext = Substr(it, it.size() - 3, 3).toLower();
				if (ext == "3do" || files.contains(filename))
					continue;
				files.insert(filename);
                final_file_list.push_back(it);
			}
            for(const QString &it : file_list)
			{
                const QString filename = Substr(it, 0, it.size() - 4).toLower();
                const QString ext = Substr(it, it.size() - 3, 3).toLower();
				if (ext != "3do" || files.contains(filename))
					continue;
				files.insert(filename);
                final_file_list.push_back(it);
			}

			Mutex mLoad;
            std::atomic<int> n(0);
            std::atomic<int> progressIncrement(0);
            QThread * const root_thread = QThread::currentThread();
            parallel_for<size_t>(0, final_file_list.size(), [&](const size_t e)
			{
                const QString &filename = final_file_list[e];
                LOG_DEBUG("[Mesh] Loading `" << filename << "`");
                ++progressIncrement;
                if (QThread::currentThread() == root_thread)
                    if (progressIncrement >= 25 && progress != NULL)
                    {
                        // Update the progress bar
                        (*progress)((100.0f + float(n) * 50.0f / float(final_file_list.size() + 1)) / 7.0f, loading3DModelsText);
                        // Reset the increment
                        progressIncrement -= 25;
                    }
                ++n;
                mLoad.lock();
                name.push_back(filename);

                if (get_model(Substr(filename, 0, filename.size() - 4)) == NULL) 	// Check if it's not already loaded
                {
                    mLoad.unlock();
                    Model *pModel = MeshTypeManager::load(filename);
                    mLoad.lock();
                    if (pModel)
                    {
                        model_hashtable[ToLower(filename)] = (int)model.size();
                        model.push_back(pModel);
                    }
                    else
                    {
                        name.pop_back();
                        LOG_ERROR("could not load model " << filename);
                    }
                }
                else
                    name.pop_back();
                mLoad.unlock();
            });
		}
		return 0;
	}


	void Model::init()
	{
		nb_obj = 0;
		mesh = NULL;
		center.reset();
		size = 0.0f;
		size2 = 0.0f;
		animated = false;
		top = bottom = 0.0f;
		from_2d = false;
	}



	void Model::destroy()
	{
		if (mesh)
			delete mesh;
		init();
	}


	void Model::postLoadComputations()
	{
		if (!mesh)   return;
		nb_obj = mesh->set_obj_id( 0 );

		animated = mesh->has_animation_data();

		Vector3D O;
		int coef(0);
		center.reset();
		mesh->compute_center(&center, O, &coef);
		center = (1.0f / float(coef)) * center;
		size = 2.0f * mesh->compute_size_sq(center);			// On garde le carrÃ© pour les comparaisons et on prend une marge en multipliant par 2.0f
		size2 = sqrtf(0.5f * size);
		mesh->compute_emitter();
		compute_topbottom();
	}

	void Model::draw(float t,AnimationData *data_s,bool sel,bool notex,
					 bool c_part,int p_tex,const Vector3D *target,Vector3D *upos,
					 Matrix *M,float Size,const Vector3D* Center,bool reverse,int side,
					 bool chg_col,Mesh *src,AnimationData *src_data)
	{
		if (!mesh)  return;
		gfx->enable_model_shading();

		sel &= !gfx->getShadowMapMode();        // Don't render selection primitive while in shadow map mode !! otherwise it would cast a shadow :/

		if (notex)
        {
            gfx->glDisable(GL_TEXTURE_2D);
            CHECK_GL();
        }
        if (chg_col && notex)
		{
			const byte var = (byte)abs(255 - (((int)(t * 256) & 0xFF) << 1));
            glColor3ub(0, var, 0);
            CHECK_GL();
        }

        mesh->draw(t, data_s, sel, false, notex, side, chg_col);
        if (data_s && data_s->explode)
            mesh->draw(t, data_s, sel, false, notex, side, chg_col, true);

		gfx->disable_model_shading();

		if (c_part)                 // It is safe to do this even in shadow map mode because this is done only once in a while
		{
			Vector3D pos;
			mesh->compute_coord(data_s,&pos,c_part,p_tex,target,upos,M,Size,Center,reverse,src,src_data);
		}
		if (chg_col && notex)
        {
			glColor3ub(0xFF,0xFF,0xFF);
            CHECK_GL();
        }
    }


	void Model::compute_coord(AnimationData* data_s, Matrix* M) const
	{
		if (!mesh)  return;
		Vector3D pos;
		mesh->compute_coord(data_s, &pos, false, 0, NULL, NULL, M);
	}


	void Model::compute_topbottom()
	{
		Vector3D O;
		top = mesh->compute_top(-99999.0f, O);
		bottom = mesh->compute_bottom(99999.0f, O);
	}

	Model *MeshTypeManager::load(const QString &filename)
	{
		if (lMeshExtension == NULL)
			return NULL;

		const QString ext = Paths::ExtractFileExt(filename).toLower();
		for(uint32 i = 0 ; i < lMeshExtension->size() ; ++i)
		{
			if (ext == lMeshExtension->at(i))
				return lMeshLoader->at(i)(filename);
		}
		LOG_WARNING(LOG_PREFIX_MODEL << "model could not be loaded : file extension unknown");
		return NULL;
	}

	ModelManager::ModelManager() : nb_models(0), model()
	{
	}
}
