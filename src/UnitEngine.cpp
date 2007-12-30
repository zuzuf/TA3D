/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005   Roland BROCHARD

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

/*----------------------------------------------------------\
|                       UnitEngine.cpp                      |
|    Contains the unit engine, which simulates units and    |
| make them interact with each other.                       |
|                                                           |
\----------------------------------------------------------*/

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"
#include "UnitEngine.h"

using namespace TA3D::EXCEPTION;

INGAME_UNITS units;

float sq( const float &a )	{	return a * a;	}

#define PUT_IN_BUFFER( o, b, p ) 	memcpy( b + p, &o, sizeof( o ) );	p += sizeof( o );
#define PUT_VEC_IN_BUFFER_LOW( v, b, p )	{ sint16 tmp16;\
													tmp16 = (sint16)(v.x * 128.0f); PUT_IN_BUFFER( tmp16, b, p ); \
													tmp16 = (sint16)(v.y * 128.0f); PUT_IN_BUFFER( tmp16, b, p ); \
													tmp16 = (sint16)(v.z * 128.0f); PUT_IN_BUFFER( tmp16, b, p ); }

inline void MAKE_HASH( uint32 &h, int f )
{
	for( int i = 0 ; i < 4 ; i++ )
		h = (h << 5) - h + ((byte*)&f)[ i ];
}

uint32 UNIT::write_sync_data( byte *buf, int buf_pos )
{
	EnterCS();

	uint32 new_hash = 0;
	uint32 check_hash = 0;
	MAKE_HASH( check_hash, (int)Pos.x );
	MAKE_HASH( check_hash, (int)Pos.y );
	MAKE_HASH( check_hash, (int)Pos.z );

	MAKE_HASH( new_hash, (int)(V.x * 16.0f) );
	MAKE_HASH( new_hash, (int)(V.y * 16.0f) );
	MAKE_HASH( new_hash, (int)(V.z * 16.0f) );

	MAKE_HASH( check_hash, (int)Angle.x );
	MAKE_HASH( check_hash, (int)Angle.y );
	MAKE_HASH( check_hash, (int)Angle.z );

	MAKE_HASH( new_hash, (int)(V_Angle.x * 16.0f) );
	MAKE_HASH( new_hash, (int)(V_Angle.y * 16.0f) );
	MAKE_HASH( new_hash, (int)(V_Angle.z * 16.0f) );

	if( sync_hash != new_hash ) {
		buf[ buf_pos++ ] = 1;
		PUT_IN_BUFFER( new_hash, buf, buf_pos );
//		PUT_IN_BUFFER( Pos, buf, buf_pos );

		PUT_VEC_IN_BUFFER_LOW( V, buf, buf_pos );
//		PUT_VEC_IN_BUFFER_LOW( Angle, buf, buf_pos );
		PUT_VEC_IN_BUFFER_LOW( V_Angle, buf, buf_pos );
		}
	else
		buf[ buf_pos++ ] = 0;

	PUT_IN_BUFFER( check_hash, buf, buf_pos );

	sync_hash = new_hash;

	LeaveCS();

	return buf_pos;
}

bool UNIT::is_on_radar( byte &p_mask )
{
	int px = cur_px>>1;
	int py = cur_py>>1;
	if( px >= 0 && py >= 0 && px < units.map->radar_map->w && py < units.map->radar_map->h )
		return ( (units.map->radar_map->line[py][px] & p_mask) && !unit_manager.unit_type[ type_id ].Stealth && (unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTSUB) )
								|| ( (units.map->sonar_map->line[py][px] & p_mask) && !(unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTSUB) );
	return false;
}

	void UNIT::add_mission(int mission_type,VECTOR *target,bool step,int dat,void *pointer,PATH_NODE *path,byte m_flags,int move_data,int patrol_node)
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::add_mission );
#endif
		EnterCS();

		bool def_mode = false;
		if( !unit_manager.unit_type[type_id].BMcode )
			switch( mission_type )
			{
			case MISSION_MOVE:
			case MISSION_PATROL:
			case MISSION_GUARD:
				def_mode = true;
				break;
			};

		if( pointer == this && !def_mode ) {			// A unit cannot target itself
			LeaveCS();
			return;
			}

		if( mission_type == MISSION_MOVE || mission_type == MISSION_PATROL )
			m_flags |= MISSION_FLAG_MOVE;

		if( mission_type == MISSION_BUILD && unit_manager.unit_type[ type_id ].BMcode && unit_manager.unit_type[ type_id ].Builder && target != NULL ) {
			bool removed = false;
			MISSION *cur_mission = mission;
			MISSION *prec = cur_mission;
			if( cur_mission )		cur_mission = cur_mission->next;		// Don't read the first one ( which is being executed )

			while( cur_mission ) {					// Reads the mission list
				float x_space = fabs(cur_mission->target.x - target->x);
				float z_space = fabs(cur_mission->target.z - target->z);
				if( !cur_mission->step && cur_mission->mission == MISSION_BUILD && cur_mission->data >= 0 && cur_mission->data < unit_manager.nb_unit
				&& x_space < (unit_manager.unit_type[ dat ].FootprintX + unit_manager.unit_type[ cur_mission->data ].FootprintX << 2)
				&& z_space < (unit_manager.unit_type[ dat ].FootprintZ + unit_manager.unit_type[ cur_mission->data ].FootprintZ << 2) ) {		// Remove it
					MISSION *tmp = cur_mission;
					cur_mission = cur_mission->next;
					prec->next = cur_mission;
					if(tmp->path)				// Destroy the path if needed
						destroy_path(tmp->path);
					free( tmp );
					removed = true;
					}
				else {
					prec = cur_mission;
					cur_mission = cur_mission->next;
					}
				}
			if( removed ) {
				LeaveCS();
				return;
				}
			}

		MISSION *new_mission = (MISSION*) malloc(sizeof(MISSION));
		new_mission->next = NULL;
		new_mission->mission = mission_type;
		new_mission->step = step;
		new_mission->time = 0.0f;
		new_mission->data = dat;
		new_mission->p = pointer;
		new_mission->path = path;
		new_mission->last_d = 9999999.0f;
		new_mission->flags = m_flags;
		new_mission->move_data = move_data;
		new_mission->node = patrol_node;

		bool inserted = false;

		if( patrol_node == -1 && mission_type == MISSION_PATROL ) {
			MISSION *mission_base = def_mode ? def_mission : mission;
			if( mission_base ) {				// Ajoute l'ordre aux autres
				MISSION *cur = mission_base;
				MISSION *last = NULL;
				patrol_node = 0;
				while( cur != NULL )	{
					if( cur->mission == MISSION_PATROL && patrol_node <= cur->node ) {
						patrol_node = cur->node;
						last = cur;
						}
					cur=cur->next;
					}
				new_mission->node = patrol_node + 1;

				if( last ) {
					new_mission->next = last->next;
					last->next = new_mission;
					inserted = true;
					}
				}
			else
				new_mission->node = 1;
			}

		if(target)
			new_mission->target = *target;

		MISSION *stop = !inserted ? (MISSION*) malloc(sizeof(MISSION)) : NULL;
		if( !inserted ) {
			stop->mission = MISSION_STOP;
			stop->step = true;
			stop->time = 0.0f;
			stop->p = NULL;
			stop->data = 0;
			stop->path = NULL;
			stop->last_d = 9999999.0f;
			stop->flags = m_flags & ~MISSION_FLAG_MOVE;
			stop->move_data = move_data;
			if(step) {
				stop->next=NULL;
				new_mission->next=stop;
				}
			else {
				stop->next=new_mission;
				new_mission=stop;
				}
			}

		if( step && mission && !inserted ) {
			stop->next = def_mode ? def_mission : mission;
			mission = new_mission;
			if( !def_mode )
				start_mission_script(mission->mission);
			}
		else {
			MISSION *mission_base = def_mode ? def_mission : mission;
			if( mission_base && !inserted ) {				// Ajoute l'ordre aux autres
				MISSION *cur = mission_base;
				while(cur->next!=NULL)	cur=cur->next;
				if( ( ( cur->mission == MISSION_MOVE || cur->mission == MISSION_PATROL )		// Don't stop if it's not necessary
					&& ( mission_type == MISSION_MOVE || mission_type == MISSION_PATROL ) )
				|| ( ( cur->mission == MISSION_BUILD || cur->mission == MISSION_BUILD_2 )
					&& mission_type == MISSION_BUILD && !unit_manager.unit_type[type_id].BMcode ) ) {		// Prevent factories from closing when already building a unit
					stop = new_mission->next;
					free( new_mission );
					new_mission = stop;
					new_mission->next = NULL;
					}
				cur->next=new_mission;
				}
			else if( mission_base == NULL ) {
				if( !def_mode ) {
					mission = new_mission;
					start_mission_script(mission->mission);
					}
				else
					def_mission = new_mission;
				}
			}
		LeaveCS();

#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
	}

	void UNIT::set_mission(int mission_type,VECTOR *target,bool step,int dat,bool stopit,void *pointer,PATH_NODE *path,byte m_flags,int move_data)
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::set_mission );
#endif
		EnterCS();

		bool def_mode = false;
		if( !unit_manager.unit_type[type_id].BMcode )
			switch( mission_type )
			{
			case MISSION_MOVE:
			case MISSION_PATROL:
			case MISSION_GUARD:
				def_mode = true;
				break;
			};

		if( pointer == this && !def_mode ) {			// A unit cannot target itself
			LeaveCS();
			return;
			}

		int old_mission=-1;
		if( !def_mode )
			if(mission!=NULL)	old_mission = mission->mission;
		else
			clear_def_mission();

		bool already_running = false;

		if( mission_type == MISSION_MOVE || mission_type == MISSION_PATROL )
			m_flags |= MISSION_FLAG_MOVE;

		switch(old_mission)		// Commandes de fin de mission
		{
		case MISSION_REPAIR:
		case MISSION_RECLAIM:
		case MISSION_BUILD_2:
			launch_script(get_script_index(SCRIPT_stopbuilding));
			launch_script(get_script_index(SCRIPT_stop));
			if( !unit_manager.unit_type[type_id].BMcode ) {		// Delete the unit we were building
				((UNIT*)(mission->p))->built = false;
				((UNIT*)(mission->p))->hp = 0.0f;
				}
			break;
		case MISSION_ATTACK:
			if( mission_type != MISSION_ATTACK && ( !unit_manager.unit_type[type_id].canfly
			|| ( unit_manager.unit_type[type_id].canfly && mission_type != MISSION_MOVE && mission_type != MISSION_PATROL ) ) )
				deactivate();
			else {
				stopit = false;
				already_running = true;
				}
			break;
		case MISSION_MOVE:
		case MISSION_PATROL:
			if( mission_type == MISSION_MOVE || mission_type == MISSION_PATROL
			|| ( unit_manager.unit_type[type_id].canfly && mission_type == MISSION_ATTACK ) ) {
				stopit = false;
				already_running = true;
				}
			break;
		};

		if( !def_mode ) {
			while(mission!=NULL) 	clear_mission();			// Efface les ordres précédents
			last_path_refresh = 10.0f;
			}

		if( def_mode ) {
			def_mission = (MISSION*) malloc(sizeof(MISSION));
			def_mission->next = NULL;
			def_mission->mission = mission_type;
			def_mission->step = step;
			def_mission->time = 0.0f;
			def_mission->data = dat;
			def_mission->p = pointer;
			def_mission->path = path;
			def_mission->last_d = 9999999.0f;
			def_mission->flags = m_flags;
			def_mission->move_data = move_data;
			def_mission->node = 1;
			if(target)
				def_mission->target=*target;

			if(stopit) {
				MISSION *stop = (MISSION*) malloc(sizeof(MISSION));
				stop->next = def_mission;
				stop->mission = MISSION_STOP;
				stop->step = true;
				stop->time = 0.0f;
				stop->p = NULL;
				stop->data = 0;
				stop->path = NULL;
				stop->last_d = 9999999.0f;
				stop->flags = m_flags & ~MISSION_FLAG_MOVE;
				stop->move_data = move_data;
				def_mission = stop;
				}
			}
		else {
			mission = (MISSION*) malloc(sizeof(MISSION));
			mission->next = NULL;
			mission->mission = mission_type;
			mission->step = step;
			mission->time = 0.0f;
			mission->data = dat;
			mission->p = pointer;
			mission->path = path;
			mission->last_d = 9999999.0f;
			mission->flags = m_flags;
			mission->move_data = move_data;
			mission->node = 1;
			if(target)
				mission->target=*target;

			if(stopit) {
				MISSION *stop = (MISSION*) malloc(sizeof(MISSION));
				stop->next = mission;
				stop->mission = MISSION_STOP;
				stop->step = true;
				stop->time = 0.0f;
				stop->p = NULL;
				stop->data = 0;
				stop->path = NULL;
				stop->last_d = 9999999.0f;
				stop->flags = m_flags & ~MISSION_FLAG_MOVE;
				stop->move_data = move_data;
				mission = stop;
				}
			else if( !already_running )
				start_mission_script(mission->mission);
			c_time=0.0f;
			}
		LeaveCS();

#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
	}

	void UNIT::draw(float t, CAMERA *cam,MAP *map,bool height_line)
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::draw );
#endif

		EnterCS();

		visible = false;
		on_radar = false;
		on_mini_radar = false;

		drawn_Pos = Pos;
		drawn_Angle = Angle;

		if( model==NULL || hidden ) {
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			LeaveCS();
			return;		// S'il n'y a pas de modèle associé, on quitte la fonction
			}

		int px=cur_px>>1;
		int py=cur_py>>1;
		if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	{
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			LeaveCS();
			return;	// Unité hors de la carte
			}
		byte player_mask = 1 << players.local_human_id;

		on_radar = on_mini_radar = is_on_radar( player_mask );
		if( map->view[py][px] == 0 || ( map->view[py][px] > 1 && !on_radar ) || ( !on_radar && !(map->sight_map->line[py][px] & player_mask) ) ) {
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			LeaveCS();
			return;	// Unité non visible
			}

		on_radar &= map->view[py][px] > 1;

		VECTOR D;
		D=Pos-cam->Pos;		// Vecteur "viseur unité" partant de la caméra vers l'unité

		float dist=D.Sq();
		if(dist>=16384.0f && (D%cam->Dir)<=0.0f) {
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			LeaveCS();
			return;
			}
		if((D%cam->Dir)>cam->zfar2) {
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			LeaveCS();
			return;		// Si l'objet est hors champ on ne le dessine pas
			}

		visible=true;

		on_radar |= cam->RPos.y>gfx->low_def_limit;

		MATRIX_4x4 M;
		glPushMatrix();
		if( on_radar ) {			// for mega zoom, draw only an icon
			glTranslatef( Pos.x, max(Pos.y,map->sealvl), Pos.z );
			glEnable(GL_TEXTURE_2D);
			int unit_nature = ICON_UNKNOWN;
			float size = (D%cam->Dir) * 12.0f / gfx->height;

			if( unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_KAMIKAZE )
				unit_nature = ICON_KAMIKAZE;
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_COMMANDER ) == CLASS_COMMANDER )
				unit_nature = ICON_COMMANDER;
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_ENERGY ) == CLASS_ENERGY )
				unit_nature = ICON_ENERGY;
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_METAL ) == CLASS_METAL )
				unit_nature = ICON_METAL;
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_TANK ) == CLASS_TANK )
				unit_nature = ICON_TANK;
			else if( unit_manager.unit_type[ type_id ].Builder ) {
				if( !unit_manager.unit_type[ type_id ].BMcode )
					unit_nature = ICON_FACTORY;
				else
					unit_nature = ICON_BUILDER;
				}
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_SHIP ) == CLASS_SHIP )
				unit_nature = ICON_WATERUNIT;
			else if( ( unit_manager.unit_type[ type_id ].TEDclass & CLASS_FORT ) == CLASS_FORT )
				unit_nature = ICON_DEFENSE;
			else if( ( unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTAIR ) && ( unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTSUB ) )
				unit_nature = ICON_LANDUNIT;
			else if( !( unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTAIR ) )
				unit_nature = ICON_AIRUNIT;
			else if( !( unit_manager.unit_type[ type_id ].fastCategory & CATEGORY_NOTSUB ) )
				unit_nature = ICON_SUBUNIT;

			glBindTexture( GL_TEXTURE_2D, units.icons[ unit_nature ] );
			glDisable( GL_CULL_FACE );
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glColor3f(player_color[player_color_map[owner_id]*3],player_color[player_color_map[owner_id]*3+1],player_color[player_color_map[owner_id]*3+2]);
			glTranslatef( model->center.x, model->center.y, model->center.z );
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);		glVertex3f( -size, 0.0f, -size);
				glTexCoord2f(1.0f, 0.0f);		glVertex3f(  size, 0.0f, -size);
				glTexCoord2f(1.0f, 1.0f);		glVertex3f(  size, 0.0f,  size);
				glTexCoord2f(0.0f, 1.0f);		glVertex3f( -size, 0.0f,  size);
			glEnd();
			glEnable( GL_CULL_FACE );
			if(owner_id==players.local_human_id && sel) {
				glDisable( GL_TEXTURE_2D );
				glColor3f(1.0f,1.0f,0.0f);
				glBegin(GL_LINE_STRIP);
					glVertex3f( -size, 0.0f, -size);
					glVertex3f(  size, 0.0f, -size);
					glVertex3f(  size, 0.0f,  size);
					glVertex3f( -size, 0.0f,  size);
					glVertex3f( -size, 0.0f, -size);
				glEnd();
				}
			}
		else {
			glTranslatef( Pos.x, Pos.y, Pos.z );
			glRotatef(Angle.x,1.0f,0.0f,0.0f);
			glRotatef(Angle.z,0.0f,0.0f,1.0f);
			glRotatef(Angle.y,0.0f,1.0f,0.0f);
			float scale=unit_manager.unit_type[type_id].Scale;
			glScalef(scale,scale,scale);

			M=RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité

			VECTOR *target=NULL,*center=NULL;
			POINTF upos;
			bool c_part=false;
			bool reverse=false;
			float size=0.0f;
			OBJECT *src = NULL;
			SCRIPT_DATA *src_data = NULL;

			if(build_percent_left==0.0f && mission!=NULL
			&& port[ INBUILDSTANCE ] != 0 )
				if(c_time>=0.125f) {
					reverse=(mission->mission==MISSION_RECLAIM);
					c_time=0.0f;
					c_part=true;
					upos.x=upos.y=upos.z=0.0f;
					upos=upos+Pos;
					if(mission->p!=NULL
					&& (mission->mission == MISSION_REPAIR || mission->mission == MISSION_BUILD || mission->mission == MISSION_BUILD_2 )
					&& ((UNIT*)mission->p)->flags && ((UNIT*)mission->p)->model!=NULL) {
						size=((UNIT*)mission->p)->model->size2;
						center=&((UNIT*)mission->p)->model->center;
						src = &((UNIT*)mission->p)->model->obj;
						src_data = &((UNIT*)mission->p)->data;
						((UNIT*)mission->p)->compute_model_coord();
						}
					else if( mission->mission == MISSION_RECLAIM || mission->mission == MISSION_REVIVE ) {		// Récupération d'objets
						int feature_type = features.feature[ mission->data ].type;
						if( mission->data >= 0 && feature_type >= 0 && feature_manager.feature[ feature_type ].model ) {
							size = feature_manager.feature[ feature_type ].model->size2;
							center = &feature_manager.feature[ feature_type ].model->center;
							src = &feature_manager.feature[ feature_type ].model->obj;
							src_data = NULL;
							}
						else {
							D.x = D.y = D.z = 0.f;
							center = &D;
							size = 32.0f;
							}
						}
					else
						c_part=false;
					target=&(mission->target);
					}

			if( c_part )					// Get the nanolathing points
				if( !unit_manager.unit_type[ type_id ].emitting_points_computed ) {		// Compute model emitting points if not already done
					unit_manager.unit_type[ type_id ].emitting_points_computed = true;
					int param[] = { -1 };
					int querynanopiece_idx = get_script_index( "QueryNanoPiece" );
					run_script_function( NULL, querynanopiece_idx, 1, param );
					int first = param[0];
					int i = 0;
					do
					{
						model->obj.compute_emitter_point( param[ 0 ] );
						run_script_function( NULL, querynanopiece_idx, 1, param );
						i++;
					}while( first != param[0] && i < 1000 );
					}

			if(build_percent_left==0.0f) {
				model->draw(t,&data,owner_id==players.local_human_id && sel,false,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,true,src,src_data);
				if(height_line && h>1.0f && unit_manager.unit_type[type_id].canfly) {		// For flying units, draw a line that shows how high is the unit
					glPopMatrix();
					glPushMatrix();
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_LIGHTING);
					glColor3f(1.0f,1.0f,0.0f);
					glBegin(GL_LINES);
						for(float y=Pos.y;y>Pos.y-h;y-=10.0f) {
							glVertex3f(Pos.x,y,Pos.z);
							glVertex3f(Pos.x,y-5.0f,Pos.z);
							}
					glEnd();
					}
				}
			else if(build_percent_left<=33.0f) {
				float h = model->top - model->bottom;
				double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h*(33.0f-build_percent_left)*0.033333f};

				glClipPlane(GL_CLIP_PLANE0, eqn);
				glEnable(GL_CLIP_PLANE0);
				model->draw(t,&data,owner_id==players.local_human_id && sel,true,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,true,src,src_data);

				eqn[1]=-eqn[1];	eqn[3]=-eqn[3];
				glClipPlane(GL_CLIP_PLANE0, eqn);
				model->draw(t,&data,owner_id==players.local_human_id && sel,false,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
				glDisable(GL_CLIP_PLANE0);

				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
				model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
				}
			else if(build_percent_left<=66.0f) {
				float h = model->top - model->bottom;
				double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h*(66.0f-build_percent_left)*0.033333f};

				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
				glClipPlane(GL_CLIP_PLANE0, eqn);
				glEnable(GL_CLIP_PLANE0);
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
				model->draw(t,&data,owner_id==players.local_human_id && sel,true,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,true,src,src_data);
				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

				eqn[1]=-eqn[1];	eqn[3]=-eqn[3];
				glClipPlane(GL_CLIP_PLANE0, eqn);
				model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
				glDisable(GL_CLIP_PLANE0);
				}
			else {
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
				model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
				}
			}
		glPopMatrix();
		LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
	}

	void UNIT::draw_shadow(CAMERA *cam,VECTOR Dir,MAP *map)
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::draw_shadow );
#endif
		EnterCS();
		if( on_radar || hidden )	{
			LeaveCS();
			return;
			}

		if( model == NULL )
			Console->AddEntry( "Warning : model is NULL!!" );

		if(!visible) {
			VECTOR S_Pos = drawn_Pos-(h/Dir.y)*Dir;//map->hit(Pos,Dir);
			int px=((int)(S_Pos.x)+map->map_w_d)>>4;
			int py=((int)(S_Pos.z)+map->map_h_d)>>4;
			if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	{
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				LeaveCS();
				return;	// Shadow out of the map
				}
			if(map->view[py][px]!=1) {
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				LeaveCS();
				return;	// Unvisible shadow
				}
			}

		glPushMatrix();
		glTranslatef(drawn_Pos.x,drawn_Pos.y,drawn_Pos.z);
		glRotatef(drawn_Angle.x,1.0f,0.0f,0.0f);
		glRotatef(drawn_Angle.z,0.0f,0.0f,1.0f);
		glRotatef(drawn_Angle.y,0.0f,1.0f,0.0f);
		float scale = unit_manager.unit_type[type_id].Scale;
		glScalef(scale,scale,scale);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

		if( unit_manager.unit_type[ type_id ].canmove || shadow_scale_dir < 0.0f ) {
			VECTOR H = drawn_Pos;
			H.y += 2.0f * model->size2 + 1.0f;
			VECTOR D = map->hit( H, Dir, true, 2000.0f);
			shadow_scale_dir = (D - H).Norm();
			}
		model->draw_shadow(((shadow_scale_dir*Dir*RotateX(-drawn_Angle.x*DEG2RAD))*RotateZ(-drawn_Angle.z*DEG2RAD))*RotateY(-drawn_Angle.y*DEG2RAD),0.0f,&data);

		glPopMatrix();

		LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
	}

	void UNIT::draw_shadow_basic(CAMERA *cam,VECTOR Dir,MAP *map)
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::draw_shadow_basic );
#endif
		EnterCS();
		if( on_radar || hidden )	{
			LeaveCS();
			return;
			}

		if(!visible) {
			VECTOR S_Pos = drawn_Pos-(h/Dir.y)*Dir;//map->hit(Pos,Dir);
			int px=((int)(S_Pos.x+map->map_w_d))>>4;
			int py=((int)(S_Pos.z+map->map_h_d))>>4;
			if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	{
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				LeaveCS();
				return;	// Shadow out of the map
				}
			if(map->view[py][px]!=1) {
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				LeaveCS();
				return;	// Unvisible shadow
				}
			}

		glPushMatrix();
		glTranslatef(drawn_Pos.x,drawn_Pos.y,drawn_Pos.z);
		glRotatef(drawn_Angle.x,1.0f,0.0f,0.0f);
		glRotatef(drawn_Angle.z,0.0f,0.0f,1.0f);
		glRotatef(drawn_Angle.y,0.0f,1.0f,0.0f);
		float scale = unit_manager.unit_type[type_id].Scale;
		glScalef(scale,scale,scale);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		if( unit_manager.unit_type[ type_id ].canmove || shadow_scale_dir < 0.0f ) {
			VECTOR H = drawn_Pos;
			H.y += 2.0f * model->size2 + 1.0f;
			VECTOR D = map->hit( H, Dir, true, 2000.0f);
			shadow_scale_dir = (D - H).Norm();
			}
		model->draw_shadow_basic(((shadow_scale_dir*Dir*RotateX(-drawn_Angle.x*DEG2RAD))*RotateZ(-drawn_Angle.z*DEG2RAD))*RotateY(-drawn_Angle.y*DEG2RAD),0.0f,&data);

		glPopMatrix();
		LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
	}

	const int UNIT::run_script(const float &dt,const int &id,MAP *map,int max_code)			// Interprète les scripts liés à l'unité
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( run_script );
#endif
		if(flags==0) {
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return 2;	}
		if( id >= script_env->size() && !(*script_env)[id].running)	{
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return 2;	}
		if((*script_env)[id].wait>0.0f) {
			(*script_env)[id].wait-=dt;
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return 1;
			}
		if(script==NULL || (*script_env)[id].env==NULL) {
			(*script_env)[id].running=false;
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return 2;	// S'il n'y a pas de script associé on quitte la fonction
			}
		sint16 script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
		sint16 pos=((*script_env)[id].env->cur>>8);

		if(script_id<0 || script_id>=script->nb_script)	{
			(*script_env)[id].running=false;
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return 2;		// Erreur, ce n'est pas un script repertorié
			}

		float divisor=i2pwr16;
		float div=0.5f*divisor;
		POINTF O;
		O.x=O.y=O.z=0.0f;
		bool done=false;
		int nb_code=0;

//#define PRINT_CODE			// Affiche le code éxécuté dans la console
//bool	print_code = Lowercase( unit_manager.unit_type[ type_id ].Unitname ) == "armtship" && ( Lowercase( script->name[script_id] ) == "transportpickup" || Lowercase( script->name[script_id] ) == "boomcalc" );

		do
		{
//			uint32 code=script->script_code[script_id][pos];			// Lit un code
//			pos++;
			nb_code++;
			if( nb_code >= max_code ) done=true;			// Pour éviter que le programme ne fige à cause d'un script
//			switch(code)			// Code de l'interpréteur
			switch(script->script_code[script_id][pos++])
			{
			case SCRIPT_MOVE_OBJECT:
#ifdef PRINT_CODE
			if( print_code )
				printf("MOVE_OBJECT\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					if(axis==0)
						v1=-v1;
					data.axe[axis][obj].reset_move();
					data.axe[axis][obj].move_distance=v1*div;
					data.axe[axis][obj].move_distance-=data.axe[axis][obj].pos;
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					if(data.axe[axis][obj].move_distance<0.0f)
						data.axe[axis][obj].move_speed=-fabs(v2*div*0.5f);
					else
						data.axe[axis][obj].move_speed=fabs(v2*div*0.5f);
				}
				break;
			case SCRIPT_WAIT_FOR_TURN:
#ifdef PRINT_CODE
			if( print_code )
				printf("WAIT_FOR_TURN\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					float a=data.axe[axis][obj].rot_angle;
					if((data.axe[axis][obj].rot_speed!=0.0f || data.axe[axis][obj].rot_accel!=0.0f) && (a!=0.0f && data.axe[axis][obj].rot_limit))
						pos-=3;
					else if(data.axe[axis][obj].rot_speed!=data.axe[axis][obj].rot_target_speed && data.axe[axis][obj].rot_speed_limit)
						pos-=3;
					else {
						data.axe[axis][obj].rot_speed = 0.0f;
						data.axe[axis][obj].rot_accel = 0.0f;
						}
				}
				done=true;
				break;
			case SCRIPT_RANDOM_NUMBER:
#ifdef PRINT_CODE
			if( print_code )
				printf("RANDOM_NUMBER\n");
#endif
				{
					int high=(*script_env)[id].pop();
					int low=(*script_env)[id].pop();
					(*script_env)[id].push((rand_from_table()%(high-low+1))+low);
				}
				break;
			case SCRIPT_GREATER_EQUAL:
#ifdef PRINT_CODE
			if( print_code )
				printf("GREATER_EQUAL\n");
#endif
				{
					int v2=(*script_env)[id].pop();
					int v1=(*script_env)[id].pop();
					(*script_env)[id].push(v1>=v2 ? 1 : 0);
				}
				break;
			case SCRIPT_GREATER:
#ifdef PRINT_CODE
			if( print_code )
				printf("GREATER\n");
#endif
				{
					int v2=(*script_env)[id].pop();
					int v1=(*script_env)[id].pop();
					(*script_env)[id].push(v1>v2 ? 1 : 0);
				}
				break;
			case SCRIPT_LESS:
#ifdef PRINT_CODE
			if( print_code )
				printf("LESS\n");
#endif
				{
					int v2=(*script_env)[id].pop();
					int v1=(*script_env)[id].pop();
					(*script_env)[id].push(v1<v2 ? 1 : 0);
				}
				break;
			case SCRIPT_EXPLODE:
#ifdef PRINT_CODE
			if( print_code )
				printf("EXPLODE\n");
#endif
				{
					int obj = script->script_code[script_id][pos++];
					int explosion_type = (*script_env)[id].pop();
					data.axe[0][obj].pos = 0.0f;
					data.axe[0][obj].angle = 0.0f;
					data.axe[1][obj].pos = 0.0f;
					data.axe[1][obj].angle = 0.0f;
					data.axe[2][obj].pos = 0.0f;
					data.axe[2][obj].angle = 0.0f;
					if(visible) {					// Don't draw things which could tell the player there is something there
						compute_model_coord();
						particle_engine.make_fire( O + Pos + data.pos[obj],1,10,45.0f);
						}
					data.flag[obj]|=FLAG_EXPLODE;
					data.explosion_flag[obj]=explosion_type;
					data.axe[0][obj].move_speed=(25.0f+(rand_from_table()%2501)*0.01f)*(rand_from_table()&1 ? 1.0f : -1.0f);
					data.axe[0][obj].rot_speed=(rand_from_table()%7201)*0.1f-360.0f;
					data.axe[1][obj].move_speed=25.0f+(rand_from_table()%2501)*0.01f;
					data.axe[1][obj].rot_speed=(rand_from_table()%7201)*0.1f-360.0f;
					data.axe[2][obj].move_speed=(25.0f+(rand_from_table()%2501)*0.01f)*(rand_from_table()&1 ? 1.0f : -1.0f);
					data.axe[2][obj].rot_speed=(rand_from_table()%7201)*0.1f-360.0f;
					data.explode = true;
					data.explode_time = 1.0f;
				}
				break;
			case SCRIPT_TURN_OBJECT:
#ifdef PRINT_CODE
			if( print_code )
				printf("TURN_OBJECT\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					if(axis!=2) {
						v1=-v1;
						v2=-v2;
						}
					data.axe[axis][obj].reset_rot();
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					data.axe[axis][obj].rot_angle=-v1*TA2DEG;
					data.axe[axis][obj].rot_accel=0.0f;
					data.axe[axis][obj].rot_angle-=data.axe[axis][obj].angle;
					while(data.axe[axis][obj].rot_angle>180.0f)					// Fait le tour dans le sens le plus rapide
						data.axe[axis][obj].rot_angle-=360.0f;
					while(data.axe[axis][obj].rot_angle<-180.0f)					// Fait le tour dans le sens le plus rapide
						data.axe[axis][obj].rot_angle+=360.0f;
					if(data.axe[axis][obj].rot_angle>0.0f)
						data.axe[axis][obj].rot_speed=fabs(v2*TA2DEG);
					else
						data.axe[axis][obj].rot_speed=-fabs(v2*TA2DEG);
					data.axe[axis][obj].rot_limit=true;
					data.axe[axis][obj].rot_speed_limit=false;
				}
				break;
			case SCRIPT_WAIT_FOR_MOVE:
#ifdef PRINT_CODE
			if( print_code )
				printf("WAIT_FOR_MOVE\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
//					float a=data.axe[axis][obj].rot_angle;
					if(data.axe[axis][obj].move_distance!=0.0f)
						pos-=3;
				}
				done=true;
				break;
			case SCRIPT_CREATE_LOCAL_VARIABLE:
#ifdef PRINT_CODE
			if( print_code )
				printf("CREATE_LOCAL_VARIABLE\n");
#endif
				break;
			case SCRIPT_SUBTRACT:
#ifdef PRINT_CODE
			if( print_code )
				printf("SUBSTRACT\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v2-v1);
				}
				break;
			case SCRIPT_GET_VALUE_FROM_PORT:
#ifdef PRINT_CODE
			if( print_code )
				printf("GET_VALUE_FROM_PORT: ");
#endif
				{
					int value=(*script_env)[id].pop();
#ifdef PRINT_CODE
			if( print_code )
					printf("%d\n",value);
#endif
					switch(value)
					{
					case MIN_ID:		// returns the lowest valid unit ID number
						value = 0;
						break;
					case MAX_ID:		// returns the highest valid unit ID number
						value = units.max_unit-1;
						break;
					case MY_ID:		// returns ID of current unit
						value = idx;
						break;
					case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
						value = owner_id;
						break;
					case VETERAN_LEVEL:		// gets kills * 100
						value = 0;			// not yet implemented
						break;
					case ATAN:
						{
							int v1=(*script_env)[id].pop();
							int v2=(*script_env)[id].pop();
							value = (int)(atan((float)v1/v2)+0.5f);
						}
						break;
					case HYPOT:
						{
							int v1=(*script_env)[id].pop();
							int v2=(*script_env)[id].pop();
							value = (int)(sqrt((float)(v1*v1+v2*v2))+0.5f);
						}
						break;
					case BUGGER_OFF:
						value = map->check_rect((((int)(Pos.x+map->map_w_d))>>3)-(unit_manager.unit_type[type_id].FootprintX>>1),(((int)(Pos.z+map->map_h_d))>>3)-(unit_manager.unit_type[type_id].FootprintZ>>1),unit_manager.unit_type[type_id].FootprintX,unit_manager.unit_type[type_id].FootprintZ,idx) ? 0 : 1;
						break;
					case BUILD_PERCENT_LEFT:
						port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left>(int)build_percent_left) ? 1 : 0 );
					case YARD_OPEN:
					case ACTIVATION:
					case HEALTH:
					case INBUILDSTANCE:
					case BUSY:
					case ARMORED:
					case STANDINGMOVEORDERS:			// A faire : ajouter le support des ordres de mouvement/feu
					case STANDINGFIREORDERS:
						value = port[ value ];
						break;
					default:
						{
							char *op[]={"INCONNU","ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y",
							"UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN","HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
							"BUGGER_OFF","ARMORED"};
							if(value>20)
								value=0;
							printf("GET_VALUE_FROM_PORT: ");
							printf("opération non gérée : %s\n",op[value]);
						}
						break;
					};
					(*script_env)[id].push(value);
				}
				break;
			case SCRIPT_LESS_EQUAL:
#ifdef PRINT_CODE
			if( print_code )
				printf("LESS_EQUAL\n");
#endif
				{
					int v2=(*script_env)[id].pop();
					int v1=(*script_env)[id].pop();
					(*script_env)[id].push(v1<=v2 ? 1 : 0);
				}
				break;
			case SCRIPT_SPIN_OBJECT:
#ifdef PRINT_CODE
			if( print_code )
				printf("SPIN_OBJECT\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					if(axis==1) {
						v1=-v1;
						v2=-v2;
						}
					data.axe[axis][obj].reset_rot();
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					data.axe[axis][obj].rot_limit=false;
					data.axe[axis][obj].rot_speed_limit=true;
					data.axe[axis][obj].rot_target_speed=v1*TA2DEG;
					if(v2) {
						if(data.axe[axis][obj].rot_target_speed>data.axe[axis][obj].rot_speed)
							data.axe[axis][obj].rot_accel=fabs(v2*TA2DEG);
						else
							data.axe[axis][obj].rot_accel=-fabs(v2*TA2DEG);
						}
					else {
						data.axe[axis][obj].rot_accel=0;
						data.axe[axis][obj].rot_speed=data.axe[axis][obj].rot_target_speed;
						}
				}
				break;
			case SCRIPT_SLEEP:
#ifdef PRINT_CODE
			if( print_code )
				printf("SLEEP\n");
#endif
				(*script_env)[id].wait=(*script_env)[id].pop()*0.001f;
				done=true;
				break;
			case SCRIPT_MULTIPLY:
#ifdef PRINT_CODE
			if( print_code )
				printf("MULTIPLY\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1*v2);
				}
				break;
			case SCRIPT_CALL_SCRIPT:
#ifdef PRINT_CODE
			if( print_code )
				printf("CALL_SCRIPT\n");
#endif
				{
				int function_id=script->script_code[script_id][pos];			// Lit un code
				pos++;
				int num_param=script->script_code[script_id][pos];			// Lit un code
				pos++;
				(*script_env)[id].env->cur=script_id+(pos<<8);
				SCRIPT_ENV_STACK *old=(*script_env)[id].env;
				(*script_env)[id].env=new SCRIPT_ENV_STACK;
				(*script_env)[id].env->init();
				(*script_env)[id].env->next=old;
				(*script_env)[id].env->cur=function_id;
				for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
					(*script_env)[id].env->var[i]=(*script_env)[id].pop();
				done=true;
				pos=0;
				script_id=function_id;
				}
				break;
			case SCRIPT_SHOW_OBJECT:
#ifdef PRINT_CODE
			if( print_code )
				printf("SHOW_OBJECT\n");
#endif
				data.flag[script->script_code[script_id][pos++]]&=(~FLAG_HIDE);
				break;
			case SCRIPT_EQUAL:
#ifdef PRINT_CODE
			if( print_code )
				printf("EQUAL\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1==v2 ? 1 : 0);
				}
				break;
			case SCRIPT_NOT_EQUAL:
#ifdef PRINT_CODE
			if( print_code )
				printf("NOT_EQUAL\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1!=v2 ? 1 : 0);
				}
				break;
			case SCRIPT_IF:
#ifdef PRINT_CODE
			if( print_code )
				printf("IF\n");
#endif
				if((*script_env)[id].pop()!=0)
					pos++;
				else {
					int target_offset=script->script_code[script_id][pos];			// Lit un code
					pos=target_offset-script->dec_offset[script_id];								// Déplace l'éxecution
					}
				break;
			case SCRIPT_HIDE_OBJECT:
#ifdef PRINT_CODE
			if( print_code )
				printf("HIDE_OBJECT\n");
#endif
				data.flag[script->script_code[script_id][pos++]]|=FLAG_HIDE;
				break;
			case SCRIPT_SIGNAL:
#ifdef PRINT_CODE
			if( print_code )
				printf("SIGNAL\n");
#endif
				(*script_env)[id].env->cur=script_id+(pos<<8);			// Sauvegarde la position
				raise_signal((*script_env)[id].pop());					// Tue tout les processus utilisant ce signal
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				return 0;												// Quitte la fonction d'émulation de scripts
			case SCRIPT_DONT_CACHE:
#ifdef PRINT_CODE
			if( print_code )
				printf("DONT_CACHE\n");
#endif
				pos++;
				break;
			case SCRIPT_SET_SIGNAL_MASK:
#ifdef PRINT_CODE
			if( print_code )
				printf("SET_SIGNAL_MASK\n");
#endif
				(*script_env)[id].env->signal_mask=(*script_env)[id].pop();
				break;
			case SCRIPT_NOT:
#ifdef PRINT_CODE
			if( print_code )
				printf("NOT\n");
#endif
				(*script_env)[id].push(!(*script_env)[id].pop());
				break;
			case SCRIPT_DONT_SHADE:
#ifdef PRINT_CODE
			if( print_code )
				printf("DONT_SHADE\n");
#endif
				pos++;
				break;
			case SCRIPT_EMIT_SFX:
#ifdef PRINT_CODE
			if( print_code )
				printf("EMIT_SFX: ");
#endif
				{
					int smoke_type=(*script_env)[id].pop();
					int from_piece=script->script_code[script_id][pos++];
#ifdef PRINT_CODE
			if( print_code )
					printf("smoke_type %d from %d\n",smoke_type,from_piece);
#endif
					if(visible) {
						compute_model_coord();
						if(data.dir[from_piece].x!=0.0f || data.dir[from_piece].y!=0.0f || data.dir[from_piece].z!=0.0f) {
							VECTOR dir=data.dir[from_piece];
							switch(smoke_type)
							{
							case 0:
								particle_engine.emit_part(O+Pos+data.pos[from_piece],dir,fire,1,10.0f,2.5f,5.0f,true);
								break;
							case 2:
							case 3:
								particle_engine.emit_part(O+Pos+data.pos[from_piece],dir,0,1,10.0f,10.0f,10.0f,false, 0.3f);
								break;
							case 257:			// Fumée
							case 258:
								particle_engine.emit_part(O+Pos+data.pos[from_piece],dir,0,1,10.0f,10.0f,10.0f,true, 0.3f);
								break;
							};
							}
						else
							switch(smoke_type)
							{
							case 0:
								particle_engine.make_smoke(O+Pos+data.pos[from_piece],fire,1,0.0f,0.0f,0.0f,0.5f);
								break;
							case 257:
							case 258:
								particle_engine.make_smoke(O+Pos+data.pos[from_piece],0,1,10.0f,-1.0f,0.0f,0.5f);
								break;
							};
						}
				}
				break;
			case SCRIPT_PUSH_CONST:
#ifdef PRINT_CODE
			if( print_code )
				printf("PUSH_CONST (%d)\n", script->script_code[script_id][pos]);
#endif
				(*script_env)[id].push(script->script_code[script_id][pos]);
				pos++;
				break;
			case SCRIPT_PUSH_VAR:
#ifdef PRINT_CODE
			if( print_code )
				printf("PUSH_VAR (%d) = %d\n", script->script_code[script_id][pos], (*script_env)[id].env->var[script->script_code[script_id][pos]] );
#endif
				(*script_env)[id].push((*script_env)[id].env->var[script->script_code[script_id][pos]]);
				pos++;
				break;
			case SCRIPT_SET_VAR:
#ifdef PRINT_CODE
			if( print_code )
				printf("SET_VAR (%d)\n", script->script_code[script_id][pos]);
#endif
				(*script_env)[id].env->var[script->script_code[script_id][pos]]=(*script_env)[id].pop();
				pos++;
				break;
			case SCRIPT_PUSH_STATIC_VAR:
#ifdef PRINT_CODE
			if( print_code )
				printf("PUSH_STATIC_VAR\n");
#endif
				if( script->script_code[script_id][pos] >= s_var->size() )
					s_var->resize( script->script_code[script_id][pos] + 1 );
				(*script_env)[id].push((*s_var)[script->script_code[script_id][pos]]);
				pos++;
				break;
			case SCRIPT_SET_STATIC_VAR:
#ifdef PRINT_CODE
			if( print_code )
				printf("SET_STATIC_VAR\n");
#endif
				if( script->script_code[script_id][pos] >= s_var->size() )
					s_var->resize( script->script_code[script_id][pos] + 1 );
				(*s_var)[script->script_code[script_id][pos]]=(*script_env)[id].pop();
				pos++;
				break;
			case SCRIPT_OR:
#ifdef PRINT_CODE
			if( print_code )
				printf("OR\n");
#endif
				{
					int v1=(*script_env)[id].pop(),v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1|v2);
				}
				break;
			case SCRIPT_START_SCRIPT:				// Transfère l'éxecution à un autre script
#ifdef PRINT_CODE
			if( print_code )
				printf("START_SCRIPT\n");
#endif
				{
				int function_id=script->script_code[script_id][pos++];			// Lit un code
				int num_param=script->script_code[script_id][pos++];			// Lit un code
				int s_id=launch_script(function_id, 0, NULL, true);
				if(s_id>=0) {
					for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
						(*script_env)[s_id].env->var[i]=(*script_env)[id].pop();
					(*script_env)[s_id].env->signal_mask=(*script_env)[id].env->signal_mask;
					}
				else
					for(int i=0;i<num_param;i++)		// Enlève les paramètres de la pile
						(*script_env)[id].pop();
				done=true;
				}
				break;
			case SCRIPT_RETURN:		// Retourne au script précédent
#ifdef PRINT_CODE
			if( print_code )
				printf("RETURN\n");
#endif
				{
					if( script_val->size() <= script_id )
						script_val->resize( script_id + 1 );
					(*script_val)[script_id]=(*script_env)[id].env->var[0];
					SCRIPT_ENV_STACK *old=(*script_env)[id].env;
					(*script_env)[id].env=(*script_env)[id].env->next;
					delete old;
					(*script_env)[id].pop();		// Enlève la valeur retournée
				}
				if((*script_env)[id].env) {
					script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
					pos=((*script_env)[id].env->cur>>8);
					}
				done=true;
				break;
			case SCRIPT_JUMP:						// Commande de saut
#ifdef PRINT_CODE
			if( print_code )
				printf("JUMP\n");
#endif
				{
					int target_offset=script->script_code[script_id][pos];			// Lit un code
					pos=target_offset-script->dec_offset[script_id];								// Déplace l'éxecution
				}
				break;
			case SCRIPT_ADD:
#ifdef PRINT_CODE
			if( print_code )
				printf("ADD\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1+v2);
				}
				break;	//added
			case SCRIPT_STOP_SPIN:
#ifdef PRINT_CODE
			if( print_code )
				printf("STOP_SPIN\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					int v=(*script_env)[id].pop();
					if(axis!=2)
						v=-v;
					data.axe[axis][obj].reset_rot();
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					data.axe[axis][obj].rot_limit=false;
					data.axe[axis][obj].rot_speed_limit=true;
					data.axe[axis][obj].rot_target_speed=0.0f;
					if(v==0) {
						data.axe[axis][obj].rot_speed=0.0f;
						data.axe[axis][obj].rot_accel=0.0f;
						}
					else {
						if(data.axe[axis][obj].rot_speed>0.0f)
							data.axe[axis][obj].rot_accel=-abs(v);
						else
							data.axe[axis][obj].rot_accel=abs(v);
						}
				}
				break;	//added
			case SCRIPT_DIVIDE:
#ifdef PRINT_CODE
			if( print_code )
				printf("DIVIDE\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v2 / v1);
				}
				break;	//added
			case SCRIPT_MOVE_PIECE_NOW:
#ifdef PRINT_CODE
			if( print_code )
				printf("MOVE_PIECE_NOW\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					data.axe[axis][obj].reset_move();
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					if(axis==0)
						data.axe[axis][obj].pos=-(*script_env)[id].pop()*div;
					else
						data.axe[axis][obj].pos=(*script_env)[id].pop()*div;
				}
				break;	//added
			case SCRIPT_TURN_PIECE_NOW:
#ifdef PRINT_CODE
			if( print_code )
				printf("TURN_PIECE_NOW\n");
#endif
				{
					int obj=script->script_code[script_id][pos++];
					int axis=script->script_code[script_id][pos++];
					int v=(*script_env)[id].pop();
					data.axe[axis][obj].reset_rot();
					data.axe[axis][obj].is_moving=true;
					data.is_moving=true;
					if(axis!=2)
						v=-v;
					data.axe[axis][obj].angle=-v*TA2DEG;
				}
				break;	//added
			case SCRIPT_CACHE:
#ifdef PRINT_CODE
			if( print_code )
				printf("CACHE\n");
#endif
				pos++;
				break;	//added
			case SCRIPT_COMPARE_AND:
#ifdef PRINT_CODE
			if( print_code )
				printf("COMPARE_AND\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1 && v2);
				}
				break;	//added
			case SCRIPT_COMPARE_OR:
#ifdef PRINT_CODE
			if( print_code )
				printf("COMPARE_OR\n");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					(*script_env)[id].push(v1 || v2);
				}
				break;	//added
			case SCRIPT_CALL_FUNCTION:
#ifdef PRINT_CODE
			if( print_code )
				printf("CALL_FUNCTION\n");
#endif
				{
				int function_id=script->script_code[script_id][pos++];			// Lit un code
				int num_param=script->script_code[script_id][pos++];			// Lit un code
				(*script_env)[id].env->cur=script_id+(pos<<8);
				SCRIPT_ENV_STACK *old=(*script_env)[id].env;
				(*script_env)[id].env=new SCRIPT_ENV_STACK;
				(*script_env)[id].env->init();
				(*script_env)[id].env->next=old;
				(*script_env)[id].env->cur=function_id;
				for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
					(*script_env)[id].env->var[i]=(*script_env)[id].pop();
				done=true;
				pos=0;
				script_id=function_id;
				}
				break;	//added
			case SCRIPT_GET:
#ifdef PRINT_CODE
			if( print_code )
				printf("GET *\n");
#endif
				{
					(*script_env)[id].pop();
					(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
					int v1=(*script_env)[id].pop();
					int val=(*script_env)[id].pop();
					switch(val)
					{
					case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
						if( v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
							(*script_env)[id].push( units.unit[ v1 ].owner_id );
						else
							(*script_env)[id].push(-1);
						break;
					case UNIT_BUILD_PERCENT_LEFT:		// basically BUILD_PERCENT_LEFT, but comes with a unit parameter
						if( v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
							(*script_env)[id].push((int)units.unit[ v1 ].build_percent_left + ( (units.unit[ v1 ].build_percent_left > (int)units.unit[ v1 ].build_percent_left) ? 1 : 0));
						else
							(*script_env)[id].push(0);
						break;
					case UNIT_ALLIED:		// is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
						if( v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
							(*script_env)[id].push( units.unit[ v1 ].owner_id == owner_id );
						else
							(*script_env)[id].push(0);
						break;
					case UNIT_IS_ON_THIS_COMP:		// indicates if the 1st parameter(a unit ID) is local to this computer
						if( v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
							(*script_env)[id].push( !(players.control[ units.unit[ v1 ].owner_id ] & PLAYER_CONTROL_FLAG_REMOTE) );
						else
							(*script_env)[id].push(0);
						break;
					case BUILD_PERCENT_LEFT:
						port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0);
					case ACTIVATION:
					case STANDINGMOVEORDERS:
					case STANDINGFIREORDERS:
					case HEALTH:
					case INBUILDSTANCE:
					case BUSY:
					case YARD_OPEN:
					case BUGGER_OFF:
					case ARMORED:
						(*script_env)[id].push((int)port[val]);
						break;
					case PIECE_XZ:
						compute_model_coord();
						(*script_env)[id].push( PACKXZ((data.pos[v1].x+Pos.x)*2.0f+map->map_w, (data.pos[v1].z+Pos.z)*2.0f+map->map_h));
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("PIECE_XZ=%d\n", c);
						}
#endif
						break;
					case PIECE_Y:
						compute_model_coord();
						(*script_env)[id].push((int)((data.pos[v1].y + Pos.y)*2.0f)<<16);
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("PIECE_Y=%d\n", c);
						}
#endif
						break;
					case UNIT_XZ:
						if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
							(*script_env)[id].push( PACKXZ( units.unit[v1].Pos.x*2.0f+map->map_w, units.unit[v1].Pos.z*2.0f+map->map_h ));
						else
							(*script_env)[id].push( PACKXZ( Pos.x*2.0f+map->map_w, Pos.z*2.0f+map->map_h ));
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("UNIT_XY=%d\n", c);
						}
#endif
						break;
					case UNIT_Y:
						if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
							(*script_env)[id].push((int)(units.unit[v1].Pos.y * 2.0f)<<16);
						else
							(*script_env)[id].push((int)(Pos.y * 2.0f)<<16);
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("UNIT_Y=%d\n", c);
						}
#endif
						break;
					case UNIT_HEIGHT:
						if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
							(*script_env)[id].push((int)(units.unit[v1].model->top * 2.0f)<<16);
						else
							(*script_env)[id].push((int)(model->top * 2.0f)<<16);
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("UNIT_HEIGHT=%d\n", c);
						}
#endif
						break;
					case XZ_ATAN:
							(*script_env)[id].push((int)(atan2( UNPACKX(v1) , UNPACKZ(v1) ) * RAD2TA - Angle.y * DEG2TA) + 32768);
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("XZ_ATAN[(%d)=(%d,%d)]=%d\n", v1, UNPACKX(v1), UNPACKZ(v1), c);
						}
#endif
						break;
					case XZ_HYPOT:
							(*script_env)[id].push((int)hypot( UNPACKX(v1), UNPACKZ(v1) )<<16);
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("XZ_HYPOT[(%d)=(%d,%d)]=%d\n", v1,UNPACKX(v1), UNPACKZ(v1), c);
						}
#endif
						break;
					case ATAN:
						(*script_env)[id].push((int)(atan2(v1,v2) * RAD2TA ));
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("ATAN=%d\n", c);
						}
#endif
						break;
					case HYPOT:
						(*script_env)[id].push((int)hypot(v1,v2));
#ifdef PRINT_CODE
						{
							int c = (*script_env)[id].pop();
							(*script_env)[id].push( c );
			if( print_code )
							printf("HYPOT(%d,%d)=%d\n", v1, v2, c);
						}
#endif
						break;
					case GROUND_HEIGHT:
						(*script_env)[id].push((int)(map->get_unit_h(( UNPACKX(v1) - map->map_w)*0.5f,( UNPACKZ(v1) - map->map_h)*0.5f)*2.0f)<<16);
						break;
					default:
						printf("GET constante inconnue %d\n", val);
					};
				}
				break;	//added
			case SCRIPT_SET_VALUE:
#ifdef PRINT_CODE
			if( print_code )
				printf("SET_VALUE *: ");
#endif
				{
					int v1=(*script_env)[id].pop();
					int v2=(*script_env)[id].pop();
#ifdef PRINT_CODE
			if( print_code )
					printf("%d %d\n",v1,v2);
#endif
					port[v2]=v1;
					switch(v2)
					{
					case ACTIVATION:
						if( v1 == 0 )
							deactivate();
						else
							activate();
						break;
					case YARD_OPEN:
						if(!map->check_rect((((int)(Pos.x+map->map_w_d))>>3)-(unit_manager.unit_type[type_id].FootprintX>>1),(((int)(Pos.z+map->map_h_d))>>3)-(unit_manager.unit_type[type_id].FootprintZ>>1),unit_manager.unit_type[type_id].FootprintX,unit_manager.unit_type[type_id].FootprintZ,idx))
							port[v2] ^= 1;
						break;
					case BUGGER_OFF:
						if(port[v2])	{
							int px=((int)(Pos.x)+map->map_w_d)>>3;
							int py=((int)(Pos.z)+map->map_h_d)>>3;
							for(int y=py-(unit_manager.unit_type[type_id].FootprintZ>>1);y<=py+(unit_manager.unit_type[type_id].FootprintZ>>1);y++)
								if(y>=0 && y<(map->bloc_h<<1)-1)
									for(int x=px-(unit_manager.unit_type[type_id].FootprintX>>1);x<=px+(unit_manager.unit_type[type_id].FootprintX>>1);x++)
										if(x>=0 && x<(map->bloc_w<<1)-1)
											if(map->map_data[y][x].unit_idx >= 0 && map->map_data[y][x].unit_idx!=idx ) {
												int cur_idx=map->map_data[y][x].unit_idx;
												if(units.unit[cur_idx].owner_id==owner_id && units.unit[cur_idx].build_percent_left == 0.0f && (units.unit[cur_idx].mission==NULL || units.unit[cur_idx].mission->mission!=MISSION_MOVE)) {
													units.unit[cur_idx].Lock();
													VECTOR target = units.unit[cur_idx].Pos;
													target.z+=100.0f;
													units.unit[cur_idx].add_mission(MISSION_MOVE,&target,true);
													units.unit[cur_idx].UnLock();
													}
												}
							}
						break;
					};
				}
				break;	//added
			case SCRIPT_ATTACH_UNIT:
#ifdef PRINT_CODE
			if( print_code )
				printf("ATTACH_UNIT *\n");
#endif
				{
					int v3 = (*script_env)[id].pop();
					int v2 = (*script_env)[id].pop();
					int v1 = (*script_env)[id].pop();
					if( v1 >= 0 && v1 < units.max_unit && units.unit[ v1 ].flags ) {
						UNIT *target_unit = &(units.unit[v1]);
						target_unit->hidden = (v2 < 0);
						bool already_in = false;
						if( target_unit->attached )
							for( int i = 0 ; i < nb_attached ; i++ )		// Check if this unit is already in
								if( attached_list[ i ] == v1 ) {
									already_in = true;
									link_list[ i ] = v2;
									}
						if( !already_in ) {
							link_list[nb_attached]=v2;
							attached_list[nb_attached++]=target_unit->idx;
							}
						target_unit->attached=true;
						if( !already_in )
							target_unit->clear_from_map();
						}
				}
				break;	//added
			case SCRIPT_DROP_UNIT:
#ifdef PRINT_CODE
			if( print_code )
				printf("DROP_UNIT *\n");
#endif
				{
					int v1 = (*script_env)[id].pop();
#ifdef PRINT_CODE
			if( print_code )
					printf("dropping %d\n", v1);
#endif
					if( v1 >= 0 && v1 < units.max_unit && units.unit[ v1 ].flags ) {
						UNIT *target_unit = &(units.unit[v1]);
						target_unit->attached = false;
						target_unit->hidden = false;
						nb_attached--;					// Remove the unit from the attached list
						for( int i = 0 ; i < nb_attached ; i++ )
							if( attached_list[ i ] == v1 ) {
								link_list[ i ] = link_list[ nb_attached ];
								attached_list[ i ] = attached_list[ nb_attached ];
								break;
								}
											// Redraw the unit on presence map
						LeaveCS();
						target_unit->draw_on_map();
						EnterCS();
						}
				}
				break;	//added
			default:
				printf("UNKNOWN %d, STOPPING SCRIPT\n",script->script_code[script_id][--pos]);
				{
					if( script_val->size() <= script_id )
						script_val->resize( script_id + 1 );
					(*script_val)[script_id]=(*script_env)[id].env->var[0];
					SCRIPT_ENV_STACK *old=(*script_env)[id].env;
					(*script_env)[id].env=(*script_env)[id].env->next;
					delete old;
				}
				if((*script_env)[id].env) {
					script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
					pos=((*script_env)[id].env->cur>>8);
					}
				else
					(*script_env)[id].running=false;
				done=true;
			};
		}while(!done);
		if((*script_env)[id].env)
			(*script_env)[id].env->cur=script_id+(pos<<8);
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
		return 0;
	}

	inline float ballistic_angle(float v,float g,float d,float y_s,float y_e)			// Calculs de ballistique pour l'angle de tir
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( ballistic_angle );
#endif
		float v2 = v*v;
		float gd = g*d;
		float v2gd = v2/gd;
		float a = v2gd*(4.0f*v2gd-8.0f*(y_e-y_s)/d)-4.0f;
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
		if(a<0.0f)				// Pas de solution
			return 360.0f;
		return RAD2DEG*atan(v2gd-0.5f*sqrt(a));
 	}

	const int UNIT::move( const float dt,MAP *map, int *path_exec, const int key_frame )
	{
#ifdef	ADVANCED_DEBUG_MODE
		GuardEnter( UNIT::move );
#endif
		EnterCS();

		bool was_open = port[YARD_OPEN] != 0;
		bool was_flying = flying;
		sint32	o_px = cur_px;
		sint32	o_py = cur_py;
		compute_coord = true;
		VECTOR	old_V = V;			// Store the speed, so we can do some calculations
		bool	b_TargetAngle = false;		// Do we aim, move, ... ?? Need to change unit angle
		float	f_TargetAngle = 0.0f;

		VECTOR NPos = Pos;
		int n_px = cur_px;
		int n_py = cur_py;
		bool precomputed_position = false;

		if( type_id < 0 || type_id >= unit_manager.nb_unit || flags == 0 )	{		// A unit which cannot exist
			LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return	-1;		// Should NEVER happen
			}

		if( build_percent_left == 0.0f && unit_manager.unit_type[type_id].isfeature) {		// Turn this unit into a feature
			int x=(int)((Pos.x)+map->map_w_d-8)>>3;
			int y=(int)((Pos.z)+map->map_h_d-8)>>3;
			if( x > 0 && y > 0 && x < (map->bloc_w<<1) && y < (map->bloc_h<<1) )
				if(map->map_data[y][x].stuff==-1) {
					int type=feature_manager.get_feature_index(unit_manager.unit_type[type_id].Unitname);
					if( type >= 0 ) {
						map->map_data[y][x].stuff=features.add_feature(Pos,type);
						clear_from_map();
						if(type!=-1 && feature_manager.feature[type].blocking)
							map->rect(x-(feature_manager.feature[type].footprintx>>1),y-(feature_manager.feature[type].footprintz>>1),feature_manager.feature[type].footprintx,feature_manager.feature[type].footprintz,-2-map->map_data[y][x].stuff);
						flags = 4;
						}
					}
			LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
			GuardLeave();
#endif
			return -1;
			}

		if(map->ota_data.waterdoesdamage && Pos.y < map->sealvl)		// The unit is damaged by the "acid" water
			hp -= dt*map->ota_data.waterdamage;

		bool jump_commands = (((idx+key_frame)&0xF) == 0);		// Saute certaines commandes / Jump some commands so it runs faster with lots of units

		if( build_percent_left == 0.0f && self_destruct >= 0.0f ) {		// Self-destruction code
			int old = (int)self_destruct;
			self_destruct -= dt;
			if( old != (int)self_destruct )		// Play a sound :-)
				play_sound( format( "count%d", old ) );
			if( self_destruct <= 0.0f ) {
				self_destruct = 0.0f;
				hp = 0.0f;
				severity = unit_manager.unit_type[type_id].MaxDamage;
				}
			}

		if(hp<=0.0f) {			// L'unité est détruite
			if( mission
			&& !unit_manager.unit_type[ type_id ].BMcode
			&& ( mission->mission == MISSION_BUILD_2 || mission->mission == MISSION_BUILD )		// It was building something that we must destroy too
			&& mission->p != NULL ) {
				((UNIT*)(mission->p))->Lock();
				((UNIT*)(mission->p))->hp = 0.0f;
				((UNIT*)(mission->p))->built = false;
				((UNIT*)(mission->p))->UnLock();
				}
			switch(flags&0x17)
			{
			case 1:				// Début de la mort de l'unité	(Lance le script)
				flags = 4;		// Don't remove the data on the position map because they will be replaced
				if( build_percent_left == 0.0f ) {
					fx_manager.add_flash( Pos, max(unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ) * 32 );
					int param[]={ severity * 100 / unit_manager.unit_type[type_id].MaxDamage, 0 };
					run_script_function(map,get_script_index(SCRIPT_killed),2,param);
					if( attached )	param[1] = 3;			// When we were flying we just disappear
					bool sinking = map->get_unit_h( Pos.x, Pos.z ) <= map->sealvl;
					switch( param[1] )
					{
					case 1:			// Some good looking corpse
						{
							clear_from_map();
							int x=((int)(Pos.x)+map->map_w_d-8)>>3;
							int y=((int)(Pos.z)+map->map_h_d-8)>>3;
							if(x>0 && y>0 && x<(map->bloc_w<<1) && y<(map->bloc_h<<1))
								if(map->map_data[y][x].stuff==-1) {
									int type=feature_manager.get_feature_index(unit_manager.unit_type[type_id].Corpse);
									if( type >= 0 ) {
										Pos.x=(x<<3)-0.5f*map->map_w+8.0f;
										Pos.z=(y<<3)-0.5f*map->map_h+8.0f;
										map->map_data[y][x].stuff=features.add_feature(Pos,type);
										if( map->map_data[y][x].stuff >= 0 ) {			// Keep unit orientation
											features.feature[ map->map_data[y][x].stuff ].angle = Angle.y;
											features.feature[ map->map_data[y][x].stuff ].sinking = sinking;
											}
										if(type!=-1 && map->map_data[y][x].stuff != -1 && feature_manager.feature[type].blocking)
											map->rect(x-(feature_manager.feature[type].footprintx>>1),y-(feature_manager.feature[type].footprintz>>1),feature_manager.feature[type].footprintx,feature_manager.feature[type].footprintz,-2-map->map_data[y][x].stuff);
										}
								}
						}
						break;
					case 2:			// Some exploded corpse
						{
							clear_from_map();
							int x=(int)((Pos.x)+map->map_w_d-8)>>3;
							int y=(int)((Pos.z)+map->map_h_d-8)>>3;
							if(x>0 && y>0 && x<(map->bloc_w<<1) && y<(map->bloc_h<<1))
								if(map->map_data[y][x].stuff==-1) {
									int type=feature_manager.get_feature_index( (String( unit_manager.unit_type[type_id].name) + "_heap").c_str() );
									if( type >= 0 ) {
										Pos.x=(x<<3)-0.5f*map->map_w+8.0f;
										Pos.z=(y<<3)-0.5f*map->map_h+8.0f;
										map->map_data[y][x].stuff=features.add_feature(Pos,type);
										if( map->map_data[y][x].stuff >= 0 ) {			// Keep unit orientation
											features.feature[ map->map_data[y][x].stuff ].angle = Angle.y;
											features.feature[ map->map_data[y][x].stuff ].sinking = sinking;
											}
										if(type!=-1 && map->map_data[y][x].stuff != -1 && feature_manager.feature[type].blocking)
											map->rect(x-(feature_manager.feature[type].footprintx>>1),y-(feature_manager.feature[type].footprintz>>1),feature_manager.feature[type].footprintx,feature_manager.feature[type].footprintz,-2-map->map_data[y][x].stuff);
										}
								}
						}
						break;
					default:
						flags = 1;		// Nothing replaced just remove the unit from position map
					};
					LeaveCS();
					int w_id = weapons.add_weapon(weapon_manager.get_weapon_index( self_destruct == 0.0f ? unit_manager.unit_type[type_id].SelfDestructAs : unit_manager.unit_type[type_id].ExplodeAs ),idx);
					EnterCS();
					if(w_id>=0) {
						weapons.weapon[w_id].Pos = Pos;
						weapons.weapon[w_id].target_pos = Pos;
						weapons.weapon[w_id].target = -1;
						weapons.weapon[w_id].just_explode = true;
						}
					for(int i=0;i<data.nb_piece;i++)
						if(!(data.flag[i]&FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
							data.flag[i]|=FLAG_HIDE;
				 }
				else
					flags = 1;
				weapon[0].delay=1.0f;
				LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
				GuardLeave();
#endif
				return -1;
				break;
			case 4:				// Vérifie si le script est terminé
				if(!is_running(get_script_index(SCRIPT_killed))) {
					if(weapon[0].delay<=0.0f && !data.explode ) {
						flags = 1;
						LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
						GuardLeave();
#endif
						return -1;
						}
					weapon[0].delay-=dt;
					for(int i=0;i<data.nb_piece;i++)
						if(!(data.flag[i]&FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
							data.flag[i]|=FLAG_HIDE;
					}
				break;
			default:		// It doesn't explode (it has been reclaimed for example)
				flags=4;
			};
			if(data.nb_piece>0 && build_percent_left == 0.0f) {
				data.move(dt,map->ota_data.gravity);
				if(c_time>=0.1f) {
					c_time=0.0f;
					POINTF O;
					O.x=O.y=O.z=0.0f;
					for(int i=0;i<data.nb_piece;i++)
						if(data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)!=EXPLODE_BITMAPONLY) {
							if(data.explosion_flag[i]&EXPLODE_FIRE) {
								compute_model_coord();
								particle_engine.make_smoke(O+Pos+data.pos[i],fire,1,0.0f,0.0f);
								}
							if(data.explosion_flag[i]&EXPLODE_SMOKE) {
								compute_model_coord();
								particle_engine.make_smoke(O+Pos+data.pos[i],0,1,0.0f,0.0f);
								}
							}
					}
				}
			goto script_exec;
			}
		else if(!jump_commands && do_nothing())
			if(Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d) {
				VECTOR target = Pos;
				if(target.x < -map->map_w_d+256)
					target.x = -map->map_w_d+256;
				else if(target.x > map->map_w_d-256)
					target.x = map->map_w_d-256;
				if(target.z < -map->map_h_d+256)
					target.z = -map->map_h_d+256;
				else if(target.z > map->map_h_d-256)
					target.z = map->map_h_d-256;
				add_mission(MISSION_MOVE,&target,true,0,NULL,NULL,0,1);		// Stay on map
				}

		flags &= 0xEF;		// To fix a bug

		if(build_percent_left > 0.0f)	{		// Unit isn't finished
			if(!built) {
				hp -= dt * 1000.0f / ( 6 * unit_manager.unit_type[type_id].BuildTime ) * unit_manager.unit_type[type_id].MaxDamage;
				build_percent_left += dt * 100000.0f / ( 6 * unit_manager.unit_type[type_id].BuildTime );
				}
			goto script_exec;
			}
		else if( hp < unit_manager.unit_type[ type_id ].MaxDamage && unit_manager.unit_type[ type_id ].HealTime > 0 ) {
			hp += unit_manager.unit_type[ type_id ].MaxDamage * dt / unit_manager.unit_type[ type_id ].HealTime;
			if( hp > unit_manager.unit_type[ type_id ].MaxDamage )
				hp = unit_manager.unit_type[ type_id ].MaxDamage;
			}

		if(data.nb_piece>0)
			data.move(dt,units.g_dt);

		if(attached)
			goto script_exec;

		if(unit_manager.unit_type[type_id].canload && nb_attached>0) {
			int e = 0;
			compute_model_coord();
			for(int i=0;i+e<nb_attached;i++) {
				if(units.unit[attached_list[i]].flags) {
					units.unit[attached_list[i]].Pos = Pos+data.pos[link_list[i]];
					units.unit[attached_list[i]].cur_px = ((int)(units.unit[attached_list[i]].Pos.x)+map->map_w_d)>>3;
					units.unit[attached_list[i]].cur_py = ((int)(units.unit[attached_list[i]].Pos.z)+map->map_h_d)>>3;
					units.unit[attached_list[i]].Angle = Angle;
					}
				else {
					e++;
					i--;
					continue;
					}
				attached_list[i]=attached_list[i+e];
				}
			nb_attached-=e;
			}

		if(planned_weapons>0.0f) {			// Construit des armes / build weapons
			float old=planned_weapons-(int)planned_weapons;
			int idx=-1;
			if(unit_manager.unit_type[type_id].weapon[0] && unit_manager.unit_type[type_id].weapon[0]->stockpile)	idx=0;
			else if(unit_manager.unit_type[type_id].weapon[1] && unit_manager.unit_type[type_id].weapon[1]->stockpile)	idx=0;
			else if(unit_manager.unit_type[type_id].weapon[2] && unit_manager.unit_type[type_id].weapon[2]->stockpile)	idx=0;
			if(idx!=-1 && unit_manager.unit_type[type_id].weapon[idx]->reloadtime!=0.0f)	{
				float dn=dt/unit_manager.unit_type[type_id].weapon[idx]->reloadtime;
				float conso_metal=((float)unit_manager.unit_type[type_id].weapon[idx]->metalpershot)/unit_manager.unit_type[type_id].weapon[idx]->reloadtime;
				float conso_energy=((float)unit_manager.unit_type[type_id].weapon[idx]->energypershot)/unit_manager.unit_type[type_id].weapon[idx]->reloadtime;
				if(players.metal[owner_id]>=conso_metal*dt && players.energy[owner_id]>=conso_energy*dt) {
					metal_cons+=conso_metal;
					energy_cons+=conso_energy;
					planned_weapons-=dn;
					float last=planned_weapons-(int)planned_weapons;
					if((last==0.0f && last!=old) || (last>old && old>0.0f) || planned_weapons<=0.0f)		// On en a fini une / one is finished
						weapon[idx].stock++;
					if(planned_weapons<0.0f)
						planned_weapons=0.0f;
					}
				}
			}

		V_Angle.x=V_Angle.y=V_Angle.z=0.0f;
		c_time+=dt;

		//------------------------------ Beginning of weapon related code ---------------------------------------
		for( byte i = 0 ; i < 3 ; i++ ) {
			if( unit_manager.unit_type[type_id].weapon[ i ] == NULL )	continue;		// Skip that weapon if not present on the unit
			weapon[i].delay += dt;
			weapon[i].time += dt;
			switch( (weapon[i].state & 3) )
			{
			case WEAPON_FLAG_IDLE:										// Doing nothing, waiting for orders
				if(jump_commands)	break;
				weapon[i].data = -1;
				break;
			case WEAPON_FLAG_AIM:											// Vise une unité / aiming code
				if( weapon[i].target == NULL || ((weapon[i].state&WEAPON_FLAG_WEAPON)==WEAPON_FLAG_WEAPON && ((WEAPON*)(weapon[i].target))->weapon_id!=-1)
				|| ((weapon[i].state&WEAPON_FLAG_WEAPON)!=WEAPON_FLAG_WEAPON && (((UNIT*)(weapon[i].target))->flags&1))) {
					if( !(weapon[i].state & WEAPON_FLAG_COMMAND_FIRE) && unit_manager.unit_type[type_id].weapon[ i ]->commandfire ) {	// Not allowed to fire
						weapon[i].data = -1;
						weapon[i].state = WEAPON_FLAG_IDLE;
						break;
						}
					int query_id=-1;
					switch( i )
					{
					case 0:
						query_id = get_script_index(SCRIPT_QueryPrimary);		break;
					case 1:
						query_id = get_script_index(SCRIPT_QuerySecondary);		break;
					case 2:
						query_id = get_script_index(SCRIPT_QueryTertiary);		break;
					};
					if(!is_running(get_script_index(SCRIPT_RequestState))) {
						if(weapon[i].delay >= unit_manager.unit_type[type_id].weapon[ i ]->reloadtime || unit_manager.unit_type[type_id].weapon[ i ]->stockpile) {

							run_script_function( map, query_id );			// Run the script that tell us from where to shoot

							UNIT *target_unit = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? NULL : (UNIT*) weapon[i].target;
							WEAPON *target_weapon = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? (WEAPON*) weapon[i].target : NULL;

							VECTOR target = target_unit==NULL ? (target_weapon==NULL ? weapon[i].target_pos-Pos : target_weapon->Pos-Pos) : target_unit->Pos-Pos;
							float dist = target.Sq();
							int maxdist = 0;
							int mindist = 0;

							if(unit_manager.unit_type[type_id].attackrunlength>0) {
								if( target % V < 0.0f ) {
									weapon[i].state = WEAPON_FLAG_IDLE;
									weapon[i].data = -1;
									break;	// We're not shooting at the target
									}
								float t = 2.0f/map->ota_data.gravity*fabs(target.y);
								mindist = (int)sqrt(t*V.Sq())-(unit_manager.unit_type[type_id].attackrunlength+1>>1);
								maxdist = mindist+(unit_manager.unit_type[type_id].attackrunlength);
								}
							else
								maxdist = unit_manager.unit_type[type_id].weapon[ i ]->range>>1;
							
							if( dist > maxdist * maxdist || dist < mindist * mindist )	{
								weapon[i].state = WEAPON_FLAG_IDLE;
								weapon[i].data = -1;
								break;	// We're too far from the target
								}

							VECTOR target_translation;
							if( target_unit != NULL )
								target_translation = ( target.Norm() / unit_manager.unit_type[type_id].weapon[ i ]->weaponvelocity) * (target_unit->V - V);

							if(unit_manager.unit_type[type_id].weapon[ i ]->turret) {			// Si l'unité doit viser, on la fait viser / if it must aim, we make it aim
								if( script_val->size() <= query_id )
									script_val->resize( query_id + 1 );
								int start_piece = (*script_val)[query_id];
								if(start_piece<0 || start_piece>=data.nb_piece)
									start_piece=0;
								compute_model_coord();

								VECTOR target_pos_on_unit;						// Read the target piece on the target unit so we better know where to aim
								target_pos_on_unit.x = target_pos_on_unit.y = target_pos_on_unit.z = 0.0f;
								if( target_unit != NULL ) {
									if( weapon[i].data == -1 ) {
										int target_piece[1] = {0};
										target_unit->run_script_function( map, target_unit->get_script_index( SCRIPT_SweetSpot ), 1, target_piece );
										weapon[i].data = target_piece[0];
										}
									if( weapon[i].data >= 0 )
										target_pos_on_unit = target_unit->data.pos[ weapon[i].data ];
									}

								target = target + target_translation - data.pos[start_piece];
								if( target_unit!=NULL )
									target = target + target_pos_on_unit;
								dist = target.Norm();
								target=(1.0f/dist)*target;
								VECTOR I,J,IJ,RT;
								I.x=0.0f;	I.z=1.0f;	I.y=0.0f;
								J.x=1.0f;	J.z=0.0f;	J.y=0.0f;
								IJ.x=0.0f;	IJ.z=0.0f;	IJ.y=1.0f;
								RT=target;
								RT.y=0.0f;
								RT.Unit();
								float angle=acos(I%RT)*RAD2DEG;
								if(J%RT<0.0f) angle=-angle;
								angle-=Angle.y;
								if(angle<-180.0f)	angle+=360.0f;
								else if(angle>180.0f)	angle-=360.0f;
								int aiming[]={ (int)(angle*DEG2TA), -4096 };
								if(unit_manager.unit_type[type_id].weapon[ i ]->ballistic) {		// Calculs de ballistique / ballistic calculations
									VECTOR D=target_unit==NULL ? ( target_weapon == NULL ? Pos + data.pos[start_piece] - weapon[i].target_pos : (Pos+data.pos[start_piece]-target_weapon->Pos) ) : (Pos+data.pos[start_piece]-target_unit->Pos-target_pos_on_unit);
									D.y=0.0f;
									float v;
									if(unit_manager.unit_type[type_id].weapon[ i ]->startvelocity==0.0f)
										v=unit_manager.unit_type[type_id].weapon[ i ]->weaponvelocity;
									else
										v=unit_manager.unit_type[type_id].weapon[ i ]->startvelocity;
									if(target_unit==NULL) {
										if( target_weapon==NULL )
											aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.Norm(),(Pos+data.pos[start_piece]).y,weapon[i].target_pos.y)*DEG2TA);
										else
											aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.Norm(),(Pos+data.pos[start_piece]).y,target_weapon->Pos.y)*DEG2TA);
										}
									else
										aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.Norm(),(Pos+data.pos[start_piece]).y,target_unit->Pos.y+target_unit->model->center.y*0.5f)*DEG2TA);
									}
								else {
									VECTOR K=target;
									K.y=0.0f;
									K.Unit();
									angle=acos(K%target)*RAD2DEG;
									if(target.y<0.0f)
										angle=-angle;
									angle-=Angle.x;
									if(angle>180.0f)	angle-=360.0f;
									if(angle<-180.0f)	angle+=360.0f;
									if(fabs(angle)>180.0f) {
										weapon[i].state = WEAPON_FLAG_IDLE;
										weapon[i].data = -1;
										break;
										}
									aiming[1]=(int)(angle*DEG2TA);
									}
								if(unit_manager.unit_type[type_id].weapon[ i ]->lineofsight) {
									if(target_unit==NULL) {
										if( target_weapon == NULL )
											aim_dir=weapon[i].target_pos-(Pos+data.pos[start_piece]);
										else
											aim_dir=((WEAPON*)(weapon[i].target))->Pos-(Pos+data.pos[start_piece]);
										}
									else
										aim_dir = ((UNIT*)(weapon[i].target))->Pos+target_pos_on_unit-(Pos+data.pos[start_piece]);
									aim_dir = aim_dir + target_translation;
									aim_dir.Unit();
									}
								else
									aim_dir=cos(aiming[1]*TA2RAD)*(cos(aiming[0]*TA2RAD+Angle.y*DEG2RAD)*I+sin(aiming[0]*TA2RAD+Angle.y*DEG2RAD)*J)+sin(aiming[1]*TA2RAD)*IJ;
								switch( i )
								{
								case 0:
									launch_script(get_script_index(SCRIPT_AimPrimary),2,aiming);	break;
								case 1:
									launch_script(get_script_index(SCRIPT_AimSecondary),2,aiming);	break;
								case 2:
									launch_script(get_script_index(SCRIPT_AimTertiary),2,aiming);	break;
								};
								}
							else
								switch( i )
								{
								case 0:
									launch_script(get_script_index(SCRIPT_AimPrimary));	break;
								case 1:
									launch_script(get_script_index(SCRIPT_AimSecondary));	break;
								case 2:
									launch_script(get_script_index(SCRIPT_AimTertiary));	break;
								};
							weapon[i].time = 0.0f;
							weapon[i].state = WEAPON_FLAG_SHOOT;									// (puis) on lui demande de tirer / tell it to fire
							}
						}
					}
				else {
					launch_script(get_script_index(SCRIPT_TargetCleared));
					weapon[i].state = WEAPON_FLAG_IDLE;
					weapon[i].data = -1;
					}
				break;
			case WEAPON_FLAG_SHOOT:											// Tire sur une unité / fire!
				if( weapon[i].target == NULL || (( weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON && ((WEAPON*)(weapon[i].target))->weapon_id!=-1)
				|| (( weapon[i].state & WEAPON_FLAG_WEAPON ) != WEAPON_FLAG_WEAPON && (((UNIT*)(weapon[i].target))->flags&1))) {
					if( weapon[i].burst > 0 && weapon[i].delay <= unit_manager.unit_type[type_id].weapon[ i ]->burstrate )		break;
					int query_id=-1;
					int Aim_script = -1;
					int Fire_script = -1;
					switch( i )
					{
					case 0:
						query_id=get_script_index(SCRIPT_QueryPrimary);
						Aim_script=SCRIPT_AimPrimary;
						Fire_script=SCRIPT_FirePrimary;
						break;
					case 1:
						query_id=get_script_index(SCRIPT_QuerySecondary);
						Aim_script=SCRIPT_AimSecondary;
						Fire_script=SCRIPT_FireSecondary;
						break;
					case 2:
						query_id=get_script_index(SCRIPT_QueryTertiary);
						Aim_script=SCRIPT_AimTertiary;
						Fire_script=SCRIPT_FireTertiary;
						break;
					};
					if(!is_running(get_script_index(Aim_script))) {
						if((players.metal[owner_id]<unit_manager.unit_type[type_id].weapon[ i ]->metalpershot
						|| players.energy[owner_id]<unit_manager.unit_type[type_id].weapon[ i ]->energypershot)
						&& !unit_manager.unit_type[type_id].weapon[ i ]->stockpile) {
							weapon[i].state = WEAPON_FLAG_AIM;		// Pas assez d'énergie pour tirer / not enough energy to fire
							weapon[i].data = -1;
							break;
							}
						if(unit_manager.unit_type[type_id].weapon[ i ]->stockpile && weapon[i].stock<=0) {
							weapon[i].state = WEAPON_FLAG_AIM;		// Plus rien pour tirer / nothing to fire
							weapon[i].data = -1;
							break;
							}
						else if( unit_manager.unit_type[type_id].weapon[ i ]->stockpile )
							weapon[i].stock--;
						else {													// We use energy and metal only for weapons with no prebuilt ammo
							players.c_metal[owner_id] -= unit_manager.unit_type[type_id].weapon[ i ]->metalpershot;
							players.c_energy[owner_id] -= unit_manager.unit_type[type_id].weapon[ i ]->energypershot;
							}
						run_script_function( map, get_script_index(Fire_script) );			// Run the script that tell us from where to shoot
						if(unit_manager.unit_type[type_id].weapon[ i ]->soundstart)	sound_manager->PlaySound( unit_manager.unit_type[type_id].weapon[ i ]->soundstart , &Pos );
						if( script_val->size() <= query_id )
							script_val->resize( query_id + 1 );
						int start_piece = (*script_val)[query_id];
						if(start_piece>=0 && start_piece<data.nb_piece) {
							compute_model_coord();
							VECTOR Dir = data.dir[start_piece];
							if(Dir.x==0.0f && Dir.y==0.0f && Dir.z==0.0f) {
								if(unit_manager.unit_type[type_id].weapon[ i ]->vlaunch) {
									Dir.x=0.0f;
									Dir.y=1.0f;
									Dir.z=0.0f;
									}
								else {
									Dir = aim_dir;
									if(unit_manager.unit_type[type_id].weapon[ i ]->lineofsight && !unit_manager.unit_type[type_id].weapon[ i ]->turret) {
										if( weapon[i].target == NULL )
											Dir = weapon[i].target_pos-(Pos+data.pos[start_piece]);
										else {
											if( weapon[i].state & WEAPON_FLAG_WEAPON )
												Dir = ((WEAPON*)(weapon[i].target))->Pos-(Pos+data.pos[start_piece]);
											else
												Dir = ((UNIT*)(weapon[i].target))->Pos+((UNIT*)(weapon[i].target))->model->center-(Pos+data.pos[start_piece]);
											}
										Dir.Unit();
										}
									}
								}
							if( weapon[i].target == NULL )
								shoot(-1,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos );
							else {
								if( weapon[i].state & WEAPON_FLAG_WEAPON )
									shoot(((WEAPON*)(weapon[i].target))->idx,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos);
								else
									shoot(((UNIT*)(weapon[i].target))->idx,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos);
								}
							weapon[i].burst++;
							if(weapon[i].burst>=unit_manager.unit_type[type_id].weapon[i]->burst)
								weapon[i].burst=0;
							weapon[i].delay=0.0f;
							}
						if( unit_manager.unit_type[type_id].weapon[ i ]->commandfire && !unit_manager.unit_type[type_id].weapon[ i ]->dropped ) {		// Shoot only once
							weapon[i].state = WEAPON_FLAG_IDLE;
							weapon[i].data = -1;
							if( mission != NULL )
								mission->flags |= MISSION_FLAG_COMMAND_FIRED;
							break;
							}
						if( weapon[i].target != NULL && (weapon[i].state & WEAPON_FLAG_WEAPON)!=WEAPON_FLAG_WEAPON && ((UNIT*)(weapon[i].target))->hp>0) {				// La cible est-elle détruite ?? / is target destroyed ??
							if(weapon[i].burst==0) {
								weapon[i].state = WEAPON_FLAG_AIM;
								weapon[i].data = -1;
								weapon[i].time = 0.0f;
								}
							}
						else {
							launch_script(get_script_index(SCRIPT_TargetCleared));
							weapon[i].state = WEAPON_FLAG_IDLE;
							weapon[i].data = -1;
							}
						}
					}
				else {
					launch_script(get_script_index(SCRIPT_TargetCleared) );
					weapon[i].state = WEAPON_FLAG_IDLE;
					weapon[i].data = -1;
					}
				break;
			};
			}

		//---------------------------- Beginning of mission execution code --------------------------------------

		if( mission == NULL )	was_moving = false;

		if(mission) {
				mission->time+=dt;
				last_path_refresh += dt;

				//----------------------------------- Beginning of moving code ------------------------------------

				if( (mission->flags & MISSION_FLAG_MOVE) && unit_manager.unit_type[type_id].canmove && unit_manager.unit_type[type_id].BMcode ) {
					if( !was_moving ) {
						if(unit_manager.unit_type[type_id].canfly)
							activate();
						launch_script(get_script_index(SCRIPT_MotionControl));
						launch_script(get_script_index(SCRIPT_startmoving));
						if(nb_attached==0)
							launch_script(get_script_index(SCRIPT_MoveRate1));		// For the armatlas
						else
							launch_script(get_script_index(SCRIPT_MoveRate2));
						was_moving = true;
						}
					VECTOR J,I,K;
					K.x = 0.0f;
					K.y = 1.0f;
					K.z = 0.0f;
					VECTOR Target = mission->target;
					if( mission->path && ( !(mission->flags & MISSION_FLAG_REFRESH_PATH) || last_path_refresh < 5.0f ) )
						Target = mission->path->Pos;
					else {														// Look for a path to the target
						if( mission->path ) {		// If we want to refresh the path
							Target = mission->target;//mission->path->Pos;
							destroy_path( mission->path );
							mission->path = NULL;
							}
						mission->flags &= ~MISSION_FLAG_REFRESH_PATH;
						float dist = (Target-Pos).Sq();
						if( ( mission->move_data <= 0 && dist > 100.0f ) || ( ( mission->move_data * mission->move_data << 6 ) < dist ) ) {
							if( !requesting_pathfinder && last_path_refresh >= 5.0f ) {
								requesting_pathfinder = true;
								units.requests[ owner_id ].push_back( idx );
								}
							if( path_exec[ owner_id ] < MAX_PATH_EXEC && last_path_refresh >= 5.0f && !units.requests[ owner_id ].empty() && idx == units.requests[ owner_id ].front() ) {
								units.requests[ owner_id ].pop_front();
								requesting_pathfinder = false;

								path_exec[ owner_id ]++;
								move_target_computed = mission->target;
								last_path_refresh = 0.0f;
								if(unit_manager.unit_type[type_id].canfly) {
									if(mission->move_data<=0)
										mission->path = direct_path(mission->target);
									else {
										VECTOR Dir = mission->target-Pos;
										Dir.Unit();
										mission->path = direct_path(mission->target-(mission->move_data<<3)*Dir);
										}
									}
								else {
									float dh_max = unit_manager.unit_type[type_id].MaxSlope * H_DIV;
									float h_min = unit_manager.unit_type[type_id].canhover ? -100.0f : map->sealvl - unit_manager.unit_type[type_id].MaxWaterDepth * H_DIV;
									float h_max = map->sealvl - unit_manager.unit_type[type_id].MinWaterDepth * H_DIV;
									float hover_h = unit_manager.unit_type[type_id].canhover ? map->sealvl : -100.0f;
									if(mission->move_data <= 0)
										mission->path = find_path(map->map_data,map->h_map,map->path,map->map_w,map->map_h,map->bloc_w<<1,map->bloc_h<<1,
																dh_max, h_min, h_max, Pos, mission->target, unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx, 0, hover_h );
									else
										mission->path = find_path(map->map_data,map->h_map,map->path,map->map_w,map->map_h,map->bloc_w<<1,map->bloc_h<<1,
																dh_max, h_min, h_max, Pos, mission->target, unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx, mission->move_data, hover_h );

									if( mission->path == NULL ) {
										bool place_is_empty = map->check_rect( cur_px-(unit_manager.unit_type[type_id].FootprintX>>1), cur_py-(unit_manager.unit_type[type_id].FootprintZ>>1), unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx);
										if( !place_is_empty ) {
											Console->AddEntry("Unit is blocked!! (3)");
											mission->flags &= ~MISSION_FLAG_MOVE;
											}
										else
											mission->flags |= MISSION_FLAG_REFRESH_PATH;			// Retry later
										launch_script(get_script_index(SCRIPT_StopMoving));
										was_moving = false;
										}

									if( mission->path == NULL )					// Can't find a path to get where it has been ordered to go
										play_sound( "cant1" );
									}
								if( mission->path )			// Update required data
									Target = mission->path->Pos;
								}
							}
						else
							stop_moving();
						}
					if( mission->path ) {				// If we have a path, follow it
						if( (mission->target - move_target_computed).Sq() >= 10000.0f )			// Follow the target above all...
							mission->flags |= MISSION_FLAG_REFRESH_PATH;
						J = Target - Pos;
						J.y = 0.0f;
						float dist = J.Norm();
						if((dist > mission->last_d && dist < 15.0f) || mission->path == NULL) {
							if(mission->path) {
								float dh_max = unit_manager.unit_type[type_id].MaxSlope * H_DIV;
								float h_min = unit_manager.unit_type[type_id].canhover ? -100.0f : map->sealvl - unit_manager.unit_type[type_id].MaxWaterDepth * H_DIV;
								float h_max = map->sealvl - unit_manager.unit_type[type_id].MinWaterDepth * H_DIV;
								float hover_h = unit_manager.unit_type[type_id].canhover ? map->sealvl : -100.0f;
								mission->path = next_node(mission->path, map->map_data, map->h_map, map->bloc_w_db, map->bloc_h_db, dh_max, h_min, h_max, unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx, hover_h );
								}
							mission->last_d = 9999999.0f;
							if(mission->path == NULL) {		// End of path reached
								J = move_target_computed - Pos;
								J.y = 0.0f;
								if( J.Sq() <= 16384.0f || flying ) {
									if( !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) && (mission == NULL || mission->mission != MISSION_PATROL ) )
										play_sound( "arrived1" );
									mission->flags &= ~MISSION_FLAG_MOVE;
									}
								else										// We are not where we are supposed to be !!
									mission->flags |= MISSION_FLAG_REFRESH_PATH;
								if( !( unit_manager.unit_type[ type_id ].canfly && nb_attached > 0 ) ) {		// Once charged with units the Atlas cannot land
									launch_script(get_script_index(SCRIPT_StopMoving));
									was_moving = false;
									}
								if( !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) )
									V.x = V.y = V.z = 0.0f;		// Stop unit's movement
								}
							}
						else
							mission->last_d = dist;
						if( mission->flags & MISSION_FLAG_MOVE ) {			// Are we still moving ??
							if( dist > 0.0f )
								J = 1.0f / dist * J;

							b_TargetAngle = true;
							f_TargetAngle = acos( J.z ) * RAD2DEG;
							if( J.x < 0.0f ) f_TargetAngle = -f_TargetAngle;

							if( Angle.y - f_TargetAngle >= 360.0f )	f_TargetAngle += 360.0f;
							else if( Angle.y - f_TargetAngle <= -360.0f )	f_TargetAngle -= 360.0f;

							J.z = cos(Angle.y*DEG2RAD);
							J.x = sin(Angle.y*DEG2RAD);
							J.y = 0.0f;
							I.z = -J.x;
							I.x = J.z;
							I.y = 0.0f;
							V = (V%K)*K + (V%J)*J;
							if(!(dist < 15.0f && fabs( Angle.y - f_TargetAngle ) >= 1.0f)) {
								if( fabs( Angle.y - f_TargetAngle ) >= 45.0f ) {
									if( J % V > 0.0f && V.Norm() > unit_manager.unit_type[type_id].BrakeRate * dt )
										V = V - ( fabs( Angle.y - f_TargetAngle ) - 45.0f ) / 135.0f * unit_manager.unit_type[type_id].BrakeRate * dt * J;
									}
								else {
									float speed = V.Norm();
									float time_to_stop = speed / unit_manager.unit_type[type_id].BrakeRate;
									float min_dist = time_to_stop * (speed-unit_manager.unit_type[type_id].BrakeRate*0.5f*time_to_stop);
									if(min_dist>=dist && !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE)
									&& ( mission->next == NULL || ( mission->next->mission != MISSION_MOVE && mission->next->mission != MISSION_PATROL ) ) )	// Brake if needed
										V = V-unit_manager.unit_type[type_id].BrakeRate*dt*J;
									else if( speed < unit_manager.unit_type[type_id].MaxVelocity )
										V = V + unit_manager.unit_type[type_id].Acceleration*dt*J;
									else
										V = unit_manager.unit_type[type_id].MaxVelocity / speed * V;
									}
								}
							else {
								float speed = V.Norm();
								if( speed > unit_manager.unit_type[type_id].MaxVelocity )
									V = unit_manager.unit_type[type_id].MaxVelocity / speed * V;
								}
							}
						}

					NPos = Pos + dt*V;			// Check if the unit can go where V brings it
					if( was_locked ) {				// Random move to solve the unit lock problem
						NPos.x += (rand_from_table() % 201) * 0.01f - 1.0f;
						NPos.z += (rand_from_table() % 201) * 0.01f - 1.0f;
						was_locked = false;
						}
					n_px = ((int)(NPos.x)+map->map_w_d)>>3;
					n_py = ((int)(NPos.z)+map->map_h_d)>>3;
					precomputed_position = true;
					if( !flying ) {
						if( n_px != cur_px || n_py != cur_py ) {			// has something changed ??
							bool place_is_empty = can_be_there( n_px, n_py, map, type_id, owner_id, idx );
							if( !(flags & 64) && !place_is_empty) {
								mission->flags |= MISSION_FLAG_REFRESH_PATH;			// Refresh path because this shouldn't happen unless
								if(!unit_manager.unit_type[type_id].canfly) {			// obstacles have moved
									was_locked = true;
									// Check some basic solutions first
									if( fabs( V.x ) > 0.0f
									&& can_be_there( n_px, cur_py, map, type_id, owner_id, idx )) {
										V.x = (V.x < 0.0f ? -sqrt( sq(V.z) + sq(V.x) ) : sqrt( sq(V.z) + sq(V.x) ) );
										V.z = 0.0f;
										NPos.z = Pos.z;
										n_py = cur_py;
										}
									else if( fabs( V.z ) > 0.0f
									&& can_be_there( cur_px, n_py, map, type_id, owner_id, idx )) {
										V.z = (V.z < 0.0f ? -sqrt( sq(V.z) + sq(V.x) ) : sqrt( sq(V.z) + sq(V.x) ) );
										V.x = 0.0f;
										NPos.x = Pos.x;
										n_px = cur_px;
										}
									else if( can_be_there( cur_px, cur_py, map, type_id, owner_id, idx )) {
										V.x = V.y = V.z = 0.0f;		// Don't move since we can't
										NPos = Pos;
										n_px = cur_px;
										n_py = cur_py;
										mission->flags |= MISSION_FLAG_MOVE;
										if( fabs( Angle.y - f_TargetAngle ) <= 0.1f || !b_TargetAngle ) {		// Don't prevent unit from rotating!!
											if( mission->path )
												destroy_path(mission->path);
											mission->path = NULL;
											}
										}
									else
										Console->AddEntry("Unit is blocked!! (1)");
									}
								else if( !flying ) {
									if(Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d) {
										VECTOR target = Pos;
										if(target.x < -map->map_w_d+256)
											target.x = -map->map_w_d+256;
										else if(target.x > map->map_w_d-256)
											target.x = map->map_w_d-256;
										if(target.z < -map->map_h_d+256)
											target.z = -map->map_h_d+256;
										else if(target.z > map->map_h_d-256)
											target.z = map->map_h_d-256;
										next_mission();
										add_mission(MISSION_MOVE,&target,true,0,NULL,NULL,0,1);		// Stay on map
										}
									else if( !can_be_there( cur_px, cur_py, map, type_id, owner_id, idx )) {
										NPos = Pos;
										n_px = cur_px;
										n_py = cur_py;
										VECTOR target = Pos;
										target.x += (rand_from_table()&0x1F)-16;		// Look for a place to land
										target.z += (rand_from_table()&0x1F)-16;
										mission->flags |= MISSION_FLAG_MOVE;
										if( mission->path )
											destroy_path(mission->path);
										mission->path = direct_path( target );
										}
									}
								}
							else if( !(flags & 64) && unit_manager.unit_type[type_id].canfly && ( mission == NULL ||
								( mission->mission != MISSION_MOVE && mission->mission != MISSION_ATTACK ) ) )
								flags |= 64;
							}
						else {
							bool place_is_empty = map->check_rect(n_px-(unit_manager.unit_type[type_id].FootprintX>>1),n_py-(unit_manager.unit_type[type_id].FootprintZ>>1),unit_manager.unit_type[type_id].FootprintX,unit_manager.unit_type[type_id].FootprintZ,idx);
							if( !place_is_empty ) {
								clear_from_map();
								Console->AddEntry("Unit is blocked!! (2) -> probably spawned on something");
								}
							}
						}
					}
				else {
					was_moving = false;
					requesting_pathfinder = false;
					}

				if( flying ) {						// Force planes to stay on map
					if(Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d) {
						if( Pos.x < -map->map_w_d )
							V.x += dt * ( -map->map_w_d - Pos.x ) * 0.1f;
						else if( Pos.x > map->map_w_d )
							V.x -= dt * ( Pos.x - map->map_w_d ) * 0.1f;
						if( Pos.z < -map->map_h_d )
							V.z += dt * ( -map->map_h_d - Pos.z ) * 0.1f;
						else if( Pos.z > map->map_h_d )
							V.z -= dt * ( Pos.z - map->map_h_d ) * 0.1f;
						float speed = V.Norm();
						if( speed > unit_manager.unit_type[ type_id ].MaxVelocity && speed > 0.0f ) {
							V = unit_manager.unit_type[ type_id ].MaxVelocity / speed * V;
							speed = unit_manager.unit_type[ type_id ].MaxVelocity;
							}
						if( speed > 0.0f ) {
							Angle.y = acos( V.z / speed ) * RAD2DEG;
							if( V.x < 0.0f )
								Angle.y = -Angle.y;
							}
						}
					}

				//----------------------------------- End of moving code ------------------------------------

				switch(mission->mission)						// Commandes générales / General orders
				{
				case MISSION_STANDBY_MINE:		// Don't even try to do something else, the unit must die !!
					if( self_destruct < 0.0f ) {
						int dx = unit_manager.unit_type[type_id].SightDistance+(int)(h+0.5f)>>3;
						int enemy_idx=-1;
						int sx=rand_from_table()&1;
						int sy=rand_from_table()&1;
						byte mask=1<<owner_id;
						for(int y=cur_py-dx+sy;y<=cur_py+dx;y+=2) {
							if(y>=0 && y<map->bloc_h_db-1)
								for(int x=cur_px-dx+sx;x<=cur_px+dx;x+=2)
									if(x>=0 && x<map->bloc_w_db-1 ) {
										int cur_idx = map->map_data[y][x].unit_idx;
										if(cur_idx>=0 && cur_idx<units.max_unit && (units.unit[cur_idx].flags & 1) && units.unit[cur_idx].owner_id != owner_id
										&& unit_manager.unit_type[units.unit[cur_idx].type_id].ShootMe ) {		// This unit is on the sight_map since dx = sightdistance !!
											enemy_idx = cur_idx;
											break;
											}
										}
							if(enemy_idx>=0)	break;
							sx ^= 1;
							}
						if( enemy_idx >= 0 )					// Annihilate it !!!
							toggle_self_destruct();
						}
					break;
				case MISSION_UNLOAD:
					if( nb_attached > 0 ) {
						VECTOR Dir = mission->target-Pos;
						Dir.y=0.0f;
						float dist = Dir.Sq();
						int maxdist=0;
						if(unit_manager.unit_type[type_id].TransMaxUnits==1)		// Code for units like the arm atlas
							maxdist=3;
						else
							maxdist=unit_manager.unit_type[type_id].SightDistance;
						if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode ) {	// Si l'unité est trop loin du chantier
							c_time = 0.0f;
							mission->flags |= MISSION_FLAG_MOVE;
							mission->move_data = maxdist*8/80;
							}
						else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
							if(mission->last_d>=0.0f) {
								if(unit_manager.unit_type[type_id].TransMaxUnits==1) {		// Code for units like the arm atlas
									if( attached_list[0] >= 0 && attached_list[0] < units.max_unit				// Check we can do that
									&& units.unit[ attached_list[0] ].flags && can_be_built( Pos, map, units.unit[ attached_list[0] ].type_id, owner_id ) ) {
										launch_script(get_script_index(SCRIPT_EndTransport));

										UNIT *target_unit = &(units.unit[ attached_list[0] ]);
										target_unit->attached = false;
										target_unit->hidden = false;
										nb_attached = 0;
										LeaveCS();
										target_unit->draw_on_map();
										EnterCS();
										}
									else if( attached_list[0] < 0 || attached_list[0] >= units.max_unit
									|| units.unit[ attached_list[0] ].flags == 0 )
										nb_attached = 0;

									next_mission();
									}
								else {
									if( attached_list[ nb_attached - 1 ] >= 0 && attached_list[ nb_attached - 1 ] < units.max_unit				// Check we can do that
									&& units.unit[ attached_list[ nb_attached - 1 ] ].flags && can_be_built( mission->target, map, units.unit[ attached_list[ nb_attached - 1 ] ].type_id, owner_id ) ) {
										int idx = attached_list[ nb_attached - 1 ];
										int param[]= { idx, PACKXZ( mission->target.x*2.0f+map->map_w, mission->target.z*2.0f+map->map_h ) };
										launch_script(get_script_index(SCRIPT_TransportDrop),2,param);
										}
									else if( attached_list[ nb_attached - 1 ] < 0 || attached_list[ nb_attached - 1 ] >= units.max_unit
									|| units.unit[ attached_list[ nb_attached - 1 ] ].flags == 0 )
										nb_attached--;
									}
								mission->last_d=-1.0f;
								}
							else {
								if(!is_running(get_script_index(SCRIPT_TransportDrop)) && port[ BUSY ] == 0.0f )
									next_mission();
								}
							}
						}
					else
						next_mission();
					break;
				case MISSION_LOAD:
					if(mission->p!=NULL) {
						UNIT *target_unit=(UNIT*) mission->p;
						if( !(target_unit->flags & 1) ) {
							next_mission();
							break;
							}
						VECTOR Dir=target_unit->Pos-Pos;
						Dir.y=0.0f;
						mission->target=target_unit->Pos;
						float dist = Dir.Sq();
						int maxdist=0;
						if(unit_manager.unit_type[type_id].TransMaxUnits==1)		// Code for units like the arm atlas
							maxdist=3;
						else
							maxdist=unit_manager.unit_type[type_id].SightDistance;
						if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
							c_time = 0.0f;
							mission->flags |= MISSION_FLAG_MOVE;
							mission->move_data = maxdist*8/80;
							}
						else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
							if(mission->last_d>=0.0f) {
								if(unit_manager.unit_type[type_id].TransMaxUnits==1) {		// Code for units like the arm atlas
									if(nb_attached==0) {
//										int param[] = { (int)((Pos.y - target_unit->Pos.y - target_unit->model->top)*2.0f) << 16 };
										int param[] = { (int)((Pos.y - target_unit->Pos.y)*2.0f) << 16 };
										launch_script(get_script_index(SCRIPT_BeginTransport),1,param);
										run_script_function( map, get_script_index(SCRIPT_QueryTransport),1,param);
										target_unit->attached = true;
										link_list[nb_attached] = param[0];
										target_unit->hidden = param[0] < 0;
										attached_list[nb_attached++] = target_unit->idx;
										target_unit->clear_from_map();
										}
									next_mission();
									}
								else {
									if(nb_attached>=unit_manager.unit_type[type_id].TransportCapacity) {
										next_mission();
										break;
										}
									int param[]= { target_unit->idx };
									launch_script(get_script_index(SCRIPT_TransportPickup),1,param);
									}
								mission->last_d=-1.0f;
								}
							else {
								if(!is_running(get_script_index(SCRIPT_TransportPickup)) && port[ BUSY ] == 0.0f )
									next_mission();
								}
							}
						}
					else
						next_mission();
					break;
				case MISSION_REVIVE:
				case MISSION_RECLAIM:
					if(mission->p!=NULL)	{		// Récupère une unité
						UNIT *target_unit=(UNIT*) mission->p;
						if( (target_unit->flags & 1) ) {
							VECTOR Dir=target_unit->Pos-Pos;
							Dir.y=0.0f;
							mission->target=target_unit->Pos;
							float dist=Dir.Sq();
							int maxdist = (int)(unit_manager.unit_type[type_id].BuildDistance);
							if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
								c_time=0.0f;
								mission->flags |= MISSION_FLAG_MOVE | MISSION_FLAG_REFRESH_PATH;
								mission->move_data = maxdist*7/80;
								mission->last_d = 0.0f;
								}
							else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
								if(mission->last_d>=0.0f) {
									int angle = (int)( acos( Dir.z / Dir.Norm() ) * RAD2DEG );
									if( Dir.x < 0.0f )
										angle = -angle;
									angle -= (int)Angle.y;
									if(angle>180)	angle-=360;
									if(angle<-180)	angle+=360;
									int param[] = { (int)(angle*DEG2TA) };
									launch_script(get_script_index(SCRIPT_startbuilding), 1, param);
									launch_script(get_script_index(SCRIPT_go));
									mission->last_d=-1.0f;
									}

								if( unit_manager.unit_type[type_id].BMcode && port[ INBUILDSTANCE ] != 0.0f ) {
									play_sound( "working" );
									// Récupère l'unité
									float recup=dt*unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].MaxDamage/unit_manager.unit_type[target_unit->type_id].BuildTime;
									if(recup>target_unit->hp)	recup=target_unit->hp;
									target_unit->hp-=recup;
									if(dt>0.0f)
										metal_prod+=recup*unit_manager.unit_type[target_unit->type_id].BuildCostMetal/(dt*unit_manager.unit_type[target_unit->type_id].MaxDamage);
									if(target_unit->hp<=0.0f) {		// Work done
										launch_script(get_script_index(SCRIPT_stopbuilding));
										launch_script(get_script_index(SCRIPT_stop));
										target_unit->flags|=0x10;			// This unit is being reclaimed it doesn't explode!
										next_mission();
										}
									}
								}
							}
						else
							next_mission();
						}
					else if(mission->data>=0 && mission->data<features.max_features && features.feature[mission->data].type>=0)	{	// Récupère un élément du décors/une carcasse
						VECTOR Dir=features.feature[mission->data].Pos-Pos;
						Dir.y=0.0f;
						mission->target=features.feature[mission->data].Pos;
						float dist=Dir.Sq();
						int maxdist = mission->mission == MISSION_REVIVE ? (int)(unit_manager.unit_type[type_id].SightDistance) : (int)(unit_manager.unit_type[type_id].BuildDistance);
						if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
							c_time = 0.0f;
							mission->flags |= MISSION_FLAG_MOVE | MISSION_FLAG_REFRESH_PATH;
							mission->move_data = maxdist*7/80;
							mission->last_d=0.0f;
							}
						else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
							if(mission->last_d>=0.0f) {
								int angle = (int)( acos( Dir.z / Dir.Norm() ) * RAD2DEG );
								if( Dir.x < 0.0f )
									angle = -angle;
								angle -= (int)Angle.y;
								if(angle>180)	angle-=360;
								if(angle<-180)	angle+=360;
								int param[] = { (int)(angle*DEG2TA) };
								launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
								launch_script(get_script_index(SCRIPT_go));
								mission->last_d=-1.0f;
								}
							if( unit_manager.unit_type[type_id].BMcode && port[ INBUILDSTANCE ] != 0 ) {
								play_sound( "working" );
											// Récupère l'objet
								float recup=dt*unit_manager.unit_type[type_id].WorkerTime;
								if(recup>features.feature[mission->data].hp)	recup=features.feature[mission->data].hp;
								features.feature[mission->data].hp-=recup;
								if( dt > 0.0f && mission->mission == MISSION_RECLAIM ) {
									metal_prod+=recup*feature_manager.feature[features.feature[mission->data].type].metal/(dt*feature_manager.feature[features.feature[mission->data].type].damage);
									energy_prod+=recup*feature_manager.feature[features.feature[mission->data].type].energy/(dt*feature_manager.feature[features.feature[mission->data].type].damage);
									}
								if( features.feature[mission->data].hp <= 0.0f ) {		// Travail terminé
									int x=((int)(features.feature[mission->data].Pos.x)-8+map->map_w_d)>>3;		// Efface l'objet
									int y=((int)(features.feature[mission->data].Pos.z)-8+map->map_h_d)>>3;
									map->rect(x-(feature_manager.feature[features.feature[mission->data].type].footprintx>>1),y-(feature_manager.feature[features.feature[mission->data].type].footprintz>>1),feature_manager.feature[features.feature[mission->data].type].footprintx,feature_manager.feature[features.feature[mission->data].type].footprintz,-1);
									map->map_data[y][x].stuff=-1;

									if( mission->mission == MISSION_REVIVE
									&& feature_manager.feature[features.feature[mission->data].type].name ) {			// Creates the corresponding unit
										bool success = false;
										String wreckage_name = feature_manager.feature[features.feature[mission->data].type].name;
										wreckage_name = wreckage_name.substr( 0, wreckage_name.length() - 5 );		// Remove the _dead/_heap suffix

										int wreckage_type_id = unit_manager.get_unit_index( wreckage_name.c_str() );
										VECTOR obj_pos = features.feature[mission->data].Pos;
										float obj_angle = features.feature[mission->data].angle;
										features.delete_feature(mission->data);			// Supprime l'objet

										if( wreckage_type_id >= 0 ) {
											LeaveCS();
											UNIT *unit_p = (UNIT*) create_unit( wreckage_type_id, owner_id, obj_pos, map );

											if( unit_p ) {
												unit_p->Lock();

												unit_p->Angle.y = obj_angle;
												unit_p->hp = 0.01f;					// Need to be repaired :P
												unit_p->build_percent_left = 0.0f;	// It's finished ...
												unit_p->draw_on_map();

												unit_p->UnLock();
												}
											EnterCS();

											if( unit_p ) {
												mission->mission = MISSION_REPAIR;		// Now let's repair what we've resurrected
												mission->p = unit_p;
												mission->data = 1;
												success = true;
												}
											}
										if( !success ) {
											play_sound( "cant1" );
											launch_script(get_script_index(SCRIPT_stopbuilding));
											launch_script(get_script_index(SCRIPT_stop));
											next_mission();
											}
										}
									else {
										features.delete_feature(mission->data);			// Supprime l'objet
										launch_script(get_script_index(SCRIPT_stopbuilding));
										launch_script(get_script_index(SCRIPT_stop));
										next_mission();
										}
									}
								}
							}
						}
					else
						next_mission();
					break;
				case MISSION_GUARD:
					if(jump_commands)	break;
					if(mission->p!=NULL && (((UNIT*)mission->p)->flags & 1) && ((UNIT*)mission->p)->owner_id==owner_id) {		// On ne défend pas n'importe quoi
						if(unit_manager.unit_type[type_id].Builder) {
							if(((UNIT*)mission->p)->build_percent_left > 0.0f || ((UNIT*)mission->p)->hp<unit_manager.unit_type[((UNIT*)mission->p)->type_id].MaxDamage) {		// Répare l'unité
								add_mission(MISSION_REPAIR,&((UNIT*)mission->p)->Pos,true,0,((UNIT*)mission->p),NULL);
								break;
								}
							else if(((UNIT*)mission->p)->mission!=NULL && (((UNIT*)mission->p)->mission->mission==MISSION_BUILD_2 || ((UNIT*)mission->p)->mission->mission==MISSION_REPAIR)) {		// L'aide à construire
								add_mission(MISSION_REPAIR,&((UNIT*)mission->p)->mission->target,true,0,((UNIT*)mission->p)->mission->p,NULL);
								break;
								}
							}
						if(unit_manager.unit_type[type_id].canattack) {
							if(((UNIT*)mission->p)->mission!=NULL && ((UNIT*)mission->p)->mission->mission==MISSION_ATTACK) {		// L'aide à attaquer
								add_mission(MISSION_ATTACK,&((UNIT*)mission->p)->mission->target,true,0,((UNIT*)mission->p)->mission->p,NULL);
								break;
								}
							}
						if(((VECTOR)(Pos-((UNIT*)mission->p)->Pos)).Sq()>=25600.0f) {			// On reste assez près
							mission->flags |= MISSION_FLAG_MOVE | MISSION_FLAG_REFRESH_PATH;
							mission->move_data = 10;
							mission->target = ((UNIT*)mission->p)->Pos;
							c_time=0.0f;
							break;
							}
						}
					else
						next_mission();
					break;
				case MISSION_PATROL:					// Mode patrouille
					{
						if( mission->next == NULL )
							add_mission(MISSION_PATROL,&Pos,false,0,NULL,NULL,MISSION_FLAG_CAN_ATTACK,0,0);	// Retour à la case départ après l'éxécution de tous les ordres / back to beginning

						mission->flags |= MISSION_FLAG_CAN_ATTACK;
						if( unit_manager.unit_type[ type_id ].canfly )
							mission->flags |= MISSION_FLAG_DONT_STOP_MOVE;
						if( (mission->flags & MISSION_FLAG_MOVE) == 0 ) {			// Monitor the moving process
							if( !unit_manager.unit_type[ type_id ].canfly || ( mission->next == NULL || ( mission->next != NULL && mission->mission != MISSION_PATROL ) ) ) {
								V.x = V.y = V.z = 0.0f;			// Stop the unit
								if( precomputed_position ) {
									NPos = Pos;
									n_px = cur_px;
									n_py = cur_py;
									}
								}

							MISSION *cur = mission;					// Make a copy of current list to make it loop 8)
							while( cur->next )	cur = cur->next;
							cur->next = (MISSION*) malloc(sizeof( MISSION ));
							*(cur->next) = *mission;
							cur->next->path = NULL;
							cur->next->next = NULL;
							cur->next->flags |= MISSION_FLAG_MOVE;

							MISSION *cur_start = mission->next;
							while( cur_start != NULL && cur_start->mission != MISSION_PATROL ) {
								cur = cur_start;
								while( cur->next )	cur = cur->next;
								cur->next = (MISSION*) malloc(sizeof( MISSION ));
								*(cur->next) = *cur_start;
								cur->next->path = NULL;
								cur->next->next = NULL;
								cur_start = cur_start->next;
								}

							next_mission();
							}
					}
					break;
				case MISSION_STANDBY:
				case MISSION_VTOL_STANDBY:
					if(jump_commands)	break;
					if(mission->data>5) {
						if(mission->next)		// If there is a mission after this one
							next_mission();
						}
					else
						mission->data++;
					break;
				case MISSION_ATTACK:										// Attaque une unité / attack a unit
					{
						UNIT *target_unit = (mission->flags & MISSION_FLAG_TARGET_WEAPON) == MISSION_FLAG_TARGET_WEAPON ? NULL : (UNIT*) mission->p;
						WEAPON *target_weapon = (mission->flags & MISSION_FLAG_TARGET_WEAPON) == MISSION_FLAG_TARGET_WEAPON ? (WEAPON*) mission->p : NULL;
						if((target_unit!=NULL && (target_unit->flags&1)) || (target_weapon!=NULL && target_weapon->weapon_id!=-1) || (target_weapon==NULL && target_unit==NULL) ) {

							if( jump_commands && mission->data != 0
							&& unit_manager.unit_type[type_id].attackrunlength == 0 )	break;					// Just do basic checks every tick, and advanced ones when needed

							if( unit_manager.unit_type[type_id].weapon[0] == NULL
							&& unit_manager.unit_type[type_id].weapon[1] == NULL
							&& unit_manager.unit_type[type_id].weapon[2] == NULL
							&& !unit_manager.unit_type[type_id].kamikaze ) {		// Check if this units has weapons
								next_mission();		break;	}

							VECTOR Dir = target_unit==NULL ? (target_weapon == NULL ? mission->target-Pos : target_weapon->Pos-Pos) : target_unit->Pos-Pos;
							Dir.y = 0.0f;
							if( target_weapon || target_unit )
								mission->target = target_unit==NULL ? target_weapon->Pos : target_unit->Pos;
							float dist = Dir.Sq();
							int maxdist = 0;
							int mindist = 0xFFFFF;

							if( target_unit != NULL && unit_manager.unit_type[ target_unit->type_id ].checkCategory( unit_manager.unit_type[ type_id ].BadTargetCategory ) ) {
								next_mission();
								break;
								}

							for( byte i = 0 ; i < 3 ; i++ ) {
								if(unit_manager.unit_type[type_id].weapon[ i ]==NULL || unit_manager.unit_type[ type_id ].weapon[ i ]->interceptor)	continue;
								int cur_mindist;
								int cur_maxdist;
								bool allowed_to_fire = true;
								if(unit_manager.unit_type[type_id].attackrunlength>0) {
									if( Dir % V < 0.0f )	allowed_to_fire = false;
									float t = 2.0f/map->ota_data.gravity*fabs(Pos.y-mission->target.y);
									cur_mindist = (int)sqrt(t*V.Sq())-(unit_manager.unit_type[type_id].attackrunlength+1>>1);
									cur_maxdist = cur_mindist+(unit_manager.unit_type[type_id].attackrunlength);
									}
								else {
									cur_maxdist = unit_manager.unit_type[type_id].weapon[ i ]->range>>1;
									cur_mindist = 0;
									}
								if( maxdist < cur_maxdist )	maxdist = cur_maxdist;
								if( mindist > cur_mindist )	mindist = cur_mindist;
								if( allowed_to_fire && dist >= cur_mindist * cur_mindist && dist <= cur_maxdist * cur_maxdist && !unit_manager.unit_type[ type_id ].weapon[ i ]->interceptor ) {
									if( ( (weapon[i].state & 3) == WEAPON_FLAG_IDLE || ( (weapon[i].state & 3) != WEAPON_FLAG_IDLE && weapon[i].target != mission->p ) )
										&& ( target_unit == NULL || ( (!unit_manager.unit_type[type_id].weapon[ i ]->toairweapon
										   || ( unit_manager.unit_type[type_id].weapon[ i ]->toairweapon && target_unit->flying ) )
										&& !unit_manager.unit_type[ target_unit->type_id ].checkCategory( unit_manager.unit_type[type_id].w_badTargetCategory[i] ) ) )
										&& ( ((mission->flags & MISSION_FLAG_COMMAND_FIRE) && (unit_manager.unit_type[type_id].weapon[ i ]->commandfire || !unit_manager.unit_type[type_id].candgun) )
										   || (!(mission->flags & MISSION_FLAG_COMMAND_FIRE) && !unit_manager.unit_type[type_id].weapon[ i ]->commandfire)
										   || unit_manager.unit_type[type_id].weapon[ i ]->dropped ) ) {
										weapon[i].state = WEAPON_FLAG_AIM;
										weapon[i].target = mission->p;
										weapon[i].target_pos = mission->target;
										weapon[i].data = -1;
										if( mission->flags & MISSION_FLAG_TARGET_WEAPON )
											weapon[i].state |= WEAPON_FLAG_WEAPON;
										if( unit_manager.unit_type[type_id].weapon[ i ]->commandfire )
											weapon[i].state |= WEAPON_FLAG_COMMAND_FIRE;
										}
									}
								}

							if( unit_manager.unit_type[type_id].kamikaze && unit_manager.unit_type[type_id].kamikazedistance > maxdist )
								maxdist = unit_manager.unit_type[type_id].kamikazedistance;

							if( mindist > maxdist )	mindist = maxdist;
							
							mission->flags |= MISSION_FLAG_CAN_ATTACK;

							if( unit_manager.unit_type[type_id].kamikaze				// Kamikaze attack !!
							&& dist <= unit_manager.unit_type[type_id].kamikazedistance * unit_manager.unit_type[type_id].kamikazedistance
							&& self_destruct < 0.0f )
								self_destruct = 0.01f;

							if(dist>maxdist*maxdist || dist<mindist*mindist) {	// Si l'unité est trop loin de sa cible / if unit isn't where it should be
								if(!unit_manager.unit_type[type_id].canmove) {		// Bah là si on peut pas bouger faut changer de cible!! / need to change target
									next_mission();
									break;
									}
								else if( !unit_manager.unit_type[type_id].canfly || unit_manager.unit_type[type_id].hoverattack ) {
									c_time=0.0f;
									mission->flags |= MISSION_FLAG_MOVE;
									mission->move_data = maxdist*7/80;
									}
								}
							else if( mission->data == 0 ) {
								mission->data = 2;
								int param[] = { 0 };
								for( int i = 0 ; i < 3 ; i++ )
									if( unit_manager.unit_type[type_id].weapon[ i ] )
										param[ 0 ] = max( param[0], (int)( unit_manager.unit_type[type_id].weapon[ i ]->reloadtime * 1000.0f ) * max( 1, (int)unit_manager.unit_type[type_id].weapon[ i ]->burst ) );
								launch_script(get_script_index(SCRIPT_SetMaxReloadTime),1,param);
								}

							if( mission->flags & MISSION_FLAG_COMMAND_FIRED )
								next_mission();
							}
						else
							next_mission();
					}
					break;
				case MISSION_GUARD_NOMOVE:
					mission->flags |= MISSION_FLAG_CAN_ATTACK;
					mission->flags &= ~MISSION_FLAG_MOVE;
					if( mission->next )
						next_mission();
					break;
				case MISSION_STOP:											// Arrête tout ce qui était en cours / stop everything running
					if( jump_commands && mission->data != 0 )	break;
					if(mission->data>5) {
						if(mission->next) {
							next_mission();
							if(mission!=NULL && mission->mission==MISSION_STOP)		// Mode attente / wait mode
								mission->data=6;
							}
						}
					else {
						if(mission->data==0) {
							launch_script(get_script_index(SCRIPT_StopMoving));		// Arrête tout / stop everything
							launch_script(get_script_index(SCRIPT_stopbuilding));
							if( weapon[0].state || weapon[1].state || weapon[2].state )
								launch_script(get_script_index(SCRIPT_TargetCleared));
							for( int i = 0 ; i < 3 ; i++ ) {			// Stop weapons
								weapon[i].state = WEAPON_FLAG_IDLE;
								weapon[i].data = -1;
								}
							}
						mission->data++;
						}
					break;
				case MISSION_REPAIR:
					{
						UNIT *target_unit=(UNIT*) mission->p;
						if(target_unit!=NULL && (target_unit->flags & 1) && target_unit->build_percent_left == 0.0f) {
							if(target_unit->hp>=unit_manager.unit_type[target_unit->type_id].MaxDamage || !unit_manager.unit_type[type_id].BMcode) {
								if(unit_manager.unit_type[type_id].BMcode)
									target_unit->hp=unit_manager.unit_type[target_unit->type_id].MaxDamage;
								next_mission();
								}
							else {
								VECTOR Dir=target_unit->Pos-Pos;
								Dir.y=0.0f;
								mission->target=target_unit->Pos;
								float dist=Dir.Sq();
								int maxdist=(int)(unit_manager.unit_type[type_id].BuildDistance
									+ ( unit_manager.unit_type[target_unit->type_id].FootprintX + unit_manager.unit_type[target_unit->type_id].FootprintZ << 1 ) );
								if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
									mission->flags |= MISSION_FLAG_MOVE;
									mission->move_data = maxdist * 7 / 80;
									mission->data = 0;
									c_time = 0.0f;
									}
								else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
									if(mission->data==0) {
										mission->data=1;
										int angle = (int)( acos( Dir.z / Dir.Norm() ) * RAD2DEG );
										if( Dir.x < 0.0f )
											angle = -angle;
										angle -= (int)Angle.y;
										if(angle>180)	angle-=360;
										if(angle<-180)	angle+=360;
										int param[] = { (int)(angle*DEG2TA) };
										launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
										launch_script(get_script_index(SCRIPT_go));
										}

									if( port[ INBUILDSTANCE ] != 0.0f ) {
										float conso_energy=((float)(unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].BuildCostEnergy))/unit_manager.unit_type[target_unit->type_id].BuildTime;
										if( players.energy[owner_id] >= conso_energy * dt ) {
											energy_cons += conso_energy;
											target_unit->hp += dt*unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].MaxDamage/unit_manager.unit_type[target_unit->type_id].BuildTime;
											}
/*										float conso_metal=((float)(unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].BuildCostMetal))/unit_manager.unit_type[target_unit->type_id].BuildTime;
										float conso_energy=((float)(unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].BuildCostEnergy))/unit_manager.unit_type[target_unit->type_id].BuildTime;
										if(players.metal[owner_id]>=conso_metal*dt && players.energy[owner_id]>=conso_energy*dt) {
											metal_cons+=conso_metal;
											energy_cons+=conso_energy;
											target_unit->hp+=dt*unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].MaxDamage/unit_manager.unit_type[target_unit->type_id].BuildTime;
											}*/
										target_unit->built=true;
										}
									}
								}
							}
						else if(target_unit!=NULL && target_unit->flags) {
							VECTOR Dir=target_unit->Pos-Pos;
							Dir.y=0.0f;
							mission->target=target_unit->Pos;
							float dist=Dir.Sq();
							int maxdist=(int)(unit_manager.unit_type[type_id].BuildDistance
									+ ( unit_manager.unit_type[target_unit->type_id].FootprintX + unit_manager.unit_type[target_unit->type_id].FootprintZ << 1 ));
							if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
								c_time=0.0f;
								mission->flags |= MISSION_FLAG_MOVE;
								mission->move_data = maxdist*7/80;
								}
							else if( !(mission->flags & MISSION_FLAG_MOVE) ) {
								if( unit_manager.unit_type[type_id].BMcode ) {
									int angle = (int)( acos( Dir.z / Dir.Norm() ) * RAD2DEG );
									if( Dir.x < 0.0f )
										angle = -angle;
									angle -= (int)Angle.y;
									if(angle>180)	angle-=360;
									if(angle<-180)	angle+=360;
									int param[] = { (int)(angle*DEG2TA) };
									launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
									mission->mission = MISSION_BUILD_2;		// Change de type de mission
									}
								}
							}
					}
					break;
				case MISSION_BUILD_2:
					{
						UNIT *target_unit=(UNIT*) mission->p;
						if(target_unit->flags) {
							if(target_unit->build_percent_left <= 0.0f) {
								target_unit->build_percent_left = 0.0f;
								if(unit_manager.unit_type[target_unit->type_id].ActivateWhenBuilt ) {				// Activation automatique
									target_unit->port[ ACTIVATION ] = 0;
									target_unit->activate();
									}
								if(!unit_manager.unit_type[type_id].BMcode) {		// Ordre de se déplacer
									VECTOR target=Pos;
									target.z+=128.0f;
									target_unit->set_mission(MISSION_MOVE,&target,false,5,true,NULL,NULL,0,5);		// Fait sortir l'unité du bâtiment
									MISSION *target_mission = target_unit->mission;
									while( target_mission->next != NULL )	target_mission = target_mission->next;
									MISSION *cur = def_mission;
									while( cur ) {							// Copy mission list
										target_mission->next = (MISSION*) malloc(sizeof(MISSION));
										target_mission = target_mission->next;
										*target_mission = *cur;
										target_mission->next = NULL;
										target_mission->path = NULL;
										cur = cur->next;
										}
									}
								mission->p=NULL;
								next_mission();
								}
							else if( port[ INBUILDSTANCE ] != 0 ) {
								float conso_metal=((float)(unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].BuildCostMetal))/unit_manager.unit_type[target_unit->type_id].BuildTime;
								float conso_energy=((float)(unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].BuildCostEnergy))/unit_manager.unit_type[target_unit->type_id].BuildTime;
								if(players.metal[owner_id]>=conso_metal*dt && players.energy[owner_id]>=conso_energy*dt) {
									metal_cons+=conso_metal;
									energy_cons+=conso_energy;
									target_unit->build_percent_left-=dt*unit_manager.unit_type[type_id].WorkerTime*100.0f/unit_manager.unit_type[target_unit->type_id].BuildTime;
									target_unit->hp+=dt*unit_manager.unit_type[type_id].WorkerTime*unit_manager.unit_type[target_unit->type_id].MaxDamage/unit_manager.unit_type[target_unit->type_id].BuildTime;
									}
								if(!unit_manager.unit_type[type_id].BMcode) {
									int script_id_buildinfo = get_script_index(SCRIPT_QueryBuildInfo);
									if(script_id_buildinfo>=0) {
										compute_model_coord();
										VECTOR old_pos = target_unit->Pos;
										if( script_val->size() <= script_id_buildinfo )
											script_val->resize( script_id_buildinfo + 1 );
										target_unit->Pos=Pos+data.pos[(*script_val)[script_id_buildinfo]];
										if( unit_manager.unit_type[ target_unit->type_id ].Floater || ( unit_manager.unit_type[ target_unit->type_id ].canhover && old_pos.y <= map->sealvl ) )
											target_unit->Pos.y = old_pos.y;
										if(((VECTOR)(old_pos-target_unit->Pos)).Sq() > 1000000.0f) {			// It must be continuous
											target_unit->Pos.x = old_pos.x;
											target_unit->Pos.z = old_pos.z;
											}
										else {
											target_unit->cur_px = ((int)(target_unit->Pos.x)+map->map_w_d)>>3;
											target_unit->cur_py = ((int)(target_unit->Pos.z)+map->map_h_d)>>3;
											}
										target_unit->Angle = Angle;
										target_unit->Angle.y += data.axe[1][(*script_val)[script_id_buildinfo]].angle;
										LeaveCS();
										target_unit->draw_on_map();
										EnterCS();
										}
									}
								mission->target = target_unit->Pos;
								target_unit->built=true;
								}
							else {
								activate();
								target_unit->built=true;
								}
							}
						else
							next_mission();
					}
					break;
				case MISSION_BUILD:
					if(mission->p) {
						VECTOR Dir = mission->target - Pos;
						Dir.y = 0.0f;
						int angle = (int)( acos( Dir.z / Dir.Norm() ) * RAD2DEG );
						if( Dir.x < 0.0f )
							angle = -angle;
						angle -= (int)Angle.y;
						if(angle>180)	angle-=360;
						if(angle<-180)	angle+=360;
						int param[] = { (int)(angle*DEG2TA) };
						launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
						mission->mission = MISSION_BUILD_2;		// Change mission type
						((UNIT*)(mission->p))->built = true;
						play_sound( "build" );
						}
					else {
						VECTOR Dir = mission->target - Pos;
						Dir.y = 0.0f;
						float dist = Dir.Sq();
						int maxdist = (int)(unit_manager.unit_type[type_id].BuildDistance
									+ ( unit_manager.unit_type[mission->data].FootprintX + unit_manager.unit_type[mission->data].FootprintZ << 1 ) );
						if(dist>maxdist*maxdist && unit_manager.unit_type[type_id].BMcode) {	// Si l'unité est trop loin du chantier
							mission->flags |= MISSION_FLAG_MOVE;
							mission->move_data = maxdist * 7 / 80;
							}
						else {
							if( mission->flags & MISSION_FLAG_MOVE )			// Stop moving if needed
								stop_moving();
							if(unit_manager.unit_type[type_id].BMcode || (!unit_manager.unit_type[type_id].BMcode && port[ INBUILDSTANCE ] && port[YARD_OPEN] && !port[BUGGER_OFF])) {
								LeaveCS();
								draw_on_map();
								EnterCS();
								V.x = 0.0f;
								V.y = 0.0f;
								V.z = 0.0f;
								if(map->check_rect((((int)(mission->target.x)+map->map_w_d)>>3)-(unit_manager.unit_type[mission->data].FootprintX>>1),(((int)(mission->target.z)+map->map_h_d)>>3)-(unit_manager.unit_type[mission->data].FootprintZ>>1),unit_manager.unit_type[mission->data].FootprintX,unit_manager.unit_type[mission->data].FootprintZ,-1)) {				// Vérifie s'il y a la place de construire l'unité
									LeaveCS();
									mission->p=create_unit(mission->data,owner_id,mission->target,map);
									EnterCS();
									if( !unit_manager.unit_type[type_id].BMcode && mission->p != NULL ) {
										int script_id_buildinfo = get_script_index(SCRIPT_QueryBuildInfo);
										if( script_id_buildinfo >= 0 ) {
											int param[] = { 0 };
											run_script_function( map, script_id_buildinfo, 1, param );
											if( param[0] >= 0 ) {
												compute_model_coord();
												((UNIT*)(mission->p))->Pos = Pos + data.pos[ param[0] ];
												LeaveCS();
												((UNIT*)(mission->p))->draw_on_map();
												EnterCS();
												mission->target = ((UNIT*)(mission->p))->Pos;
												}
											}
										}
									if(mission->p) {
										((UNIT*)(mission->p))->hp=0.000001f;
										((UNIT*)(mission->p))->built=true;
										}
									else
										Console->AddEntry("%d can't create unit!! (%s, l.%d)", idx, __FILE__,__LINE__);
									}
								else if(unit_manager.unit_type[type_id].BMcode)
									next_mission();
//								else
//									Console->AddEntry("%d waiting for a place to build unit of type %d (%s, l.%d)", idx, mission->data, __FILE__,__LINE__);
								}
							else {
								activate();
								run_script_function( map, get_script_index(SCRIPT_QueryBuildInfo) );
								}
							}
						}
					break;
				};

			switch(unit_manager.unit_type[type_id].TEDclass)			// Commandes particulières
			{
			case CLASS_PLANT:
				switch(mission->mission)
				{
				case MISSION_STANDBY:
				case MISSION_BUILD:
				case MISSION_BUILD_2:
				case MISSION_REPAIR:
					break;
				default:
					next_mission();
				};
				break;
			case CLASS_WATER:
			case CLASS_VTOL:
			case CLASS_KBOT:
			case CLASS_COMMANDER:
			case CLASS_TANK:
			case CLASS_CNSTR:
			case CLASS_SHIP:
				{
				if( !(mission->flags & MISSION_FLAG_MOVE) && !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE)
				&& ((mission->mission!=MISSION_ATTACK && unit_manager.unit_type[type_id].canfly) || !unit_manager.unit_type[type_id].canfly)) {
					if( !flying )
						V.x = V.z = 0.0f;
					if( precomputed_position ) {
						NPos = Pos;
						n_px = cur_px;
						n_py = cur_py;
						}
					}
				switch(mission->mission)
				{
				case MISSION_ATTACK:
				case MISSION_PATROL:
				case MISSION_REPAIR:
				case MISSION_BUILD:
				case MISSION_BUILD_2:
					if(unit_manager.unit_type[type_id].canfly)
						activate();
					break;
				case MISSION_STANDBY:
					if(mission->next)
						next_mission();
					V.x = V.y = V.z = 0.0f;			// Frottements
					break;
				case MISSION_MOVE:
					mission->flags |= MISSION_FLAG_CAN_ATTACK;
					if( !(mission->flags & MISSION_FLAG_MOVE) ) {			// Monitor the moving process
						if( mission->next
						&& (mission->next->mission == MISSION_MOVE
						   || (mission->next->mission == MISSION_STOP && mission->next->next && mission->next->next->mission == MISSION_MOVE) ) )
							mission->flags |= MISSION_FLAG_DONT_STOP_MOVE;

						if( !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) )			// If needed
							V.x = V.y = V.z = 0.0f;			// Stop the unit
						if( precomputed_position ) {
							NPos = Pos;
							n_px = cur_px;
							n_py = cur_py;
							}
						if( (mission->flags & MISSION_FLAG_DONT_STOP_MOVE) && mission->next && mission->next->mission == MISSION_STOP )			// If needed
							next_mission();
						next_mission();
						}
					break;
				default:
					if(unit_manager.unit_type[type_id].canfly)
						deactivate();
					};
				}
				break;
			case CLASS_UNDEF:
			case CLASS_METAL:
			case CLASS_ENERGY:
			case CLASS_SPECIAL:
			case CLASS_FORT:
				break;
			default:
				printf("type inconnu %d\n",unit_manager.unit_type[type_id].TEDclass);
			};

			switch(mission->mission)		// Pour le code post déplacement
			{
			case MISSION_ATTACK:
				if( unit_manager.unit_type[type_id].canfly && !unit_manager.unit_type[type_id].hoverattack ) {			// Un avion??
					activate();
					mission->flags &= ~MISSION_FLAG_MOVE;			// We're doing it here, so no need to do it twice
					VECTOR J,I,K;
					K.x=K.z=0.0f;
					K.y=1.0f;
					VECTOR Target = mission->target;
					J = Target-Pos;
					J.y = 0.0f;
					float dist = J.Norm();
					mission->last_d = dist;
					if(dist > 0.0f)
						J = 1.0f / dist * J;
					if( dist > unit_manager.unit_type[type_id].ManeuverLeashLength ) {
						b_TargetAngle = true;
						f_TargetAngle = acos(J.z) * RAD2DEG;
						if(J.x < 0.0f) f_TargetAngle = -f_TargetAngle;
						}

					J.z = cos(Angle.y * DEG2RAD);
					J.x = sin(Angle.y * DEG2RAD);
					J.y = 0.0f;
					I.z = -J.x;
					I.x = J.z;
					I.y = 0.0f;
					V = (V%K)*K+(V%J)*J+units.exp_dt_4*(V%I)*I;
					float speed = V.Sq();
					if( speed < unit_manager.unit_type[type_id].MaxVelocity * unit_manager.unit_type[type_id].MaxVelocity )
						V=V+unit_manager.unit_type[type_id].Acceleration*dt*J;
					}
				break;
			};

			if( (mission->flags & MISSION_FLAG_MOVE) && !jump_commands ) {		// Set unit orientation if it's on the ground
				if(!unit_manager.unit_type[type_id].canfly && !unit_manager.unit_type[type_id].Upright
				&& unit_manager.unit_type[type_id].TEDclass!=CLASS_SHIP
				&& unit_manager.unit_type[type_id].TEDclass!=CLASS_WATER
				&& !( unit_manager.unit_type[type_id].canhover && Pos.y <= map->sealvl ) ) {
					VECTOR I,J,K,A,B,C;
					MATRIX_4x4 M = RotateY((Angle.y+90.0f)*DEG2RAD);
					I.x = 4.0f;
					J.z = 4.0f;
					K.y = 1.0f;
					A = Pos - unit_manager.unit_type[type_id].FootprintZ*I*M;
					B = Pos + (unit_manager.unit_type[type_id].FootprintX*I-unit_manager.unit_type[type_id].FootprintZ*J)*M;
					C = Pos + (unit_manager.unit_type[type_id].FootprintX*I+unit_manager.unit_type[type_id].FootprintZ*J)*M;
					A.y = map->get_unit_h(A.x,A.z);	// Projete le triangle
					B.y = map->get_unit_h(B.x,B.z);
					C.y = map->get_unit_h(C.x,C.z);
					VECTOR D=(B-A)*(B-C);
					if(D.y>=0.0f) {					// On ne met pas une unité à l'envers!!
						D.Unit();
						float dist_sq = sqrt( D.y*D.y+D.z*D.z );
						float angle_1= dist_sq != 0.0f ? acos( D.y / dist_sq )*RAD2DEG : 0.0f;
						if(D.z<0.0f)	angle_1=-angle_1;
						D=D*RotateX(-angle_1*DEG2RAD);
						float angle_2=VAngle(D,K)*RAD2DEG;
						if(D.x>0.0f)	angle_2=-angle_2;
						if(fabs(angle_1-Angle.x)<=10.0f && fabs(angle_2-Angle.z)<=10.0f) {
							Angle.x=angle_1;
							Angle.z=angle_2;
							}
						}
					}
				else if( !unit_manager.unit_type[type_id].canfly )
					Angle.x = Angle.z = 0.0f;
				}

			bool returning_fire = ( port[ STANDINGFIREORDERS ] == SFORDER_RETURN_FIRE && attacked );
			if( ( ((mission->flags & MISSION_FLAG_CAN_ATTACK) == MISSION_FLAG_CAN_ATTACK) || do_nothing() )
			&& ( port[ STANDINGFIREORDERS ] == SFORDER_FIRE_AT_WILL || returning_fire )
			&& !jump_commands ) {
				// Si l'unité peut attaquer d'elle même les unités enemies proches, elle le fait / Attack nearby enemies

				bool can_fire = unit_manager.unit_type[type_id].AutoFire && unit_manager.unit_type[type_id].canattack;

				if( !can_fire )
					for( int i = 0 ; i < 3 && !can_fire ; i++ )
						can_fire = unit_manager.unit_type[type_id].weapon[i]!=NULL && !unit_manager.unit_type[type_id].weapon[i]->commandfire && weapon[0].state == WEAPON_FLAG_IDLE;

				if( can_fire ) {
					int dx=unit_manager.unit_type[type_id].SightDistance+(int)(h+0.5f)>>3;
					int enemy_idx=-1;
					if(unit_manager.unit_type[type_id].weapon[0]!=NULL && (unit_manager.unit_type[type_id].weapon[0]->range>>4)>dx)
						dx=unit_manager.unit_type[type_id].weapon[0]->range>>4;
					if(unit_manager.unit_type[type_id].weapon[1]!=NULL && (unit_manager.unit_type[type_id].weapon[1]->range>>4)>dx)
						dx=unit_manager.unit_type[type_id].weapon[1]->range>>4;
					if(unit_manager.unit_type[type_id].weapon[2]!=NULL && (unit_manager.unit_type[type_id].weapon[2]->range>>4)>dx)
						dx=unit_manager.unit_type[type_id].weapon[2]->range>>4;
					if( unit_manager.unit_type[type_id].kamikaze && (unit_manager.unit_type[type_id].kamikazedistance>>3) > dx )
						dx=unit_manager.unit_type[type_id].kamikazedistance;

					int sx=rand_from_table()&0xF;
					int sy=rand_from_table()&0xF;
					byte mask=1<<owner_id;
					for(int y=cur_py-dx+sy;y<=cur_py+dx;y+=0x8) {
						if(y>=0 && y<map->bloc_h_db-1)
							for(int x=cur_px-dx+sx;x<=cur_px+dx;x+=0x8)
								if(x>=0 && x<map->bloc_w_db-1 ) {
									bool land_test = true;
									IDX_LIST_NODE *cur = map->map_data[y][x].air_idx.head;
									for( ; land_test || cur != NULL ; ) {
										int cur_idx;
										if( land_test ) {
											cur_idx = map->map_data[y][x].unit_idx;
											land_test = false;
											}
										else {
											cur_idx = cur->idx;
											cur = cur->next;
											}
										if(cur_idx>=0 && cur_idx<units.max_unit && units.unit[cur_idx].flags && units.unit[cur_idx].owner_id != owner_id
										&& unit_manager.unit_type[units.unit[cur_idx].type_id].ShootMe && ( (units.map->sight_map->line[y>>1][x>>1] & mask) || units.unit[cur_idx].is_on_radar( mask ) )
										&& !unit_manager.unit_type[ units.unit[cur_idx].type_id ].checkCategory( unit_manager.unit_type[type_id].BadTargetCategory ) ) {
											if( !returning_fire
											|| ( units.unit[cur_idx].weapon[0].state != WEAPON_FLAG_IDLE && units.unit[cur_idx].weapon[0].target == this )
											|| ( units.unit[cur_idx].weapon[1].state != WEAPON_FLAG_IDLE && units.unit[cur_idx].weapon[1].target == this )
											|| ( units.unit[cur_idx].weapon[2].state != WEAPON_FLAG_IDLE && units.unit[cur_idx].weapon[2].target == this ) ) {
												enemy_idx = cur_idx;
												x = cur_px + dx;
												y = cur_py + dx;
												break;
												}
											}
										}
									}
						if(enemy_idx>=0)	break;
						}
					if(enemy_idx>=0) {			// Si on a trouvé une unité, on l'attaque
						if( do_nothing() )
							set_mission(MISSION_ATTACK,&(units.unit[enemy_idx].Pos),false,0,true,&(units.unit[enemy_idx]),NULL);
						else
							for( int i = 0 ; i < 3 ; i++ )
								if( weapon[i].state == WEAPON_FLAG_IDLE && unit_manager.unit_type[type_id].weapon[ i ] != NULL
								&& !unit_manager.unit_type[type_id].weapon[ i ]->commandfire
								&& !unit_manager.unit_type[type_id].weapon[ i ]->interceptor
								&& (!unit_manager.unit_type[type_id].weapon[ i ]->toairweapon
								   || ( unit_manager.unit_type[type_id].weapon[ i ]->toairweapon && units.unit[enemy_idx].flying )
								&& !unit_manager.unit_type[ units.unit[enemy_idx].type_id ].checkCategory( unit_manager.unit_type[type_id].w_badTargetCategory[i] ) ) ) {
									weapon[i].state = WEAPON_FLAG_AIM;
									weapon[i].target = &(units.unit[enemy_idx]);
									weapon[i].data = -1;
									}
						}
					}
				if(unit_manager.unit_type[type_id].antiweapons && unit_manager.unit_type[type_id].weapon[0]) {
					float coverage=unit_manager.unit_type[type_id].weapon[0]->coverage*unit_manager.unit_type[type_id].weapon[0]->coverage;
					float range=unit_manager.unit_type[type_id].weapon[0]->range*unit_manager.unit_type[type_id].weapon[0]->range>>2;
					int enemy_idx=-1;
					byte e=0;
					for(byte i=0;i+e<mem_size;i++) {
						if(memory[i+e]<0 || memory[i+e]>=weapons.nb_weapon || weapons.weapon[memory[i+e]].weapon_id==-1) {
							e++;
							i--;
							continue;
							}
						memory[i] = memory[i+e];
						}
					mem_size -= e;
					for(uint32 f=0;f<weapons.index_list_size;f+=(rand_from_table()&7)+1) {
						uint32 i = weapons.idx_list[f];
						if(weapons.weapon[i].weapon_id!=-1 && units.unit[weapons.weapon[i].shooter_idx].owner_id!=owner_id
						&& weapon_manager.weapon[weapons.weapon[i].weapon_id].targetable)
						if(((VECTOR)(weapons.weapon[i].target_pos-Pos)).Sq()<=coverage
						&& ((VECTOR)(weapons.weapon[i].Pos-Pos)).Sq()<=range) {
							int idx=-1;
							for(e=0;e<mem_size;e++)
								if(memory[e]==i) {
									idx=i;
									break;
									}
							if(idx==-1) {
								enemy_idx=i;
								if(mem_size<10) {
									memory[mem_size]=i;
									mem_size++;
									}
								break;
								}
							}
						}
					if(enemy_idx>=0)			// If we found a target, then attack it, here  we use attack because we need the mission list to act properly
						add_mission(MISSION_ATTACK,&(weapons.weapon[enemy_idx].Pos),false,0,&(weapons.weapon[enemy_idx]),NULL,12);	// 12 = 4 | 8, targets a weapon and automatic fire
					}
				}
			}

		if( unit_manager.unit_type[type_id].canfly ) {			// Set plane orientation
			VECTOR J,K;
			K.x=K.z=0.0f;
			K.y=1.0f;
			J = V * K;

			VECTOR virtual_G;						// Compute the apparent gravity force ( seen from the plane )
			virtual_G.x = virtual_G.z = 0.0f;		// Standard gravity vector
			virtual_G.y = -4.0f * units.g_dt;
			float d = J.Sq();
			if( d )
				virtual_G = virtual_G + (((old_V - V) % J) / d) * J;		// Add the opposite of the speed derivative projected on the side of the unit

			d = virtual_G.Norm();
			if( d ) {
				virtual_G = -1.0f / d * virtual_G;

				d = sqrt(virtual_G.y*virtual_G.y+virtual_G.z*virtual_G.z);
				float angle_1 = (d != 0.0f) ? acos(virtual_G.y/d)*RAD2DEG : 0.0f;
				if(virtual_G.z<0.0f)	angle_1 = -angle_1;
				virtual_G = virtual_G * RotateX(-angle_1*DEG2RAD);
				float angle_2 = acos( virtual_G % K )*RAD2DEG;
				if(virtual_G.x > 0.0f)	angle_2 = -angle_2;

				if( fabs( angle_1 - Angle.x ) < 360.0f )
					Angle.x += dt*( angle_1 - Angle.x );				// We need something continuous
				if( fabs( angle_2 - Angle.z ) < 360.0f )
					Angle.z += dt*( angle_2 - Angle.z );

				if( Angle.x < -360.0f || Angle.x > 360.0f )		Angle.x = 0.0f;
				if( Angle.z < -360.0f || Angle.z > 360.0f )		Angle.z = 0.0f;
				}
			}

		if(build_percent_left==0.0f) {

			// Change the unit's angle the way we need it to be changed

			if( b_TargetAngle && unit_manager.unit_type[type_id].BMcode ) {	// Don't remove the class check otherwise factories can spin
				while( fabs( f_TargetAngle - Angle.y ) > 180.0f ) {
					if( f_TargetAngle < Angle.y )
						Angle.y -= 360.0f;
					else
						Angle.y += 360.0f;
					}
				if( fabs( f_TargetAngle - Angle.y ) >= 1.0f ) {
					float aspeed = unit_manager.unit_type[type_id].TurnRate;
					if( f_TargetAngle < Angle.y )
						aspeed =- aspeed;
					float a = f_TargetAngle - Angle.y;
					V_Angle.y = aspeed;
					float b = f_TargetAngle - (Angle.y + dt*V_Angle.y);
					if((a < 0.0f && b > 0.0f) || (a > 0.0f && b < 0.0f)) {
						V_Angle.y = 0.0f;
						Angle.y = f_TargetAngle;
						}
					}
				}

			Angle = Angle + dt * V_Angle;
			VECTOR OPos = Pos;
			if( precomputed_position ) {
				if( unit_manager.unit_type[type_id].canmove && unit_manager.unit_type[type_id].BMcode )
					V.y-=units.g_dt;			// L'unité subit la force de gravitation
				Pos = NPos;
				Pos.y = OPos.y + V.y * dt;
				cur_px = n_px;
				cur_py = n_py;
				}
			else {
				if( unit_manager.unit_type[type_id].canmove && unit_manager.unit_type[type_id].BMcode )
					V.y-=units.g_dt;			// L'unité subit la force de gravitation
				Pos = Pos+dt*V;			// Déplace l'unité
				cur_px = ((int)(Pos.x)+map->map_w_d)>>3;
				cur_py = ((int)(Pos.z)+map->map_h_d)>>3;
				}
			if( units.current_tick - ripple_timer >= 7 && Pos.y <= map->sealvl && Pos.y + model->top >= map->sealvl && (unit_manager.unit_type[type_id].fastCategory & CATEGORY_NOTSUB)
			&& cur_px >= 0 && cur_py >= 0 && cur_px < map->bloc_w_db && cur_py < map->bloc_h_db && !map->map_data[ cur_py ][ cur_px ].lava && map->water ) {
				VECTOR Diff = OPos - Pos;
				Diff.y = 0.0f;
				if( Diff.Sq() > 0.1f && lp_CONFIG->waves ) {
					ripple_timer = units.current_tick;
					VECTOR ripple_pos = Pos;
					ripple_pos.y = map->sealvl + 1.0f;
					fx_manager.add_ripple( ripple_pos, ( (rand_from_table() % 201) - 100 ) * 0.0001f );
					}
				}
			}
		script_exec:
		if(map && !attached && ( (!jump_commands && unit_manager.unit_type[type_id].canmove) || first_move ) ) {
			float min_h = map->get_unit_h(Pos.x,Pos.z);
			h = Pos.y - min_h;
			if( !unit_manager.unit_type[type_id].Floater && !unit_manager.unit_type[type_id].canfly && !unit_manager.unit_type[type_id].canhover && h > 0.0f && unit_manager.unit_type[type_id].WaterLine == 0.0f )
				Pos.y = min_h;
			else if( unit_manager.unit_type[type_id].canhover && Pos.y < map->sealvl ) {
				Pos.y = map->sealvl;
				if(V.y<0.0f)
					V.y=0.0f;
				}
			else if( unit_manager.unit_type[type_id].Floater ) {
				Pos.y = map->sealvl+unit_manager.unit_type[type_id].AltFromSeaLevel*H_DIV;
				V.y=0.0f;
				}
			else if( unit_manager.unit_type[type_id].WaterLine ) {
				Pos.y=map->sealvl-unit_manager.unit_type[type_id].WaterLine*H_DIV;
				V.y=0.0f;
				}
			else if( !unit_manager.unit_type[type_id].canfly && Pos.y > max( min_h, map->sealvl ) && unit_manager.unit_type[type_id].BMcode ) {	// Prevent non flying units from "jumping"
				Pos.y = max( min_h, map->sealvl );
				if(V.y<0.0f)
					V.y=0.0f;
				}
			if(min_h>Pos.y) {
				Pos.y=min_h;
				if(V.y<0.0f)
					V.y=0.0f;
				}
			if(unit_manager.unit_type[type_id].canfly && build_percent_left==0.0f) {
				if(mission && ( (mission->flags & MISSION_FLAG_MOVE) || mission->mission == MISSION_BUILD || mission->mission == MISSION_BUILD_2 || mission->mission == MISSION_REPAIR
				|| mission->mission == MISSION_ATTACK || mission->mission == MISSION_MOVE || nb_attached > 0
				|| Pos.x < -map->map_w_d || Pos.x > map->map_w_d || Pos.z < -map->map_h_d || Pos.z > map->map_h_d )) {
					float ideal_h=max(min_h,map->sealvl)+unit_manager.unit_type[type_id].CruiseAlt*H_DIV;
					V.y=(ideal_h-Pos.y)*2.0f;
					flying = true;
					}
				else {
					if( can_be_there( cur_px, cur_py, units.map, type_id, owner_id, idx ) ) {		// Check it can be there
						float ideal_h = min_h;
						V.y=(ideal_h-Pos.y)*1.5f;
						flying = false;
						}
					else {				// There is someone there, find an other place to land
						flying = true;
						VECTOR next_target = Pos;
						float find_angle = (rand_from_table() % 360) * DEG2RAD;
						next_target.x += cos( find_angle ) * (32.0f + unit_manager.unit_type[type_id].FootprintX * 8.0f);
						next_target.z += sin( find_angle ) * (32.0f + unit_manager.unit_type[type_id].FootprintZ * 8.0f);
						add_mission( MISSION_MOVE, &next_target, true );
						}
					}
				}
			port[GROUND_HEIGHT] = (int)(Pos.y-min_h+0.5f);
			}
		port[HEALTH] = (int)hp*100 / unit_manager.unit_type[type_id].MaxDamage;
		if(nb_running>0) {
			for(int i=0;i<nb_running;i++)
				run_script(dt,i,map);
			int e=0;
			for(int i=0;i+e<nb_running;) {				// Efface les scripts qui se sont arrêtés
				if((*script_env)[i+e].running) {
					(*script_env)[i] = (*script_env)[i+e];
					i++;
					}
				else {
					(*script_env)[i+e].destroy();
					e++;
					}
				}
			nb_running-=e;
			}
		if( (o_px != cur_px || o_py != cur_py || first_move || was_flying ^ flying || (port[YARD_OPEN] != 0.0f) ^ was_open) && build_percent_left <= 0.0f || !drawn ) {
			first_move = build_percent_left > 0.0f;
			LeaveCS();
			draw_on_map();
			EnterCS();
			}

		built=false;
		attacked=false;
		LeaveCS();
#ifdef	ADVANCED_DEBUG_MODE
		GuardLeave();
#endif
		return 0;
	}

	bool UNIT::hit(VECTOR P,VECTOR Dir,VECTOR *hit_vec, float length)
	{
		EnterCS();
		if(!(flags&1))	{
			LeaveCS();
			return false;
			}
		if(model) {
			VECTOR c_dir=model->center+Pos-P;
			if( c_dir.Norm()-length <=model->size2 ) {
				float scale=unit_manager.unit_type[type_id].Scale;
				MATRIX_4x4 M=RotateX(-Angle.x*DEG2RAD)*RotateZ(-Angle.z*DEG2RAD)*RotateY(-Angle.y*DEG2RAD)*Scale(1.0f/scale);
				VECTOR RP=(P-Pos) * M;
				bool is_hit=model->hit(RP,Dir,&data,hit_vec,M);
				if(is_hit) {
					*hit_vec=(*hit_vec)*(RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale))+Pos;
					*hit_vec=((*hit_vec-P)%Dir)*Dir+P;
					}

				LeaveCS();
				return is_hit;
				}
			}
		LeaveCS();
		return false;
	}

	bool UNIT::hit_fast(VECTOR P,VECTOR Dir,VECTOR *hit_vec, float length)
	{
		EnterCS();
		if(!(flags&1))	{
			LeaveCS();
			return false;
			}
		if(model) {
			VECTOR c_dir = model->center+Pos-P;
			if( c_dir.Sq() <= ( model->size2 + length ) * ( model->size2 + length ) ) {
				float scale=unit_manager.unit_type[type_id].Scale;
				MATRIX_4x4 M = RotateX(-Angle.x*DEG2RAD)*RotateZ(-Angle.z*DEG2RAD)*RotateY(-Angle.y*DEG2RAD)*Scale(1.0f/scale);
				VECTOR RP = (P - Pos) * M;
				bool is_hit = model->hit_fast(RP,Dir,&data,hit_vec,M);
				if(is_hit) {
					*hit_vec=(*hit_vec)*(RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale))+Pos;
					*hit_vec=((*hit_vec-P)%Dir)*Dir+P;
					}

				LeaveCS();
				return is_hit;
				}
			}
		LeaveCS();
		return false;
	}

	void UNIT::show_orders(bool only_build_commands, bool def_orders)				// Dessine les ordres reçus
	{
		if( !def_orders )	show_orders( only_build_commands, true );
		EnterCS();

		MISSION *cur = def_orders ? def_mission : mission;
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		VECTOR p_target=Pos;
		VECTOR n_target=Pos;
		float rab=(msec_timer%1000)*0.001f;
		uint32	remaining_build_commands = !(unit_manager.unit_type[ type_id ].BMcode) ? 0 : 0xFFFFFFF;
		while(cur) {
			if(cur->step) {				// S'il s'agit d'une étape on ne la montre pas
				cur=cur->next;
				continue;
				}
			if(!only_build_commands)
			{
			int curseur=anim_cursor(CURSOR_CROSS_LINK);
			float dx=0.5f*cursor.anm[CURSOR_CROSS_LINK].ofs_x[curseur];
			float dz=0.5f*cursor.anm[CURSOR_CROSS_LINK].ofs_y[curseur];
			float sx=0.5f*(cursor.anm[CURSOR_CROSS_LINK].bmp[curseur]->w-1);
			float sy=0.5f*(cursor.anm[CURSOR_CROSS_LINK].bmp[curseur]->h-1);
			float x,y,z;
			float dist=((VECTOR)(cur->target-p_target)).Norm();
			int rec=(int)(dist/30.0f);
			switch(cur->mission)
			{
			case MISSION_LOAD:
			case MISSION_UNLOAD:
			case MISSION_GUARD:
			case MISSION_PATROL:
			case MISSION_MOVE:
			case MISSION_BUILD:
			case MISSION_BUILD_2:
			case MISSION_REPAIR:
			case MISSION_ATTACK:
			case MISSION_RECLAIM:
			case MISSION_REVIVE:
				n_target=cur->target;
				n_target.y = max( units.map->get_unit_h( n_target.x, n_target.z ), units.map->sealvl );
				if(rec>0) {
					glBindTexture(GL_TEXTURE_2D,cursor.anm[CURSOR_CROSS_LINK].glbmp[curseur]);
					glBegin(GL_QUADS);
						for(int i=0;i<rec;i++) {
							x=p_target.x+(n_target.x-p_target.x)*(i+rab)/rec;
							z=p_target.z+(n_target.z-p_target.z)*(i+rab)/rec;
							y = max( units.map->get_unit_h( x, z ), units.map->sealvl );
							x-=dx;
							y+=0.75f;
							z-=dz;
							glTexCoord2f(0.0f,0.0f);		glVertex3f(x,y,z);
							glTexCoord2f(1.0f,0.0f);		glVertex3f(x+sx,y,z);
							glTexCoord2f(1.0f,1.0f);		glVertex3f(x+sx,y,z+sy);
							glTexCoord2f(0.0f,1.0f);		glVertex3f(x,y,z+sy);
							}
					glEnd();
					}
				p_target = n_target;
			};
			}
			glDisable(GL_DEPTH_TEST);
			switch(cur->mission)
			{
			case MISSION_BUILD:
				if(cur->p!=NULL)
					cur->target=((UNIT*)(cur->p))->Pos;
				if(cur->data>=0 && cur->data<unit_manager.nb_unit && remaining_build_commands > 0 ) {
					remaining_build_commands--;
					float DX = (unit_manager.unit_type[cur->data].FootprintX<<2);
					float DZ = (unit_manager.unit_type[cur->data].FootprintZ<<2);
					float blue = 0.0f, green = 1.0f;
					if(only_build_commands) {
						blue = 1.0f;
						green = 0.0f;
						}
					glPushMatrix();
					glTranslatef(cur->target.x,cur->target.y,cur->target.z);
					glDisable(GL_CULL_FACE);
					glDisable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glBegin(GL_QUADS);
						glColor4f(0.0f,green,blue,1.0f);
						glVertex3f(-DX,0.0f,-DZ);			// First quad
						glVertex3f(DX,0.0f,-DZ);
						glColor4f(0.0f,green,blue,0.0f);
						glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
						glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

						glColor4f(0.0f,green,blue,1.0f);
						glVertex3f(-DX,0.0f,-DZ);			// Second quad
						glVertex3f(-DX,0.0f,DZ);
						glColor4f(0.0f,green,blue,0.0f);
						glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
						glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

						glColor4f(0.0f,green,blue,1.0f);
						glVertex3f(DX,0.0f,-DZ);			// Third quad
						glVertex3f(DX,0.0f,DZ);
						glColor4f(0.0f,green,blue,0.0f);
						glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
						glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
					glEnd();
					glDisable(GL_BLEND);
					glEnable(GL_TEXTURE_2D);
					glEnable(GL_CULL_FACE);
					glPopMatrix();
					if(unit_manager.unit_type[cur->data].model!=NULL) {
						glEnable(GL_LIGHTING);
						glEnable(GL_CULL_FACE);
						glEnable(GL_DEPTH_TEST);
						glPushMatrix();
						glTranslatef(cur->target.x,cur->target.y,cur->target.z);
						glColor4f(0.0f,green,blue,0.5f);
						unit_manager.unit_type[cur->data].model->obj.draw(0.0f,NULL,false,false,false);
						glPopMatrix();
						glEnable(GL_BLEND);
						glEnable(GL_TEXTURE_2D);
						glDisable(GL_LIGHTING);
						glDisable(GL_CULL_FACE);
						glDisable(GL_DEPTH_TEST);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						}
					glPushMatrix();
					glTranslatef(cur->target.x,cur->target.y,cur->target.z);
					glDisable(GL_CULL_FACE);
					glDisable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glBegin(GL_QUADS);
						glColor4f(0.0f,green,blue,1.0f);
						glVertex3f(-DX,0.0f,DZ);			// Fourth quad
						glVertex3f(DX,0.0f,DZ);
						glColor4f(0.0f,green,blue,0.0f);
						glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
						glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
					glEnd();
					glPopMatrix();
					glEnable(GL_BLEND);
					glEnable(GL_TEXTURE_2D);
					glDisable(GL_CULL_FACE);
					glColor4f(1.0f,1.0f,1.0f,1.0f);
					}
				break;
			case MISSION_UNLOAD:
			case MISSION_LOAD:
			case MISSION_MOVE:
			case MISSION_BUILD_2:
			case MISSION_REPAIR:
			case MISSION_RECLAIM:
			case MISSION_REVIVE:
			case MISSION_PATROL:
			case MISSION_GUARD:
			case MISSION_ATTACK:
				if(!only_build_commands)
				{
				if(cur->p!=NULL)
					cur->target=((UNIT*)(cur->p))->Pos;
				int cursor_type = CURSOR_ATTACK;
				switch( cur->mission )
				{
				case MISSION_GUARD:		cursor_type = CURSOR_GUARD;		break;
				case MISSION_ATTACK:	cursor_type = CURSOR_ATTACK;	break;
				case MISSION_PATROL:	cursor_type = CURSOR_PATROL;	break;
				case MISSION_RECLAIM:	cursor_type = CURSOR_RECLAIM;	break;
				case MISSION_BUILD_2:
				case MISSION_REPAIR:	cursor_type = CURSOR_REPAIR;	break;
				case MISSION_MOVE:		cursor_type = CURSOR_MOVE;		break;
				case MISSION_LOAD:		cursor_type = CURSOR_LOAD;		break;
				case MISSION_UNLOAD:	cursor_type = CURSOR_UNLOAD;	break;
				case MISSION_REVIVE:	cursor_type = CURSOR_REVIVE;	break;
				};
				int curseur=anim_cursor( cursor_type );
				float x=cur->target.x-0.5f*cursor.anm[ cursor_type ].ofs_x[curseur];
				float y=cur->target.y+1.0f;
				float z=cur->target.z-0.5f*cursor.anm[ cursor_type ].ofs_y[curseur];
				float sx=0.5f*(cursor.anm[ cursor_type ].bmp[curseur]->w-1);
				float sy=0.5f*(cursor.anm[ cursor_type ].bmp[curseur]->h-1);
				glBindTexture(GL_TEXTURE_2D,cursor.anm[ cursor_type ].glbmp[curseur]);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f,0.0f);		glVertex3f(x,y,z);
					glTexCoord2f(1.0f,0.0f);		glVertex3f(x+sx,y,z);
					glTexCoord2f(1.0f,1.0f);		glVertex3f(x+sx,y,z+sy);
					glTexCoord2f(0.0f,1.0f);		glVertex3f(x,y,z+sy);
				glEnd();
				}
				break;
			};
			glEnable(GL_DEPTH_TEST);
			cur=cur->next;
			}
		glDisable(GL_BLEND);
		
		LeaveCS();
	}

	bool INGAME_UNITS::select(CAMERA *cam,int sel_x[],int sel_y[])
	{
		EnterCS();

		bool selected=false;
		cam->SetView();
		MATRIX_4x4 ModelView,Project,T;
		int	viewportCoords[4] = {0, 0, 0, 0};
		glGetIntegerv(GL_VIEWPORT, viewportCoords);
		glGetFloatv(GL_MODELVIEW_MATRIX, (float*)ModelView.E);
		glGetFloatv(GL_PROJECTION_MATRIX, (float*)Project.E);
		ModelView=Transpose(ModelView);
		Project=Transpose(Project);
		float VW=(viewportCoords[2]-viewportCoords[0])*0.5f;
		float VH=-(viewportCoords[3]-viewportCoords[1])*0.5f;
		T=ModelView*Project;		// Matrice de transformation

		POINTF UPos,O;
		O.x=O.y=O.z=0.0f;
		int X1,Y1,X2,Y2;
		X1=min(sel_x[0],sel_x[1]);
		Y1=min(sel_y[0],sel_y[1]);
		X2=max(sel_x[0],sel_x[1]);
		Y2=max(sel_y[0],sel_y[1]);

		for(uint16 e=0;e<index_list_size;e++) {
			uint16 i = idx_list[e];
			unit[ i ].Lock();
			if( (unit[i].flags & 1) && unit[i].owner_id==players.local_human_id && unit[i].build_percent_left==0.0f && unit[i].visible) {		// Ne sélectionne que les unités achevées
				if(key[KEY_LSHIFT] && unit[i].sel) {
					selected=true;
					unit[ i ].UnLock();
					continue;
					}
				if(!key[KEY_LSHIFT])
					unit[i].sel=false;

				VECTOR Vec=unit[i].Pos-cam->Pos;
				float d=Vec.Sq();
				if(d>16384.0f && (Vec%cam->Dir)<=0.0f) {
					unit[ i ].UnLock();
					continue;
					}

				UPos=glNMult((O+unit[i].Pos),T);		// Transforme la position de l'unité
				UPos.x=UPos.x*VW+VW;
				UPos.y=UPos.y*VH-VH;

				if(X1<=UPos.x && X2>=UPos.x && Y1<=UPos.y && Y2>=UPos.y)
					selected=unit[i].sel=true;
				}
			unit[ i ].UnLock();
			}

		LeaveCS();

		return selected;
	}

	int INGAME_UNITS::pick(CAMERA *cam,int sensibility)
	{
		int index=-1;

		if(nb_unit<=0)
			return index;

		if( last_on != -1 )	return last_on;		// Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed

		VECTOR Dir;
		Dir=cam->Dir+cam->width_factor*2.0f*(mouse_x-gfx->SCREEN_W_HALF)*gfx->SCREEN_W_INV*cam->Side-1.5f*(mouse_y-gfx->SCREEN_H_HALF)*gfx->SCREEN_H_INV*cam->Up;
		Dir.Unit();		// Direction pointée par le curseur

		bool detectable=false;
		int i;

		for(uint16 e=0;e<index_list_size;e++) {
			EnterCS();
			i = idx_list[e];
			LeaveCS();

			unit[ i ].Lock();
			if( !(unit[i].flags & 1) || !unit[i].visible ) {
				unit[ i ].UnLock();
				continue;		// Si l'unité n'existe pas on la zappe
				}
			unit[i].flags &= 0xFD;	// Enlève l'indicateur de possibilité d'intersection
			VECTOR center=unit[i].model->center+unit[i].Pos-cam->Pos;
			float size=unit[i].model->size*unit_manager.unit_type[unit[i].type_id].Scale*unit_manager.unit_type[unit[i].type_id].Scale;
			center=Dir*center;
			float dist=center.Sq();
			if(dist<size) {
				detectable=true;
				unit[i].flags|=0x2;		// Unité détectable
				}
			unit[ i ].UnLock();
			}

		if(!detectable) {			// If no unit is near the cursor, then skip the precise method
			last_on = index;

			return index;
			}

		float best_dist = 1000000.0f;

		for(uint16 e=0;e<index_list_size;e++) {
			EnterCS();
			i = idx_list[e];
			LeaveCS();

			unit[ i ].Lock();
			if( !(unit[i].flags & 1) || !unit[i].visible ) {
				unit[ i ].UnLock();
				continue;		// Si l'unité n'existe pas on la zappe
				}
			if((unit[i].flags&0x2)==0x2) {			// Si l'unité existe et est sélectionnable
				unit[i].flags&=0xFD;
				VECTOR D;
				if( unit[i].hit( cam->Pos, Dir, &D, 1000000.0f ) ) {		// Vecteur "viseur unité" partant de la caméra vers l'unité
					float dist = (D-cam->Pos).Sq();
					if( dist < best_dist || index == -1 ) {
						best_dist = dist;
						index = i;
						}
					}
				}
			unit[ i ].UnLock();
			}

		last_on = index;

		return index;
	}

	int INGAME_UNITS::pick_minimap()
	{
		int index=-1;

		if(nb_unit<=0)
			return index;

		if( last_on != -1 )	return last_on;		// Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed

		int i;

		float conv_x = ((float)map->mini_w) / map->map_w * 128.0f / 252.0f;
		float conv_z = ((float)map->mini_h) / map->map_h * 128.0f / 252.0f;

		byte player_mask = 1 << players.local_human_id;

		for(uint16 e=0;e<index_list_size;e++) {
			EnterCS();
			i = idx_list[e];
			LeaveCS();

			unit[ i ].Lock();
			if( !(unit[i].flags & 1) ) {
				unit[ i ].UnLock();
				continue;		// Si l'unité n'existe pas on la zappe
				}

			if( !unit[i].visible ) {			// Additional checks that have to be done
				int px = unit[i].cur_px >> 1;
				int py = unit[i].cur_py >> 1;
				if( px < 0 || py < 0 || px >= map->bloc_w || py >= map->bloc_h ) {
					unit[ i ].UnLock();
					continue;	// Out of the map
					}
				if( !( map->view_map->line[ py ][ px ] & player_mask ) && !(map->sight_map->line[ py ][ px ] & player_mask)
				&& !unit[i].is_on_radar( player_mask ) ) {
					unit[ i ].UnLock();
					continue;	// Not visible
					}
				}

			int x = (int)(unit[i].Pos.x * conv_x + 64.5f);
			int y = (int)(unit[i].Pos.z * conv_z + 64.5f);

			if( x == mouse_x && y == mouse_y )	{
				last_on = i;
				unit[ i ].UnLock();
				return i;
				}

			if( abs(mouse_x - x) <= 1 && abs(mouse_y - y) <= 1 )	index = i;

			unit[ i ].UnLock();
			}

		last_on = index;
		return index;
	}

	void UNIT::shoot(int target,VECTOR startpos,VECTOR Dir,int w_id,const VECTOR &target_pos)
	{
		if( get_script_index( SCRIPT_RockUnit ) >= 0 ) {		// Don't do calculations that won't be used
			VECTOR D = Dir * RotateY( -Angle.y * DEG2RAD );
			int param[] = { (int)(-10.0f*DEG2TA*D.z), (int)(-10.0f*DEG2TA*D.x) };
			launch_script( get_script_index( SCRIPT_RockUnit ), 2, param );
			}

		POINTF O;
		O.x=O.y=O.z=0.0f;
		if(unit_manager.unit_type[type_id].weapon[w_id]->startsmoke && visible)
			particle_engine.make_smoke(O+startpos,0,1,0.0f,-1.0f,0.0f, 0.3f);
		LeaveCS();
		int w_idx=weapons.add_weapon(unit_manager.unit_type[type_id].weapon[w_id]->nb_id,idx);
		EnterCS();
		weapons.weapon[w_idx].damage = unit_manager.unit_type[type_id].weapon_damage[ w_id ];
		weapons.weapon[w_idx].Pos=startpos;
		if(unit_manager.unit_type[type_id].weapon[w_id]->startvelocity==0.0f && !unit_manager.unit_type[type_id].weapon[w_id]->selfprop)
			weapons.weapon[w_idx].V = unit_manager.unit_type[type_id].weapon[w_id]->weaponvelocity*Dir;
		else
			weapons.weapon[w_idx].V = unit_manager.unit_type[type_id].weapon[w_id]->startvelocity*Dir;
		if( unit_manager.unit_type[type_id].weapon[w_id]->dropped || !(unit_manager.unit_type[type_id].weapon[w_id]->rendertype & RENDER_TYPE_LASER) )
			weapons.weapon[w_idx].V=weapons.weapon[w_idx].V+V;
		weapons.weapon[w_idx].owner=owner_id;
		weapons.weapon[w_idx].target=target;
		if( target >= 0 ) {
			if(unit_manager.unit_type[type_id].weapon[w_id]->interceptor)
				weapons.weapon[w_idx].target_pos=weapons.weapon[target].Pos;
			else
				weapons.weapon[w_idx].target_pos=units.unit[target].Pos+units.unit[target].model->center;
			}
		else
			weapons.weapon[w_idx].target_pos = target_pos;
		weapons.weapon[w_idx].stime=0.0f;
		weapons.weapon[w_idx].visible=visible;
	}

void UNIT::draw_on_map()
{
	if( drawn )	clear_from_map();
	if( attached )	return;

	drawn_flying = flying;
	if( flying )
		units.map->air_rect( cur_px-(unit_manager.unit_type[type_id].FootprintX>>1), cur_py-(unit_manager.unit_type[type_id].FootprintZ>>1), unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx );
	else {
		// First check we're on a "legal" place if it can move
		if( unit_manager.unit_type[ type_id ].canmove && unit_manager.unit_type[ type_id ].BMcode
		&& !can_be_there( cur_px, cur_py, units.map, type_id, owner_id ) ) {
			// Try to find a suitable place

			int X = cur_px-(unit_manager.unit_type[type_id].FootprintX>>1);
			int Y = cur_py-(unit_manager.unit_type[type_id].FootprintZ>>1);
			bool found = false;
			for( int r = 1 ; r < 20 && !found ; r++ ) {		// Circular check
				int r2 = r * r;
				for( int y = 0 ; y <= r ; y++ ) {
					int x = (int)(sqrt( r2 - y * y ) + 0.5f);
					if( can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) ) {
						cur_px += x;
						cur_py += y;
						found = true;
						break;
						}
					if( can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) ) {
						cur_px -= x;
						cur_py += y;
						found = true;
						break;
						}
					if( can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) ) {
						cur_px += x;
						cur_py -= y;
						found = true;
						break;
						}
					if( can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) ) {
						cur_px -= x;
						cur_py -= y;
						found = true;
						break;
						}
					}
				}
			if( found ) {
				Pos.x = (cur_px<<3) + 8 - units.map->map_w_d;
				Pos.z = (cur_py<<3) + 8 - units.map->map_h_d;
				if( mission && (mission->flags & MISSION_FLAG_MOVE) )
					mission->flags |= MISSION_FLAG_REFRESH_PATH;
				}

			}

		units.map->rect( cur_px-(unit_manager.unit_type[type_id].FootprintX>>1), cur_py-(unit_manager.unit_type[type_id].FootprintZ>>1), unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx, unit_manager.unit_type[type_id].yardmap, port[YARD_OPEN]!=0.0f );
		drawn_open = port[YARD_OPEN]!=0.0f;
		}
	drawn_x = cur_px;
	drawn_y = cur_py;
	drawn = true;
}

void UNIT::clear_from_map()
{
	if( !drawn )	return;

	drawn = false;
	if( drawn_flying )
		units.map->air_rect( drawn_x-(unit_manager.unit_type[type_id].FootprintX>>1), drawn_y-(unit_manager.unit_type[type_id].FootprintZ>>1), unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, idx, true );
	else
		units.map->rect( drawn_x-(unit_manager.unit_type[type_id].FootprintX>>1), drawn_y-(unit_manager.unit_type[type_id].FootprintZ>>1), unit_manager.unit_type[type_id].FootprintX, unit_manager.unit_type[type_id].FootprintZ, -1, unit_manager.unit_type[type_id].yardmap, drawn_open );
}

void UNIT::draw_on_FOW( bool jamming )
{
	if( hidden || build_percent_left != 0.0f )
		return;

	bool system_activated = (port[ACTIVATION] && unit_manager.unit_type[ type_id ].onoffable) || !unit_manager.unit_type[ type_id ].onoffable;

	if( jamming ) {
		radar_jam_range = system_activated ? (unit_manager.unit_type[ type_id ].RadarDistanceJam >> 3) : 0;
		sonar_jam_range = system_activated ? (unit_manager.unit_type[ type_id ].SonarDistanceJam >> 3) : 0;

		units.map->update_player_visibility( owner_id, cur_px, cur_py, 0, 0, 0, radar_jam_range, sonar_jam_range, true );
		}
	else {
		sint16 cur_sight = (int)h + unit_manager.unit_type[ type_id ].SightDistance >> 3;
		radar_range = system_activated ? (unit_manager.unit_type[ type_id ].RadarDistance >> 3) : 0;
		sonar_range = system_activated ? (unit_manager.unit_type[ type_id ].SonarDistance >> 3) : 0;

		units.map->update_player_visibility( owner_id, cur_px, cur_py, cur_sight, radar_range, sonar_range, 0, 0, false, old_px != cur_px || old_py != cur_py || cur_sight != sight );

		sight = cur_sight;
		old_px = cur_px;
		old_py = cur_py;
		}
}

const void UNIT::play_sound( const String &key )
{
	EnterCS();
	if( owner_id == players.local_human_id && msec_timer - last_time_sound >= units.sound_min_ticks ) {
		last_time_sound = msec_timer;
		sound_manager->PlayTDFSound( unit_manager.unit_type[ type_id ].soundcategory, key , &Pos );
		}
	LeaveCS();
}

void *create_unit(int type_id,int owner,VECTOR pos,MAP *map)
{
	int id = units.create(type_id,owner);
	if(id>=0) {
		units.unit[id].Lock();

		units.unit[id].Pos=pos;
		units.unit[id].build_percent_left=100.0f;
		units.unit[id].cur_px = ((int)(units.unit[id].Pos.x)+map->map_w_d)>>3;
		units.unit[id].cur_py = ((int)(units.unit[id].Pos.z)+map->map_h_d)>>3;
		units.unit[id].UnLock();

		units.unit[id].draw_on_map();

		return &(units.unit[id]);
		}

	return NULL;
}

const bool can_be_there_ai( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id )
{
	if(unit_type_id<0 || unit_type_id>=unit_manager.nb_unit)	return false;	// test the ID

	if(map==NULL)	return false;		// to avoid an error

	int w = unit_manager.unit_type[unit_type_id].FootprintX;
	int h = unit_manager.unit_type[unit_type_id].FootprintZ;
	int x = px-(w>>1);
	int y = py-(h>>1);
	int side = unit_manager.unit_type[unit_type_id].ExtractsMetal == 0.0f ? 12 : 0;
	if(x<0 || y<0 || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))	return false;	// check if it is inside the map

	if(!map->check_rect( px - (w + side>>1), py - (h + side>>1), w + side, h + side, unit_id))	return false;		// There is already something
	float dh = fabs(map->check_rect_dh(x,y,w,h));
	float max_depth = map->check_max_depth(x,y,w,h);
	float min_depth = map->check_min_depth(x,y,w,h);

	if(dh>unit_manager.unit_type[unit_type_id].MaxSlope*H_DIV
	&& !( unit_manager.unit_type[unit_type_id].canhover && min_depth <= map->sealvl ) )	return false;	// Check the slope, check if hovering too

			// Check if unit can be there
	if(min_depth<unit_manager.unit_type[unit_type_id].MinWaterDepth*H_DIV
	|| (!unit_manager.unit_type[unit_type_id].canhover && max_depth>unit_manager.unit_type[unit_type_id].MaxWaterDepth*H_DIV))	return false;

	if(!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id].yardmap))	return false;

	if(map->check_lava(x+1>>1,y+1>>1,w+1>>1,h+1>>1))	return false;

	return true;
}

const bool can_be_there( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id )
{
	if(unit_type_id<0 || unit_type_id>=unit_manager.nb_unit)	return false;	// test the ID

	if(map==NULL)	return false;		// to avoid an error

	int w = unit_manager.unit_type[unit_type_id].FootprintX;
	int h = unit_manager.unit_type[unit_type_id].FootprintZ;
	int x = px-(w>>1);
	int y = py-(h>>1);
	if(x<0 || y<0 || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))	return false;	// check if it is inside the map

	if(!map->check_rect(x,y,w,h,unit_id))	return false;		// There is already something
	float dh = fabs(map->check_rect_dh(x,y,w,h));
	float max_depth = map->check_max_depth(x,y,w,h);
	float min_depth = map->check_min_depth(x,y,w,h);

	if(dh>unit_manager.unit_type[unit_type_id].MaxSlope*H_DIV
	&& !( unit_manager.unit_type[unit_type_id].canhover && min_depth <= map->sealvl ) )	return false;	// Check the slope, check if hovering too

			// Check if unit can be there
	if(min_depth<unit_manager.unit_type[unit_type_id].MinWaterDepth*H_DIV
	|| (!unit_manager.unit_type[unit_type_id].canhover && max_depth>unit_manager.unit_type[unit_type_id].MaxWaterDepth*H_DIV))	return false;

	if(!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id].yardmap))	return false;

	if(map->check_lava(x+1>>1,y+1>>1,w+1>>1,h+1>>1))	return false;

	return true;
}

const bool can_be_built(const VECTOR Pos,MAP *map,const int unit_type_id, const int player_id )
{
	if(unit_type_id<0 || unit_type_id>=unit_manager.nb_unit)	return false;	// test the ID

	if(map==NULL)	return false;		// to avoid an error

	int w = unit_manager.unit_type[unit_type_id].FootprintX;
	int h = unit_manager.unit_type[unit_type_id].FootprintZ;
	int x = ((int)(Pos.x+map->map_w_d)>>3)-(w>>1);
	int y = ((int)(Pos.z+map->map_h_d)>>3)-(h>>1);
	if(x<0 || y<0 || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))	return false;	// check if it is inside the map

	if(!map->check_rect(x,y,w,h,-1))	return false;		// There already something
	float dh = fabs(map->check_rect_dh(x,y,w,h));
	float max_depth = map->check_max_depth(x,y,w,h);
	float min_depth = map->check_min_depth(x,y,w,h);

	if( !map->check_rect_discovered( x, y, w, h, 1<<player_id ) )	return false;

	if(dh>unit_manager.unit_type[unit_type_id].MaxSlope*H_DIV)	return false;	// Check the slope

			// Check if unit can be there
	if(min_depth<unit_manager.unit_type[unit_type_id].MinWaterDepth*H_DIV || max_depth>unit_manager.unit_type[unit_type_id].MaxWaterDepth*H_DIV)	return false;
//	if(depth>0 && (unit_manager.unit_type[unit_type_id].Category&NOTSUB))	return false;

	if(!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id].yardmap))	return false;

	if(map->check_lava(x+1>>1,y+1>>1,w+1>>1,h+1>>1))	return false;

	return true;
}

void INGAME_UNITS::complete_menu(int index,bool hide_info,bool hide_bpic,int dec_x,int dec_y)
{
	EnterCS();

	bool	pointed_only = false;
	if( last_on >= 0 && ( last_on >= max_unit || unit[ last_on ].flags == 0 ) ) 	last_on = -1;
	if(index<0 || index>=max_unit || unit[index].flags==0 || unit[index].type_id < 0 ) {
		if( last_on >= 0 )
			pointed_only = true;
		else {
			LeaveCS();
			return;		// On n'affiche que des données sur les unités EXISTANTES
			}
		}

	set_uformat(U_ASCII);

	UNIT *target = pointed_only ? NULL : (unit[index].mission!=NULL ? (UNIT*) unit[index].mission->p : NULL);
	if(target && target->flags==0)
		target=NULL;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	if( !pointed_only && !hide_bpic ) {
		int stock=0;
		if(unit_manager.unit_type[unit[index].type_id].weapon[0] && unit_manager.unit_type[unit[index].type_id].weapon[0]->stockpile)	stock=unit[index].weapon[0].stock;
		else if(unit_manager.unit_type[unit[index].type_id].weapon[1] && unit_manager.unit_type[unit[index].type_id].weapon[1]->stockpile)	stock=unit[index].weapon[1].stock;
		else if(unit_manager.unit_type[unit[index].type_id].weapon[2] && unit_manager.unit_type[unit[index].type_id].weapon[2]->stockpile)	stock=unit[index].weapon[2].stock;
		if((unit_manager.unit_type[unit[index].type_id].Builder && !unit_manager.unit_type[unit[index].type_id].BMcode)
		|| unit[index].planned_weapons>0.0f || stock>0) {		// Affiche la liste de construction
			int page=unit_manager.unit_type[unit[index].type_id].page;

			glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
			for(int i=6*page;i<min((int)unit_manager.unit_type[unit[index].type_id].nb_unit,6*page+6);i++) {		// Affiche les différentes images d'unités constructibles
				int nb=0;
				MISSION *m=unit[index].mission;
				while(m) {
					if((m->mission==MISSION_BUILD || m->mission==MISSION_BUILD_2) && m->data==unit_manager.unit_type[unit[index].type_id].BuildList[i])
						nb++;
					m=m->next;
					}
				if(nb>0) {
					char buf[10];
					uszprintf(buf,10,"%d",nb);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					gfx->print(gfx->TA_font,(i&1)*64+32-0.5f*gfx->TA_font.length(buf)+dec_x,dec_y+(i-6*page>>1)*64+32-0.5f*gfx->TA_font.height(),0.0f,0xFFFFFFFF,buf);
					}
				else if(unit_manager.unit_type[unit[index].type_id].BuildList[i]==-1) {		// Il s'agit d'une arme / It's a weapon
					char buf[10];
					if((int)unit[index].planned_weapons==unit[index].planned_weapons)
						uszprintf(buf,10,"%d(%d)",(int)unit[index].planned_weapons,stock);
					else
						uszprintf(buf,10,"%d(%d)",(int)unit[index].planned_weapons+1,stock);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					gfx->print(gfx->TA_font,dec_x+(i&1)*64+32-0.5f*gfx->TA_font.length(buf),dec_y+(i-6*page>>1)*64+32-0.5f*gfx->TA_font.height(),0.0f,0xFFFFFFFF,buf);
					}
				}
			}
		}

	if( last_on >= 0 ) {
		index = last_on;
		if( unit[index].owner_id == players.local_human_id ) {
			target = unit[index].mission!=NULL ? (UNIT*) unit[index].mission->p : NULL;
			if(target && target->flags==0)
				target=NULL;
			}
		else
			target = NULL;
		}

	if( !hide_info ) {
		LeaveCS();

		unit[index].Lock();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		gfx->print_center(gfx->normal_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitName.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitName.y1,0.0f,0xFFFFFFFF,unit_manager.unit_type[unit[index].type_id].name);
		if(target && unit[index].mission && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON) != MISSION_FLAG_TARGET_WEAPON) {
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			gfx->print_center(gfx->normal_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitName2.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitName2.y1,0.0f,0xFFFFFFFF,unit_manager.unit_type[target->type_id].name);
			}
		else if( unit[index].planned_weapons>0.0f && unit[index].owner_id == players.local_human_id ) {
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			gfx->print_center(gfx->normal_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitName2.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitName2.y1,0.0f,0xFFFFFFFF,TRANSLATE("weapon"));
			}

		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		if( unit[index].owner_id == players.local_human_id  ) {
			char buf[10];
			gfx->set_color( ta3d_sidedata.side_int_data[ players.side_view ].metal_color );
			uszprintf(buf,10,"+%f",unit[index].cur_metal_prod);	*(strstr(buf,".")+2)=0;
			gfx->print_center(gfx->small_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitMetalMake.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitMetalMake.y1,0.0f,buf);
			uszprintf(buf,10,"-%f",unit[index].cur_metal_cons);	*(strstr(buf,".")+2)=0;
			gfx->print_center(gfx->small_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitMetalUse.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitMetalUse.y1,0.0f,buf);

			gfx->set_color( ta3d_sidedata.side_int_data[ players.side_view ].energy_color );
			uszprintf(buf,10,"+%f",unit[index].cur_energy_prod);	*(strstr(buf,".")+2)=0;
			gfx->print_center(gfx->small_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitEnergyMake.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitEnergyMake.y1,0.0f,buf);
			uszprintf(buf,10,"-%f",unit[index].cur_energy_cons);	*(strstr(buf,".")+2)=0;
			gfx->print_center(gfx->small_font, ta3d_sidedata.side_int_data[ players.side_view ].UnitEnergyUse.x1, ta3d_sidedata.side_int_data[ players.side_view ].UnitEnergyUse.y1,0.0f,buf);
			}

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glDisable(GL_TEXTURE_2D);

		glDisable(GL_BLEND);

		glBegin(GL_QUADS);
			glColor4f(1.0f,0.0f,0.0f,1.0f);

			if( unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id].HideDamage ) {
				glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y1 );
				glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y1 );
				glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y2 );
				glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y2 );
				}

			if( unit[index].owner_id == players.local_human_id ) {
				if(target && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON)!=MISSION_FLAG_TARGET_WEAPON && !unit_manager.unit_type[target->type_id].HideDamage) {			// Si l'unité a une cible
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					}
				else if( unit[index].planned_weapons>0.0f ) {
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					glVertex2i( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					}
				}

			glColor3f(0.0f,1.0f,0.0f);

			if( unit[index].hp>0 && ( unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id].HideDamage ) ) {
				glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y1 );
				glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1 + unit[index].hp / unit_manager.unit_type[unit[index].type_id].MaxDamage * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y1 );
				glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1 + unit[index].hp / unit_manager.unit_type[unit[index].type_id].MaxDamage * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y2 );
				glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar.y2 );
				}

			if( unit[index].owner_id == players.local_human_id ) {
				if(target && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON)!=MISSION_FLAG_TARGET_WEAPON && !unit_manager.unit_type[target->type_id].HideDamage) {			// Si l'unité a une cible
					if(target->hp>0) {
						glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
						glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1 + target->hp / unit_manager.unit_type[target->type_id].MaxDamage * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
						glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1 + target->hp / unit_manager.unit_type[target->type_id].MaxDamage * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
						glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
						}
					}
				else if( unit[index].planned_weapons>0.0f ) {						// construit une arme / build a weapon
					float p=1.0f-(unit[index].planned_weapons-(int)unit[index].planned_weapons);
					if(p==1.0f)	p=0.0f;
					glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1 + p * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y1 );
					glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1 + p * (ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x2-ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1), ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					glVertex2f( ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.x1, ta3d_sidedata.side_int_data[ players.side_view ].DamageBar2.y2 );
					}
				}

		glEnd();

		unit[index].UnLock();
		EnterCS();
		}
	else {
		glDisable( GL_BLEND );
		glDisable( GL_TEXTURE_2D );
		}
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	set_uformat(U_UTF8);

	LeaveCS();
}

void INGAME_UNITS::move(float dt,MAP *map,int key_frame,bool wind_change)
{
	if(nb_unit<=0 || unit==NULL)	{	rest( 1 );	return;	}		// No units to move

	players.clear();		// Réinitialise le compteur de ressources

	if( requests.empty() )
		requests.resize( 10 );

	int pathfinder_calls[ 10 ];

	for( int i = 0 ; i < 10 ; i++ )
		pathfinder_calls[ i ] = requests[ i ].empty() ? -1 : requests[ i ].front();

	uint32 i;
	for( uint16 e = 0 ; e < index_list_size ; e++ ) {		// Compte les stocks de ressources et les productions
		EnterCS();
		i = idx_list[e];
		LeaveCS();

		unit[i].Lock();
	
		if( unit[i].just_created && unit_manager.unit_type[unit[i].type_id].ExtractsMetal ) {	// Compute amount of metal extracted by sec
			int metal_base = 0;
			int px=unit[i].cur_px;
			int py=unit[i].cur_py;
			int end_y = py + (unit_manager.unit_type[unit[i].type_id].FootprintZ >> 1 );
			int end_x = px + (unit_manager.unit_type[unit[i].type_id].FootprintX >> 1 );
			int start_x = px - (unit_manager.unit_type[unit[i].type_id].FootprintX >> 1 );
			for( int ry = py - (unit_manager.unit_type[unit[i].type_id].FootprintZ >> 1 ) ; ry <= end_y ; ry++ )
				if( ry >= 0 && ry < map->bloc_h_db )
					for( int rx = start_x ; rx <= end_x ; rx++ )
						if( rx >= 0 && rx < map->bloc_w_db )
							if(map->map_data[ry][rx].stuff>=0)
								metal_base += feature_manager.feature[features.feature[map->map_data[ry][rx].stuff].type].metal;
			if( metal_base == 0 ) {
				if( unit_manager.unit_type[unit[i].type_id].fastCategory & CATEGORY_LEVEL3 )
					metal_base = map->ota_data.MohoMetal+map->ota_data.SurfaceMetal;
				else
					metal_base = map->ota_data.SurfaceMetal;
				}
			unit[i].metal_extracted = 9.09f*metal_base*unit_manager.unit_type[unit[i].type_id].ExtractsMetal;

			int param[] = { metal_base<<2 };
			unit[i].run_script_function( map, unit[i].get_script_index(SCRIPT_SetSpeed),1,param);
			unit[i].just_created=false;
			}

		if(unit[i].build_percent_left==0.0f) {
			unit[i].metal_prod=0.0f;
			unit[i].metal_cons=0.0f;
			unit[i].energy_prod=0.0f;
			unit[i].energy_cons=0.0f;
			players.c_metal_s[unit[i].owner_id]+=unit_manager.unit_type[unit[i].type_id].MetalStorage;
			players.c_energy_s[unit[i].owner_id]+=unit_manager.unit_type[unit[i].type_id].EnergyStorage;
			players.c_commander[unit[i].owner_id]|=(unit_manager.unit_type[unit[i].type_id].TEDclass==CLASS_COMMANDER);
			unit[i].energy_prod+=unit_manager.unit_type[unit[i].type_id].EnergyMake;
			if((unit[i].port[ACTIVATION] || !unit_manager.unit_type[unit[i].type_id].onoffable)
			&& unit_manager.unit_type[unit[i].type_id].EnergyUse<=players.energy[unit[i].owner_id]) {
				unit[i].metal_prod+=unit_manager.unit_type[unit[i].type_id].MakesMetal+unit_manager.unit_type[unit[i].type_id].MetalMake;
				if(unit_manager.unit_type[unit[i].type_id].ExtractsMetal)	// Extracteur de métal
					unit[i].metal_prod += unit[i].metal_extracted;
				if(unit_manager.unit_type[unit[i].type_id].WindGenerator) {	// Wind Generator
					unit[i].energy_prod+=map->wind*unit_manager.unit_type[unit[i].type_id].WindGenerator*0.0002f;
					if(wind_change) {
						int param[] = { (int)(map->wind*50.0f) };
						unit[i].launch_script(unit[i].get_script_index(SCRIPT_SetSpeed),1,param);
						param[0]=(int)((map->wind_dir-unit[i].Angle.y)*DEG2TA);
						unit[i].launch_script(unit[i].get_script_index(SCRIPT_SetDirection),1,param);
						unit[i].launch_script(unit[i].get_script_index(SCRIPT_go));
						}
					}
				if(unit_manager.unit_type[unit[i].type_id].TidalGenerator)	// Tidal Generator
					unit[i].energy_prod+=map->ota_data.tidalstrength;
				if(unit_manager.unit_type[unit[i].type_id].EnergyUse<0)
					unit[i].energy_prod-=unit_manager.unit_type[unit[i].type_id].EnergyUse;
				else
					unit[i].energy_cons=unit_manager.unit_type[unit[i].type_id].EnergyUse;
				}
			}
		unit[i].UnLock();
		}

	exp_dt_1=exp(-dt);
	exp_dt_2=exp(-2.0f*dt);
	exp_dt_4=exp(-4.0f*dt);
	g_dt=dt*map->ota_data.gravity;
	int *path_exec = new int[ players.nb_player ];
	memset( path_exec, 0, sizeof( int ) * players.nb_player );
	for( uint16 e = 0 ; e < index_list_size ; e++ ) {
		EnterCS();
		i = idx_list[e];
		LeaveCS();

		unit[ i ].Lock();

		if( !(unit[ i ].flags & 1) ) {		// ho ho what is it doing there ??
			unit[ i ].clear_from_map();
			kill(i,map,e);
			e--;			// Can't skip a unit
			unit[ i ].UnLock();
			continue;
			}

		if(unit[i].owner_id==players.local_human_id) {
			if(unit[i].attacked
			|| (unit[i].mission!=NULL && unit[i].mission->mission==MISSION_ATTACK))	nb_attacked+=100;
			if(unit[i].built)		nb_built++;
			}
		players.c_nb_unit[unit[i].owner_id]++;			// Compte les unités de chaque joueur
		unit[ i ].UnLock();
		if(unit[i].move(dt,map,path_exec,key_frame)==-1) {			// Vérifie si l'unité a été détruite
			kill(i,map,e);
			e--;			// Can't skip a unit
			}
		else {
			unit[ i ].Lock();
			players.c_metal_t[unit[i].owner_id] += unit[i].metal_prod;
			players.c_metal_u[unit[i].owner_id] += unit[i].metal_cons;
			players.c_energy_t[unit[i].owner_id] += unit[i].energy_prod;
			players.c_energy_u[unit[i].owner_id] += unit[i].energy_cons;

			unit[i].cur_energy_cons = unit[i].energy_cons;
			unit[i].cur_energy_prod = unit[i].energy_prod;
			unit[i].cur_metal_cons = unit[i].metal_cons;
			unit[i].cur_metal_prod = unit[i].metal_prod;
			unit[ i ].UnLock();
			}
		}

	delete[] path_exec;

	float exp_r = exp(-dt*0.1f);
	nb_attacked*=exp_r;
	nb_built*=exp_r;

	EnterCS();

	for(i=0;i<players.nb_player;i++) {
		players.c_annihilated[ i ] = !players.c_nb_unit[ i ];		// Has this player units ?
		if(players.c_commander[i]) {
			players.c_metal_s[i]+=players.com_metal[i];
			players.c_energy_s[i]+=players.com_energy[i];
			}
		}
	for(i=0;i<players.nb_player;i++) {
		players.c_metal[i]+=dt*(players.c_metal_t[i]-players.c_metal_u[i]);
		players.c_energy[i]+=dt*(players.c_energy_t[i]-players.c_energy_u[i]);
		players.metal_total[i]+=dt*players.metal_t[i];
		players.energy_total[i]+=dt*players.energy_t[i];
		if(players.c_metal[i]<0.0f)
			players.c_metal[i]=0.0f;
		else if(players.c_metal[i]>players.c_metal_s[i])
			players.c_metal[i]=players.c_metal_s[i];
		if(players.c_energy[i]<0.0f)
			players.c_energy[i]=0.0f;
		else if(players.c_energy[i]>players.c_energy_s[i])
			players.c_energy[i]=players.c_energy_s[i];
		}

	for( int i = 0 ; i < 10 ; i++ )
		if( !requests[ i ].empty() && pathfinder_calls[ i ] == requests[ i ].front() )
			requests[ i ].pop_front();

	players.refresh();

	LeaveCS();
}

int INGAME_UNITS::create(int type_id,int owner)
{
	if(type_id<0 || type_id>=unit_manager.nb_unit)	return -1;
	if(owner<0 || owner>=NB_PLAYERS)	return -1;
	if(nb_unit>=MAX_UNIT_PER_PLAYER*NB_PLAYERS)		return -1;

	EnterCS();

	nb_unit++;
	if(nb_unit>=max_unit) {
		if( mini_idx )		delete[]	mini_idx;
		if( mini_col )		delete[]	mini_col;
		if( mini_pos )		delete[]	mini_pos;

		max_unit=MAX_UNIT_PER_PLAYER*NB_PLAYERS;

		mini_idx = new GLushort[ max_unit ];
		mini_col = new uint32[ max_unit ];
		mini_pos = new float[ max_unit * 2 ];

		for( int i = 0 ; i < max_unit ; i++ )
			mini_idx[ i ] = i;

		UNIT *n_unit=(UNIT*) malloc(sizeof(UNIT)*max_unit);
		uint16	*n_idx = new uint16[max_unit];
		uint16	*n_new_idx = new uint16[max_unit];
		if(index_list_size>0)
			memcpy(n_idx,idx_list,index_list_size<<1);
		if(free_idx)
			memcpy(n_new_idx,free_idx,max_unit<<1);
		if(idx_list)	delete[]	idx_list;
		if(free_idx)	delete[]	free_idx;
		idx_list = n_idx;
		free_idx = n_new_idx;
		for(uint16 i = 0; i<max_unit;i++)
			free_idx[i] = i;
		for(uint16 i = 0; i<10;i++)
			free_index_size[i] = MAX_UNIT_PER_PLAYER;
		for(int i=0;i<max_unit;i++) {
			n_unit[i].init(-1,-1,i>=nb_unit-1);
			n_unit[i].flags=0;
			n_unit[i].idx=i;
			}
		if(unit) {
			memcpy(n_unit,unit,sizeof(UNIT)*(nb_unit-1));
			free(unit);
			}
		unit=n_unit;
		}
		if(unit==NULL)
			printf("error: memory alloc failed\n");
		if(free_index_size[owner]<=0) {
			printf("unit limit reached!\n");

		LeaveCS();

		return -1;
		}
	int unit_index = free_idx[owner*MAX_UNIT_PER_PLAYER+free_index_size[owner]-1];
	free_index_size[owner]--;
	idx_list[index_list_size++] = unit_index;
	unit[unit_index].init(type_id,owner);
	unit[unit_index].Angle.y=((rand_from_table()%2001)-1000)*0.01f;	// Angle de 10° maximum

	players.nb_unit[owner]++;

	LeaveCS();

	return unit_index;
}

void INGAME_UNITS::draw_mini(float map_w,float map_h,int mini_w,int mini_h,SECTOR **map_data)				// Repère les unités sur la mini-carte
{
	if(nb_unit<=0 || unit==NULL)	{
		last_on = -1;
		return;		// Pas d'unités à dessiner
		}

	float rw = 128.0f * mini_w / 252 / map_w;
	float rh = 128.0f * mini_h / 252 / map_h;

	glDisable(GL_TEXTURE_2D);
	glPointSize(3.0f);

	byte mask=1<<players.local_human_id;
	int b_w=(int)map_w>>3;
	int b_h=(int)map_h>>3;
	int nb = 0;

	uint32 player_col_32[ 10 ];
	uint32 player_col_32_h[ 10 ];
	for( int i = 0 ; i < players.nb_player ; i++ ) {
		player_col_32[ i ] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 255.0f),
										(int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 255.0f),
										(int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 255.0f),
										i );
		player_col_32_h[ i ] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 127.5f),
										(int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 127.5f),
										(int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 127.5f),
										i );
		}

	for( uint16 e=0 ; e < index_list_size ; e++) {
		uint16 i = idx_list[e];

		units.unit[ i ].Lock();

		if(unit[i].flags&1) {
			int px=unit[i].cur_px;
			int py=unit[i].cur_py;
			if(px<0 || py<0 || px>=b_w || py>=b_h) {
				units.unit[ i ].UnLock();
				continue;
				}
			if( (!(map->view_map->line[py>>1][px>>1]&mask) || !(map->sight_map->line[py>>1][px>>1]&mask)) && !unit[i].on_mini_radar ) {
				units.unit[ i ].UnLock();
				continue;	// Unité non visible / Unit is not visible
				}
//			unit[i].flags|=0x10;
			mini_pos[ nb << 1 ] = unit[i].Pos.x;
			mini_pos[ (nb << 1) + 1 ] = unit[i].Pos.z;
			mini_col[ nb++ ] = player_col_32_h[ unit[i].owner_id ];
			}
		units.unit[ i ].UnLock();
		}
	glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
	glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);		// vertex coordinates

	glColorPointer(4, GL_UNSIGNED_BYTE, 0, mini_col);
	glVertexPointer( 2, GL_FLOAT, 0, mini_pos);
	glPushMatrix();
	glTranslatef( 63.0f, 64.0f, 0.0f );
	glScalef( rw, rh, 0.0f );
	glDrawElements(GL_POINTS, nb, GL_UNSIGNED_SHORT, mini_idx);		// draw the points

	glPopMatrix();
	glPushMatrix();
	glTranslatef( 65.0f, 64.0f, 0.0f );
	glScalef( rw, rh, 0.0f );
	glDrawElements(GL_POINTS, nb, GL_UNSIGNED_SHORT, mini_idx);		// draw the points

	glPopMatrix();
	glPushMatrix();
	glTranslatef( 64.0f, 65.0f, 0.0f );
	glScalef( rw, rh, 0.0f );
	glDrawElements(GL_POINTS, nb, GL_UNSIGNED_SHORT, mini_idx);		// draw the points

	glPopMatrix();
	glPushMatrix();
	glTranslatef( 64.0f, 63.0f, 0.0f );
	glScalef( rw, rh, 0.0f );
	glDrawElements(GL_POINTS, nb, GL_UNSIGNED_SHORT, mini_idx);		// draw the points

	glPopMatrix();
	glPushMatrix();
	glTranslatef( 64.0f, 64.0f, 0.0f );
	glScalef( rw, rh, 0.0f );
	for( int i = 0 ; i < nb ; i++ )
		mini_col[ i ] = player_col_32[ geta( mini_col[ i ] ) ];
	glDrawElements(GL_POINTS, nb, GL_UNSIGNED_SHORT, mini_idx);		// draw the points
	glPopMatrix();

	int cur_id = -1;
	glBegin( GL_POINTS );
	for(uint16 e=0;e<index_list_size;e++) {
		uint16 i = idx_list[e];

		units.unit[ i ].Lock();
		if( units.unit[ i ].cur_px < 0 || units.unit[ i ].cur_py < 0 || units.unit[ i ].cur_px >= b_w || units.unit[ i ].cur_py >= b_h ) {
			units.unit[ i ].UnLock();
			continue;
			}

		if( (unit[i].flags&1) && ( (unit[i].owner_id==players.local_human_id && unit[i].sel) || i == last_on ) ) {
			cur_id = unit[i].owner_id;
			float pos_x=unit[i].Pos.x*rw+64.0f;
			float pos_y=unit[i].Pos.z*rh+64.0f;
			if( unit[i].radar_range > 0 ) {
				glEnd();
				glPointSize(1.0f);
				gfx->circle_zoned( pos_x, pos_y, (unit[i].radar_range << 3) * rw, 0.0f, 0.0f, 127.0f, 127.0f, 0xFFFFFFFF );
				glPointSize(3.0f);
				glBegin( GL_POINTS );
				}
			if( unit[i].sonar_range > 0 ) {
				glEnd();
				glPointSize(1.0f);
				gfx->circle_zoned( pos_x, pos_y, (unit[i].sonar_range << 3) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makecol( 0, 255, 0 ) );
				glPointSize(3.0f);
				glBegin( GL_POINTS );
				}
			glColor3f(1.0f,1.0f,1.0f);
			glVertex2f(pos_x-1.0f,pos_y);
			glVertex2f(pos_x+1.0f,pos_y);
			glVertex2f(pos_x,pos_y-1.0f);
			glVertex2f(pos_x,pos_y+1.0f);

			glColor3f(player_color[3*player_color_map[cur_id]],player_color[3*player_color_map[cur_id]+1],player_color[3*player_color_map[cur_id]+2]);
			glVertex2f(pos_x,pos_y);
			}
		units.unit[ i ].UnLock();
		}
	glEnd();
	glPointSize(1.0f);
	glEnable(GL_TEXTURE_2D);

	last_on = -1;
}

void INGAME_UNITS::kill(int index,MAP *map,int prev)			// Détruit une unité
{
	if(index<0 || index>max_unit || prev<0 || prev>=index_list_size)	return;		// On ne peut pas détruire une unité qui n'existe pas

	unit[index].Lock();

	if(unit[index].flags) {
		if( unit[index].flags & 1 ) {
			if( unit[ index ].mission
			&& !unit_manager.unit_type[ unit[ index ].type_id ].BMcode
			&& ( unit[ index ].mission->mission == MISSION_BUILD_2 || unit[ index ].mission->mission == MISSION_BUILD )		// It was building something that we must destroy too
			&& unit[ index ].mission->p != NULL ) {
				((UNIT*)(unit[ index ].mission->p))->Lock();
				((UNIT*)(unit[ index ].mission->p))->hp = 0.0f;
				((UNIT*)(unit[ index ].mission->p))->built = false;
				((UNIT*)(unit[ index ].mission->p))->UnLock();
				}
			unit[index].clear_from_map();
			players.nb_unit[ unit[index].owner_id ]--;
			players.losses[ unit[index].owner_id ]++;		// Statistiques
			}
		if(unit_manager.unit_type[unit[index].type_id].canload && unit[index].nb_attached>0)
			for( int i = 0 ; i < unit[index].nb_attached ; i++ ) {
				unit[unit[index].attached_list[i]].Lock();
				unit[unit[index].attached_list[i]].hp = 0.0f;
				unit[unit[index].attached_list[i]].UnLock();
				}
		unit[index].destroy();		// Détruit l'unité

		EnterCS();

		uint16 owner = index/MAX_UNIT_PER_PLAYER;
		free_idx[ MAX_UNIT_PER_PLAYER * owner + free_index_size[ owner ]++ ] = index;
		idx_list[ prev ] = idx_list[ --index_list_size ];
		nb_unit--;		// Unité détruite

		LeaveCS();
		}

	unit[index].UnLock();
}

void INGAME_UNITS::draw(CAMERA *cam,MAP *map,bool underwater,bool limit,bool cullface,bool height_line)					// Dessine les unités visibles
{
	if(nb_unit<=0 || unit==NULL)	return;		// Pas d'unités à dessiner

	glEnable(GL_LIGHTING);
	if( cullface )
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	float sea_lvl = limit ? map->sealvl-5.0f : map->sealvl;
	float virtual_t = (float)current_tick / TICKS_PER_SEC;
	cam->SetView();
	for(uint16 e=0;e<index_list_size;e++) {
		EnterCS();
		uint16 i = idx_list[e];
		LeaveCS();

		gfx->GFX_EnterCS();
		unit[i].Lock();
		if( (unit[i].flags & 1) && ((unit[i].Pos.y + unit[i].model->bottom <= map->sealvl && underwater) || (unit[i].Pos.y + unit[i].model->top >= sea_lvl && !underwater)))				// Si il y a une unité
			unit[i].draw(virtual_t,cam,map,height_line);
		unit[i].UnLock();
		gfx->GFX_LeaveCS();
		}

	if( !cullface )
		glEnable(GL_CULL_FACE);
}

void INGAME_UNITS::draw_shadow(CAMERA *cam,VECTOR Dir,MAP *map,float alpha)					// Dessine les ombres des unités visibles
{
	if(nb_unit<=0 || unit==NULL)	return;		// Pas d'unités à dessiner

	cam->SetView();

	if(g_useStencilTwoSide) {					// Si l'extension GL_EXT_stencil_two_side est disponible
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		glColorMask(0,0,0,0);
		glStencilFunc(GL_ALWAYS,128, 0xffffffff);

		glActiveStencilFaceEXT(GL_FRONT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
		glActiveStencilFaceEXT(GL_BACK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);

		for(uint16 e=0;e<index_list_size;e++) {
			EnterCS();
			uint16 i = idx_list[e];
			LeaveCS();

			gfx->GFX_EnterCS();

			unit[i].Lock();
			if(unit[i].flags & 1)				// Si il y a une unité
				unit[i].draw_shadow(cam,Dir,map);
			unit[i].UnLock();

			gfx->GFX_LeaveCS();
			}
		}
	else {										// Si elle ne l'est pas
		glDepthMask(GL_FALSE);
		glColorMask(0,0,0,0);

		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS,128, 0xffffffff);
		glEnable(GL_CULL_FACE);

		for(uint16 e=0;e<index_list_size;e++) {
			EnterCS();
			uint16 i = idx_list[e];
			LeaveCS();

			gfx->GFX_EnterCS();
			unit[i].Lock();
			if(unit[i].flags & 1)					// Si il y a une unité
				unit[i].draw_shadow_basic(cam,Dir,map);
			unit[i].UnLock();
			gfx->GFX_LeaveCS();
			}
		}

	gfx->GFX_EnterCS();
	features.draw_shadow(cam,Dir);

	glColorMask(0xFF,0xFF,0xFF,0xFF);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4f(0.0f,0.0f,0.0f,alpha);
	glBegin(GL_QUADS);
		VECTOR P = cam->RPos + cam->shake_vec + 1.1f*( cam->Dir + 0.75f * cam->Up - cam->width_factor * cam->Side );
		glVertex3fv( (const GLfloat*) &P );
		P = cam->RPos + cam->shake_vec + 1.1f * ( cam->Dir + 0.75f * cam->Up + cam->width_factor * cam->Side );
		glVertex3fv( (const GLfloat*) &P );
		P = cam->RPos + cam->shake_vec + 1.1f * ( cam->Dir - 0.75f * cam->Up + cam->width_factor * cam->Side );
		glVertex3fv( (const GLfloat*) &P );
		P = cam->RPos + cam->shake_vec + 1.1f * ( cam->Dir - 0.75f * cam->Up - cam->width_factor * cam->Side );
		glVertex3fv( (const GLfloat*) &P );
	glEnd();
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	gfx->GFX_LeaveCS();
}

void INGAME_UNITS::remove_order(int player_id,VECTOR target)
{
	EnterCS();
	for(uint16 e=0;e<index_list_size;e++) {
		uint16 i = idx_list[e];
		if(unit[i].flags!=0 && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left==0.0f ) {	// && unit_manager.unit_type[unit[i].type_id].Builder) {
			MISSION *mission = unit_manager.unit_type[unit[i].type_id].BMcode ? unit[i].mission : unit[i].def_mission;
			MISSION *prec = mission;
			if( mission != NULL && unit_manager.unit_type[unit[i].type_id].BMcode )		mission = mission->next;		// Don't read the first one ( which is being executed )

			MISSION fake;
			if( !unit_manager.unit_type[unit[i].type_id].BMcode ) {			// It's a hack to make sure it will work with first given order
				fake.next = mission;
				prec = &fake;
				}

			while( mission ) {					// Reads the mission list
				if( mission->mission == MISSION_BUILD ) {
					prec = mission;
					mission = mission->next;
					}
				else {
					if( !mission->step && (mission->target - target).Sq() < 256.0f ) {		// Remove it
						MISSION *tmp = mission;
						mission = mission->next;
						prec->next = mission;
						if(tmp->path)				// Destroy the path if needed
							destroy_path(tmp->path);
						free( tmp );
						}
					else {
						prec = mission;
						mission = mission->next;
						}
					}
				}
			}
		}
	LeaveCS();
}

uint32 INGAME_UNITS::InterfaceMsg( const lpcImsg msg )
{
	return INTERFACE_RESULT_CONTINUE;						// Temporary, for now it does nothing
}

int INGAME_UNITS::Run()
{
	thread_running = true;
	float dt = 1.0f / TICKS_PER_SEC;
	int unit_timer = msec_timer;
	int tick_timer;
	float counter = 0.0f;
	int tick = 1000 / TICKS_PER_SEC;
	tick_timer = msec_timer;
	float step = 1.0f;
	if( lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
	current_tick = 0;
	apparent_timefactor = lp_CONFIG->timefactor;

	unit_engine_thread_sync = 0;

	ThreadSynchroniser->EnterSync();

	while( !thread_ask_to_stop ) {
		counter += step;

		move( dt, map, current_tick, wind_change );					// Animate units

		EnterCS();

		gfx->GFX_EnterCS();

		if( map->fog_of_war != FOW_DISABLED & (current_tick & 0xF) == 0 ) {
			if( map->fog_of_war & FOW_GREY )
				memset( map->sight_map->line[0], 0, map->sight_map->w * map->sight_map->h );		// Clear FOW map
			memset( map->radar_map->line[0], 0, map->radar_map->w * map->radar_map->h );		// Clear radar map
			memset( map->sonar_map->line[0], 0, map->sonar_map->w * map->sonar_map->h );		// Clear sonar map

			for( int i = 0; i < index_list_size ; i++ )			// update fog of war, radar and sonar data
				unit[ idx_list[ i ] ].draw_on_FOW();

			for( int i = 0; i < index_list_size ; i++ )			// update radar and sonar jamming data
				unit[ idx_list[ i ] ].draw_on_FOW( true );
			}

		gfx->GFX_LeaveCS();

			wind_change = false;
		LeaveCS();

		while( msec_timer - tick_timer < tick )
			rest( 1 );

		while( msec_timer - tick_timer >= tick + 200 ) {		// Prevent the game to run too fast for too long, we don't have to speed up to compute what we hadn't time to
			counter += 1.0f;
			tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
			}

		tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
		if( lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
		else	step = 1.0f;

		ThreadSynchroniser->LeaveSync();

		unit_engine_thread_sync = 1;
		while( unit_engine_thread_sync && !thread_ask_to_stop ) {
			if( unit_engine_thread_sync && weapon_engine_thread_sync && particle_engine_thread_sync && players_thread_sync )	{			// Sync engine threads
				unit_engine_thread_sync = 0;
				weapon_engine_thread_sync = 0;
				particle_engine_thread_sync = 0;
				players_thread_sync = 0;

				current_tick++;		// To have a common time value
				break;
				}
			rest( 1 );			// Wait until other thread sync with this one
			}

		ThreadSynchroniser->EnterSync();

		last_tick[ 0 ] = last_tick[ 1 ];
		last_tick[ 1 ] = last_tick[ 2 ];
		last_tick[ 2 ] = last_tick[ 3 ];
		last_tick[ 3 ] = last_tick[ 4 ];
		last_tick[ 4 ] = msec_timer;

		if( last_tick[ 0 ] != 0 && last_tick[4] != last_tick[0] )
			apparent_timefactor = 4000.0f / ( (last_tick[ 4 ] - last_tick[ 0 ]) * TICKS_PER_SEC );
		}

	ThreadSynchroniser->LeaveSync();

	thread_running = false;
	thread_ask_to_stop = false;

	printf("unit engine: %f ticks/sec.\n", (float)(current_tick * 1000) / ( msec_timer - unit_timer ) );

	return 0;
}

void INGAME_UNITS::SignalExitThread()
{
	if( thread_running )
		thread_ask_to_stop = true;
}
