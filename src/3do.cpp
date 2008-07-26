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
  |                                         3do.cpp                                    |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers 3do de total annihilation qui sont les fichiers contenant les modèles |
  | 3d des objets du jeu.                                                              |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"
#include "gfx/particles/particles.h"
#include "ingame/sidedata.h"
#include "languages/i18n.h"
#include "jpeg/ta3d_jpg.h"
#include "misc/math.h"
#include "logs/logs.h"




namespace TA3D
{


    TEXTURE_MANAGER	texture_manager;
    MODEL_MANAGER	model_manager;



    void TEXTURE_MANAGER::init()
    {
        nbtex = 0;
        tex = NULL;
    }

    void TEXTURE_MANAGER::destroy()
    {
        if (tex)
        {
            for (int i=0; i < nbtex; ++i)
                tex[i].destroy();
            free(tex);
        }
        init();
    }



    int TEXTURE_MANAGER::get_texture_index(const String& texture_name)
    {
        if (nbtex == 0)
            return -1;
        for (int i = 0; i < nbtex; ++i)
        {
            if (texture_name == tex[i].name)
                return i;
        }
        return -1;
    }


    GLuint TEXTURE_MANAGER::get_gl_texture(const String& texture_name, const int frame)
    {
        int index = get_texture_index(texture_name);
        return (index == -1) ? 0 : tex[index].glbmp[frame];
    }


    BITMAP* TEXTURE_MANAGER::get_bmp_texture(const String& texture_name, const int frame)
    {
        int index = get_texture_index(texture_name);
        return (index== -1) ? NULL : tex[index].bmp[frame];
    }


    int TEXTURE_MANAGER::all_texture()
    {
        // Crée des textures correspondant aux couleurs de la palette de TA
        nbtex = 256;
        tex = (ANIM*) malloc(sizeof(ANIM) * nbtex);
        for (int i = 0; i < 256; ++i)
        {
            tex[i].init();
            tex[i].nb_bmp = 1;
            tex[i].bmp = (BITMAP**) malloc(sizeof(BITMAP*));
            tex[i].glbmp = (GLuint*) malloc(sizeof(GLuint));
            tex[i].ofs_x = (short*) malloc(sizeof(short));
            tex[i].ofs_y = (short*) malloc(sizeof(short));
            tex[i].w = (short*) malloc(sizeof(short));
            tex[i].h = (short*) malloc(sizeof(short));
            char tmp[10];
            uszprintf(tmp,10,"_%d",i);
            tex[i].name = strdup(tmp);

            tex[i].ofs_x[0] = 0;
            tex[i].ofs_y[0] = 0;
            tex[i].w[0] = 16;
            tex[i].h[0] = 16;
            tex[i].bmp[0] = create_bitmap_ex(32,16,16);
            clear_to_color(tex[i].bmp[0], makeacol(pal[i].r << 2, pal[i].g << 2, pal[i].b << 2, 0xFF));
        }

        String::List file_list;
        HPIManager->getFilelist("textures\\*.gaf", file_list);
        for (String::List::const_iterator cur_file = file_list.begin(); cur_file != file_list.end(); ++cur_file)
        {
            byte *data=HPIManager->PullFromHPI(*cur_file);
            load_gaf(data);
            delete[] data;
        }
        return 0;
    }


    int TEXTURE_MANAGER::load_gaf(byte* data)
    {
        int nb_entry = get_gaf_nb_entry(data);
        int n_nbtex = nbtex + nb_entry;
        int i;
        ANIM* n_tex = (ANIM*) malloc(sizeof(ANIM) * n_nbtex);
        for (i = 0; i < n_nbtex; ++i)
            n_tex[i].init();
        for (i = 0; i < nbtex; ++i)
            n_tex[i] = tex[i];
        if (tex)
            free(tex);
        tex = n_tex;
        for (i = 0; i < nb_entry; ++i)
            tex[nbtex + i].load_gaf(data, i, false);
        nbtex += nb_entry;
        return 0;
    }


    void SCRIPT_DATA::init()
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

    void SCRIPT_DATA::destroy()
    {
        for (byte i = 0; i < 3; ++i)
        {
            if (axe[i])
                delete[] axe[i];
        }
        if (matrix)
            delete[] matrix;
        if (dir)
            delete[] dir;
        if (pos)
            delete[] pos;
        if (flag)
            delete[] flag;
        if (explosion_flag)
            delete[] explosion_flag;
        init();
    }


    void SCRIPT_DATA::load(const int nb)
    {
        destroy();		// Au cas où
        nb_piece = nb;
        flag = new short[nb_piece];
        explosion_flag = new short[nb_piece];
        pos = new Vector3D[nb_piece];
        dir = new Vector3D[nb_piece];
        matrix = new MATRIX_4x4[nb_piece];
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



    const void SCRIPT_DATA::move(const float dt, const float g)
    {
        if (!is_moving)
            return;
        is_moving = false;

        if (explode_time > 0.0f)
            explode_time -= dt;
        explode = explode_time > 0.0f;

        for (uint16 e = 0; e < nb_piece; ++e)
        {
            if (flag[e] & FLAG_EXPLODE)// && (explosion_flag[e]&EXPLODE_BITMAPONLY)!=EXPLODE_BITMAPONLY)		// This piece is exploding
            {
                for (byte i = 0; i < 3; ++i)
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
                for (byte i = 0; i < 3; ++i)
                {
                    if (!axe[i][e].is_moving)
                        continue;
                    axe[i][e].is_moving = false;
                    float a = axe[i][e].move_distance;
                    if (a != 0.0f)
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
                    if ((axe[i][e].rot_speed!=0.0f || axe[i][e].rot_accel!=0.0f) && ((a!=0.0f && axe[i][e].rot_limit) || !axe[i][e].rot_limit))
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


    bool OBJECT::coupe(int x1,int y1,int dx1,int dy1,int x2,int y2,int dx2,int dy2)
    {
        int u1=x1, v1=y1, u2=x2+dx2, v2=y2+dy2;
        if (u1>x2) u1=x2;
        if (v1>y2) v1=y2;
        if (x1+dx1>u2) u2=x1+dx1;
        if (y1+dy1>v2) v2=y1+dy1;
        return (u2-u1+1<dx1+dx2 && v2-v1+1<dy1+dy2);
    }


    bool OBJECT::compute_emitter()
    {
        emitter=((nb_t_index==0 || nb_vtx==0) && child==NULL && next==NULL);
        if(child)
            emitter|=child->compute_emitter();
        if(next)
            emitter|=next->compute_emitter();
        return emitter;
    }

    bool OBJECT::compute_emitter_point( int &obj_idx )
    {
        emitter_point |= ( script_index == obj_idx );
        emitter |= emitter_point;
        if(child)
            emitter |= child->compute_emitter_point( obj_idx );
        if(next)
            emitter |= next->compute_emitter_point( obj_idx );
        return emitter;
    }

    void OBJECT::init()
    {
        optimised = false;
        optimised_I = NULL;
        optimised_P = NULL;
        optimised_N = NULL;
        optimised_T = NULL;
        optimised_nb_idx = 0;
        optimised_nb_vtx = 0;
        vbo_id = 0;
        ebo_id = 0;

        animation_data = NULL;

        compute_min_max = true;

        last_nb_idx = 0;
        last_dir.x = last_dir.y = last_dir.z = 0.0f;

        use_strips = false;

        line_on=NULL;
        emitter_point=false;
        emitter=false;
        t_line=NULL;
        line_v_idx[0]=NULL;
        line_v_idx[1]=NULL;
        nb_line=0;
        face_reverse=NULL;
        shadow_index=NULL;
        tcoord=NULL;
        dtex=0;
        pos_from_parent.x=pos_from_parent.y=pos_from_parent.z=0.0f;
        nb_vtx=0;
        nb_prim=0;
        name=NULL;
        next=child=NULL;
        points=NULL;
        p_index=NULL;
        l_index=NULL;
        t_index=NULL;
        nb_p_index=0;
        nb_l_index=0;
        nb_t_index=0;
        F_N=NULL;
        N=NULL;
        tex=NULL;
        nb_index=NULL;
        usetex=NULL;
        selprim=-1;
        script_index=-1;
        for (byte i = 0; i < 10; ++i)
            gltex[i] = gl_dlist[i] = 0;
        surface.Flag=0;
        for(byte i = 0; i < 4; ++i)
            surface.Color[i]=surface.RColor[i]=1.0f;
        surface.NbTex=0;
        for(byte i = 0; i < 8; ++i)
            surface.gltex[i]=0;
        surface.s_shader.destroy();
        surface.frag_shader_src.clear();
        surface.vert_shader_src.clear();
    }



    void OBJECT::destroy()
    {
        if (animation_data)
            delete animation_data;
        surface.s_shader.destroy();
        surface.frag_shader_src.clear();
        surface.vert_shader_src.clear();
        if (surface.NbTex > 0)
        {
            for (int i = 0; i < surface.NbTex; ++i)
            {
                if (surface.gltex[i])
                    glDeleteTextures(1, &(surface.gltex[i]));
            }
        }
        if (line_on)
            delete[] line_on;
        if (t_line)			delete[] t_line;
        if (line_v_idx[0])	delete[] line_v_idx[0];
        if (line_v_idx[1])	delete[] line_v_idx[1];
        if (shadow_index)	delete[] shadow_index;
        if (tcoord)			delete[] tcoord;
        if (dtex)
        {
            for(int i=0;i<dtex;i++)
                glDeleteTextures(1,&(gltex[i]));
        }
        for(int i=0;i<10;i++)
        {
            if( gl_dlist[ i ] )
                glDeleteLists(gl_dlist[i],1);
        }
        if (usetex)
            delete[] usetex;
        if (nb_index)
            delete[] nb_index;
        if (tex)				// Ne détruit pas les textures qui le seront par la suite(celles-ci ne sont chargées qu'une fois
            delete[] tex;		// mais peuvent être utilisées par plusieurs objets
        if (face_reverse)
            delete[] face_reverse;
        if (F_N)
            delete[] F_N;
        if (N)
            delete[] N;
        if (points)	
            delete[] points;
        if (p_index)
            delete[] p_index;
        if (l_index)
            delete[] l_index;
        if (t_index)
            delete[] t_index;
        if (name)
            free(name);
        if (optimised_I)
            delete[] optimised_I;
        if (optimised_T)
            delete[] optimised_T;
        if (optimised_P)
            delete[] optimised_P;
        if (optimised_N)
            delete[]optimised_N;
        if (vbo_id)	
            glDeleteBuffersARB( 1, &vbo_id );
        if (ebo_id)
            glDeleteBuffersARB( 1, &ebo_id );
        if (next)
            delete next;
        if (child)
            delete child;
        init();
    }


    void OBJECT::Identify(int nb_piece,char **piece_name)			// Identifie les pièces utilisées par le script
    {
        script_index=-1;				// Pièce non utilisée
        if (name)
            for (int i = 0; i < nb_piece; ++i)
            {
                if (strcasecmp(name,piece_name[i]) == 0) // Pièce identifiée
                {
                    script_index = i;
                    break;
                }
            }
        if (next)
            next->Identify(nb_piece,piece_name);
        if (child)
            child->Identify(nb_piece,piece_name);
    }


    void OBJECT::compute_center(Vector3D *center,Vector3D dec, int *coef)		// Calcule les coordonnées du centre de l'objet, objets liés compris
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


    float OBJECT::compute_size_sq(Vector3D center)		// Carré de la taille(on fera une racine après)
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


    float OBJECT::compute_top(float top, Vector3D dec)
    {
        for(int i = 0;i < nb_vtx; ++i)
            top = Math::Max(top, points[i].y + dec.y + pos_from_parent.y);
        if (next)
            top = next->compute_top(top, dec);
        if (child)
            top = child->compute_top(top, dec + pos_from_parent );
        return top;
    }


    float OBJECT::compute_bottom(float bottom, Vector3D dec)
    {
        for (int i = 0; i < nb_vtx; ++i)
            bottom = Math::Min(bottom, points[i].y + dec.y + pos_from_parent.y);
        if (next)
            bottom = next->compute_bottom(bottom, dec);
        if (child)
            bottom = child->compute_bottom(bottom, dec + pos_from_parent);
        return bottom;
    }

    bool OBJECT::has_animation_data()
    {
        if (animation_data)
            return true;
        if (next)
            return next->has_animation_data();
        if (child)
            return child->has_animation_data();
        return false;
    }


    void OBJECT::optimise_mesh()			// EXPERIMENTAL, function to merge all objects in one vertex array (assume the object is clean)
    {
        return;				// Currently it's experimental, so don't waste time
        if (use_strips )	return;		// Can't optimise that!!
        int total_index = nb_t_index;			// Count the number of vertices and indexes to allocate the required space
        int total_vtx = nb_vtx;

        std::list< OBJECT* >	obj_stack;
        if (next )	obj_stack.push_front( next );
        if (child )	obj_stack.push_front( child );

        while( !obj_stack.empty() ) {
            OBJECT *cur = obj_stack.front();	obj_stack.pop_front();

            total_index += cur->nb_t_index;
            total_vtx += cur->nb_vtx;

            if (cur->next )		obj_stack.push_front( cur->next );
            if (cur->child )	obj_stack.push_front( cur->child );
        }

        Vector3D	*opt_vtx = new Vector3D[total_vtx];
        Vector3D	*opt_N = new Vector3D[total_vtx];
        float	*opt_T = new float[total_vtx << 1];
        GLushort *opt_idx = new GLushort[total_index];
        total_vtx = 0;
        total_index = 0;
        std::list<Vector3D>	pos_stack;
        Vector3D pos_offset;

        obj_stack.push_front( this );			// Fill the arrays
        pos_stack.push_front( pos_offset );

        while( !obj_stack.empty() )
        {
            OBJECT *cur = obj_stack.front();	obj_stack.pop_front();

            pos_offset = pos_stack.front();		pos_stack.pop_front();
            Vector3D dec = pos_offset + cur->pos_from_parent;

            for ( int i = 0 ; i < cur->nb_t_index ; i++ )
                opt_idx[ i + total_index ] = cur->t_index[ i ] + total_vtx;
            total_index += cur->nb_t_index;

            for ( int i = 0 ; i < cur->nb_vtx ; i++ ) {
                if (cur->tcoord ) {
                    opt_T[ (i + total_vtx << 1) ] = cur->tcoord[ (i << 1) ];
                    opt_T[ (i + total_vtx << 1) + 1 ] = cur->tcoord[ (i << 1) + 1 ];
                }
                if (cur->points )
                    opt_vtx[ i + total_vtx ] = cur->points[ i ] + dec;
                if (cur->N )
                    opt_N[ i + total_vtx ] = cur->N[ i ];
            }
            total_vtx += cur->nb_vtx;

            if (cur->next )		{	obj_stack.push_front( cur->next );	pos_stack.push_front( pos_offset );	}
            if (cur->child )	{	obj_stack.push_front( cur->child );		pos_stack.push_front( dec );	}
        }

        glGenBuffersARB( 1, &vbo_id );
        glGenBuffersARB( 1, &ebo_id );

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, total_vtx * ( sizeof( Vector3D ) + sizeof( Vector3D ) + sizeof( float ) * 2 ), NULL, GL_STATIC_DRAW_ARB);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, total_vtx * sizeof( Vector3D ), opt_vtx );
        N_offset = total_vtx * sizeof( Vector3D );
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, N_offset, total_vtx * sizeof( Vector3D ), opt_N );
        T_offset = N_offset + total_vtx * sizeof( Vector3D );
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, T_offset, total_vtx * sizeof( float ) * 2, opt_T );

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo_id);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, total_index * sizeof( GLushort ), opt_idx, GL_STATIC_DRAW_ARB);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        optimised = true;
        optimised_I = opt_idx;
        optimised_P = opt_vtx;
        optimised_N = opt_N;
        optimised_T = opt_T;
        optimised_nb_idx = total_index;
        optimised_nb_vtx = total_vtx;
    }

    int OBJECT::load_obj(byte *data,int offset,int dec,const char *filename)
    {
        destroy();					// Au cas où l'objet ne serait pas vierge

        if (data == NULL)
            return -1;

        tagObject header;				// Lit l'en-tête
        header.VersionSignature=*((int*)(data+offset));
        header.NumberOfVertexes=*((int*)(data+offset+4));
        header.NumberOfPrimitives=*((int*)(data+offset+8));
        header.OffsetToselectionPrimitive=*((int*)(data+offset+12));
        header.XFromParent=*((int*)(data+offset+16));
        header.YFromParent=*((int*)(data+offset+20));
        header.ZFromParent=*((int*)(data+offset+24));
        header.OffsetToObjectName=*((int*)(data+offset+28));
        header.Always_0=*((int*)(data+offset+32));
        header.OffsetToVertexArray=*((int*)(data+offset+36));
        header.OffsetToPrimitiveArray=*((int*)(data+offset+40));
        header.OffsetToSiblingObject=*((int*)(data+offset+44));
        header.OffsetToChildObject=*((int*)(data+offset+48));

        if (header.NumberOfVertexes + offset < 0)
            return -1;
        if (header.NumberOfPrimitives + offset < 0)
            return -1;
        if (header.OffsetToChildObject + offset < 0)
            return -1;
        if (header.OffsetToSiblingObject + offset < 0)
            return -1;
        if (header.OffsetToVertexArray + offset < 0)
            return -1;
        if (header.OffsetToPrimitiveArray + offset < 0)
            return -1;
        if (header.OffsetToObjectName + offset < 0 || header.OffsetToObjectName > 102400)
            return -1;
        int i;

        try
        {
            name = (char*)(data+header.OffsetToObjectName);
            i = 0;
            while( name[i] && i < 128 ) i++;
            if (name[i] != 0 && i >= 128)
            {
                name = NULL;
                return -1;
            }
        }
        catch( ... )
        {
            name = NULL;
            return -1;
        };

        nb_vtx = header.NumberOfVertexes;
        nb_prim = header.NumberOfPrimitives;
        name = strdup((char*)(data+header.OffsetToObjectName));
#ifdef DEBUG_MODE
        /*		for (i=0;i<dec;i++)
                printf("  ");
                printf("%s",name);
                for (i=0;i<20-2*dec-strlen(name);i++)
                printf(" ");
                printf("-> nb_vtx=%d | nb_prim=%d\n",nb_vtx,nb_prim);*/
#endif
        if (header.OffsetToChildObject) // Charge récursivement les différents objets du modèle
        {
            child = new OBJECT;
            if (child->load_obj(data,header.OffsetToChildObject,dec+1,filename))
            {
                destroy();
                return -1;
            }
        }
        if (header.OffsetToSiblingObject) // Charge récursivement les différents objets du modèle
        {
            next = new OBJECT;
            if (next->load_obj(data,header.OffsetToSiblingObject,dec,filename))
            {
                destroy();
                return -1;
            }
        }
        points = new Vector3D[nb_vtx];		// Alloue la mémoire nécessaire pour stocker les points
        int f_pos;
        float div=0.5f/65536.0f;
        pos_from_parent.x=header.XFromParent*div;
        pos_from_parent.y=header.YFromParent*div;
        pos_from_parent.z=-header.ZFromParent*div;
        f_pos=header.OffsetToVertexArray;

        for (i = 0; i < nb_vtx; ++i) // Lit le tableau de points stocké dans le fichier
        {
            tagVertex vertex;
            vertex.x = *((int*)(data + f_pos));   f_pos += 4;
            vertex.y = *((int*)(data + f_pos));   f_pos += 4;
            vertex.z = *((int*)(data + f_pos));   f_pos += 4;
            points[i].x = vertex.x  * div;
            points[i].y = vertex.y  * div;
            points[i].z = -vertex.z * div;
        }

        f_pos = header.OffsetToPrimitiveArray;
        int n_index = 0;
        selprim = -1;//header.OffsetToselectionPrimitive;
        sel[0] = sel[1] = sel[2] = sel[3] = 0;
        for (i = 0; i < nb_prim; ++i)// Compte le nombre de primitive de chaque sorte
        {
            tagPrimitive primitive;
            primitive.ColorIndex = *((int*)(data + f_pos));						f_pos += 4;
            primitive.NumberOfVertexIndexes = *((int*)(data + f_pos));			f_pos += 4;
            primitive.Always_0 = *((int*)(data + f_pos));						f_pos += 4;
            primitive.OffsetToVertexIndexArray = *((int*)(data + f_pos));		f_pos += 4;
            primitive.OffsetToTextureName = *((int*)(data + f_pos));			f_pos += 4;
            primitive.Unknown_1 = *((int*)(data + f_pos));						f_pos += 4;
            primitive.Unknown_2 = *((int*)(data + f_pos));						f_pos += 4;
            primitive.IsColored = *((int*)(data + f_pos));						f_pos += 4;

            switch(primitive.NumberOfVertexIndexes)
            {
                case 1:		++nb_p_index;    break;
                case 2:		nb_l_index += 2; break;
                default:
                            if (i == header.OffsetToselectionPrimitive)
                            {
                                selprim = 1;//nb_t_index;
                                break;
                            }
                            else
                            {
                                if (primitive.IsColored && primitive.ColorIndex == 1)
                                    break;
                                if (!primitive.IsColored && (!primitive.OffsetToTextureName || !data[primitive.OffsetToTextureName]))
                                    break;
                            }
                            n_index += primitive.NumberOfVertexIndexes;
                            ++nb_t_index;
            }
        }
#ifdef DEBUG_MODE
        //		printf("(%d,%d,%d)\n",nb_p_index,nb_l_index,nb_t_index);
#endif

        if (nb_p_index > 0)				// Alloue la mémoire nécessaire pour stocker les primitives
            p_index = new GLushort[nb_p_index];
        if (nb_l_index > 0)
            l_index = new GLushort[nb_l_index];
        if (nb_t_index > 0)
        {
            tex = new int[nb_t_index];
            usetex = new byte[nb_t_index];
            nb_index = new short[nb_t_index];
            t_index = new GLushort[n_index];
        }

        f_pos = header.OffsetToPrimitiveArray;
        int pos_p = 0;
        int pos_l = 0;
        int pos_t = 0;
        int cur = 0;
        int nb_diff_tex = 0;
        int* index_tex = new int[nb_prim];
        int t_m = 0;
        for (i = 0; i < nb_prim; ++i) // Compte le nombre de primitive de chaque sorte
        {
            tagPrimitive primitive;
            primitive.ColorIndex = *((int*)(data + f_pos));                 f_pos += 4;
            primitive.NumberOfVertexIndexes = *((int*)(data + f_pos));      f_pos += 4;
            primitive.Always_0 = *((int*)(data + f_pos));                   f_pos += 4;
            primitive.OffsetToVertexIndexArray = *((int*)(data + f_pos));   f_pos += 4;
            primitive.OffsetToTextureName = *((int*)(data + f_pos));        f_pos += 4;
            primitive.Unknown_1 = *((int*)(data + f_pos));                  f_pos += 4;
            primitive.Unknown_2 = *((int*)(data + f_pos));                  f_pos += 4;
            primitive.IsColored = *((int*)(data + f_pos));                  f_pos += 4;

            switch (primitive.NumberOfVertexIndexes)
            {
                case 1:
                    p_index[pos_p++] = *((short*)(data+primitive.OffsetToVertexIndexArray));
                    break;
                case 2:
                    l_index[pos_l++] = *((short*)(data+primitive.OffsetToVertexIndexArray));
                    l_index[pos_l++] = *((short*)(data+primitive.OffsetToVertexIndexArray + 2));
                    break;
                default:
                    if (i != header.OffsetToselectionPrimitive)
                    {
                        if (primitive.IsColored && primitive.ColorIndex == 1)
                            break;
                        if (!primitive.IsColored && (!primitive.OffsetToTextureName || !data[primitive.OffsetToTextureName]))
                            break;
                    }
                    else
                    {
                        for (int e = 0; e < primitive.NumberOfVertexIndexes && e < 4; ++e)
                            sel[e] = *((short*)(data + primitive.OffsetToVertexIndexArray + (e << 1)));
                        break;
                    }
                    nb_index[cur] = primitive.NumberOfVertexIndexes;
                    tex[cur] = t_m = texture_manager.get_texture_index((char*)(data+primitive.OffsetToTextureName));
                    usetex[cur] = 1;
                    if (t_m == -1)
                    {
                        if (primitive.ColorIndex >= 0 && primitive.ColorIndex < 256)
                        {
                            usetex[cur] = 1;
                            tex[cur] = t_m = primitive.ColorIndex;
                        }
                        else
                            usetex[cur] = 0;
                    }
                    if (t_m >= 0)
                    {														// Code pour la création d'une texture propre à chaque modèle
                        bool al_in = false;
                        int indx = t_m;
                        for (int e = 0; e < nb_diff_tex; ++e)
                            if (index_tex[e] == indx)
                            {
                                al_in=true;
                                break;
                            }
                        if (!al_in)
                            index_tex[nb_diff_tex++]=indx;
                    }
                    for (int e = 0; e < nb_index[cur]; ++e)
                        t_index[pos_t++] = *((short*)(data + primitive.OffsetToVertexIndexArray + (e << 1)));
                    ++cur;
            }
        }

        /*------------------------------Création de la texture unique pour l'unité--------------*/
        int* px = new int[nb_diff_tex];
        int* py = new int[nb_diff_tex];			// Pour placer les différentes mini-textures sur une grande texture
        int mx = 0;
        int my = 0;

        for (i = 0; i < nb_diff_tex; ++i)
        {
            int dx = texture_manager.tex[index_tex[i]].bmp[0]->w;
            int dy = texture_manager.tex[index_tex[i]].bmp[0]->h;
            px[i]=py[i]=0;
            if (i!=0)
                for (int e = 0; e < i; ++e)
                {
                    int fx = texture_manager.tex[index_tex[e]].bmp[0]->w, fy=texture_manager.tex[index_tex[e]].bmp[0]->h;
                    bool found[3];
                    found[0] = found[1] = found[2] = true;
                    int j;

                    px[i] = px[e] + fx;	
                    py[i] = py[e];
                    for (j = 0; j < i; ++j)
                    {
                        int gx = texture_manager.tex[index_tex[j]].bmp[0]->w, gy=texture_manager.tex[index_tex[j]].bmp[0]->h;
                        if (coupe(px[i], py[i], dx, dy, px[j], py[j], gx, gy))
                        {
                            found[0] = false;
                            break;
                        }
                    }

                    px[i] = px[e];
                    py[i] = py[e] + fy;
                    for (j = 0; j < i; ++j)
                    {
                        int gx = texture_manager.tex[index_tex[j]].bmp[0]->w, gy = texture_manager.tex[index_tex[j]].bmp[0]->h;
                        if (coupe(px[i], py[i], dx, dy, px[j], py[j], gx, gy)) 
                        {
                            found[2] = false;
                            break;
                        }
                    }
                    px[i] = px[e] + fx;
                    py[i] = 0;

                    for (j = 0; j < i; ++j)
                    {
                        int gx = texture_manager.tex[index_tex[j]].bmp[0]->w, gy = texture_manager.tex[index_tex[j]].bmp[0]->h;
                        if (coupe(px[i], py[i], dx, dy, px[j], py[j], gx, gy))
                        {
                            found[1] = false;
                            break;
                        }
                    }
                    bool deborde = false;
                    bool found_one = false;
                    int deb = 0;

                    if (found[1])
                    {
                        px[i] = px[e] + fx;
                        py[i] = 0;
                        deborde = false;
                        if (px[i] + dx > mx || py[i] + dy > my)
                            deborde = true;
                        deb = Math::Max(mx, px[i] + dx) * Math::Max(py[i] + dy, my) - mx * my;
                        found_one = true;
                    }
                    if (found[0] && (!found_one || deborde))
                    {
                        px[i] = px[e]+fx;
                        py[i] = py[e];
                        deborde = false;
                        if (px[i] + dx > mx || py[i] + dy > my)
                            deborde = true;
                        deb = Math::Max(mx, px[i] + dx) * Math::Max(py[i] + dy, my) - mx * my;
                        found_one = true;
                    }
                    if (found[2] && deborde)
                    {
                        int ax = px[i],ay = py[i];
                        px[i] = px[e];
                        py[i] = py[e] + fy;
                        deborde = false;
                        if (px[i]+dx>mx || py[i] + dy > my)
                            deborde = true;
                        int deb2 = Math::Max(mx, px[i] + dx) * Math::Max(py[i] + dy, my) - mx * my;
                        if (found_one && deb<deb2)
                        {
                            px[i] = ax;
                            py[i] = ay;
                        }
                        else
                            found_one=true;
                    }
                    if (found_one)			// On a trouvé une position qui convient
                        break;
                }
            if (px[i] + dx > mx)   mx = px[i] + dx;
            if (py[i] + dy > my)   my = py[i] + dy;
        }

        BITMAP* bmp = create_bitmap_ex(32, mx, my);
        if (bmp != NULL && mx != 0 && my != 0)
        {
            if (g_useTextureCompression)
                allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
            else
                allegro_gl_set_texture_format(GL_RGB8);
            clear(bmp);
            for (short e = 0; e < expected_players; ++e)
            {
                bool mtex_needed = false;
                for (i = 0; i < nb_diff_tex; ++i)
                {
                    if (texture_manager.tex[index_tex[i]].nb_bmp == 10)
                    {
                        blit(texture_manager.tex[index_tex[i]].bmp[player_color_map[e]], bmp,
                             0, 0, px[i], py[i],
                             texture_manager.tex[index_tex[i]].bmp[player_color_map[e]]->w,
                             texture_manager.tex[index_tex[i]].bmp[player_color_map[e]]->h);
                        mtex_needed=true;
                    }
                    else
                    {
                        blit(texture_manager.tex[index_tex[i]].bmp[0], bmp, 0, 0,
                             px[i],py[i],
                             texture_manager.tex[index_tex[i]].bmp[0]->w,
                             texture_manager.tex[index_tex[i]].bmp[0]->h);
                    }
                }
                dtex = e + 1;
                String cache_filename = filename ? String( filename ) + format("-%s-%d.bin", name ? name : "none", player_color_map[e] ) : String( "" );
                gltex[e] = gfx->load_texture_from_cache( cache_filename );
                if (!gltex[e])
                {
                    gltex[e] = allegro_gl_make_texture(bmp);
                    glBindTexture(GL_TEXTURE_2D,gltex[e]);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
                    if (filename)
                        gfx->save_texture_to_cache(cache_filename, gltex[e], bmp->w, bmp->h);
                }
                if (!mtex_needed)
                    break;
            }
        }
        else
            dtex=0;
        if (bmp)
            destroy_bitmap(bmp);

        int nb_total_point = 0;
        for (i = 0; i < nb_t_index; ++i)
            nb_total_point += nb_index[i];

        nb_total_point += nb_l_index;
        if (selprim >= 0)
            nb_total_point += 4;

        Vector3D *p = new Vector3D[nb_total_point<<1];			// *2 pour le volume d'ombre
        int prim_dec = selprim >= 0 ? 4 : 0;
        for (i = 0; i < nb_total_point - nb_l_index - prim_dec; ++i)
        {
            p[i + nb_total_point]  = p[i] = points[t_index[i]];
            t_index[i] = i;
        }
        if (selprim >= 0)
        {
            p[nb_total_point - nb_l_index - prim_dec]     = points[sel[0]];  sel[0] = nb_total_point - nb_l_index - prim_dec;
            p[nb_total_point - nb_l_index - prim_dec + 1] = points[sel[1]];  sel[1] = nb_total_point - nb_l_index - prim_dec + 1;
            p[nb_total_point - nb_l_index - prim_dec + 2] = points[sel[2]];  sel[2] = nb_total_point - nb_l_index - prim_dec + 2;
            p[nb_total_point - nb_l_index - prim_dec + 3] = points[sel[3]];  sel[3] = nb_total_point - nb_l_index - prim_dec + 3;
        }
        for (i = nb_total_point - nb_l_index; i < nb_total_point; ++i)
        {
            int e = i - nb_total_point + nb_l_index;
            p[i + nb_total_point] = p[i] = points[l_index[e]];
            l_index[e] = i;
        }
        if (nb_l_index == 2)
        {
            if (p[l_index[0]].x < 0.0f)
            {
                int tmp=l_index[0];
                l_index[0]=l_index[1];
                l_index[1]=tmp;
            }
        }
        delete[] points;
        points = p;
        nb_vtx = nb_total_point;

        int nb_triangle=0;
        for (i = 0; i < nb_t_index; ++i)
            nb_triangle += nb_index[i] - 2;
        GLushort *index = new GLushort[nb_triangle * 3];
        tcoord = new float[nb_vtx << 1];
        cur = 0;
        int curt = 0;
        pos_t = 0;
        for (i = 0; i < nb_t_index; ++i)
        {
            int indx = 0;
            for (int f = 0; f < nb_diff_tex; ++f)
            {
                if (tex[i] == index_tex[f])
                {
                    indx = f;
                    break;
                }
            }
            for (int e = 0; e < nb_index[i]; ++e)
            {
                if (e < 3)
                    index[pos_t++] = t_index[cur];
                else
                {
                    index[pos_t]   = index[pos_t-3]; ++pos_t;
                    index[pos_t]   = index[pos_t-2]; ++pos_t;
                    index[pos_t++] = t_index[cur];
                }
                tcoord[curt]     = 0.0f;
                tcoord[curt + 1] = 0.0f;

                if (usetex[i])
                {
                    switch (e & 3)
                    {
                        case 1:
                            tcoord[curt]     += ((float)texture_manager.tex[tex[i]].bmp[0]->w-1)/(mx-1);
                            break;
                        case 2:
                            tcoord[curt]     += ((float)texture_manager.tex[tex[i]].bmp[0]->w-1)/(mx-1);
                            tcoord[curt + 1] += ((float)texture_manager.tex[tex[i]].bmp[0]->h-1)/(my-1);
                            break;
                        case 3:
                            tcoord[curt + 1] += ((float)texture_manager.tex[tex[i]].bmp[0]->h-1)/(my-1);
                            break;
                    };
                    tcoord[curt]     += ((float)px[indx] + 0.5f) / (mx - 1);
                    tcoord[curt + 1] += ((float)py[indx] + 0.5f) / (my - 1);
                }
                ++cur;
                curt += 2;
            }
        }
        for (cur = 0; cur < pos_t; cur += 3)// Petite inversion pour avoir un affichage correct
        {
            GLushort t = index[cur + 1];
            index[cur + 1] = index[cur + 2];
            index[cur + 2] = t;
        }
        nb_t_index = nb_triangle * 3;
        delete[] t_index;
        t_index = index;
        delete[] usetex;
        usetex = NULL;
        /*--------------------------------------------------------------------------------------*/

        if (nb_t_index > 0) // Calcule les normales pour l'éclairage
        {
            N = new Vector3D[nb_vtx << 1];
            F_N = new Vector3D[nb_t_index / 3];
            for (i = 0; i  < nb_vtx << 1; ++i)
                N[i].x=N[i].z=N[i].y=0.0f;
            int e = 0;
            for (i = 0; i < nb_t_index; i += 3)
            {
                Vector3D AB,AC,Normal;
                AB = points[t_index[i+1]] - points[t_index[i]];
                AC = points[t_index[i+2]] - points[t_index[i]];
                Normal = AB * AC;
                Normal.unit();
                F_N[e++] = Normal;
                for (int e = 0; e < 3; ++e)
                    N[t_index[i + e]] = N[t_index[i+e]] + Normal;
            }
            for (i = 0; i < nb_vtx; ++i)
                N[i].unit();
            for (i = nb_vtx; i < (nb_vtx << 1) ; ++i)
                N[i] = N[i - nb_vtx];
        }
        delete[] px;
        delete[] py;
        delete[] index_tex;
        return 0;
    }



    void OBJECT::create_from_2d(BITMAP *bmp,float w,float h,float max_h)
    {
        destroy();					// Au cas où l'objet ne serait pas vierge

        pos_from_parent.x = 0.0f;
        pos_from_parent.y = 0.0f;
        pos_from_parent.z = 0.0f;
        selprim = -1;
        child = NULL;
        next = NULL;
        nb_l_index = 0;
        nb_p_index = 0;
        l_index = NULL;
        p_index = NULL;

        use_strips = true;

        nb_vtx = 64;
        nb_t_index=119;
        points = new Vector3D[nb_vtx];
        tcoord = new float[nb_vtx<<1];
        t_index = new GLushort[nb_t_index];
        if (!points || !tcoord || !t_index)
            LOG_CRITICAL("Not enough memory !");

        uint16	i;
        uint8	x,y;

        float ww = w * 0.1333333333333f;
        float hh = h * 0.1333333333333f;

        for (y = 0; y < 8; ++y) // Maillage (sommets)
        {
            uint16 seg = y << 3;
            float yy = y * 0.1333333333333f;
            for (x = 0; x < 8; ++x)
            {
                uint16	offset = seg+x;
                points[offset].x=(x-3.5f)*ww;
                points[offset].z=(y-3.5f)*hh;
                tcoord[offset<<1]=x*0.1333333333333f;
                tcoord[(offset<<1)+1]=yy;
            }
        }
        uint16 offset = 0;
        for (y = 0; y < 7; ++y)						// Maillage (triangles)
        {
            if (y & 1)
            {
                t_index[offset++] = (y << 3);
                t_index[offset++] = ((y + 1) << 3);
                for (x = 0; x < 7; ++x)
                {
                    t_index[offset++]=(y<<3)+x+1;
                    t_index[offset++]=(y+1<<3)+x+1;
                }
                t_index[offset++]=(y+1<<3)+7;
            }
            else
            {
                t_index[offset++]=(y<<3)+7;
                t_index[offset++]=((y+1)<<3)+7;
                for (x = 0; x < 7; ++x)
                {
                    t_index[offset++] = (y << 3) + 6 - x;
                    t_index[offset++] = (y + 1 << 3) + 6 - x;
                }
                t_index[offset++] = (y + 1 << 3);
            }
        }

        uint32 tmp[8][8];

        uint32 med=0;
        uint32 div=0;
        for (y = 0 ; y < 8; ++y) // Carte miniature en nuances de gris
        {
            for (x = 0; x < 8; ++x)
            {
                uint32 c=0;
                uint32 n=0;
                bool zero=false;
                for (int py = y * bmp->h >> 3; py < (y + 1) * bmp->h >> 3; ++py)
                {
                    for (int px = x * bmp->w >> 3; px < (x + 1) * bmp->w >> 3; ++px)
                    {
                        uint32 pc = getpixel(bmp,  px, py);
                        c += getr(pc) + getg(pc) + getb(pc);
                        if (geta(pc) < 128 || (pc&0xFFFFFF) == 0xFF00FF)
                            zero = true;
                        n += 3;
                    }
                }
                if (zero)
                {
                    c = 0x0;
                    n = 1;
                }
                tmp[y][x] = c / n;
                if (!zero)
                {
                    med += tmp[y][x];
                    ++div;
                }
            }
        }
        if (div == 0)
            div = 1; // Il y a des trucs bizarres des fois!
        med = (med + (div >> 1)) / div;
        for (y = 0; y < 8; ++y)  // Carte miniature en nuances de gris
        {
            for (x = 0; x < 8; ++x)
            {
                if (tmp[y][x] == uint32(-0xFFFFFF))
                    tmp[y][x] = med;
            }
        }

        points[0].y=0.0f;
        for (y = 1; y < 8; ++y) // x=0
            points[y << 3].y = 0.0f;
        for (x = 1; x < 8; ++x) // y=0
            points[x].y = 0.0f;
        for (y = 1; y < 8; ++y)
        {
            for (x = 1; x < 8; ++x)
            {
                int d_h0 = tmp[y][x - 1] - med;
                int d_h1 = tmp[y - 1][x] - med;
                int d_h = tmp[y][x] - med;
                float l = sqrt((float)(d_h0 * d_h0  +  d_h1 * d_h1));
                float dhx;
                float dhy;
                if (l == 0.0f)
                    dhx = dhy = 0.0f;
                else
                {
                    dhx = d_h * abs(d_h0) / l;
                    dhy = d_h * abs(d_h1) / l;
                }
                points[(y << 3) + x].y = (points[(y << 3) + x - 1].y + dhx + points[((y - 1) << 3) + x].y + dhy) * 0.5f;
            }
        }

        for (y = 1; y < 8; ++y)
            for (x = 1; x < 8; ++x)
                points[(y << 3) + x].y -= (x / 7.0f) * points[(y << 3) + 7].y;

        for (x = 1; x < 8; ++x)
            for (y = 1; y < 8; ++y)
                points[(y << 3) + x].y -= (y / 7.0f) * points[(7 << 3) + x].y;

        float maxh = 0.0f;
        float minh = 0.0f;
        for (i = 0; i < 64; ++i)
        {
            if (minh>points[i].y)  minh = points[i].y;
            if (maxh<points[i].y)  maxh = points[i].y;
        }
        if (maxh == minh || maxh == 0.0f )
        {
            for (i = 0; i < 64; ++i)
                points[i].y = 0.0f;
        }
        else
        {
            for (i = 0; i < 64; ++i)
                points[i].y = points[i].y > 0.0f ? points[i].y * max_h / maxh : 0.0f;
        }

        if (g_useTextureCompression)
            allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
        else
            allegro_gl_set_texture_format(GL_RGB5_A1);
        gltex[0] = allegro_gl_make_texture(bmp);
        dtex = 1;
        glBindTexture(GL_TEXTURE_2D,gltex[0]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

        /*--------------------------------------------------------------------------------------*/

        N = new Vector3D[nb_vtx];
        F_N = NULL;
        for (i = 0; i < nb_vtx; ++i)
            N[i].x = N[i].z = N[i].y = 0.0f;
        for (i = 0; i < nb_t_index - 2; ++i)
        {
            Vector3D AB = points[t_index[i + 1]] - points[t_index[i]];
            Vector3D AC = points[t_index[i + 2]] - points[t_index[i]];
            Vector3D Normal;
            Normal = AB * AC;
            Normal.unit();
            if (Normal.y < 0.0f)
                Normal = -Normal;
            for (int e = 0; e < 3; ++e)
                N[t_index[i + e]] = N[t_index[i + e]] + Normal;
        }
        for (i = 0; i < nb_vtx; ++i)
            N[i].unit();
    }



    void OBJECT::draw_optimised(const bool set)
    {
        if (set)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo_id);

            glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
            glEnableClientState(GL_NORMAL_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            glNormalPointer(GL_FLOAT, 0, (GLvoid*)N_offset);
            //		glVertexPointer( 3, GL_FLOAT, 0, optimised_P);
            //		glNormalPointer( GL_FLOAT, 0, optimised_N);
        }
        glDrawElements(GL_TRIANGLES, optimised_nb_idx, GL_UNSIGNED_SHORT, NULL);
        //	glDrawElements(GL_TRIANGLES, optimised_nb_idx,GL_UNSIGNED_SHORT,optimised_I);				// draw everything
    }




    bool OBJECT::draw(float t,SCRIPT_DATA *data_s,bool sel_primitive,bool alset,bool notex,int side,bool chg_col,bool exploding_parts)
    {
        bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
        bool hide=false;
        bool set=false;
        float color_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glPushMatrix();
        if (explodes && !exploding_parts)
            goto draw_next;

        glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);
        if (script_index>=0 && data_s)
        {
            if (animation_data != NULL && (data_s->flag[script_index] & FLAG_ANIMATE)) // Used only by the 3dmeditor
            {
                Vector3D R;
                Vector3D T;
                animation_data->animate(t, R, T);
                glTranslatef(T.x, T.y, T.z);
                glRotatef(R.x, 1.0f, 0.0f, 0.0f);
                glRotatef(R.y, 0.0f, 1.0f, 0.0f);
                glRotatef(R.z, 0.0f, 0.0f, 1.0f);
            }
            else
                if (!explodes ^ exploding_parts)
                {
                    glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
                    glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
                    glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
                    glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
                }
            hide = data_s->flag[script_index]&FLAG_HIDE;
        }
        else
        {
            if (animation_data)
            {
                Vector3D R;
                Vector3D T;
                animation_data->animate(t, R, T);
                glTranslatef(T.x, T.y, T.z);
                glRotatef(R.x, 1.0f, 0.0f, 0.0f);
                glRotatef(R.y, 0.0f, 1.0f, 0.0f);
                glRotatef(R.z, 0.0f, 0.0f, 1.0f);
            }
        }

        hide |= explodes ^ exploding_parts;
        if (!chg_col)
            glGetFloatv(GL_CURRENT_COLOR, color_factor);
        if (gl_dlist[ side ] && !hide && chg_col && !notex)
        {
            glCallList( gl_dlist[ side ] );
            alset = false;
            set = false;
        }
        else
            if (!hide)
            {
                bool creating_list = false;
                if (chg_col && !notex && gl_dlist[side] == 0)
                {
                    gl_dlist[side] = glGenLists(1);
                    glNewList(gl_dlist[side], GL_COMPILE_AND_EXECUTE);
                    alset = false;
                    set = false;
                    creating_list = true;
                }
                if (nb_t_index > 0 && nb_vtx > 0 && t_index != NULL)
                {
                    bool activated_tex=false;
                    if (surface.Flag&SURFACE_ADVANCED)
                    {
                        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                        glEnableClientState(GL_NORMAL_ARRAY);
                        alset=false;
                        set=false;
                        if (chg_col || notex)
                        {
                            if (surface.Flag&SURFACE_PLAYER_COLOR)
                                glColor4f(player_color[side*3],player_color[side*3+1],player_color[side*3+2],surface.Color[3]);		// Couleur de matière
                            else
                                glColor4fv(surface.Color);		// Couleur de matière
                        }
                        else
                            if (!chg_col && !notex)
                            {
                                if (surface.Flag&SURFACE_PLAYER_COLOR)
                                    glColor4f(player_color[player_color_map[side]*3]*color_factor[0],player_color[player_color_map[side]*3+1]*color_factor[1],player_color[player_color_map[side]*3+2]*color_factor[2],surface.Color[3]*color_factor[3]);		// Couleur de matière
                                else
                                    glColor4f(surface.Color[0]*color_factor[0],surface.Color[1]*color_factor[1],surface.Color[2]*color_factor[2],surface.Color[3]*color_factor[3]);		// Couleur de matière
                            }

                        if (surface.Flag&SURFACE_GLSL)			// Using vertex and fragment programs
                        {
                            for (int j = 0; j < surface.NbTex; ++j)
                                surface.s_shader.setvar1i( format("tex%d",j).c_str(), j );
                            surface.s_shader.on();
                        }

                        if (surface.Flag&SURFACE_GOURAUD)			// Type d'éclairage
                            glShadeModel (GL_SMOOTH);
                        else
                            glShadeModel (GL_FLAT);

                        if (surface.Flag&SURFACE_LIGHTED)			// Eclairage
                            glEnable(GL_LIGHTING);
                        else
                            glDisable(GL_LIGHTING);

                        if (surface.Flag&SURFACE_BLENDED || (!chg_col && color_factor[3] != 1.0f)) // La transparence
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

                        if (surface.Flag&SURFACE_TEXTURED && !notex) // Les textures et effets de texture
                        {
                            activated_tex = true;
                            for (int j = 0; j < surface.NbTex; ++j)
                            {
                                glActiveTextureARB(GL_TEXTURE0_ARB + j);
                                glEnable(GL_TEXTURE_2D);
                                glBindTexture(GL_TEXTURE_2D,surface.gltex[j]);
                                if (j==surface.NbTex-1 && surface.Flag&SURFACE_REFLEC)
                                {
                                    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                                    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                                    glEnable(GL_TEXTURE_GEN_S);
                                    glEnable(GL_TEXTURE_GEN_T);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE);

                                    glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_TEXTURE);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_PREVIOUS_EXT);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_CONSTANT_EXT);
                                    glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);
                                    glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,surface.RColor);
                                }
                                else
                                {
                                    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                                    glDisable(GL_TEXTURE_GEN_S);
                                    glDisable(GL_TEXTURE_GEN_T);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                                }
                            }
                            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                            for (int j = 0; j < surface.NbTex; ++j)
                            {
                                glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                                glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                            }
                        }
                        else
                        {
                            for (int j = 7; j >= 0; --j)
                            {
                                glActiveTextureARB(GL_TEXTURE0_ARB + j);
                                glDisable(GL_TEXTURE_2D);
                                glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            }
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        }
                    }
                    else
                    {
                        if (!alset)
                        {
                            glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                            glEnableClientState(GL_NORMAL_ARRAY);
                            if (notex)
                                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            else
                                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                            glEnable(GL_LIGHTING);
                            if (notex)
                                glDisable(GL_TEXTURE_2D);
                            else
                                glEnable(GL_TEXTURE_2D);
                            alset=true;
                        }
                        if (!chg_col && color_factor[3] != 1.0f) // La transparence
                        {
                            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                            glEnable(GL_BLEND);
                        }
                        else
                            glDisable(GL_BLEND);
                        set = true;
                        if (!dtex)
                        {
                            alset=false;
                            glDisable(GL_TEXTURE_2D);
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                        }
                        if (!notex && dtex)
                        {
                            if (side<dtex && side>=0)
                                glBindTexture(GL_TEXTURE_2D,gltex[side]);
                            else
                                glBindTexture(GL_TEXTURE_2D,gltex[0]);
                            glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                        }
                    }
                    glVertexPointer(3, GL_FLOAT, 0, points);
                    glNormalPointer(GL_FLOAT, 0, N);
                    if (!use_strips)
                        glDrawElements(GL_TRIANGLES, nb_t_index,GL_UNSIGNED_SHORT,t_index);				// draw everything
                    else
                    {
                        glDisable( GL_CULL_FACE );
                        glDrawElements(GL_TRIANGLE_STRIP, nb_t_index,GL_UNSIGNED_SHORT,t_index);		// draw everything
                        glEnable( GL_CULL_FACE );
                    }

                    if ((surface.Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD))==SURFACE_ADVANCED)
                        glShadeModel (GL_SMOOTH);
                    if ((surface.Flag&SURFACE_GLSL) && (surface.Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
                        surface.s_shader.off();

                    if (activated_tex)
                    {
                        for (int j = 0; j < surface.NbTex; ++j)
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
#ifdef DEBUG_MODE_3DO
        if (nb_l_index > 0 && nb_vtx > 0)
        {
            glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            alset=false;
            if (!set)
                glVertexPointer( 3, GL_FLOAT, 0, points);
            set=true;
            glDrawElements(GL_LINES, nb_l_index,GL_UNSIGNED_SHORT,l_index);		// dessine le tout
        }
#endif
        if (sel_primitive && selprim >= 0 && nb_vtx > 0) // && (data_s==NULL || (data_s!=NULL && !data_s->explode))) {
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);
            if (!set)
                glVertexPointer( 3, GL_FLOAT, 0, points);
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
            glColor3f(0.0f,1.0f,0.0f);
            glTranslatef( 0.0f, 2.0f, 0.0f );
            glDrawElements(GL_QUADS, 4,GL_UNSIGNED_SHORT,sel);		// dessine la primitive de sélection
            glTranslatef( 0.0f, -2.0f, 0.0f );
            if (notex)
            {
                float var=fabs(1.0f-(msec_timer%1000)*0.002f);
                glColor3f(0.0f,var,0.0f);
            }
            else
                glColor3f(1.0f,1.0f,1.0f);
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
            alset=false;
        }
        if (!chg_col)
            glColor4fv(color_factor);
        if (child && !(explodes && !exploding_parts))
        {
            glPushMatrix();
            alset=child->draw(t,data_s,sel_primitive,alset,notex,side,chg_col, exploding_parts && !explodes );
            glPopMatrix();
        }
draw_next:
        if (next)
        {
            glPopMatrix();
            glPushMatrix();
            alset=next->draw(t,data_s,sel_primitive,alset,notex,side,chg_col,exploding_parts);
            glPopMatrix();
        }
        else
            glPopMatrix();

        return alset;
    }




    bool OBJECT::draw_dl(SCRIPT_DATA *data_s,bool alset,int side,bool chg_col)
    {
        glPushMatrix();
        glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);
        bool hide = false;
        if (script_index >= 0 && data_s)
        {
            glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
            glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
            glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
            glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
            hide=data_s->flag[script_index]&FLAG_HIDE;
        }
        bool set=false;
        if (nb_t_index > 0 && nb_vtx > 0 && !hide && t_index != NULL)
        {
            bool activated_tex=false;
            if (surface.Flag & SURFACE_ADVANCED)
            {
                alset=false;
                set=false;
                if (chg_col)
                {
                    if (surface.Flag&SURFACE_PLAYER_COLOR)
                        glColor4f(player_color[player_color_map[side]*3],player_color[player_color_map[side]*3+1],player_color[player_color_map[side]*3+2],surface.Color[3]);		// Couleur de matière
                    else
                        glColor4f(surface.Color[0],surface.Color[1],surface.Color[2],surface.Color[3]);		// Couleur de matière
                }

                if (surface.Flag&SURFACE_GLSL)			// Using vertex and fragment programs
                {
                    for (int j = 0; j < surface.NbTex; ++j)
                        surface.s_shader.setvar1i( format("tex%d",j).c_str(), j );
                    surface.s_shader.on();
                }

                if (surface.Flag&SURFACE_GOURAUD)			// Type d'éclairage
                    glShadeModel (GL_SMOOTH);
                else
                    glShadeModel (GL_FLAT);

                if (surface.Flag&SURFACE_LIGHTED)			// Eclairage
                    glEnable(GL_LIGHTING);
                else
                    glDisable(GL_LIGHTING);

                if (surface.Flag&SURFACE_BLENDED) // La transparence
                {
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_BLEND);
                    glAlphaFunc(GL_GREATER, 0.1);
                    glEnable(GL_ALPHA_TEST);
                }
                else
                {
                    glDisable(GL_ALPHA_TEST);
                    glDisable(GL_BLEND);
                }

                if (surface.Flag&SURFACE_TEXTURED)// Les textures et effets de texture
                {
                    activated_tex = true;
                    for (int j = 0; j < surface.NbTex; ++j)
                    {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D,surface.gltex[j]);
                        if (j == surface.NbTex - 1 && surface.Flag & SURFACE_REFLEC)
                        {
                            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
                            glEnable(GL_TEXTURE_GEN_S);
                            glEnable(GL_TEXTURE_GEN_T);
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);
                            glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT,GL_INTERPOLATE);

                            glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_TEXTURE);
                            glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);
                            glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT,GL_PREVIOUS_EXT);
                            glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);
                            glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_CONSTANT_EXT);
                            glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);
                            glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,surface.RColor);
                        }
                        else
                        {
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                            glDisable(GL_TEXTURE_GEN_S);
                            glDisable(GL_TEXTURE_GEN_T);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        }
                    }
                }
                else
                    for (int j = 7; j >= 0; --j)
                    {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                        glDisable(GL_TEXTURE_2D);
                    }
            }
            else
            {
                glEnable(GL_LIGHTING);
                activated_tex=true;
                glEnable(GL_TEXTURE_2D);
                if (!dtex)
                {
                    activated_tex=false;
                    alset=false;
                    glDisable(GL_TEXTURE_2D);
                }
                if (dtex)
                {
                    if (side < dtex && side >= 0)
                        glBindTexture(GL_TEXTURE_2D, gltex[side]);
                    else
                        glBindTexture(GL_TEXTURE_2D, gltex[0]);
                }
            }
            if (!use_strips)
            {
                glBegin(GL_TRIANGLES);
                for (int i = 0 ; i < nb_t_index ; ++i)
                {
                    if (activated_tex)
                        glTexCoord2f( tcoord[ t_index[ i ] << 1 ], tcoord[ (t_index[ i ] << 1) + 1 ] );
                    glNormal3f( N[ t_index[ i ] ].x, N[ t_index[ i ] ].y, N[ t_index[ i ] ].z );
                    glVertex3f( points[ t_index[ i ] ].x, points[ t_index[ i ] ].y, points[ t_index[ i ] ].z );
                }
                glEnd();
            }
            else
            {
                glDisable(GL_CULL_FACE);
                glBegin(GL_TRIANGLE_STRIP);
                for (int i = 0 ; i < nb_t_index ; ++i)
                {
                    if (activated_tex)
                        glTexCoord2f( tcoord[ t_index[ i ] << 1 ], tcoord[ (t_index[ i ] << 1) + 1 ] );
                    glNormal3f( N[ t_index[ i ] ].x, N[ t_index[ i ] ].y, N[ t_index[ i ] ].z );
                    glVertex3f( points[ t_index[ i ] ].x, points[ t_index[ i ] ].y, points[ t_index[ i ] ].z );
                }
                glEnd();
                glEnable(GL_CULL_FACE);
            }

            if ((surface.Flag&SURFACE_GLSL) && (surface.Flag&SURFACE_ADVANCED))			// Using vertex and fragment programs
                surface.s_shader.off();

            if ((surface.Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD))==SURFACE_ADVANCED)
                glShadeModel (GL_SMOOTH);

            if (activated_tex)
            {
                for (int j = 0; j < surface.NbTex; ++j)
                {
                    glActiveTextureARB(GL_TEXTURE0_ARB + j);
                    glDisable(GL_TEXTURE_2D);
                    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                    glDisable(GL_TEXTURE_GEN_S);
                    glDisable(GL_TEXTURE_GEN_T);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnable(GL_TEXTURE_2D);
            }
        }
        if (child)
        {
            //			if (data_s==NULL || !data_s->explode)
            glPushMatrix();
            /*			else {
                        glPopMatrix();
                        glPushMatrix();
                        }*/
            alset=child->draw_dl(data_s,alset,side,chg_col);
            glPopMatrix();
        }
        if (next)
        {
            glPopMatrix();
            glPushMatrix();
            alset=next->draw_dl(data_s,alset,side,chg_col);
            glPopMatrix();
        }
        else
            glPopMatrix();

        return alset;
    }




    uint16 OBJECT::set_obj_id(uint16 id)
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




    int OBJECT::random_pos(SCRIPT_DATA *data_s, int id, Vector3D* vec)
    {
        if (id == obj_id)
        {
            if (nb_t_index > 2 && (data_s == NULL || script_index < 0 || !(data_s->flag[script_index] & FLAG_HIDE)) )
            {
                int rnd_idx = (Math::RandFromTable() % (nb_t_index / 3)) * 3;
                float a = (Math::RandFromTable() & 0xFF) / 255.0f;
                float b = (1.0f - a) * (Math::RandFromTable() & 0xFF) / 255.0f;
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




    void OBJECT::compute_coord(SCRIPT_DATA* data_s, Vector3D *pos, bool c_part, int p_tex, Vector3D *target,
                               Vector3D* upos, MATRIX_4x4* M, float size, Vector3D* center, bool reverse,
                               OBJECT* src, SCRIPT_DATA* src_data)
    {
        Vector3D opos = *pos;
        MATRIX_4x4 OM;
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
                *M = RotateZ(data_s->axe[2][script_index].angle * DEG2RAD) * RotateY(data_s->axe[1][script_index].angle
                                                                                     * DEG2RAD) * RotateX(data_s->axe[0][script_index].angle * DEG2RAD) * (*M);
                data_s->matrix[script_index] = *M;
                if (nb_l_index==2) {
                    data_s->dir[script_index]=(points[l_index[1]] - points[l_index[0]])*(*M);
                    data_s->dir[script_index].unit();
                    ipos.x=points[l_index[0]].x;
                    ipos.y=points[l_index[0]].y;
                    ipos.z=points[l_index[0]].z;
                }
                data_s->pos[script_index]=*pos;
            }
        }
        else if (M)
            *pos = *pos + pos_from_parent * (*M);
        else
            *pos = *pos + pos_from_parent;

        if (c_part && emitter_point ) // Emit a  particle
        {
            Vector3D Dir;
            float life = 1.0f;
            byte nb = (Math::RandFromTable() % 60) + 1;
            ParticlesSystem* system = NULL;
            for (byte i = 0;i < nb; ++i)
            {
                Vector3D t_mod;
                bool random_vector = true;
                if (src != NULL)
                    for ( int base_n = Math::RandFromTable(), n = 0 ; random_vector && n < src->nb_sub_obj ; n++ )
                        random_vector = !src->random_pos( src_data, (base_n + n) % src->nb_sub_obj, &t_mod );
                if (random_vector)
                {
                    t_mod.x=((Math::RandFromTable()%2001)-1000)*0.001f;
                    t_mod.y=((Math::RandFromTable()%2001)-1000)*0.001f;
                    t_mod.z=((Math::RandFromTable()%2001)-1000)*0.001f;
                    t_mod.unit();
                    t_mod = (Math::RandFromTable() % 1001) * 0.001f * size * t_mod;
                    if (center)
                        t_mod=t_mod+(*center);
                }
                float speed=1.718281828f;			// exp(1.0f) - 1.0f because of speed law: S(t) = So * exp( -t / tref ) and a lifetime of 1 sec
                if (reverse)
                {
                    Dir=*pos-(t_mod+*target);
                    Dir.x+=upos->x;
                    Dir.y+=upos->y;
                    Dir.z+=upos->z;
                    system = particle_engine.emit_part_fast( system, t_mod+*target,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
                }
                else
                {
                    Dir=t_mod+*target-*pos;
                    Dir.x-=upos->x;
                    Dir.y-=upos->y;
                    Dir.z-=upos->z;
                    system = particle_engine.emit_part_fast( system, *upos+*pos,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
                }
            }
        }
        if (child)
            child->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
        *pos=opos;
        if (M)
            *M=OM;
        if (next)
            next->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
    }




    bool OBJECT::draw_shadow(Vector3D Dir,float t,SCRIPT_DATA *data_s,bool alset,bool exploding_parts)
    {
        bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
        bool hide=false;
        Vector3D ODir=Dir;
        glPushMatrix();
        if (explodes && !exploding_parts)
            goto draw_shadow_next;
        glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);

        if (script_index>=0 && data_s)
        {
            if (!explodes ^ exploding_parts)
            {
                glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
                glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
                glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
                glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
                Dir=((Dir*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD))*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD))*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
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
                Dir = ((Dir * RotateX(-R.x * DEG2RAD)) * RotateY(-R.y * DEG2RAD)) * RotateZ(-R.z * DEG2RAD);
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

            if (Dir.x != last_dir.x || Dir.y != last_dir.y || Dir.z != last_dir.z ) // Don't need to compute things twice
            {
                memset((byte*)line_on, 0, nb_line);
                uint16 e = 0;
                for (uint16 i = 0; i < nb_t_index; i += 3)
                {
                    if ((F_N[e++] % Dir ) >= 0.0f)
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
                glDrawElements(GL_QUADS, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout
            }
        }
        if (child && !(explodes && !exploding_parts))
        {
            glPushMatrix();
            alset=child->draw_shadow(Dir,t,data_s,alset,exploding_parts & !explodes);
            glPopMatrix();
        }
draw_shadow_next:
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




    bool OBJECT::draw_shadow_basic(Vector3D Dir,float t,SCRIPT_DATA *data_s,bool alset,bool exploding_parts)
    {
        bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
        bool hide=false;
        Vector3D ODir=Dir;
        glPushMatrix();
        if (explodes && !exploding_parts)
            goto draw_shadow_basic_next;

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
            Dir=((Dir*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD))*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD))*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
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
                Dir=((Dir*RotateX(-R.x*DEG2RAD))*RotateY(-R.y*DEG2RAD))*RotateZ(-R.z*DEG2RAD);
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

            if (Dir.x != last_dir.x || Dir.y != last_dir.y || Dir.z != last_dir.z ) // Don't need to compute things twice
            {
                memset((byte*)line_on,0,nb_line);

                uint16 e = 0;
                for (uint16 i = 0; i < nb_t_index; i += 3)
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
            glDrawElements(GL_QUADS, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout

            glFrontFace(GL_CCW);  						// 2nd passe
            glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
            glDrawElements(GL_QUADS, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout
        }

        if (child && !(explodes && !exploding_parts))
        {
            glPushMatrix();
            alset=child->draw_shadow_basic(Dir,t,data_s,alset, exploding_parts & !explodes);
            glPopMatrix();
        }
draw_shadow_basic_next:
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

    bool OBJECT::hit(Vector3D Pos,Vector3D Dir,SCRIPT_DATA *data_s,Vector3D *I,MATRIX_4x4 M)
    {
        MATRIX_4x4 OM = M;
        MATRIX_4x4 AM = Scale(1.0f);
        MATRIX_4x4 M_Dir = M;
        bool hide = false;
        Vector3D ODir = Dir;
        Vector3D OPos = Pos;
        bool is_hit = false;

        Vector3D T = pos_from_parent;
        Vector3D MP;
        if (script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE))	 // We can't select that
            goto hit_is_exploding;

        if (script_index>=0 && data_s)
        {
            T.x += data_s->axe[0][script_index].pos;
            T.y += data_s->axe[1][script_index].pos;
            T.z += data_s->axe[2][script_index].pos;
            MATRIX_4x4 l_M = Scale( 1.0f );
            if (data_s->axe[0][script_index].angle != 0.0f)
                l_M = l_M * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD);
            if (data_s->axe[1][script_index].angle != 0.0f)
                l_M = l_M * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD);
            if (data_s->axe[2][script_index].angle != 0.0f)
                l_M = l_M * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            M_Dir = M * l_M;
            M = l_M;

            //			M_Dir=M*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            //			M=RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            AM=RotateZ(data_s->axe[2][script_index].angle*DEG2RAD)*RotateY(data_s->axe[1][script_index].angle*DEG2RAD)*RotateX(data_s->axe[0][script_index].angle*DEG2RAD);
            hide=data_s->flag[script_index]&FLAG_HIDE;
        }
        else
            M = Scale(1.0f);
        Pos = (Pos - T) * M;

        if (( nb_t_index>0 || selprim >= 0 ) && !hide)
        {
            Vector3D A;
            Vector3D B;
            Vector3D C;
            Dir = Dir * M_Dir;
            Dir.unit();
            //-----------------Code de calcul d'intersection--------------------------
            for (short i = 0; i < nb_t_index; i += 3)
            {
                A = points[t_index[i]];
                B = points[t_index[i + 1]];
                C = points[t_index[i + 2]];
                Vector3D AB = B - A;
                Vector3D AC = C - A;
                Vector3D N  = AB * AC;
                if (N%Dir == 0.0f)
                    continue;
                float dist = -((Pos - A) % N) / (N % Dir);
                if (dist < 0.0f)
                    continue;
                Vector3D P_p = Pos + dist * Dir;

                //					if (is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
                if (is_hit && MP % Dir < P_p % Dir)
                    continue;

                float a;
                float b;
                float c;		// Coefficients pour que P soit le barycentre de A,B,C
                Vector3D AP = P_p - A;
                float pre_cal = AB.x * AC.y - AB.y * AC.x;
                if (AC.y != 0.0f && pre_cal != 0.0f)
                {
                    b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
                    a = (AP.y - b * AB.y) / AC.y;
                }
                else
                {
                    if (AB.x != 0.0f && pre_cal != 0.0f)
                    {
                        a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
                        b = (AP.x - a * AC.x) / AB.x;
                    }
                    else
                    {
                        pre_cal = AB.x * AC.z - AB.z * AC.x;
                        if (AC.z != 0.0f && pre_cal != 0.0f)
                        {
                            b=(AP.x*AC.z-AP.z*AC.x)/pre_cal;
                            a=(AP.z-b*AB.z)/AC.z;
                        }
                        else
                        {
                            pre_cal=-pre_cal;
                            if (AB.z!=0.0f && pre_cal!=0.0f)
                            {
                                a =(AP.x*AB.z-AP.z*AB.x)/pre_cal;
                                b=(AP.z-a*AC.z)/AB.z;
                            }
                            else
                            {
                                pre_cal = AB.y*AC.x-AB.x*AC.y;
                                if (AC.x!=0.0f && pre_cal!=0.0f)
                                {
                                    b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
                                    a=(AP.x-b*AB.x)/AC.x;
                                }
                                else
                                {
                                    if (AB.y!=0.0f && pre_cal!=0.0f)
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
                    if (N % Dir == 0.0f)
                        continue;
                    float dist = -((Pos - A) % N) / (N % Dir);
                    if (dist < 0.0f)
                        continue;
                    Vector3D P_p=Pos+dist*Dir;

                    //						if (is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
                    if (is_hit && MP % Dir < P_p % Dir)
                        continue;

                    float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
                    Vector3D AP = P_p - A;
                    float pre_cal = AB.x * AC.y - AB.y * AC.x;
                    if (AC.y != 0.0f && pre_cal != 0.0f)
                    {
                        b = (AP.x * AC.y - AP.y * AC.x) / pre_cal;
                        a = (AP.y - b * AB.y) / AC.y;
                    }
                    else
                    {
                        if (AB.x != 0.0f && pre_cal != 0.0f)
                        {
                            a = (AP.y * AB.x - AP.x * AB.y) / pre_cal;
                            b = (AP.x - a * AC.x) / AB.x;
                        }
                        else
                        {
                            pre_cal = AB.x * AC.z - AB.z * AC.x;
                            if (AC.z != 0.0f && pre_cal != 0.0f)
                            {
                                b = (AP.x * AC.z - AP.z * AC.x) / pre_cal;
                                a = (AP.z - b * AB.z) / AC.z;
                            }
                            else
                            {
                                pre_cal = -pre_cal;
                                if (AB.z != 0.0f && pre_cal != 0.0f)
                                {
                                    a = (AP.x * AB.z - AP.z * AB.x) / pre_cal;
                                    b = (AP.z - a * AC.z) / AB.z;
                                }
                                else
                                {
                                    pre_cal = AB.y*AC.x-AB.x*AC.y;
                                    if (AC.x!=0.0f && pre_cal!=0.0f)
                                    {
                                        b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
                                        a=(AP.x-b*AB.x)/AC.x;
                                    }
                                    else
                                    {
                                        if (AB.y != 0.0f && pre_cal != 0.0f)
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
                }
            }
        }

        if (child)
        {
            Vector3D MP2;
            bool nhit = child->hit(Pos, ODir, data_s, &MP2, M_Dir);
            if (nhit && !is_hit)
                MP = MP2;
            else
            {
                if (nhit && is_hit)
                {
                    if (MP2 % Dir < MP % Dir)
                        MP = MP2;
                }
            }
            is_hit |= nhit;
        }
        if (is_hit)
            MP = (MP * AM) + T;
hit_is_exploding:
        if (next)
        {
            Vector3D MP2;
            bool nhit = next->hit(OPos, ODir, data_s, &MP2, OM);
            Dir = ODir * OM;
            if (nhit && !is_hit)
                MP = MP2;
            else
            {
                if (nhit && is_hit)
                    if (MP2 % Dir < MP % Dir)
                        MP = MP2;
            }
            is_hit |= nhit;
        }
        if (is_hit)
            *I = MP;
        return is_hit;
    }



    // hit_fast is a faster version of hit but less precise, designed for use in weapon code
    bool OBJECT::hit_fast(Vector3D Pos,Vector3D Dir,SCRIPT_DATA *data_s,Vector3D *I)
    {
        bool hide = false;
        Vector3D ODir = Dir;
        Vector3D OPos = Pos;
        MATRIX_4x4 AM;
        bool is_hit = false;


        Vector3D T = pos_from_parent;
        Vector3D MP;
        if (script_index >= 0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE))
            goto hit_fast_is_exploding;
        if (script_index >= 0 && data_s)
        {
            T.x += data_s->axe[0][script_index].pos;
            T.y += data_s->axe[1][script_index].pos;
            T.z += data_s->axe[2][script_index].pos;
            MATRIX_4x4 l_M = Scale( 1.0f );
            if (data_s->axe[0][script_index].angle != 0.0f)
                l_M = l_M * RotateX(-data_s->axe[0][script_index].angle * DEG2RAD);
            if (data_s->axe[1][script_index].angle != 0.0f)
                l_M = l_M * RotateY(-data_s->axe[1][script_index].angle * DEG2RAD);
            if (data_s->axe[2][script_index].angle != 0.0f)
                l_M = l_M * RotateZ(-data_s->axe[2][script_index].angle * DEG2RAD);
            Dir = Dir * l_M;
            Pos = (Pos - T) * l_M;
            //			Dir = ((Dir * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)) * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)) * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            //			Pos = (((Pos - T) * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)) * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)) * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            AM = RotateZ(data_s->axe[2][script_index].angle * DEG2RAD)
                * RotateY(data_s->axe[1][script_index].angle * DEG2RAD) * RotateX(data_s->axe[0][script_index].angle * DEG2RAD);
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
                if (Dir.x != 0.0f ) // 2 x planes
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
                if (!is_hit && Dir.y != 0.0f )// 2 y planes
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
                if (!is_hit && Dir.z != 0.0f )// 2 z planes
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
                if (!is_hit || MP2%Dir<MP%Dir)
                    MP = MP2;
                is_hit = true;
            }
        }
        if (is_hit)
            MP = (MP * AM) + T;

hit_fast_is_exploding:
        if (next && !is_hit)
        {
            Vector3D MP2;
            bool nhit = next->hit_fast( OPos, ODir, data_s, &MP2);
            if (nhit)
            {
                if (!is_hit || MP2%ODir<MP%ODir)
                    MP = MP2;
                is_hit = true;
            }
        }
        if (is_hit)
            *I = MP;
        return is_hit;
    }






    float OBJECT::print_struct(float Y, float X, TA3D::GfxFont& fnt)
    {
        gfx->print(fnt, X, Y, 0.0f,      0xFFFFFF, format("%s [%d]", name,script_index));
        gfx->print(fnt, 320.0f, Y, 0.0f, 0xFFFFFF, format("(v:%d",   nb_vtx));
        gfx->print(fnt, 368.0f, Y, 0.0f, 0xFFFFFF, format(",p:%d",   nb_prim));
        gfx->print(fnt, 416.0f, Y, 0.0f, 0xFFFFFF, format(",t:%d",   nb_t_index));
        gfx->print(fnt, 464.0f, Y, 0.0f, 0xFFFFFF, format(",l:%d",   nb_l_index));
        gfx->print(fnt, 512.0f, Y, 0.0f, 0xFFFFFF, format(",p:%d)",  nb_p_index));
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
            return &(model[e]);

        e = model_hashtable.find("objects3d\\" + l + ".3dm") - 1;
        if (e >= 0)
            return &(model[e]);

        e = model_hashtable.find(l) - 1;
        if (e >= 0)
            return &(model[e]);

        e = model_hashtable.find(l + ".3do") - 1;
        if (e >= 0)
            return &(model[e]);

        e = model_hashtable.find(l + ".3dm") - 1;
        if (e >= 0)
            return &(model[e]);
        return NULL;
    }


    MODEL_MANAGER::~MODEL_MANAGER()
    {
        destroy();
        model_hashtable.emptyHashTable();
    }


    void MODEL_MANAGER::destroy()
    {
        if (model)
        {
            for (int i = 0; i < nb_models; ++i)
                model[i].destroy();
            free(model);
        }
        if (name)
        {
            for (int i = 0; i < nb_models; ++i)
                free(name[i]);
            free(name);
        }
        model_hashtable.emptyHashTable();
        model_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
        init();
    }



    void MODEL_MANAGER::compute_ids()
    {
        for (int i = 0; i < nb_models; ++i)
            model[i].id = i;
    }

    void MODEL_MANAGER::optimise_all()
    {
        for (int i = 0; i < nb_models; ++i)
            model[i].obj.optimise_mesh();
    }


    void MODEL_MANAGER::create_from_2d(BITMAP *bmp,float w,float h,float max_h,const String& filename)
    {
        MODEL *n_model=(MODEL*) malloc(sizeof(MODEL)*(nb_models+1));
        char **n_name=(char**) malloc(sizeof(char*)*(nb_models+1));
        if(model)
        {
            memcpy(n_model,model,sizeof(MODEL)*nb_models);
            free(model);
            memcpy(n_name,name,sizeof(char*)*nb_models);
            free(name);
        }
        model=n_model;
        name=n_name;
        model[nb_models].init();
        name[nb_models] = strdup(filename.c_str());

        model_hashtable.insert(String::ToLower(filename), nb_models + 1);

        model[nb_models].create_from_2d(bmp,w,h,max_h);
        ++nb_models;
    }


    int MODEL_MANAGER::load_all(void (*progress)(float percent,const String &msg))
    {
        const String loading3DModelsText = I18N::Translate("Loading 3D Models");

        String::List file_list;
        sint32 new_nb_models = HPIManager->getFilelist(ta3dSideData.model_dir + "*.3dm", file_list);

        if (new_nb_models > 0)
        {
            MODEL *n_model = (MODEL*) malloc(sizeof(MODEL)*(nb_models + new_nb_models));
            char **n_name = (char**) malloc(sizeof(char*)*(nb_models + new_nb_models));
            if (model)
            {
                memcpy(n_model,model,sizeof(MODEL)*nb_models);
                free(model);
                memcpy(n_name,name,sizeof(char*)*nb_models);
                free(name);
            }
            model = n_model;
            name  = n_name;
            int i = 0;
            int n = 0;
            for (String::List::const_iterator e = file_list.begin(); e != file_list.end(); ++e)
            {
                LOG_DEBUG("[3dm] Loading `" << *e << "`");
                if (progress!=NULL && n % 25 == 0)
                    progress((100.0f + n * 50.0f / (new_nb_models + 1)) / 7.0f, loading3DModelsText);
                ++n;
                model[i+nb_models].init();
                name[i+nb_models] = strdup(e->c_str());

                if (get_model(String( name[i+nb_models] ).substr(0, e->size() - 4).c_str())==NULL) 	// Vérifie si le modèle n'est pas déjà chargé
                {
                    byte *data = HPIManager->PullFromHPI(*e);
                    if (data)
                    {
                        if (data[0] == 0 )
                        {
                            String real_name = (char*)(data+1);
                            real_name.trim();
                            delete[] data;
                            data = HPIManager->PullFromHPI( real_name );
                        }
                        if (data)
                        {
                            model[i+nb_models].load_3dm(data);
                            delete[] data;
                            model_hashtable.insert(String::ToLower(*e), nb_models + i + 1);
                            ++i;
                        }
                    }
                }
            }
            nb_models += i;
        }

        file_list.clear();
        new_nb_models = HPIManager->getFilelist(ta3dSideData.model_dir + "*.3do", file_list);

        if (new_nb_models > 0)
        {
            MODEL *n_model = (MODEL*) malloc(sizeof(MODEL)*(nb_models+new_nb_models));
            char **n_name = (char**) malloc(sizeof(char*)*(nb_models+new_nb_models));
            if (model)
            {
                memcpy(n_model,model,sizeof(MODEL)*nb_models);
                free(model);
                memcpy(n_name,name,sizeof(char*)*nb_models);
                free(name);
            }
            model = n_model;
            name = n_name;
            int i = 0, n = 0;
            for (String::List::const_iterator e = file_list.begin();e != file_list.end(); ++e)
            {
                LOG_DEBUG("[3do] Loading `" << *e << "`");
                if (progress != NULL && n % 25 == 0)
                    progress((100.0f + (50.0f + n * 50.0f / (new_nb_models + 1))) / 7.0f, loading3DModelsText);
                ++n;
                model[i+nb_models].init();
                name[i+nb_models] = strdup(e->c_str());

                if (get_model(String(name[i+nb_models]).substr(0, e->size() - 4).c_str() ) == NULL) // Vérifie si le modèle n'est pas déjà chargé
                {
                    uint32 data_size = 0;
                    byte *data = HPIManager->PullFromHPI(*e, &data_size);
                    if (data)
                    {
                        if (data_size > 0 )						// If the file isn't empty
                            model[i+nb_models].load_3do(data,e->c_str());
                        delete[] data;
                        model_hashtable.insert(String::ToLower(*e), nb_models + i + 1);
                        ++i;
                    }
                }
            }
            nb_models += i;
        }
        return 0;
    }


    void MODEL::init()
    {
        nb_obj = 0;
        obj.init();
        center.x=center.y=center.z=0.0f;
        size=0.0f;
        size2=0.0f;
        dlist = 0;
        animated = false;
        top = bottom = 0.0f;
        from_2d = false;
    }



    void MODEL::destroy()
    {
        obj.destroy();
        if (dlist)
            glDeleteLists(dlist, 1);
        init();
    }


    void MODEL::load_3dm(byte *data)					// Load a model in 3DM format
    {
        destroy();
        obj.load_3dm(data);
        nb_obj = obj.set_obj_id( 0 );

        animated = obj.has_animation_data();

        Vector3D O;
        O.x=O.y=O.z=0.0f;
        int coef=0;
        center.x=center.y=center.z=0.0f;
        obj.compute_center(&center,O,&coef);
        center=(1.0f/coef)*center;
        size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
        size2=sqrt(0.5f*size);
        obj.compute_emitter();
        compute_topbottom();
    }


    int MODEL::load_3do(byte *data,const char *filename)
    {
        int err=obj.load_obj(data,0,0,filename);		// Charge les objets composant le modèle
        if (err == 0)
        {
            nb_obj = obj.set_obj_id( 0 );

            Vector3D O;
            O.x=O.y=O.z=0.0f;
            int coef=0;
            center.x=center.y=center.z=0.0f;
            obj.compute_center(&center,O,&coef);
            center=(1.0f/coef)*center;
            size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
            size2=sqrt(0.5f*size);
            obj.compute_emitter();
            compute_topbottom();
        }
        return err;
    }


    void MODEL::create_from_2d(BITMAP *bmp,float w,float h,float max_h)
    {
        obj.create_from_2d(bmp,w,h,max_h);

        nb_obj = obj.set_obj_id( 0 );

        from_2d = true;
        Vector3D O;
        O.x=O.y=O.z=0.0f;
        int coef=0;
        center.x=center.y=center.z=0.0f;
        obj.compute_center(&center,O,&coef);
        center=(1.0f/coef)*center;
        obj.compute_emitter();
        size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
        size2=sqrt(0.5f*size);
        compute_topbottom();
    }

    void MODEL::draw(float t,SCRIPT_DATA *data_s,bool sel,bool notex,bool c_part,int p_tex,Vector3D *target,Vector3D *upos,MATRIX_4x4 *M,float Size,Vector3D* Center,bool reverse,int side,bool chg_col,OBJECT *src,SCRIPT_DATA *src_data)
    {
        if (notex)
            glDisable(GL_TEXTURE_2D);
        Vector3D pos;
        if (chg_col)
        {
            if (notex)
            {
                byte var = abs(255 - (((int)(t * 256) & 0xFF)<<1));
                glColor3ub(0,var,0);
            }
            else
                glColor3ub(255,255,255);
        }

        if( data_s == NULL && animated )
            obj.draw(t,NULL,sel,false,notex,side,chg_col);
        else
        {
            if( data_s == NULL && dlist == 0 && !sel && !notex && !chg_col )
            {
                dlist = glGenLists (1);
                glNewList (dlist, GL_COMPILE);
                obj.draw_dl(data_s,false,side,chg_col);
                glEndList();
                glCallList( dlist );
            }
            else 
            {
                if( data_s == NULL && !sel && !notex && !chg_col )
                    glCallList( dlist );
                else
                {
                    obj.draw(t,data_s,sel,false,notex,side,chg_col);
                    if( data_s && data_s->explode )
                        obj.draw(t,data_s,sel,false,notex,side,chg_col,true);
                }
            }
        }
        if(c_part)
            obj.compute_coord(data_s,&pos,c_part,p_tex,target,upos,M,Size,Center,reverse,src,src_data);
    }


    void MODEL::draw_shadow(Vector3D Dir,float t,SCRIPT_DATA *data_s)
    {
        glDisable(GL_TEXTURE_2D);
        obj.draw_shadow(Dir,t,data_s,false);
        if( data_s && data_s->explode )
            obj.draw_shadow(Dir,t,data_s,false,true);
    }

    void MODEL::draw_shadow_basic(Vector3D Dir,float t,SCRIPT_DATA *data_s)
    {
        glDisable(GL_TEXTURE_2D);
        obj.draw_shadow_basic(Dir,t,data_s,false);
        if( data_s && data_s->explode )
            obj.draw_shadow_basic(Dir,t,data_s,false,true);
    }


    void MODEL::compute_coord(SCRIPT_DATA *data_s,MATRIX_4x4 *M)
    {
        Vector3D pos;
        pos.x=pos.y=pos.z=0.0f;
        obj.compute_coord(data_s,&pos,false,0,NULL,NULL,M);
    }


    void MODEL::save_3dm(char *filename,bool compressed)					// Save the model to the 3DM format
    {
        FILE *dst = TA3D_OpenFile(filename,"wb");
        if(dst)
        {
            obj.save_3dm(dst, compressed);
            fclose(dst);
        }
        else
            LOG_ERROR(LOG_PREFIX_3DM << "Impossible to save the 3DM file: `"
                      << (filename == NULL ? "NULL" : filename) << "` can not be opened for writing");
    }


    void MODEL::compute_topbottom()
    {
        Vector3D O;
        O.x = O.y = O.z = 0.0f;
        top = obj.compute_top( -99999.0f, O );
        bottom = obj.compute_bottom( 99999.0f, O );
    }



    void MODEL::load_asc(char *filename,float size)		// Charge un fichier au format *.ASC
    {

        destroy();			// Puisqu'on charge

        float *coor[3];
        coor[0] = new float[100000];
        coor[1] = new float[100000];
        coor[2] = new float[100000];
        int *face[3];
        face[0] = new int[100000];
        face[1] = new int[100000];
        face[2] = new int[100000];

        if (coor[0] == NULL || coor[1] == NULL || coor[2] == NULL
            || face[0] == NULL || face[1] == NULL || face[2] == NULL)
        {
            if (coor[0] != NULL) delete[] coor[0];
            if (coor[1] != NULL) delete[] coor[1];
            if (coor[2] != NULL) delete[] coor[2];
            if (face[0] != NULL) delete[] face[0];
            if (face[1] != NULL) delete[] face[1];
            if (face[2] != NULL) delete[] face[2];
            return;
        }

        long nbp=0,nbf=0;

        FILE  *fichier;
        char  chaine[200];
        char  *fin;
        long  i,j;
        char  temp[50];
        float x,y,z;
        int   decalage=0;
        int nbpt;
        int test;
        float dx,dy,dz;
        float xmin=0xFFFFFF,ymin=0xFFFFFF,zmin=0xFFFFFF,
              xmax=-0xFFFFFF,ymax=-0xFFFFFF,zmax=-0xFFFFFF;

        int StructD[4096];     // Données pour la restitution de la structure
        char *StructName[4096];
        int NbStruct=0;

        nbpt=0;
        test=0;
        nbf=0;
        nbp=0;

        if ((fichier = TA3D_OpenFile(filename, "rt")) == NULL)
        {
            LOG_ERROR(LOG_PREFIX_MODEL << "Impossible to open `" << filename << "`");
            return;
        }

        do
        {
            // On lit le fichier contenant les informations sur l'objet
            fin = fgets(chaine, 200, fichier);
            strupr(chaine);
            if (!strncmp(chaine, "VERTEX", 6))
            {
                if (strncmp(chaine,"VERTEX LIST", 11))
                {
                    // Lecture des coordonnées d'un point
                    i=6;

                    while(chaine[i]!='X') i++;
                    i+=2;
                    while(chaine[i]==' ') i++;
                    sscanf(chaine+i,"%f",&x);

                    while(chaine[i]!='Y') i++;
                    i+=2;
                    while(chaine[i]==' ') i++;
                    sscanf(chaine+i,"%f",&y);

                    while(chaine[i]!='Z') i++;
                    i+=2;
                    while(chaine[i]==' ') i++;
                    sscanf(chaine+i,"%f",&z);

                    coor[0][nbp]=x;
                    coor[1][nbp]=y;
                    coor[2][nbp]=z;

                    if (x<xmin) xmin=x;
                    if (x>xmax) xmax=x;
                    if (y<ymin) ymin=y;
                    if (y>ymax) ymax=y;
                    if (z<zmin) zmin=z;
                    if (z>zmax) zmax=z;

                    nbp++;
                }
            }
            else
            {
                if (!strncmp(chaine, "FACE", 4))
                {
                    if (strncmp(chaine, "FACE LIST", 9))
                    {
                        // Lecture d'une facette
                        i=j=4;
                        while (chaine[i] != 'A') ++i;
                        i += 2;
                        while (chaine[i] == ' ') ++i;
                        j = i;
                        while (chaine[j] != ' ') ++j;
                        strncpy(temp,chaine+i,j-i);
                        temp[j-i] = 0;
                        face[0][nbf] = atoi(temp) + decalage;

                        while (chaine[i] != 'B') ++i;
                        i += 2;
                        while (chaine[i] == ' ') ++i;
                        j = i;
                        while (chaine[j] != ' ') ++j;
                        strncpy(temp,chaine+i,j-i);
                        temp[j-i] = 0;
                        face[1][nbf] = atoi(temp) + decalage;

                        while (chaine[i] != 'C') ++i;
                        i += 2;
                        while (chaine[i] == ' ') ++i;
                        j = i;
                        while (chaine[j] != ' ') ++j;
                        strncpy(temp, chaine + i, j - i);
                        temp[j-i] = 0;
                        face[2][nbf] = atoi(temp) + decalage;
                        ++nbf;
                    }
                }
                else
                    if (!strncmp(chaine, "NAMED OBJECT", 12))
                    {
                        StructName[NbStruct] = strdup(chaine + 13);
                        char *_p = StructName[NbStruct];
                        while (_p[0])
                        {
                            if (_p[0] == 10 || _p[0] == 13)
                                _p[0] = 0;
                            ++_p;
                        }
                        decalage = nbp;
                        StructD[NbStruct++] = nbf;
                    }
            }
        } while(fin!=NULL);

        fclose(fichier);

        StructD[NbStruct] = nbf;
        OBJECT *cur = &obj;

        dx = -(xmin + xmax) * 0.5f;
        dy = -(ymin + ymax) * 0.5f;
        dz = -(zmin + zmax) * 0.5f;
        xmin = xmax - xmin;
        ymin = ymax - ymin;
        zmin = zmax - zmin;
        xmax = sqrt(xmin * xmin + ymin * ymin + zmin * zmin);
        size = size / xmax;

        for (i = 0; i < NbStruct; ++i) // Crée les différentes parties de la meshe
        {
            if (i>0)
            {
                cur->next = new OBJECT;
                cur = cur->next;
            }
            cur->name = StructName[i];
            cur->nb_prim = StructD[i + 1] - StructD[i];
            cur->nb_t_index = cur->nb_prim * 3;
            cur->nb_vtx = cur->nb_t_index;
            cur->points = new Vector3D[cur->nb_vtx];
            cur->t_index = new GLushort[cur->nb_t_index];
            cur->tcoord = new float[cur->nb_vtx<<1];

            cur->surface.Flag=SURFACE_ADVANCED|SURFACE_GOURAUD|SURFACE_LIGHTED;
            for (int k = 0; k < 4; ++k)
                cur->surface.Color[k] = cur->surface.RColor[k] = 1.0f;

            int p_nbf = 0;
            int p_nbp = 0;

            for (int k = StructD[i]; k < StructD[i+1]; ++k) // Compte et organise les points
            {
                cur->t_index[p_nbf++]=p_nbp;
                cur->t_index[p_nbf++]=p_nbp+1;
                cur->t_index[p_nbf++]=p_nbp+2;

                cur->points[p_nbp].x=coor[0][face[0][k]];
                cur->points[p_nbp].y=coor[1][face[0][k]];
                cur->points[p_nbp++].z=coor[2][face[0][k]];

                cur->points[p_nbp].x=coor[0][face[1][k]];
                cur->points[p_nbp].y=coor[1][face[1][k]];
                cur->points[p_nbp++].z=coor[2][face[1][k]];

                cur->points[p_nbp].x=coor[0][face[2][k]];
                cur->points[p_nbp].y=coor[1][face[2][k]];
                cur->points[p_nbp++].z=coor[2][face[2][k]];
            }			// Fin de for (k=StructD[i];k<StructD[i+1];k++)
        }				// Fin de for (i=0;i<NbStruct;i++)

        cur = &obj;
        while (cur)
        {
            int removed=0;
            for (i = 0; i < cur->nb_t_index; ++i) // Remove duplicate points
            {
                for (int e = 0;e < i; ++e)
                {
                    if (cur->points[cur->t_index[i]].x == cur->points[cur->t_index[e]].x && cur->points[cur->t_index[i]].y == cur->points[cur->t_index[e]].y && cur->points[cur->t_index[i]].z == cur->points[cur->t_index[e]].z)
                    {
                        cur->t_index[i] = cur->t_index[e];
                        ++removed;
                        break;
                    }
                }
            }
            cur->nb_vtx -= removed;
            Vector3D* n_points = new Vector3D[cur->nb_vtx];
            int cur_pt = 0;
            for (i = 0; i  <cur->nb_t_index; ++i)
            {
                bool ok = false;
                for (int e = 0; e < i; ++e)
                {
                    if (cur->points[cur->t_index[i]].x == n_points[cur->t_index[e]].x && cur->points[cur->t_index[i]].y==n_points[cur->t_index[e]].y && cur->points[cur->t_index[i]].z==n_points[cur->t_index[e]].z) {
                        cur->t_index[i] = cur->t_index[e];
                        ok = true;
                        break;
                    }
                }
                if (ok)
                    continue;
                n_points[cur_pt] = cur->points[cur->t_index[i]];
                cur->t_index[i] = cur_pt++;
            }
            delete[] cur->points;
            cur->points = n_points;
            for (i = 0; i < cur->nb_vtx; ++i)// Remove duplicate points
            {
                cur->points[i].x *= size;
                cur->points[i].y *= size;
                cur->points[i].z *= size;
            }

            cur->N = new Vector3D[cur->nb_vtx];	// Calculate normals
            cur->F_N = new Vector3D[cur->nb_t_index / 3];
            for (i = 0; i < cur->nb_vtx; ++i)
                cur->N[i].x=cur->N[i].z=cur->N[i].y=0.0f;
            int e = 0;
            for (i = 0; i < cur->nb_t_index; i += 3)
            {
                Vector3D AB;
                Vector3D AC;
                Vector3D Normal;
                AB = cur->points[cur->t_index[i+1]] - cur->points[cur->t_index[i]];
                AC = cur->points[cur->t_index[i+2]] - cur->points[cur->t_index[i]];
                Normal = AB * AC;
                Normal.unit();
                cur->F_N[e++] = Normal;
                for (byte e = 0; e < 3; ++e)
                    cur->N[cur->t_index[i + e]] = cur->N[cur->t_index[i + e]] + Normal;
            }
            for (i = 0; i < cur->nb_vtx; ++i)
                cur->N[i].unit();
            cur = cur->next;
        }

        delete[] coor[0];
        delete[] coor[1];
        delete[] coor[2];
        delete[] face[0];
        delete[] face[1];
        delete[] face[2];
    }




    void OBJECT::save_3dm(FILE *dst, bool compressed)
    {
        uint8 len = strlen(name);
        fwrite(&len,sizeof(len),1,dst);		// Write the object name
        fwrite(name,len,1,dst);

        fwrite(&pos_from_parent.x,sizeof(pos_from_parent.x),1,dst);
        fwrite(&pos_from_parent.y,sizeof(pos_from_parent.y),1,dst);
        fwrite(&pos_from_parent.z,sizeof(pos_from_parent.z),1,dst);

        fwrite(&nb_vtx,sizeof(nb_vtx),1,dst);
        if (points!=NULL)
            fwrite(points,sizeof(Vector3D)*nb_vtx,1,dst);

        fwrite(sel,sizeof(GLushort)*4,1,dst);				// Selection primitive

        fwrite(&nb_p_index,sizeof(nb_p_index),1,dst);		// Write point data
        if (p_index!=NULL)
            fwrite(p_index,sizeof(GLushort)*nb_p_index,1,dst);

        fwrite(&nb_l_index,sizeof(nb_l_index),1,dst);		// Write line data
        if (l_index!=NULL)
            fwrite(l_index,sizeof(GLushort)*nb_l_index,1,dst);

        fwrite(&nb_t_index,sizeof(nb_t_index),1,dst);		// Write triangle data
        if (t_index!=NULL)
            fwrite(t_index,sizeof(GLushort)*nb_t_index,1,dst);

        fwrite(tcoord,sizeof(float)*nb_vtx<<1,1,dst);		// Write texture coordinates

        fwrite(surface.Color,sizeof(float)*4,1,dst);		// Write surface data
        fwrite(surface.RColor,sizeof(float)*4,1,dst);
        fwrite(&surface.Flag,sizeof(surface.Flag),1,dst);
        int tmp=surface.NbTex;
        if (!(surface.Flag&SURFACE_TEXTURED))
            surface.NbTex=0;
        else
            if (compressed)
                surface.NbTex = -surface.NbTex; // For compatibility with older versions

        fwrite(&surface.NbTex, sizeof(surface.NbTex), 1, dst);
        surface.NbTex = tmp;
        if (surface.Flag & SURFACE_TEXTURED)
            for (uint8 i = 0; i < surface.NbTex; ++i)
            {
                BITMAP *tex;
                GLint w,h;
                glBindTexture(GL_TEXTURE_2D,surface.gltex[i]);
                glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);
                glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
                tex=create_bitmap_ex(32,w,h);
                glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,tex->line[0]);

                if (!compressed) // Store texture data without compression
                {
                    fwrite(&tex->w,sizeof(tex->w),1,dst);
                    fwrite(&tex->h,sizeof(tex->h),1,dst);
                    for (int y = 0; y < tex->h; ++y)
                    {
                        for (int x = 0; x < tex->w; ++x)
                            fwrite(&((int*)(tex->line[y]))[x], 4, 1, dst);
                    }
                }
                else 
                {
                    // Store texture data in JPG format (using two images, one for RGB and one for alpha,
                    // if needed -> check first that alpha isn't all 0xFF)

                    int buf_size = tex->w * tex->h * 5;
                    byte *buffer = new byte[buf_size];

                    int img_size = buf_size;
                    save_memory_jpg_ex(buffer, &img_size, tex, NULL, 85, JPG_SAMPLING_411 | JPG_OPTIMIZE, NULL);

                    fwrite(&img_size, sizeof(img_size), 1, dst); // Save the result
                    fwrite(buffer, img_size, 1, dst);

                    bool need_alpha = false;
                    for (int y = 0 ; y < tex->h ; ++y)
                    {
                        for (int x = 0; x < tex->w ; ++x)
                        {
                            int c = geta(getpixel(tex, x, y));
                            if (c != 255)
                                need_alpha = true;
                            ((uint32*)(tex->line[y]))[x] = c * 0x01010101;
                        }
                    }

                    if (need_alpha)
                    {
                        putc(1, dst);		// Alpha channel has to be stored
                        img_size = buf_size;
                        save_memory_jpg_ex(buffer, &img_size, tex, NULL, 100, JPG_GREYSCALE | JPG_OPTIMIZE, NULL);

                        fwrite(&img_size, sizeof(img_size), 1, dst);		// Save the result
                        fwrite(buffer, img_size, 1, dst);
                    }
                    else
                        putc(0, dst);		// No alpha channel stored
                    delete[] buffer;
                }
                destroy_bitmap(tex);
            }

        if (surface.Flag & SURFACE_GLSL)// Save the shader object
        {
            uint32 shader_size = surface.vert_shader_src.size();
            fwrite(&shader_size,4,1,dst);
            fwrite(surface.vert_shader_src.c_str(),shader_size,1,dst);

            shader_size = surface.frag_shader_src.size();
            fwrite(&shader_size,4,1,dst);
            fwrite(surface.frag_shader_src.c_str(),shader_size,1,dst);
        }

        if (animation_data) // Save animation data
        {
            fputc(2, dst);
            fputc( animation_data->type, dst);
            fwrite( &(animation_data->angle_0), sizeof(Vector3D), 1, dst);
            fwrite( &(animation_data->angle_1), sizeof(Vector3D), 1, dst);
            fwrite( &(animation_data->angle_w), sizeof(float), 1, dst);
            fwrite( &(animation_data->translate_0), sizeof(Vector3D), 1, dst);
            fwrite( &(animation_data->translate_1), sizeof(Vector3D), 1, dst);
            fwrite( &(animation_data->translate_w), sizeof(float), 1, dst);
        }

        if (child)
        {
            fputc(1,dst);
            child->save_3dm(dst, compressed);
        }
        else
            fputc(0,dst);

        if (next)
        {
            fputc(1,dst);
            next->save_3dm(dst, compressed);
        }
        else
            fputc(0,dst);
    }




    inline byte* read_from_mem(void* buf, const int len, byte* data)
    {
        memcpy(buf, data, len);
        return data + len;
    }



    byte *OBJECT::load_3dm(byte *data)
    {
        destroy();

        if (data == NULL)
            return NULL;

        uint8	len = data[0];	data++;
        name = (char*) malloc(len+1);
        data=read_from_mem(name,len,data);
        name[len]=0;

        data=read_from_mem(&pos_from_parent.x,sizeof(pos_from_parent.x),data);
        if (isNaN(pos_from_parent.x))           // Some error checks
        {
            free( name );   name = NULL;    return NULL;
        }

        data=read_from_mem(&pos_from_parent.y,sizeof(pos_from_parent.y),data);
        if (isNaN(pos_from_parent.y))           // Some error checks
        {
            free( name );   name = NULL;    return NULL;
        }

        data=read_from_mem(&pos_from_parent.z,sizeof(pos_from_parent.z),data);
        if (isNaN(pos_from_parent.z))           // Some error checks
        {
            free( name );   name = NULL;    return NULL;
        }

        data=read_from_mem(&nb_vtx,sizeof(nb_vtx),data);
        if (nb_vtx < 0)           // Some error checks
        {
            free( name );   name = NULL;    return NULL;
        }
        if (nb_vtx>0)
        {
            points = new Vector3D[nb_vtx<<1];
            data=read_from_mem(points,sizeof(Vector3D)*nb_vtx,data);
        }
        else
            points=NULL;

        data=read_from_mem(sel,sizeof(GLushort)*4,data);

        data=read_from_mem(&nb_p_index,sizeof(nb_p_index),data);	// Read point data
        if (nb_p_index < 0)
        {
            if (points) delete[] points;
            free( name );
            init();
            return NULL;
        }
        if (nb_p_index>0)
        {
            p_index = new GLushort[nb_p_index];
            data=read_from_mem(p_index,sizeof(GLushort)*nb_p_index,data);
        }
        else
            p_index=NULL;

        data=read_from_mem(&nb_l_index,sizeof(nb_l_index),data);	// Read line data
        if (nb_l_index < 0)
        {
            if (points) delete[] points;
            if (p_index) delete[] p_index;
            free( name );
            init();
            return NULL;
        }
        if (nb_l_index>0)
        {
            l_index = new GLushort[nb_l_index];
            data=read_from_mem(l_index,sizeof(GLushort)*nb_l_index,data);
        }
        else
            l_index=NULL;

        data=read_from_mem(&nb_t_index,sizeof(nb_t_index),data);	// Read triangle data
        if (nb_t_index < 0)
        {
            if (points) delete[] points;
            if (p_index) delete[] p_index;
            if (l_index) delete[] l_index;
            free( name );
            init();
            return NULL;
        }
        if (nb_t_index>0)
        {
            t_index = new GLushort[nb_t_index];
            data=read_from_mem(t_index,sizeof(GLushort)*nb_t_index,data);
        }
        else
            t_index=NULL;

        tcoord = new float[nb_vtx<<1];
        data=read_from_mem(tcoord,sizeof(float)*nb_vtx<<1,data);

        data=read_from_mem(surface.Color,sizeof(float)*4,data);	// Read surface data
        data=read_from_mem(surface.RColor,sizeof(float)*4,data);
        data=read_from_mem(&surface.Flag,sizeof(surface.Flag),data);
        surface.NbTex=0;
        data=read_from_mem(&surface.NbTex,sizeof(surface.NbTex),data);
        bool compressed = surface.NbTex < 0;
        surface.NbTex = abs( surface.NbTex );
        for (uint8 i = 0; i < surface.NbTex; ++i)
        {
            BITMAP *tex;
            if (!compressed)
            {
                int tex_w;
                int tex_h;
                data = read_from_mem(&tex_w, sizeof(tex_w), data);
                data = read_from_mem(&tex_h, sizeof(tex_h), data);

                tex = create_bitmap_ex(32, tex_w, tex_h);
                if (tex == NULL)
                {
                    destroy();
                    return NULL;
                }
                try
                {
                    for (int y = 0; y < tex->h; ++y)
                        for (int x = 0; x < tex->w; ++x)
                            data = read_from_mem(&((int*)(tex->line[y]))[x], 4, data);
                }
                catch( ... )
                {
                    destroy();
                    return NULL;
                };
            }
            else
            {
                int img_size = 0;
                data=read_from_mem(&img_size,sizeof(img_size),data);	// Read RGB data first
                byte *buffer = new byte[ img_size ];

                try
                {
                    data=read_from_mem( buffer, img_size, data );

                    set_color_depth( 32 );
                    tex = load_memory_jpg( buffer, img_size, NULL );
                }
                catch( ... )
                {
                    delete[] buffer;
                    destroy();
                    return NULL;
                };

                delete[] buffer;

                byte has_alpha;									// Read alpha channel if present
                data=read_from_mem( &has_alpha, 1, data );
                if (has_alpha)
                {
                    data=read_from_mem(&img_size,sizeof(img_size),data);
                    buffer = new byte[ img_size ];
                    data = read_from_mem( buffer, img_size, data );
                    BITMAP* alpha = load_memory_jpg(buffer, img_size, NULL);

                    if (alpha == NULL)
                    {
                        destroy();
                        return NULL;
                    }
                    for (int y = 0 ; y < tex->h; ++y)
                    {
                        for (int x = 0; x < tex->w ; ++x)
                        {
                            int c = getpixel( tex, x, y );
                            putpixel( tex, x, y, makeacol( getr(c), getg(c), getb(c), alpha->line[y][x<<2]));
                        }
                    }

                    destroy_bitmap( alpha );
                    delete[] buffer;
                }
                else
                {
                    if (tex == NULL)
                    {
                        destroy();
                        return NULL;
                    }
                    for (int y = 0; y < tex->h ; ++y)
                    {
                        for (int x = 0 ; x < tex->w ; ++x)
                        {
                            int c = getpixel(tex, x, y);
                            putpixel(tex, x, y, makeacol( getr(c), getg(c), getb(c), 0xFF));
                        }
                    }
                }
            }

            allegro_gl_use_alpha_channel(true);
            allegro_gl_set_texture_format(GL_RGBA8);
            surface.gltex[i]=allegro_gl_make_texture(tex);
            allegro_gl_use_alpha_channel(false);
            glBindTexture(GL_TEXTURE_2D,surface.gltex[i]);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

            destroy_bitmap(tex);
        }

        if (surface.Flag & SURFACE_GLSL) // Fragment & Vertex shaders
        {
            uint32 shader_size;
            data = read_from_mem(&shader_size,4,data);
            char *buf = new char[shader_size+1];
            buf[shader_size]=0;
            data=read_from_mem(buf,shader_size,data);
            surface.vert_shader_src = buf;
            delete[] buf;

            data=read_from_mem(&shader_size,4,data);
            buf = new char[shader_size+1];
            buf[shader_size]=0;
            data = read_from_mem(buf,shader_size,data);
            surface.frag_shader_src = buf;
            delete[] buf;
            surface.s_shader.load_memory(surface.frag_shader_src.c_str(),surface.frag_shader_src.size(),surface.vert_shader_src.c_str(),surface.vert_shader_src.size());
        }

        N = new Vector3D[nb_vtx << 1]; // Calculate normals
        if (nb_t_index>0 && t_index != NULL)
        {
            F_N = new Vector3D[nb_t_index / 3];
            for (int i = 0; i < nb_vtx; ++i)
                N[i].x=N[i].z=N[i].y=0.0f;
            int e = 0;
            for (int i=0;i<nb_t_index;i+=3)
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

        byte link;
        data=read_from_mem(&link,1,data);

        if (link == 2) // Load animation data if present
        {
            animation_data = new ANIMATION;
            data = read_from_mem( &(animation_data->type), 1, data );
            data = read_from_mem( &(animation_data->angle_0), sizeof(Vector3D), data);
            data = read_from_mem( &(animation_data->angle_1), sizeof(Vector3D), data);
            data = read_from_mem( &(animation_data->angle_w), sizeof(float), data );
            data = read_from_mem( &(animation_data->translate_0), sizeof(Vector3D), data);
            data = read_from_mem( &(animation_data->translate_1), sizeof(Vector3D), data);
            data = read_from_mem( &(animation_data->translate_w), sizeof(float), data);

            data=read_from_mem(&link,1,data);
        }

        if (link)
        {
            child = new OBJECT;
            data=child->load_3dm(data);
            if (data == NULL)
            {
                destroy();
                return NULL;
            }
        }
        else
            child=NULL;

        data=read_from_mem(&link,1,data);
        if (link)
        {
            next = new OBJECT;
            data=next->load_3dm(data);
            if (data == NULL)
            {
                destroy();
                return NULL;
            }
        }
        else
            next=NULL;
        return data;
    }

    DRAWING_TABLE::~DRAWING_TABLE()
    {
        for ( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for (std::list< RENDER_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                delete *e;
        hash_table.clear();
    }

    void DRAWING_TABLE::queue_instance( uint32 &model_id, INSTANCE instance )
    {
        uint32	hash = model_id & DRAWING_TABLE_MASK;
        for (std::list< RENDER_QUEUE* >::iterator i = hash_table[ hash ].begin() ; i != hash_table[ hash ].end() ; i++ )
            if ((*i)->model_id == model_id ) {		// We found an already existing render queue
                (*i)->queue.push_back( instance );
                return;
            }
        RENDER_QUEUE *render_queue = new RENDER_QUEUE( model_id );
        hash_table[ hash ].push_back( render_queue );
        render_queue->queue.push_back( instance );
    }

    void DRAWING_TABLE::draw_all()
    {
        for ( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for (std::list< RENDER_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                (*e)->draw_queue();
    }

    void RENDER_QUEUE::draw_queue()
    {
        if (queue.size() == 0 )	return;
        glPushMatrix();

        if (model_manager.model[ model_id ].from_2d )
            glEnable(GL_ALPHA_TEST);

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        if (!model_manager.model[ model_id ].dlist )// Build the display list if necessary
        {
            model_manager.model[ model_id ].dlist = glGenLists (1);
            glNewList (model_manager.model[ model_id ].dlist, GL_COMPILE);
            model_manager.model[ model_id ].obj.draw_dl(NULL,false,0,true);
            glEndList();
        }

        for (std::list< INSTANCE >::iterator i = queue.begin() ; i != queue.end() ; ++i)
        {
            glPopMatrix();
            glPushMatrix();
            glTranslatef( i->pos.x, i->pos.y, i->pos.z );
            glRotatef( i->angle, 0.0f, 1.0f, 0.0f );
            glColor4ubv( (GLubyte*) &i->col );
            glCallList( model_manager.model[ model_id ].dlist );
        }

        if (model_manager.model[ model_id ].from_2d )
            glDisable(GL_ALPHA_TEST);

        glPopMatrix();
    }

    QUAD_TABLE::~QUAD_TABLE()
    {
        for (uint16 i = 0; i < DRAWING_TABLE_SIZE; ++i)
        {
            for (std::list< QUAD_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; ++e)
                delete *e;
        }
        hash_table.clear();
    }

    void QUAD_TABLE::queue_quad(GLuint& texture_id, QUAD quad)
    {
        uint32	hash = texture_id & DRAWING_TABLE_MASK;
        for (std::list< QUAD_QUEUE* >::iterator i = hash_table[ hash ].begin() ; i != hash_table[ hash ].end() ; ++i)
        {
            if ((*i)->texture_id == texture_id ) // We found an already existing render queue
            {
                (*i)->queue.push_back( quad );
                return;
            }
        }
        QUAD_QUEUE *quad_queue = new QUAD_QUEUE(texture_id);
        hash_table[ hash ].push_back(quad_queue);
        quad_queue->queue.push_back(quad);
    }

    void QUAD_TABLE::draw_all()
    {
        uint32	max_size = 0;
        for (uint16 i = 0; i < DRAWING_TABLE_SIZE ; ++i)
        {
            for (std::list< QUAD_QUEUE* >::iterator e = hash_table[i].begin(); e != hash_table[i].end(); ++e)
                max_size = Math::Max( max_size, (uint32)(*e)->queue.size());
        }

        Vector3D	*P = new Vector3D[ max_size << 2 ];
        uint32	*C = new uint32[ max_size << 2 ];
        GLfloat	*T = new GLfloat[ max_size << 3 ];

        int e = 0;
        for (unsigned int i = 0 ; i < max_size; ++i)
        {
            T[e << 1] = 0.0f;  T[(e<<1)+1] = 0.0f;
            ++e;

            T[e << 1] = 1.0f;  T[(e<<1)+1] = 0.0f;
            ++e;

            T[e << 1] = 1.0f;  T[(e<<1)+1] = 1.0f;
            ++e;

            T[e << 1] = 0.0f;  T[(e<<1)+1] = 1.0f;
            ++e;
        }

        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable( GL_TEXTURE_2D );
        glColorPointer(4,GL_UNSIGNED_BYTE,0,C);
        glVertexPointer( 3, GL_FLOAT, 0, P);
        glTexCoordPointer(2, GL_FLOAT, 0, T);

        for (uint16 i = 0; i < DRAWING_TABLE_SIZE; ++i)
        {
            for (std::list< QUAD_QUEUE* >::iterator e = hash_table[i].begin(); e != hash_table[i].end(); ++e)
                (*e)->draw_queue(P, C, T);
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        delete[] P;
        delete[] C;
        delete[] T;
    }

    void QUAD_QUEUE::draw_queue( Vector3D *P, uint32 *C, GLfloat	*T )
    {
        if (queue.empty())
            return;
        glPushMatrix();

        int i = 0;
        for (std::list<QUAD>::iterator e = queue.begin(); e != queue.end(); ++e)
        {
            P[i].x = e->pos.x - e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z - e->size_z;
            C[i] = e->col;
            ++i;

            P[i].x = e->pos.x + e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z - e->size_z;
            C[i] = e->col;
            ++i;

            P[i].x = e->pos.x + e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z + e->size_z;
            C[i] = e->col;
            ++i;

            P[i].x = e->pos.x - e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z + e->size_z;
            C[i] = e->col;
            ++i;
        }
        glBindTexture( GL_TEXTURE_2D, texture_id );
        glDrawArrays(GL_QUADS, 0, queue.size()<<2);		// draw those quads
        glPopMatrix();
    }


    } // namespace TA3D

