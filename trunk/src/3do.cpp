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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"
#include "gfx/particles/particles.h"
#include "taconfig.h"
#include "ingame/sidedata.h"

using namespace TA3D::Exceptions;

TEXTURE_MANAGER	texture_manager;
MODEL_MANAGER	model_manager;

int TEXTURE_MANAGER::load_gaf(byte *data)
{
    int nb_entry=get_gaf_nb_entry(data);
    int n_nbtex=nbtex+nb_entry;
    int i;
    ANIM *n_tex=(ANIM*) malloc(sizeof(ANIM)*n_nbtex);
    for( i = 0 ; i < n_nbtex ; i++ )
        n_tex[i].init();
    for(i=0;i<nbtex;i++)
        n_tex[i]=tex[i];
    if(tex)
        free(tex);
    tex=n_tex;
    for(i=0;i<nb_entry;i++)
        tex[nbtex+i].load_gaf(data,i,false);
    nbtex+=nb_entry;
    return 0;
}

const void SCRIPT_DATA::move(const float dt,const float g)
{
    if(!is_moving)	return;
    is_moving=false;
    if( explode_time > 0.0f )
        explode_time -= dt;
    explode = explode_time > 0.0f;
    for(uint16 e=0;e<nb_piece;e++)
        if(flag[e]&FLAG_EXPLODE)// && (explosion_flag[e]&EXPLODE_BITMAPONLY)!=EXPLODE_BITMAPONLY)		// This piece is exploding
            for(byte i=0;i<3;i++) {
                if(i==1 && explosion_flag[e]&EXPLODE_FALL)
                    axe[i][e].move_speed-=g;
                axe[i][e].pos+=axe[i][e].move_speed*dt;
                axe[i][e].angle+=axe[i][e].rot_speed*dt;
                is_moving=true;
            }
        else
            for(byte i=0;i<3;i++) {
                if(!axe[i][e].is_moving)	continue;
                axe[i][e].is_moving=false;
                float a=axe[i][e].move_distance;
                if(a!=0.0f) {
                    axe[i][e].is_moving=true;
                    is_moving=true;
                    float c=axe[i][e].move_speed*dt;
                    axe[i][e].move_distance-=c;
                    axe[i][e].pos+=c;
                    if((a>0.0f && axe[i][e].move_distance<0.0f) || (a<0.0f && axe[i][e].move_distance>0.0f)) {
                        axe[i][e].pos+=axe[i][e].move_distance;
                        axe[i][e].move_distance=0.0f;
                    }
                }
                while(axe[i][e].angle>180.0f)	axe[i][e].angle-=360.0f;		// Maintient l'angle dans les limites
                while(axe[i][e].angle<-180.0f)	axe[i][e].angle+=360.0f;
                a=axe[i][e].rot_angle;
                if((axe[i][e].rot_speed!=0.0f || axe[i][e].rot_accel!=0.0f) && ((a!=0.0f && axe[i][e].rot_limit) || !axe[i][e].rot_limit)) {
                    axe[i][e].is_moving=true;
                    is_moving=true;
                    float b=axe[i][e].rot_speed;
                    if(b<-7200.0f)
                        b=axe[i][e].rot_speed=-7200.0f;
                    else if(b>7200.0f)
                        b=axe[i][e].rot_speed=7200.0f;
                    axe[i][e].rot_speed+=axe[i][e].rot_accel*dt;
                    if(axe[i][e].rot_speed_limit)
                        if((b <= axe[i][e].rot_target_speed && axe[i][e].rot_speed >= axe[i][e].rot_target_speed)
                           || (b >= axe[i][e].rot_target_speed && axe[i][e].rot_speed <= axe[i][e].rot_target_speed)) {
                            axe[i][e].rot_accel=0.0f;
                            axe[i][e].rot_speed=axe[i][e].rot_target_speed;
                            axe[i][e].rot_speed_limit=false;
                        }
                    float c=axe[i][e].rot_speed*dt;
                    axe[i][e].angle+=c;
                    if(axe[i][e].rot_limit) {
                        axe[i][e].rot_angle-=c;
                        if((a>=0.0f && axe[i][e].rot_angle<=0.0f) || (a<=0.0f && axe[i][e].rot_angle>=0.0f)) {
                            axe[i][e].angle+=axe[i][e].rot_angle;
                            axe[i][e].rot_angle=0.0f;
                            axe[i][e].rot_speed=0.0f;
                            axe[i][e].rot_accel=0.0f;
                        }
                    }
                }
            }
}

void OBJECT::optimise_mesh()			// EXPERIMENTAL, function to merge all objects in one vertex array (assume the object is clean)
{
    return;				// Currently it's experimental, so don't waste time
    if( use_strips )	return;		// Can't optimise that!!
    int total_index = nb_t_index;			// Count the number of vertices and indexes to allocate the required space
    int total_vtx = nb_vtx;

    List< OBJECT* >	obj_stack;
    if( next )	obj_stack.push_front( next );
    if( child )	obj_stack.push_front( child );

    while( !obj_stack.empty() ) {
        OBJECT *cur = obj_stack.front();	obj_stack.pop_front();

        total_index += cur->nb_t_index;
        total_vtx += cur->nb_vtx;

        if( cur->next )		obj_stack.push_front( cur->next );
        if( cur->child )	obj_stack.push_front( cur->child );
    }

    VECTOR	*opt_vtx = (VECTOR*) malloc( sizeof( VECTOR ) * total_vtx );
    VECTOR	*opt_N = (VECTOR*) malloc( sizeof( VECTOR ) * total_vtx );
    float	*opt_T = (float*) malloc( sizeof( float ) * total_vtx << 1 );
    GLushort *opt_idx = (GLushort*) malloc( sizeof( GLushort ) * total_index );
    total_vtx = 0;
    total_index = 0;
    List< VECTOR >	pos_stack;
    VECTOR pos_offset;

    obj_stack.push_front( this );			// Fill the arrays
    pos_stack.push_front( pos_offset );

    while( !obj_stack.empty() ) {
        OBJECT *cur = obj_stack.front();	obj_stack.pop_front();

        pos_offset = pos_stack.front();		pos_stack.pop_front();
        VECTOR dec = pos_offset + cur->pos_from_parent;

        for( int i = 0 ; i < cur->nb_t_index ; i++ )
            opt_idx[ i + total_index ] = cur->t_index[ i ] + total_vtx;
        total_index += cur->nb_t_index;

        for( int i = 0 ; i < cur->nb_vtx ; i++ ) {
            if( cur->tcoord ) {
                opt_T[ (i + total_vtx << 1) ] = cur->tcoord[ (i << 1) ];
                opt_T[ (i + total_vtx << 1) + 1 ] = cur->tcoord[ (i << 1) + 1 ];
            }
            if( cur->points )
                opt_vtx[ i + total_vtx ] = cur->points[ i ] + dec;
            if( cur->N )
                opt_N[ i + total_vtx ] = cur->N[ i ];
        }
        total_vtx += cur->nb_vtx;

        if( cur->next )		{	obj_stack.push_front( cur->next );	pos_stack.push_front( pos_offset );	}
        if( cur->child )	{	obj_stack.push_front( cur->child );		pos_stack.push_front( dec );	}
    }

    glGenBuffersARB( 1, &vbo_id );
    glGenBuffersARB( 1, &ebo_id );

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, total_vtx * ( sizeof( VECTOR ) + sizeof( VECTOR ) + sizeof( float ) * 2 ), NULL, GL_STATIC_DRAW_ARB);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, total_vtx * sizeof( VECTOR ), opt_vtx );
    N_offset = total_vtx * sizeof( VECTOR );
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, N_offset, total_vtx * sizeof( VECTOR ), opt_N );
    T_offset = N_offset + total_vtx * sizeof( VECTOR );
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

    nb_vtx=header.NumberOfVertexes;
    nb_prim=header.NumberOfPrimitives;
    name=strdup((char*)(data+header.OffsetToObjectName));
    int i;
#ifdef DEBUG_MODE
    /*		for(i=0;i<dec;i++)
            printf("  ");
            printf("%s",name);
            for(i=0;i<20-2*dec-strlen(name);i++)
            printf(" ");
            printf("-> nb_vtx=%d | nb_prim=%d\n",nb_vtx,nb_prim);*/
#endif
    if(header.OffsetToChildObject) {					// Charge récursivement les différents objets du modèle
        child=(OBJECT*) malloc(sizeof(OBJECT));
        child->init();
        child->load_obj(data,header.OffsetToChildObject,dec+1,filename);
    }
    if(header.OffsetToSiblingObject) {					// Charge récursivement les différents objets du modèle
        next=(OBJECT*) malloc(sizeof(OBJECT));
        next->init();
        next->load_obj(data,header.OffsetToSiblingObject,dec,filename);
    }
    points=(VECTOR*) malloc(sizeof(VECTOR)*nb_vtx);		// Alloue la mémoire nécessaire pour stocker les points
    int f_pos;
    float div=0.5f/65536.0f;
    pos_from_parent.x=header.XFromParent*div;
    pos_from_parent.y=header.YFromParent*div;
    pos_from_parent.z=-header.ZFromParent*div;
    f_pos=header.OffsetToVertexArray;
    for(i=0;i<nb_vtx;i++) {				// Lit le tableau de points stocké dans le fichier
        tagVertex vertex;
        vertex.x=*((int*)(data+f_pos));		f_pos+=4;
        vertex.y=*((int*)(data+f_pos));		f_pos+=4;
        vertex.z=*((int*)(data+f_pos));		f_pos+=4;
        points[i].x=vertex.x*div;
        points[i].y=vertex.y*div;
        points[i].z=-vertex.z*div;
    }
    f_pos = header.OffsetToPrimitiveArray;
    int n_index=0;
    selprim = -1;//header.OffsetToselectionPrimitive;
    sel[ 0 ] = sel[ 1 ] = sel[ 2 ] = sel[ 3 ] = 0;
    for(i=0;i<nb_prim;i++) {					// Compte le nombre de primitive de chaque sorte
        tagPrimitive primitive;
        primitive.ColorIndex=*((int*)(data+f_pos));						f_pos+=4;
        primitive.NumberOfVertexIndexes=*((int*)(data+f_pos));			f_pos+=4;
        primitive.Always_0=*((int*)(data+f_pos));						f_pos+=4;
        primitive.OffsetToVertexIndexArray=*((int*)(data+f_pos));		f_pos+=4;
        primitive.OffsetToTextureName=*((int*)(data+f_pos));			f_pos+=4;
        primitive.Unknown_1=*((int*)(data+f_pos));						f_pos+=4;
        primitive.Unknown_2=*((int*)(data+f_pos));						f_pos+=4;
        primitive.IsColored=*((int*)(data+f_pos));						f_pos+=4;

        switch(primitive.NumberOfVertexIndexes)
        {
            case 1:		nb_p_index++;	break;
            case 2:		nb_l_index+=2;	break;
            default:
                        if( i == header.OffsetToselectionPrimitive ) {
                            selprim = 1;//nb_t_index;
                            break;
                        }
                        else {
                            if( primitive.IsColored && primitive.ColorIndex == 1 )	break;
                            if( !primitive.IsColored && ( !primitive.OffsetToTextureName || !data[primitive.OffsetToTextureName] ) )	break;
                        }
                        n_index+=primitive.NumberOfVertexIndexes;
                        nb_t_index++;
        };
    }
#ifdef DEBUG_MODE
    //		printf("(%d,%d,%d)\n",nb_p_index,nb_l_index,nb_t_index);
#endif

    if(nb_p_index>0)				// Alloue la mémoire nécessaire pour stocker les primitives
        p_index=(GLushort*) malloc(sizeof(GLushort)*nb_p_index);
    if(nb_l_index>0)
        l_index=(GLushort*) malloc(sizeof(GLushort)*nb_l_index);
    if(nb_t_index>0) {
        tex=(int*) malloc(sizeof(int)*nb_t_index);
        usetex=(byte*) malloc(sizeof(byte)*nb_t_index);
        nb_index=(short*) malloc(sizeof(short)*nb_t_index);
        t_index=(GLushort*) malloc(sizeof(GLushort)*n_index);
    }

    f_pos=header.OffsetToPrimitiveArray;
    int pos_p=0,pos_l=0,pos_t=0,cur=0;
    int nb_diff_tex=0;
    int *index_tex = new int[nb_prim];
    int t_m=0;
    for(i=0;i<nb_prim;i++) {					// Compte le nombre de primitive de chaque sorte
        tagPrimitive primitive;
        primitive.ColorIndex=*((int*)(data+f_pos));						f_pos+=4;
        primitive.NumberOfVertexIndexes=*((int*)(data+f_pos));			f_pos+=4;
        primitive.Always_0=*((int*)(data+f_pos));						f_pos+=4;
        primitive.OffsetToVertexIndexArray=*((int*)(data+f_pos));		f_pos+=4;
        primitive.OffsetToTextureName=*((int*)(data+f_pos));			f_pos+=4;
        primitive.Unknown_1=*((int*)(data+f_pos));						f_pos+=4;
        primitive.Unknown_2=*((int*)(data+f_pos));						f_pos+=4;
        primitive.IsColored=*((int*)(data+f_pos));						f_pos+=4;

        switch(primitive.NumberOfVertexIndexes)
        {
            case 1:
                p_index[pos_p++]=*((short*)(data+primitive.OffsetToVertexIndexArray));
                break;
            case 2:
                l_index[pos_l++]=*((short*)(data+primitive.OffsetToVertexIndexArray));
                l_index[pos_l++]=*((short*)(data+primitive.OffsetToVertexIndexArray+2));
                break;
            default:
                if( i != header.OffsetToselectionPrimitive ) {
                    if( primitive.IsColored && primitive.ColorIndex == 1 )	break;
                    if( !primitive.IsColored && ( !primitive.OffsetToTextureName || !data[primitive.OffsetToTextureName] ) )	break;
                }
                else {
                    for( int e = 0 ; e < primitive.NumberOfVertexIndexes && e < 4 ; e++ )
                        sel[ e ] = *((short*)(data+primitive.OffsetToVertexIndexArray+(e<<1)));
                    break;
                }
                nb_index[cur] = primitive.NumberOfVertexIndexes;
                tex[cur] = t_m = texture_manager.get_texture_index((char*)(data+primitive.OffsetToTextureName));
                usetex[cur]=1;
                if(t_m==-1) {
                    if(primitive.ColorIndex>=0 && primitive.ColorIndex<256) {
                        usetex[cur]=1;
                        tex[cur]=t_m=primitive.ColorIndex;
                    }
                    else
                        usetex[cur]=0;
                }
                if(t_m>=0)	{														// Code pour la création d'une texture propre à chaque modèle
                    bool al_in=false;
                    int indx=t_m;
                    for(int e=0;e<nb_diff_tex;e++)
                        if(index_tex[e]==indx) {
                            al_in=true;
                            break;
                        }
                    if(!al_in)
                        index_tex[nb_diff_tex++]=indx;
                }
                for(int e=0;e<nb_index[cur];e++)
                    t_index[pos_t++]=*((short*)(data+primitive.OffsetToVertexIndexArray+(e<<1)));
                cur++;
        };
    }
    /*------------------------------Création de la texture unique pour l'unité--------------*/
    int *px = new int[nb_diff_tex],*py = new int[nb_diff_tex];			// Pour placer les différentes mini-textures sur une grande texture
    int mx=0,my=0;
    for(i=0;i<nb_diff_tex;i++) {
        int dx=texture_manager.tex[index_tex[i]].bmp[0]->w,dy=texture_manager.tex[index_tex[i]].bmp[0]->h;
        px[i]=py[i]=0;
        if(i!=0)
            for(int e=0;e<i;e++) {
                int fx=texture_manager.tex[index_tex[e]].bmp[0]->w,fy=texture_manager.tex[index_tex[e]].bmp[0]->h;
                bool found[3];
                found[0]=found[1]=found[2]=true;
                int j;

                px[i]=px[e]+fx;		py[i]=py[e];
                for(j=0;j<i;j++) {
                    int gx=texture_manager.tex[index_tex[j]].bmp[0]->w,gy=texture_manager.tex[index_tex[j]].bmp[0]->h;
                    if(coupe(px[i],py[i],dx,dy,px[j],py[j],gx,gy)) {
                        found[0]=false;
                        break;
                    }
                }

                px[i]=px[e];		py[i]=py[e]+fy;
                for(j=0;j<i;j++) {
                    int gx=texture_manager.tex[index_tex[j]].bmp[0]->w,gy=texture_manager.tex[index_tex[j]].bmp[0]->h;
                    if(coupe(px[i],py[i],dx,dy,px[j],py[j],gx,gy)) {
                        found[2]=false;
                        break;
                    }
                }
                px[i]=px[e]+fx;		py[i]=0;
                for(j=0;j<i;j++) {
                    int gx=texture_manager.tex[index_tex[j]].bmp[0]->w,gy=texture_manager.tex[index_tex[j]].bmp[0]->h;
                    if(coupe(px[i],py[i],dx,dy,px[j],py[j],gx,gy)) {
                        found[1]=false;
                        break;
                    }
                }
                bool deborde=false;
                bool found_one=false;
                int deb=0;
                if(found[1]) {
                    px[i]=px[e]+fx;
                    py[i]=0;
                    deborde=false;
                    if(px[i]+dx>mx || py[i]+dy>my) deborde=true;
                    deb=max(mx,px[i]+dx)*max(py[i]+dy,my)-mx*my;
                    found_one=true;
                }
                if(found[0] && (!found_one || deborde)) {
                    px[i]=px[e]+fx;
                    py[i]=py[e];
                    deborde=false;
                    if(px[i]+dx>mx || py[i]+dy>my) deborde=true;
                    deb=max(mx,px[i]+dx)*max(py[i]+dy,my)-mx*my;
                    found_one=true;
                }
                if(found[2] && deborde) {
                    int ax=px[i],ay=py[i];
                    px[i]=px[e];
                    py[i]=py[e]+fy;
                    deborde=false;
                    if(px[i]+dx>mx || py[i]+dy>my) deborde=true;
                    int deb2=max(mx,px[i]+dx)*max(py[i]+dy,my)-mx*my;
                    if(found_one && deb<deb2) {
                        px[i]=ax;	py[i]=ay;
                    }
                    else
                        found_one=true;
                }
                if(found_one)			// On a trouvé une position qui convient
                    break;
            }
        if(px[i]+dx>mx) mx=px[i]+dx;
        if(py[i]+dy>my) my=py[i]+dy;
    }
    BITMAP *bmp = create_bitmap_ex(32,mx,my);
    if(bmp!=NULL && mx!=0 && my!=0) {
        if(g_useTextureCompression)
            allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
        else
            allegro_gl_set_texture_format(GL_RGB8);
        clear(bmp);
        for(int e=0;e<expected_players;e++) {
            bool mtex_needed=false;
            for(i=0;i<nb_diff_tex;i++)
                if(texture_manager.tex[index_tex[i]].nb_bmp==10) {
                    blit(texture_manager.tex[index_tex[i]].bmp[player_color_map[e]],bmp,0,0,px[i],py[i],texture_manager.tex[index_tex[i]].bmp[player_color_map[e]]->w,texture_manager.tex[index_tex[i]].bmp[player_color_map[e]]->h);
                    mtex_needed=true;
                }
                else
                    blit(texture_manager.tex[index_tex[i]].bmp[0],bmp,0,0,px[i],py[i],texture_manager.tex[index_tex[i]].bmp[0]->w,texture_manager.tex[index_tex[i]].bmp[0]->h);
            dtex=e+1;
            String cache_filename = filename ? String( filename ) + format("-%s-%d.bin", name ? name : "none", player_color_map[e] ) : String( "" );
            gltex[e] = gfx->load_texture_from_cache( cache_filename );
            if( !gltex[ e ] ) {
                gltex[e] = allegro_gl_make_texture(bmp);
                glBindTexture(GL_TEXTURE_2D,gltex[e]);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
                if( filename )
                    gfx->save_texture_to_cache( cache_filename, gltex[ e ], bmp->w, bmp->h );
            }
            if(!mtex_needed)	break;
        }
    }
    else
        dtex=0;
    if(bmp)
        destroy_bitmap(bmp);
    int nb_total_point=0;
    for(i=0;i<nb_t_index;i++)
        nb_total_point+=nb_index[i];
    nb_total_point+=nb_l_index;
    if( selprim >= 0 )
        nb_total_point += 4;
    VECTOR *p=(VECTOR*) malloc(sizeof(VECTOR)*nb_total_point<<1);			// *2 pour le volume d'ombre
    int prim_dec = selprim >= 0 ? 4 : 0;
    for(i=0;i<nb_total_point-nb_l_index-prim_dec;i++) {
        p[i+nb_total_point]=p[i]=points[t_index[i]];
        t_index[i] = i;
    }
    if( selprim >= 0 ) {
        p[nb_total_point-nb_l_index-prim_dec] = points[ sel[ 0 ] ];			sel[0] = nb_total_point-nb_l_index-prim_dec;
        p[nb_total_point-nb_l_index-prim_dec+1] = points[ sel[ 1 ] ];		sel[1] = nb_total_point-nb_l_index-prim_dec+1;
        p[nb_total_point-nb_l_index-prim_dec+2] = points[ sel[ 2 ] ];		sel[2] = nb_total_point-nb_l_index-prim_dec+2;
        p[nb_total_point-nb_l_index-prim_dec+3] = points[ sel[ 3 ] ];		sel[3] = nb_total_point-nb_l_index-prim_dec+3;
    }
    for(i=nb_total_point-nb_l_index;i<nb_total_point;i++) {
        int e=i-nb_total_point+nb_l_index;
        p[i+nb_total_point]=p[i]=points[l_index[e]];
        l_index[e]=i;
    }
    if(nb_l_index==2)
        if(p[l_index[0]].x<0.0f) {
            int tmp=l_index[0];
            l_index[0]=l_index[1];
            l_index[1]=tmp;
        }
    free(points);
    points=p;
    nb_vtx=nb_total_point;

    int nb_triangle=0;
    for(i=0;i<nb_t_index;i++)
        nb_triangle+=nb_index[i]-2;
    GLushort *index=(GLushort*) malloc(sizeof(GLushort)*nb_triangle*3);
    tcoord=(float*) malloc(sizeof(float)*nb_vtx<<1);
    cur=0;
    int curt=0;
    pos_t=0;
    for(i=0;i<nb_t_index;i++) {
        int indx=0;
        for(int f=0;f<nb_diff_tex;f++)
            if(tex[i]==index_tex[f]) {
                indx=f;
                break;
            }
        for(int e=0;e<nb_index[i];e++) {
            if(e<3)
                index[pos_t++]=t_index[cur];
            else {
                index[pos_t]=index[pos_t-3];	pos_t++;
                index[pos_t]=index[pos_t-2];	pos_t++;
                index[pos_t++]=t_index[cur];
            }
            tcoord[curt]=0.0f;
            tcoord[curt+1]=0.0f;
            if(usetex[i]) {
                switch(e&3)
                {
                    case 1:
                        tcoord[curt]+=((float)texture_manager.tex[tex[i]].bmp[0]->w-1)/(mx-1);
                        break;
                    case 2:
                        tcoord[curt]+=((float)texture_manager.tex[tex[i]].bmp[0]->w-1)/(mx-1);
                        tcoord[curt+1]+=((float)texture_manager.tex[tex[i]].bmp[0]->h-1)/(my-1);
                        break;
                    case 3:
                        tcoord[curt+1]+=((float)texture_manager.tex[tex[i]].bmp[0]->h-1)/(my-1);
                        break;
                };
                tcoord[curt]+=((float)px[indx]+0.5f)/(mx-1);
                tcoord[curt+1]+=((float)py[indx]+0.5f)/(my-1);
            }
            cur++;
            curt+=2;
        }
    }
    for(cur=0;cur<pos_t;cur+=3) {			// Petite inversion pour avoir un affichage correct
        GLushort t=index[cur+1];
        index[cur+1]=index[cur+2];
        index[cur+2]=t;
    }
    nb_t_index=nb_triangle*3;
    free(t_index);
    t_index=index;
    free(usetex);
    usetex=NULL;
    /*--------------------------------------------------------------------------------------*/

    if(nb_t_index>0) {			// Calcule les normales pour l'éclairage
        N=(VECTOR*) malloc(sizeof(VECTOR)*nb_vtx<<1);
        F_N = new VECTOR[ nb_t_index / 3 ];
        for(i=0;i<nb_vtx<<1;i++)
            N[i].x=N[i].z=N[i].y=0.0f;
        int e = 0;
        for(i=0;i<nb_t_index;i+=3) {
            VECTOR AB,AC,Normal;
            AB=points[t_index[i+1]] - points[t_index[i]];
            AC=points[t_index[i+2]] - points[t_index[i]];
            Normal=AB*AC;	Normal.unit();
            F_N[ e++ ] = Normal;
            for(int e=0;e<3;e++)
                N[t_index[i+e]]=N[t_index[i+e]]+Normal;
        }
        for(i=0;i<nb_vtx;i++)
            N[i].unit();
        for(i = nb_vtx; i < (nb_vtx<<1) ; i++)
            N[i] = N[i-nb_vtx];
    }
    delete[] px;
    delete[] py;
    delete[] index_tex;
    return 0;
}

void OBJECT::create_from_2d(BITMAP *bmp,float w,float h,float max_h)
{
    destroy();					// Au cas où l'objet ne serait pas vierge

    pos_from_parent.x=0.0f;
    pos_from_parent.y=0.0f;
    pos_from_parent.z=0.0f;
    selprim=-1;
    child=NULL;
    next=NULL;
    nb_l_index=0;
    nb_p_index=0;
    l_index=NULL;
    p_index=NULL;

    use_strips = true;

    nb_vtx=64;
    nb_t_index=119;
    points=(VECTOR*) malloc(sizeof(VECTOR)*nb_vtx);
    tcoord=(float*) malloc(sizeof(float)*nb_vtx<<1);
    t_index=(GLushort*) malloc(sizeof(GLushort)*nb_t_index);
    if(points==NULL || tcoord==NULL || t_index==NULL) {
        printf("ERROR: not enough memory!!\n");
        exit(1);
    }

    uint16	i;
    uint8	x,y;

    float ww = w * 0.1333333333333f;
    float hh = h * 0.1333333333333f;
    for(y=0;y<8;y++) {						// Maillage (sommets)
        uint16	seg = y<<3;
        float yy = y * 0.1333333333333f;
        for(x=0;x<8;x++) {
            uint16	offset = seg+x;
            points[offset].x=(x-3.5f)*ww;
            points[offset].z=(y-3.5f)*hh;
            tcoord[offset<<1]=x*0.1333333333333f;
            tcoord[(offset<<1)+1]=yy;
        }
    }
    uint16 offset = 0;
    for(y=0;y<7;y++)						// Maillage (triangles)
        if( y & 1 ) {
            t_index[offset++]=(y<<3);
            t_index[offset++]=((y+1)<<3);
            for(x=0;x<7;x++) {
                t_index[offset++]=(y<<3)+x+1;
                t_index[offset++]=(y+1<<3)+x+1;
            }
            t_index[offset++]=(y+1<<3)+7;
        }
        else {
            t_index[offset++]=(y<<3)+7;
            t_index[offset++]=((y+1)<<3)+7;
            for(x=0;x<7;x++) {
                t_index[offset++]=(y<<3)+6-x;
                t_index[offset++]=(y+1<<3)+6-x;
            }
            t_index[offset++]=(y+1<<3);
        }

    uint32 tmp[8][8];

    uint32 med=0;
    uint32 div=0;
    for(y=0;y<8;y++)						// Carte miniature en nuances de gris
        for(x=0;x<8;x++) {
            uint32 c=0;
            uint32 n=0;
            bool zero=false;
            for(uint32 py=y*bmp->h>>3;py<(y+1)*bmp->h>>3;py++)
                for(uint32 px=x*bmp->w>>3;px<(x+1)*bmp->w>>3;px++) {
                    uint32 pc=getpixel(bmp,px,py);
                    c+=getr(pc)+getg(pc)+getb(pc);
                    if(geta(pc)<128 || (pc&0xFFFFFF)==0xFF00FF)
                        zero=true;
                    n+=3;
                }
            if(zero)	{	c=0x0;	n=1;	}
            tmp[y][x]=c/n;
            if(!zero) {
                med+=tmp[y][x];
                div++;
            }
        }
    if(div==0)	div=1;						// Il y a des trucs bizarres des fois!
    med=(med+(div>>1))/div;
    for(y=0;y<8;y++)						// Carte miniature en nuances de gris
        for(x=0;x<8;x++)
            if(tmp[y][x]==-0xFFFFFF)
                tmp[y][x]=med;

    points[0].y=0.0f;
    for(y=1;y<8;y++)			// x=0
        points[y<<3].y=0.0f;
    for(x=1;x<8;x++)			// y=0
        points[x].y=0.0f;
    for(y=1;y<8;y++)
        for(x=1;x<8;x++) {
            int d_h0=tmp[y][x-1]-med;
            int d_h1=tmp[y-1][x]-med;
            int d_h=tmp[y][x]-med;
            float l=sqrt((float)(d_h0*d_h0+d_h1*d_h1));
            float dhx,dhy;
            if(l==0.0f)
                dhx=dhy=0.0f;
            else {
                dhx=d_h*abs(d_h0)/l;
                dhy=d_h*abs(d_h1)/l;
            }
            points[(y<<3)+x].y=(points[(y<<3)+x-1].y+dhx+points[((y-1)<<3)+x].y+dhy)*0.5f;
        }

    for(y=1;y<8;y++)
        for(x=1;x<8;x++)
            points[(y<<3)+x].y -= (x / 7.0f) * points[(y<<3)+7].y;

    for(x=1;x<8;x++)
        for(y=1;y<8;y++)
            points[(y<<3)+x].y -= (y / 7.0f) * points[(7<<3)+x].y;

    float maxh=0.0f,minh=0.0f;
    for(i=0;i<64;i++) {
        if(minh>points[i].y)	minh=points[i].y;
        if(maxh<points[i].y)	maxh=points[i].y;
    }
    if(maxh==minh || maxh == 0.0f )
        for(i=0;i<64;i++)
            points[i].y=0.0f;
    else
        for(i=0;i<64;i++)
            points[i].y = points[i].y > 0.0f ? points[i].y * max_h / maxh : 0.0f;

    if(g_useTextureCompression)
        allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
    else
        allegro_gl_set_texture_format(GL_RGB5_A1);
    gltex[0]=allegro_gl_make_texture(bmp);	dtex=1;
    glBindTexture(GL_TEXTURE_2D,gltex[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

    /*--------------------------------------------------------------------------------------*/

    N=(VECTOR*) malloc(sizeof(VECTOR)*nb_vtx);
    F_N = NULL;
    for(i=0;i<nb_vtx;i++)
        N[i].x=N[i].z=N[i].y=0.0f;
    for(i=0;i<nb_t_index-2;i++) {
        VECTOR AB,AC,Normal;
        AB=points[t_index[i+1]] - points[t_index[i]];
        AC=points[t_index[i+2]] - points[t_index[i]];
        Normal=AB*AC;	Normal.unit();
        if( Normal.y < 0.0f )	Normal = -Normal;
        for(int e=0;e<3;e++)
            N[t_index[i+e]]=N[t_index[i+e]]+Normal;
    }
    for(i=0;i<nb_vtx;i++)
        N[i].unit();
}

void OBJECT::draw_optimised( bool set )
{
    if( set ) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo_id);

        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer( 3, GL_FLOAT, 0, 0 );
        glNormalPointer( GL_FLOAT, 0, (GLvoid*)N_offset );
        //		glVertexPointer( 3, GL_FLOAT, 0, optimised_P);
        //		glNormalPointer( GL_FLOAT, 0, optimised_N);
    }
    glDrawElements(GL_TRIANGLES, optimised_nb_idx,GL_UNSIGNED_SHORT,NULL);
    //	glDrawElements(GL_TRIANGLES, optimised_nb_idx,GL_UNSIGNED_SHORT,optimised_I);				// draw everything
}

bool OBJECT::draw(float t,SCRIPT_DATA *data_s,bool sel_primitive,bool alset,bool notex,int side,bool chg_col,bool exploding_parts)
{
    bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
    bool hide=false;
    bool set=false;
    float color_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glPushMatrix();
    if( explodes && !exploding_parts )
        goto draw_next;

    glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);
    if(script_index>=0 && data_s ) {
        if( animation_data != NULL && (data_s->flag[script_index] & FLAG_ANIMATE) ) {		// Used only by the 3dmeditor
            VECTOR R,T;
            animation_data->animate( t, R, T );
            glTranslatef( T.x, T.y, T.z );
            glRotatef( R.x, 1.0f, 0.0f, 0.0f );
            glRotatef( R.y, 0.0f, 1.0f, 0.0f );
            glRotatef( R.z, 0.0f, 0.0f, 1.0f );
        }
        else if( !explodes ^ exploding_parts ) {
            glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
            glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
            glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
            glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
        }
        hide = data_s->flag[script_index]&FLAG_HIDE;
    }
    else if( animation_data ) {
        VECTOR R,T;
        animation_data->animate( t, R, T );
        glTranslatef( T.x, T.y, T.z );
        glRotatef( R.x, 1.0f, 0.0f, 0.0f );
        glRotatef( R.y, 0.0f, 1.0f, 0.0f );
        glRotatef( R.z, 0.0f, 0.0f, 1.0f );
    }
    hide |= explodes ^ exploding_parts;
    if( !chg_col )
        glGetFloatv( GL_CURRENT_COLOR, color_factor );
    if( gl_dlist[ side ] && !hide && chg_col && !notex ) {
        glCallList( gl_dlist[ side ] );
        alset = false;
        set = false;
    }
    else if( !hide ) {
        bool creating_list = false;
        if( chg_col && !notex && gl_dlist[ side ] == 0 ) {
            gl_dlist[ side ] = glGenLists( 1 );
            glNewList( gl_dlist[ side ], GL_COMPILE_AND_EXECUTE);
            alset = false;
            set = false;
            creating_list = true;
        }
        if(nb_t_index>0 && nb_vtx>0 && t_index!=NULL) {
            bool activated_tex=false;
            if(surface.Flag&SURFACE_ADVANCED) {
                glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                glEnableClientState(GL_NORMAL_ARRAY);
                alset=false;
                set=false;
                if(chg_col || notex) {
                    if(surface.Flag&SURFACE_PLAYER_COLOR)
                        glColor4f(player_color[side*3],player_color[side*3+1],player_color[side*3+2],surface.Color[3]);		// Couleur de matière
                    else
                        glColor4fv(surface.Color);		// Couleur de matière
                }
                else if( !chg_col && !notex ){
                    if(surface.Flag&SURFACE_PLAYER_COLOR)
                        glColor4f(player_color[player_color_map[side]*3]*color_factor[0],player_color[player_color_map[side]*3+1]*color_factor[1],player_color[player_color_map[side]*3+2]*color_factor[2],surface.Color[3]*color_factor[3]);		// Couleur de matière
                    else
                        glColor4f(surface.Color[0]*color_factor[0],surface.Color[1]*color_factor[1],surface.Color[2]*color_factor[2],surface.Color[3]*color_factor[3]);		// Couleur de matière
                }
                
                    if(surface.Flag&SURFACE_GOURAUD)			// Type d'éclairage
                        glShadeModel (GL_SMOOTH);
                    else
                        glShadeModel (GL_FLAT);

                            if(surface.Flag&SURFACE_LIGHTED)			// Eclairage
                                glEnable(GL_LIGHTING);
                            else
                                glDisable(GL_LIGHTING);

                if(surface.Flag&SURFACE_BLENDED || (!chg_col && color_factor[3] != 1.0f ) ) {			// La transparence
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_BLEND);
                    glAlphaFunc( GL_GREATER, 0.1 );
                    glEnable( GL_ALPHA_TEST );
                }
                else {
                    glDisable( GL_ALPHA_TEST );
                    glDisable(GL_BLEND);
                }

                if( surface.Flag&SURFACE_TEXTURED && !notex ) {		// Les textures et effets de texture
                    activated_tex=true;
                    for(int j=0;j<surface.NbTex;j++) {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D,surface.gltex[j]);
                        if(j==surface.NbTex-1 && surface.Flag&SURFACE_REFLEC) {
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
                        else {
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                            glDisable(GL_TEXTURE_GEN_S);
                            glDisable(GL_TEXTURE_GEN_T);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        }
                    }
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        for(int j=0;j<surface.NbTex;j++) {
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                        }
                }
                else {
                    for(int j=7;j>=0;j--) {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            glDisable(GL_TEXTURE_2D);
                            glClientActiveTextureARB(GL_TEXTURE0_ARB + j);
                            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    }
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                }
            }
            else {
                if(!alset) {
                    glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                    glEnableClientState(GL_NORMAL_ARRAY);
                    if(notex)
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    else
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glEnable(GL_LIGHTING);
                    if(notex)
                        glDisable(GL_TEXTURE_2D);
                    else
                        glEnable(GL_TEXTURE_2D);
                    alset=true;
                }
                if( !chg_col && color_factor[3] != 1.0f ) {			// La transparence
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_BLEND);
                }
                else
                    glDisable(GL_BLEND);
                set=true;
                if(!dtex) {
                    alset=false;
                    glDisable(GL_TEXTURE_2D);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                }
                if(!notex && dtex) {
                    if(side<dtex && side>=0)
                        glBindTexture(GL_TEXTURE_2D,gltex[side]);
                    else
                        glBindTexture(GL_TEXTURE_2D,gltex[0]);
                    glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
                }
            }
            glVertexPointer( 3, GL_FLOAT, 0, points);
            glNormalPointer( GL_FLOAT, 0, N);
            if( !use_strips )
                glDrawElements(GL_TRIANGLES, nb_t_index,GL_UNSIGNED_SHORT,t_index);				// draw everything
            else {
                glDisable( GL_CULL_FACE );
                glDrawElements(GL_TRIANGLE_STRIP, nb_t_index,GL_UNSIGNED_SHORT,t_index);		// draw everything
                glEnable( GL_CULL_FACE );
            }

            if((surface.Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD))==SURFACE_ADVANCED)
                glShadeModel (GL_SMOOTH);

                    if(activated_tex) {
                        for(int j=0;j<surface.NbTex;j++) {
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
        if( creating_list )
            glEndList();
    }
#ifdef DEBUG_MODE_3DO
    if(nb_l_index>0 && nb_vtx>0) {
        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        alset=false;
        if(!set)
            glVertexPointer( 3, GL_FLOAT, 0, points);
        set=true;
        glDrawElements(GL_LINES, nb_l_index,GL_UNSIGNED_SHORT,l_index);		// dessine le tout
    }
#endif
    if(sel_primitive && selprim>=0 && nb_vtx>0 ) {// && (data_s==NULL || (data_s!=NULL && !data_s->explode))) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        if(!set)
            glVertexPointer( 3, GL_FLOAT, 0, points);
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f,1.0f,0.0f);
        glTranslatef( 0.0f, 2.0f, 0.0f );
            glDrawElements(GL_QUADS, 4,GL_UNSIGNED_SHORT,sel);		// dessine la primitive de sélection
        glTranslatef( 0.0f, -2.0f, 0.0f );
            if(notex) {
                float var=fabs(1.0f-(msec_timer%1000)*0.002f);
                glColor3f(0.0f,var,0.0f);
            }
            else
                glColor3f(1.0f,1.0f,1.0f);
                    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_CULL_FACE);
        alset=false;
    }
    if( !chg_col )
        glColor4fv( color_factor );
    if(child && !(explodes && !exploding_parts) ) {
        glPushMatrix();
        alset=child->draw(t,data_s,sel_primitive,alset,notex,side,chg_col, exploding_parts && !explodes );
        glPopMatrix();
    }
draw_next:
    if(next) {
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
        bool hide=false;
        if(script_index>=0 && data_s) {
            glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
            glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
            glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
            glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
            hide=data_s->flag[script_index]&FLAG_HIDE;
        }
        bool set=false;
        if(nb_t_index>0 && nb_vtx>0 && !hide && t_index!=NULL) {
            bool activated_tex=false;
            if(surface.Flag&SURFACE_ADVANCED) {
                alset=false;
                set=false;
                if(chg_col) {
                    if(surface.Flag&SURFACE_PLAYER_COLOR)
                        glColor4f(player_color[player_color_map[side]*3],player_color[player_color_map[side]*3+1],player_color[player_color_map[side]*3+2],surface.Color[3]);		// Couleur de matière
                    else
                        glColor4f(surface.Color[0],surface.Color[1],surface.Color[2],surface.Color[3]);		// Couleur de matière
                }
                
                    if(surface.Flag&SURFACE_GOURAUD)			// Type d'éclairage
                        glShadeModel (GL_SMOOTH);
                    else
                        glShadeModel (GL_FLAT);

                            if(surface.Flag&SURFACE_LIGHTED)			// Eclairage
                                glEnable(GL_LIGHTING);
                            else
                                glDisable(GL_LIGHTING);

                if(surface.Flag&SURFACE_BLENDED) {			// La transparence
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_BLEND);
                    glAlphaFunc( GL_GREATER, 0.1 );
                    glEnable( GL_ALPHA_TEST );
                }
                else {
                    glDisable( GL_ALPHA_TEST );
                    glDisable(GL_BLEND);
                }

                if( surface.Flag&SURFACE_TEXTURED ) {		// Les textures et effets de texture
                    activated_tex=true;
                    for(int j=0;j<surface.NbTex;j++) {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            glEnable(GL_TEXTURE_2D);
                            glBindTexture(GL_TEXTURE_2D,surface.gltex[j]);
                        if(j==surface.NbTex-1 && surface.Flag&SURFACE_REFLEC) {
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
                        else {
                            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                            glDisable(GL_TEXTURE_GEN_S);
                            glDisable(GL_TEXTURE_GEN_T);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        }
                    }
                }
                else
                    for(int j=7;j>=0;j--) {
                        glActiveTextureARB(GL_TEXTURE0_ARB + j);
                            glDisable(GL_TEXTURE_2D);
                    }
            }
            else {
                glEnable(GL_LIGHTING);
                activated_tex=true;
                glEnable(GL_TEXTURE_2D);
                if(!dtex) {
                    activated_tex=false;
                    alset=false;
                    glDisable(GL_TEXTURE_2D);
                }
                if(dtex) {
                    if(side<dtex && side>=0)
                        glBindTexture(GL_TEXTURE_2D,gltex[side]);
                    else
                        glBindTexture(GL_TEXTURE_2D,gltex[0]);
                }
            }
            if( !use_strips ) {
                glBegin( GL_TRIANGLES );
                for(uint32 i = 0 ; i < nb_t_index ; i++ ) {
                    if( activated_tex )
                        glTexCoord2f( tcoord[ t_index[ i ] << 1 ], tcoord[ (t_index[ i ] << 1) + 1 ] );
                    glNormal3f( N[ t_index[ i ] ].x, N[ t_index[ i ] ].y, N[ t_index[ i ] ].z );
                    glVertex3f( points[ t_index[ i ] ].x, points[ t_index[ i ] ].y, points[ t_index[ i ] ].z );
                }
                glEnd();
            }
            else {
                glDisable( GL_CULL_FACE );
                glBegin( GL_TRIANGLE_STRIP );
                for(uint32 i = 0 ; i < nb_t_index ; i++ ) {
                    if( activated_tex )
                        glTexCoord2f( tcoord[ t_index[ i ] << 1 ], tcoord[ (t_index[ i ] << 1) + 1 ] );
                    glNormal3f( N[ t_index[ i ] ].x, N[ t_index[ i ] ].y, N[ t_index[ i ] ].z );
                    glVertex3f( points[ t_index[ i ] ].x, points[ t_index[ i ] ].y, points[ t_index[ i ] ].z );
                }
                glEnd();
                glEnable( GL_CULL_FACE );
            }

            if((surface.Flag&(SURFACE_ADVANCED | SURFACE_GOURAUD))==SURFACE_ADVANCED)
                glShadeModel (GL_SMOOTH);

                    if(activated_tex) {
                        for(int j=0;j<surface.NbTex;j++) {
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
        if(child) {
            //			if(data_s==NULL || !data_s->explode)
            glPushMatrix();
            /*			else {
                        glPopMatrix();
                        glPushMatrix();
                        }*/
            alset=child->draw_dl(data_s,alset,side,chg_col);
            glPopMatrix();
        }
        if(next) {
            glPopMatrix();
            glPushMatrix();
            alset=next->draw_dl(data_s,alset,side,chg_col);
            glPopMatrix();
        }
        else
            glPopMatrix();
        return alset;
    }

    uint16 OBJECT::set_obj_id( uint16 id )
    {
        nb_sub_obj = id;
        if( next )
            id = next->set_obj_id( id );
        obj_id = id++;
        if( child )
            id = child->set_obj_id( id );
        nb_sub_obj = id - nb_sub_obj;
        return id;
    }

    int OBJECT::random_pos( SCRIPT_DATA *data_s, int id, VECTOR *vec )
    {
        if( id == obj_id ) {
            if( nb_t_index > 2 && (data_s == NULL || script_index < 0 || !(data_s->flag[script_index] & FLAG_HIDE)) ) {
                int rnd_idx = (rand_from_table() % (nb_t_index / 3)) * 3;
                float a = (rand_from_table() & 0xFF) / 255.0f;
                float b = (1.0f - a) * (rand_from_table() & 0xFF) / 255.0f;
                float c = 1.0f - a - b;
                vec->x = a * points[ t_index[ rnd_idx ] ].x + b * points[ t_index[ rnd_idx + 1 ] ].x + c * points[ t_index[ rnd_idx + 2 ] ].x;
                vec->y = a * points[ t_index[ rnd_idx ] ].y + b * points[ t_index[ rnd_idx + 1 ] ].y + c * points[ t_index[ rnd_idx + 2 ] ].y;
                vec->z = a * points[ t_index[ rnd_idx ] ].z + b * points[ t_index[ rnd_idx + 1 ] ].z + c * points[ t_index[ rnd_idx + 2 ] ].z;
                if( data_s && script_index >= 0 )
                    *vec = data_s->pos[script_index] + *vec * data_s->matrix[script_index];
            }
            else
                return 0;
            return (data_s && script_index >= 0) ? 2 : 1 ;
        }
        if( id > obj_id ) {
            if( child != NULL ) {
                int r = child->random_pos( data_s, id, vec );
                if( r ) {
                    if( r == 1 )
                        *vec = *vec + pos_from_parent;
                    return r;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        if( next != NULL )
            return next->random_pos( data_s, id, vec );
        return 0;
    }

    void OBJECT::compute_coord(SCRIPT_DATA *data_s,VECTOR *pos,bool c_part,int p_tex,VECTOR *target,VECTOR *upos,MATRIX_4x4 *M,float size,VECTOR *center,bool reverse,OBJECT *src,SCRIPT_DATA *src_data)
    {
        //		if(!emitter && c_part)	return;
        VECTOR opos=*pos;
        MATRIX_4x4 OM;
        if(M)
            OM=*M;
        if(script_index>=0 && data_s)
        {
            if(M)
            {
                VECTOR ipos;
                ipos.x=data_s->axe[0][script_index].pos;
                ipos.y=data_s->axe[1][script_index].pos;
                ipos.z=data_s->axe[2][script_index].pos;
                *pos=*pos+(pos_from_parent+ipos)*(*M);
                *M=RotateZ(data_s->axe[2][script_index].angle*DEG2RAD)*RotateY(data_s->axe[1][script_index].angle*DEG2RAD)*RotateX(data_s->axe[0][script_index].angle*DEG2RAD)*(*M);
                data_s->matrix[script_index] = *M;
                if(nb_l_index==2) {
                    data_s->dir[script_index]=(points[l_index[1]] - points[l_index[0]])*(*M);
                    data_s->dir[script_index].unit();
                    ipos.x=points[l_index[0]].x;
                    ipos.y=points[l_index[0]].y;
                    ipos.z=points[l_index[0]].z;
                }
                data_s->pos[script_index]=*pos;
            }
        }
        else if(M)
            *pos = *pos + pos_from_parent * (*M);
        else
            *pos = *pos + pos_from_parent;

        if( c_part && emitter_point ) // Emit a  particle
        {
            VECTOR Dir;
            float life=1.0f;
            byte nb=(rand_from_table()%60)+1;
            ParticlesSystem *system = NULL;
            for(byte i=0;i<nb;i++)
            {
                VECTOR t_mod;
                bool random_vector = true;
                if( src != NULL )
                    for( int base_n = rand_from_table(), n = 0 ; random_vector && n < src->nb_sub_obj ; n++ )
                        random_vector = !src->random_pos( src_data, (base_n + n) % src->nb_sub_obj, &t_mod );
                if( random_vector )
                {
                    t_mod.x=((rand_from_table()%2001)-1000)*0.001f;
                    t_mod.y=((rand_from_table()%2001)-1000)*0.001f;
                    t_mod.z=((rand_from_table()%2001)-1000)*0.001f;
                    t_mod.unit();
                    t_mod=(rand_from_table()%1001)*0.001f*size*t_mod;
                    if(center)
                        t_mod=t_mod+(*center);
                }
                float speed=1.718281828f;			// exp(1.0f) - 1.0f because of speed law: S(t) = So * exp( -t / tref ) and a lifetime of 1 sec
                if(reverse)
                {
                    Dir=*pos-(t_mod+*target);
                    Dir.x+=upos->x;
                    Dir.y+=upos->y;
                    Dir.z+=upos->z;
                    system = particle_engine.emit_part_fast( system, t_mod+*target,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
                }
                else {
                    Dir=t_mod+*target-*pos;
                    Dir.x-=upos->x;
                    Dir.y-=upos->y;
                    Dir.z-=upos->z;
                    system = particle_engine.emit_part_fast( system, *upos+*pos,Dir,p_tex, i == 0 ? -nb : 1,speed,life,2.0f,true);
                }
            }
        }
        if(child)
            child->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
        *pos=opos;
        if(M)
            *M=OM;
        if(next)
            next->compute_coord(data_s,pos,c_part,p_tex,target,upos,M,size,center,reverse,src,src_data);
    }


    bool OBJECT::draw_shadow(VECTOR Dir,float t,SCRIPT_DATA *data_s,bool alset,bool exploding_parts)
    {
        bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
        bool hide=false;
        VECTOR ODir=Dir;
        glPushMatrix();
        if( explodes && !exploding_parts )	goto draw_shadow_next;
        glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);
        if(script_index>=0 && data_s) {
            if( !explodes ^ exploding_parts ) {
                glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
                glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
                glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
                glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
                Dir=((Dir*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD))*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD))*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            }
            hide=data_s->flag[script_index]&FLAG_HIDE;
        }
        else if( animation_data ) {
            VECTOR R,T;
            animation_data->animate( t, R, T );
            glTranslatef( T.x, T.y, T.z );
            glRotatef( R.x, 1.0f, 0.0f, 0.0f );
            glRotatef( R.y, 0.0f, 1.0f, 0.0f );
            glRotatef( R.z, 0.0f, 0.0f, 1.0f );
            Dir=((Dir*RotateX(-R.x*DEG2RAD))*RotateY(-R.y*DEG2RAD))*RotateZ(-R.z*DEG2RAD);
        }
        hide |= explodes ^ exploding_parts;
        if(nb_t_index>0 && !hide) {
            if(!alset) {
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                glDisableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                alset=true;
            }
            /*-----------------Code de calcul du cone d'ombre-------------------------*/
            if(shadow_index==NULL)
                shadow_index=(GLushort*) malloc(sizeof(GLushort)*nb_t_index*12);
            uint16 nb_idx=0;

            if(t_line==NULL) {									// Repère les arêtes
                t_line=(short*) malloc(sizeof(short)*nb_t_index);
                line_v_idx[0]=(short*) malloc(sizeof(short)*nb_t_index);
                line_v_idx[1]=(short*) malloc(sizeof(short)*nb_t_index);
                face_reverse = new byte[ nb_t_index ];
                nb_line=0;
                for(short i=0;i<nb_t_index;i+=3)
                    for(byte a=0;a<3;a++) {
                        short idx=-1;
                        face_reverse[ i + a ] = 0;
                        for(short e=0;e<nb_line;e++)
                            if(line_v_idx[0][e]==t_index[i+a] && line_v_idx[1][e]==t_index[i+((a+1)%3)]) {
                                idx=e;
                                break;
                            }
                            else if(line_v_idx[0][e]==t_index[i+((a+1)%3)] && line_v_idx[1][e]==t_index[i+a]) {
                                idx=e;
                                face_reverse[ i + a ] = 2;
                                break;
                            }
                        if(idx==-1) {
                            line_v_idx[0][nb_line]=t_index[i+a];
                            line_v_idx[1][nb_line]=t_index[i+((a+1)%3)];
                            idx=nb_line;	nb_line++;
                        }
                        t_line[i+a]=idx;
                    }
                line_on = new byte[nb_line];
            }

            if( Dir.x != last_dir.x || Dir.y != last_dir.y || Dir.z != last_dir.z ) {		// Don't need to compute things twice
                memset((byte*)line_on,0,nb_line);

                uint16 e = 0;
                for(uint16 i=0;i<nb_t_index;i+=3) {
                    if(( F_N[ e++ ] % Dir )>=0.0f) continue;	// Use face normal
                    line_on[t_line[i]] = ((line_on[t_line[i]] ^ 1) & 1) | face_reverse[i];
                    line_on[t_line[i+1]] = ((line_on[t_line[i+1]] ^ 1) & 1) | face_reverse[i+1];
                    line_on[t_line[i+2]] = ((line_on[t_line[i+2]] ^ 1) & 1) | face_reverse[i+2];
                }
                for(short i=0;i<nb_line;i++) {
                    if(!(line_on[i]&1))	continue;
                    points[line_v_idx[0][i]+nb_vtx]=points[line_v_idx[0][i]]+Dir;		// Projection
                    points[line_v_idx[1][i]+nb_vtx]=points[line_v_idx[1][i]]+Dir;

                    if( line_on[i] & 2 ) {
                        shadow_index[nb_idx++]=line_v_idx[1][i];			shadow_index[nb_idx++]=line_v_idx[0][i];
                        shadow_index[nb_idx++]=line_v_idx[0][i]+nb_vtx;		shadow_index[nb_idx++]=line_v_idx[1][i]+nb_vtx;
                    }
                    else {
                        shadow_index[nb_idx++]=line_v_idx[0][i];			shadow_index[nb_idx++]=line_v_idx[1][i];
                        shadow_index[nb_idx++]=line_v_idx[1][i]+nb_vtx;		shadow_index[nb_idx++]=line_v_idx[0][i]+nb_vtx;
                    }
                }
                last_nb_idx = nb_idx;
                last_dir = Dir;
            }
            else
                nb_idx = last_nb_idx;
            if( nb_idx ) {
                glVertexPointer( 3, GL_FLOAT, 0, points);
                glDrawElements(GL_QUADS, nb_idx,GL_UNSIGNED_SHORT,shadow_index);		// dessine le tout
            }
        }
        if(child && !(explodes && !exploding_parts) ) {
            glPushMatrix();
            alset=child->draw_shadow(Dir,t,data_s,alset,exploding_parts & !explodes);
            glPopMatrix();
        }
draw_shadow_next:
        if(next) {
            glPopMatrix();
            glPushMatrix();
            alset=next->draw_shadow(ODir,t,data_s,alset,exploding_parts);
            glPopMatrix();
        }
        else
            glPopMatrix();
        return alset;
    }

    bool OBJECT::draw_shadow_basic(VECTOR Dir,float t,SCRIPT_DATA *data_s,bool alset,bool exploding_parts)
    {
        bool explodes = script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE);
        bool hide=false;
        VECTOR ODir=Dir;
        glPushMatrix();
        if( explodes && !exploding_parts )	goto draw_shadow_basic_next;
        glTranslatef(pos_from_parent.x,pos_from_parent.y,pos_from_parent.z);
        if(script_index>=0 && data_s) {
            if( !explodes ^ exploding_parts ) {
                glTranslatef(data_s->axe[0][script_index].pos, data_s->axe[1][script_index].pos, data_s->axe[2][script_index].pos);
                glRotatef(data_s->axe[0][script_index].angle, 1.0f, 0.0f, 0.0f);
                glRotatef(data_s->axe[1][script_index].angle, 0.0f, 1.0f, 0.0f);
                glRotatef(data_s->axe[2][script_index].angle, 0.0f, 0.0f, 1.0f);
            }
            Dir=((Dir*RotateX(-data_s->axe[0][script_index].angle*DEG2RAD))*RotateY(-data_s->axe[1][script_index].angle*DEG2RAD))*RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            hide=data_s->flag[script_index]&FLAG_HIDE;
        }
        else if( animation_data ) {
            VECTOR R,T;
            animation_data->animate( t, R, T );
            glTranslatef( T.x, T.y, T.z );
            glRotatef( R.x, 1.0f, 0.0f, 0.0f );
            glRotatef( R.y, 0.0f, 1.0f, 0.0f );
            glRotatef( R.z, 0.0f, 0.0f, 1.0f );
            Dir=((Dir*RotateX(-R.x*DEG2RAD))*RotateY(-R.y*DEG2RAD))*RotateZ(-R.z*DEG2RAD);
        }
        hide |= explodes ^ exploding_parts;
        if(nb_t_index>0 && !hide) {
            if(!alset) {
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                glDisableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                alset=true;
            }
            /*-----------------Code de calcul du cone d'ombre-------------------------*/
            if(shadow_index==NULL)
                shadow_index=(GLushort*) malloc(sizeof(GLushort)*nb_t_index*12);
            uint16 nb_idx=0;

            if(t_line==NULL) {									// Repère les arêtes
                t_line=(short*) malloc(sizeof(short)*nb_t_index);
                line_v_idx[0]=(short*) malloc(sizeof(short)*nb_t_index);
                line_v_idx[1]=(short*) malloc(sizeof(short)*nb_t_index);
                face_reverse = new byte[ nb_t_index ];
                nb_line=0;
                for(short i=0;i<nb_t_index;i+=3)
                    for(byte a=0;a<3;a++) {
                        short idx=-1;
                        face_reverse[ i + a ] = 0;
                        for(short e=0;e<nb_line;e++)
                            if(line_v_idx[0][e]==t_index[i+a] && line_v_idx[1][e]==t_index[i+((a+1)%3)]) {
                                idx=e;
                                break;
                            }
                            else if(line_v_idx[0][e]==t_index[i+((a+1)%3)] && line_v_idx[1][e]==t_index[i+a]) {
                                idx=e;
                                face_reverse[ i + a ] = 2;
                                break;
                            }
                        if(idx==-1) {
                            line_v_idx[0][nb_line]=t_index[i+a];
                            line_v_idx[1][nb_line]=t_index[i+((a+1)%3)];
                            idx=nb_line;	nb_line++;
                        }
                        t_line[i+a]=idx;
                    }
                line_on = new byte[nb_line];
            }

            if( Dir.x != last_dir.x || Dir.y != last_dir.y || Dir.z != last_dir.z ) {		// Don't need to compute things twice
                memset((byte*)line_on,0,nb_line);

                uint16 e = 0;
                for(uint16 i=0;i<nb_t_index;i+=3) {
                    if(( F_N[ e++ ] % Dir )>=0.0f) continue;	// Use face normal
                    line_on[t_line[i]] = ((line_on[t_line[i]] ^ 1) & 1) | face_reverse[i];
                    line_on[t_line[i+1]] = ((line_on[t_line[i+1]] ^ 1) & 1) | face_reverse[i+1];
                    line_on[t_line[i+2]] = ((line_on[t_line[i+2]] ^ 1) & 1) | face_reverse[i+2];
                }
                for(short i=0;i<nb_line;i++) {
                    if(!(line_on[i]&1))	continue;
                    points[line_v_idx[0][i]+nb_vtx]=points[line_v_idx[0][i]]+Dir;		// Projection calculations
                    points[line_v_idx[1][i]+nb_vtx]=points[line_v_idx[1][i]]+Dir;

                    if( line_on[i] & 2 ) {
                        shadow_index[nb_idx++]=line_v_idx[1][i];			shadow_index[nb_idx++]=line_v_idx[0][i];
                        shadow_index[nb_idx++]=line_v_idx[0][i]+nb_vtx;		shadow_index[nb_idx++]=line_v_idx[1][i]+nb_vtx;
                    }
                    else {
                        shadow_index[nb_idx++]=line_v_idx[0][i];			shadow_index[nb_idx++]=line_v_idx[1][i];
                        shadow_index[nb_idx++]=line_v_idx[1][i]+nb_vtx;		shadow_index[nb_idx++]=line_v_idx[0][i]+nb_vtx;
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
        if(child && !(explodes && !exploding_parts) ) {
            glPushMatrix();
            alset=child->draw_shadow_basic(Dir,t,data_s,alset, exploding_parts & !explodes);
            glPopMatrix();
        }
draw_shadow_basic_next:
        if(next) {
            glPopMatrix();
            glPushMatrix();
            alset=next->draw_shadow_basic(ODir,t,data_s,alset, exploding_parts);
            glPopMatrix();
        }
        else
            glPopMatrix();
        return alset;
    }

    bool OBJECT::hit(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I,MATRIX_4x4 M)
    {
        MATRIX_4x4 OM=M;
        MATRIX_4x4 AM=Scale(1.0f);
        MATRIX_4x4 M_Dir=M;
        bool hide=false;
        VECTOR ODir=Dir;
        VECTOR OPos=Pos;
        bool is_hit=false;

        VECTOR T=pos_from_parent;
        VECTOR MP;
        if( script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE) )		// We can't select that
            goto hit_is_exploding;

        if(script_index>=0 && data_s) {
            T.x+=data_s->axe[0][script_index].pos;
            T.y+=data_s->axe[1][script_index].pos;
            T.z+=data_s->axe[2][script_index].pos;
            MATRIX_4x4 l_M = Scale( 1.0f );
            if( data_s->axe[0][script_index].angle != 0.0f )
                l_M = l_M * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD);
            if( data_s->axe[1][script_index].angle != 0.0f )
                l_M = l_M * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD);
            if( data_s->axe[2][script_index].angle != 0.0f )
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
        Pos=(Pos-T)*M;
        if( ( nb_t_index>0 || selprim >= 0 ) && !hide) {
            VECTOR A,B,C;
            Dir=Dir*M_Dir;
            Dir.unit();
            //-----------------Code de calcul d'intersection--------------------------
            for(short i=0;i<nb_t_index;i+=3) {
                A=points[t_index[i]];
                B=points[t_index[i+1]];
                C=points[t_index[i+2]];
                VECTOR AB=B-A;
                VECTOR AC=C-A;
                VECTOR N=AB*AC;
                if(N%Dir==0.0f)	continue;
                float dist=-((Pos-A)%N)/(N%Dir);
                if( dist < 0.0f )	continue;
                VECTOR P_p=Pos+dist*Dir;

                //					if(is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
                if(is_hit && MP%Dir<P_p%Dir)	continue;

                float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
                VECTOR AP=P_p-A;
                float pre_cal = AB.x*AC.y-AB.y*AC.x;
                if(AC.y!=0.0f && pre_cal!=0.0f) {
                    b=(AP.x*AC.y-AP.y*AC.x)/pre_cal;
                    a=(AP.y-b*AB.y)/AC.y;
                }
                else {
                    if(AB.x!=0.0f && pre_cal!=0.0f) {
                        a=(AP.y*AB.x-AP.x*AB.y)/pre_cal;
                        b=(AP.x-a*AC.x)/AB.x;
                    }
                    else {
                        pre_cal = AB.x*AC.z-AB.z*AC.x;
                        if(AC.z!=0.0f && pre_cal!=0.0f) {
                            b=(AP.x*AC.z-AP.z*AC.x)/pre_cal;
                            a=(AP.z-b*AB.z)/AC.z;
                        }
                        else {
                            pre_cal=-pre_cal;
                            if(AB.z!=0.0f && pre_cal!=0.0f) {
                                a=(AP.x*AB.z-AP.z*AB.x)/pre_cal;
                                b=(AP.z-a*AC.z)/AB.z;
                            }
                            else {
                                pre_cal = AB.y*AC.x-AB.x*AC.y;
                                if(AC.x!=0.0f && pre_cal!=0.0f) {
                                    b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
                                    a=(AP.x-b*AB.x)/AC.x;
                                }
                                else {
                                    if(AB.y!=0.0f && pre_cal!=0.0f) {
                                        a=(AP.x*AB.y-AP.y*AB.x)/pre_cal;
                                        b=(AP.y-a*AC.y)/AB.y;
                                    }
                                    else continue;		// Saute le point s'il n'est pas positionnable
                                }	}	}	}	}
                                c=1.0f-a-b;
                                if(a<0.0f || b<0.0f || c<0.0f) continue;		// Le point n'appartient pas au triangle
                                MP=P_p;
                                is_hit=true;
            }
            if( selprim >= 0 )
                for( int i = 0 ; i < 2 ; i++ ) {			// Selection primitive ( used to allow selecting naval factories easily )
                    A=points[sel[i]];
                    B=points[sel[i+1]];
                    C=points[sel[3]];
                    VECTOR AB=B-A;
                    VECTOR AC=C-A;
                    VECTOR N=AB*AC;
                    if(N%Dir==0.0f)	continue;
                    float dist=-((Pos-A)%N)/(N%Dir);
                    if( dist < 0.0f )	continue;
                    VECTOR P_p=Pos+dist*Dir;

                    //						if(is_hit && (MP-Pos)%Dir<(P_p-Pos)%Dir)	continue;
                    if(is_hit && MP%Dir<P_p%Dir)	continue;

                    float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
                    VECTOR AP=P_p-A;
                    float pre_cal = AB.x*AC.y-AB.y*AC.x;
                    if(AC.y!=0.0f && pre_cal!=0.0f) {
                        b=(AP.x*AC.y-AP.y*AC.x)/pre_cal;
                        a=(AP.y-b*AB.y)/AC.y;
                    }
                    else {
                        if(AB.x!=0.0f && pre_cal!=0.0f) {
                            a=(AP.y*AB.x-AP.x*AB.y)/pre_cal;
                            b=(AP.x-a*AC.x)/AB.x;
                        }
                        else {
                            pre_cal = AB.x*AC.z-AB.z*AC.x;
                            if(AC.z!=0.0f && pre_cal!=0.0f) {
                                b=(AP.x*AC.z-AP.z*AC.x)/pre_cal;
                                a=(AP.z-b*AB.z)/AC.z;
                            }
                            else {
                                pre_cal=-pre_cal;
                                if(AB.z!=0.0f && pre_cal!=0.0f) {
                                    a=(AP.x*AB.z-AP.z*AB.x)/pre_cal;
                                    b=(AP.z-a*AC.z)/AB.z;
                                }
                                else {
                                    pre_cal = AB.y*AC.x-AB.x*AC.y;
                                    if(AC.x!=0.0f && pre_cal!=0.0f) {
                                        b=(AP.y*AC.x-AP.x*AC.y)/pre_cal;
                                        a=(AP.x-b*AB.x)/AC.x;
                                    }
                                    else {
                                        if(AB.y!=0.0f && pre_cal!=0.0f) {
                                            a=(AP.x*AB.y-AP.y*AB.x)/pre_cal;
                                            b=(AP.y-a*AC.y)/AB.y;
                                        }
                                        else continue;		// Saute le point s'il n'est pas positionnable
                                    }	}	}	}	}
                                    c=1.0f-a-b;
                                    if(a<0.0f || b<0.0f || c<0.0f) continue;		// Le point n'appartient pas au triangle
                                    MP=P_p;
                                    is_hit=true;
                }
        }
        if(child) {
            VECTOR MP2;
            bool nhit=child->hit(Pos,ODir,data_s,&MP2,M_Dir);
            if(nhit && !is_hit)
                MP=MP2;
            else if(nhit && is_hit)
                //				if((MP2-Pos)%Dir<(MP-Pos)%Dir)
                if(MP2%Dir<MP%Dir)
                    MP=MP2;
            is_hit|=nhit;
        }
        if(is_hit)
            MP=(MP*AM)+T;
hit_is_exploding:
        if(next) {
            VECTOR MP2;
            bool nhit=next->hit(OPos,ODir,data_s,&MP2,OM);
            Dir=ODir*OM;
            if(nhit && !is_hit)
                MP=MP2;
            else if(nhit && is_hit)
                //				if((MP2-OPos)%Dir<(MP-OPos)%Dir)
                if(MP2%Dir<MP%Dir)
                    MP=MP2;
            is_hit|=nhit;
        }
        if(is_hit)
            *I=MP;
        return is_hit;
    }

    // hit_fast is a faster version of hit but less precise, designed for use in weapon code
    bool OBJECT::hit_fast(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I)
    {
        bool hide = false;
        VECTOR ODir = Dir;
        VECTOR OPos = Pos;
        MATRIX_4x4 AM;
        bool is_hit = false;


        VECTOR T = pos_from_parent;
        VECTOR MP;
        if( script_index>=0 && data_s && (data_s->flag[script_index] & FLAG_EXPLODE) )
            goto hit_fast_is_exploding;
        if(script_index>=0 && data_s) {
            T.x += data_s->axe[0][script_index].pos;
            T.y += data_s->axe[1][script_index].pos;
            T.z += data_s->axe[2][script_index].pos;
            MATRIX_4x4 l_M = Scale( 1.0f );
            if( data_s->axe[0][script_index].angle != 0.0f )
                l_M = l_M * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD);
            if( data_s->axe[1][script_index].angle != 0.0f )
                l_M = l_M * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD);
            if( data_s->axe[2][script_index].angle != 0.0f )
                l_M = l_M * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            Dir = Dir * l_M;
            Pos = (Pos - T) * l_M;
            //			Dir = ((Dir * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)) * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)) * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            //			Pos = (((Pos - T) * RotateX(-data_s->axe[0][script_index].angle*DEG2RAD)) * RotateY(-data_s->axe[1][script_index].angle*DEG2RAD)) * RotateZ(-data_s->axe[2][script_index].angle*DEG2RAD);
            AM = RotateZ(data_s->axe[2][script_index].angle*DEG2RAD)*RotateY(data_s->axe[1][script_index].angle*DEG2RAD)*RotateX(data_s->axe[0][script_index].angle*DEG2RAD);
            hide = data_s->flag[script_index]&FLAG_HIDE;
        }
        else
            AM = Scale(1.0f);
        if( nb_t_index > 0 && nb_vtx > 0 && !hide) {
            if( compute_min_max ) {		// Required pre-calculations
                compute_min_max = false;
                min_x = max_x = points[0].x;
                min_y = max_y = points[0].y;
                min_z = max_z = points[0].z;
                for( short i=1 ; i < nb_vtx ; i++ ) {
                    min_x = min( min_x, points[i].x );
                    max_x = max( max_x, points[i].x );
                    min_y = min( min_y, points[i].y );
                    max_y = max( max_y, points[i].y );
                    min_z = min( min_z, points[i].z );
                    max_z = max( max_z, points[i].z );
                }
            }

            // Collision detector using boxes
            if( Pos.x >= min_x && Pos.x <= max_x
                && Pos.y >= min_y && Pos.y <= max_y
                && Pos.z >= min_z && Pos.z <= max_z ) {		// The ray starts from inside
                MP = Pos;
                is_hit = true;
            }
            else {
                if( Dir.x != 0.0f ) {						// 2 x planes
                    MP = Pos + ( (min_x - Pos.x) / Dir.x) * Dir;
                    if( MP.y >= min_y && MP.y <= max_y && MP.z >= min_z && MP.z <= max_z )
                        is_hit = true;
                    else {
                        MP = Pos + ( (max_x - Pos.x) / Dir.x) * Dir;
                        if( MP.y >= min_y && MP.y <= max_y && MP.z >= min_z && MP.z <= max_z )
                            is_hit = true;
                    }
                }
                if( !is_hit && Dir.y != 0.0f ) {			// 2 y planes
                    MP = Pos + ( (min_y - Pos.y) / Dir.y) * Dir;
                    if( MP.x >= min_x && MP.x <= max_x && MP.z >= min_z && MP.z <= max_z )
                        is_hit = true;
                    else {
                        MP = Pos + ( (max_y - Pos.y) / Dir.y) * Dir;
                        if( MP.x >= min_x && MP.x <= max_x && MP.z >= min_z && MP.z <= max_z )
                            is_hit = true;
                    }
                }
                if( !is_hit && Dir.z != 0.0f ) {			// 2 z planes
                    MP = Pos + ( (min_z - Pos.z) / Dir.z) * Dir;
                    if( MP.y >= min_y && MP.y <= max_y && MP.x >= min_x && MP.x <= max_x )
                        is_hit = true;
                    else {
                        MP = Pos + ( (max_z - Pos.z) / Dir.z) * Dir;
                        if( MP.y >= min_y && MP.y <= max_y && MP.x >= min_x && MP.x <= max_x )
                            is_hit = true;
                    }
                }
            }
        }
        if(child && !is_hit) {
            VECTOR MP2;
            bool nhit = child->hit_fast(Pos,Dir,data_s,&MP2);
            if( nhit ) {
                if(!is_hit || MP2%Dir<MP%Dir)
                    MP = MP2;
                is_hit = true;
            }
        }
        if(is_hit)
            MP = (MP * AM) + T;

hit_fast_is_exploding:
        if(next && !is_hit) {
            VECTOR MP2;
            bool nhit = next->hit_fast( OPos, ODir, data_s, &MP2);
            if( nhit ) {
                if(!is_hit || MP2%ODir<MP%ODir)
                    MP = MP2;
                is_hit = true;
            }
        }
        if(is_hit)
            *I = MP;
        return is_hit;
    }

    float OBJECT::print_struct(float Y,float X,TA3D::Interfaces::GfxFont fnt)
    {
        //	if(nb_vtx==0 || nb_prim==0 || nb_p_index>0 || nb_l_index>0)
        gfx->print(fnt,X,Y,0.0f,0xFFFFFF,format("%s [%d]",name,script_index));
        gfx->print(fnt,320.0f,Y,0.0f,0xFFFFFF,format("(v:%d",nb_vtx));
        gfx->print(fnt,368.0f,Y,0.0f,0xFFFFFF,format(",p:%d",nb_prim));
        gfx->print(fnt,416.0f,Y,0.0f,0xFFFFFF,format(",t:%d",nb_t_index));
        gfx->print(fnt,464.0f,Y,0.0f,0xFFFFFF,format(",l:%d",nb_l_index));
        gfx->print(fnt,512.0f,Y,0.0f,0xFFFFFF,format(",p:%d)",nb_p_index));
        //		allegro_gl_printf(fnt,320.0f,Y,0.0f,0xFFFFFF,"(v:%d,p:%d,t:%d,l:%d,p:%d)",nb_vtx,nb_prim,nb_t_index,nb_l_index,nb_p_index);
        Y+=8.0f;
        if(child)
            Y=child->print_struct(Y,X+8.0f,fnt);
        if(next)
            Y=next->print_struct(Y,X,fnt);
        return Y;
    }

    int MODEL_MANAGER::load_all(void (*progress)(float percent,const String &msg))
    {
        List<String> file_list;
        sint32 new_nb_models = HPIManager->getFilelist( ta3dSideData.model_dir + "*.3dm", file_list);

        if(new_nb_models > 0)
        {
            MODEL *n_model = (MODEL*) malloc(sizeof(MODEL)*(nb_models+new_nb_models));
            char **n_name = (char**) malloc(sizeof(char*)*(nb_models+new_nb_models));
            if(model)
            {
                memcpy(n_model,model,sizeof(MODEL)*nb_models);
                free(model);
                memcpy(n_name,name,sizeof(char*)*nb_models);
                free(name);
            }
            model=n_model;
            name=n_name;
            int i = 0, n = 0;
            for(List<String>::iterator e=file_list.begin();e!=file_list.end(); ++e)
            {
                Console->AddEntry( "loading %s", e->c_str() );
                if(progress!=NULL && !(i & 0xF))
                    progress((100.0f+n*50.0f/(new_nb_models+1))/7.0f,TRANSLATE("Loading 3D Models"));
                n++;
                model[i+nb_models].init();
                name[i+nb_models] = strdup(e->c_str());

                if(get_model( String( name[i+nb_models] ).substr( 0, e->size() - 4 ).c_str() )==NULL) 	// Vérifie si le modèle n'est pas déjà chargé
                {
                    byte *data = HPIManager->PullFromHPI(*e);
                    if( data )
                    {
                        if( data[0] == 0 )
                        {
                            String real_name = (char*)(data+1);
                            real_name = TrimString( real_name );

                            free( data );
                            data = HPIManager->PullFromHPI( real_name );
                        }
                        if( data )
                        {
                            model[i+nb_models].load_3dm(data);
                            free(data);

                            model_hashtable.Insert( Lowercase( *e ), nb_models + i + 1 );

                            i++;
                        }
                    }
                }
            }
            nb_models+=i;
        }

        file_list.clear();
        new_nb_models = HPIManager->getFilelist(ta3dSideData.model_dir + "*.3do", file_list);

        if(new_nb_models > 0)
        {
            MODEL *n_model = (MODEL*) malloc(sizeof(MODEL)*(nb_models+new_nb_models));
            char **n_name = (char**) malloc(sizeof(char*)*(nb_models+new_nb_models));
            if(model) {
                memcpy(n_model,model,sizeof(MODEL)*nb_models);
                free(model);
                memcpy(n_name,name,sizeof(char*)*nb_models);
                free(name);
            }
            model = n_model;
            name = n_name;
            int i = 0, n = 0;
            for(List<String>::iterator e=file_list.begin();e!=file_list.end();e++)
            {
                Console->AddEntry( "loading %s", e->c_str() );
                if( progress != NULL && !(i & 0xF) )
                    progress((100.0f+(50.0f+n*50.0f/(new_nb_models+1)))/7.0f,TRANSLATE("Loading 3D Models"));
                n++;
                model[i+nb_models].init();
                name[i+nb_models] = strdup(e->c_str());

                if(get_model( String( name[i+nb_models] ).substr( 0, e->size() - 4 ).c_str() )==NULL) // Vérifie si le modèle n'est pas déjà chargé
                {
                    uint32	data_size = 0;
                    byte *data = HPIManager->PullFromHPI(*e, &data_size);
                    if( data )
                    {
                        if( data_size > 0 )						// If the file isn't empty
                            model[i+nb_models].load_3do(data,e->c_str());
                        free(data);

                        model_hashtable.Insert( Lowercase( *e ), nb_models + i + 1 );

                        i++;
                    }
                }
            }
            nb_models+=i;
        }

        return 0;
    }

    void MODEL::load_asc(char *filename,float size)		// Charge un fichier au format *.ASC
    {
        
            destroy();			// Puisqu'on charge
        
            float *coor[3];
            coor[0]=(float *) malloc(100000*sizeof(float));
            coor[1]=(float *) malloc(100000*sizeof(float));
            coor[2]=(float *) malloc(100000*sizeof(float));
            int *face[3];
            face[0]=(int *) malloc(100000*sizeof(int));
            face[1]=(int *) malloc(100000*sizeof(int));
            face[2]=(int *) malloc(100000*sizeof(int));
            
            if(coor[0]==NULL || coor[1]==NULL || coor[2]==NULL
               || face[0]==NULL || face[1]==NULL || face[2]==NULL) {
                if(coor[0]!=NULL) free(coor[0]);
                    if(coor[1]!=NULL) free(coor[1]);
                        if(coor[2]!=NULL) free(coor[2]);
                            if(face[0]!=NULL) free(face[0]);
                                if(face[1]!=NULL) free(face[1]);
                                    if(face[2]!=NULL) free(face[2]);
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
            
            if ((fichier = TA3D_OpenFile(filename,"rt"))==NULL)	{
                Console->AddEntry("Impossible d'ouvrir le fichier %s en lecture",filename);
                    return;
            }
        
            do
            {
                // On lit le fichier contenant les informations sur l'objet
                fin=fgets(chaine,200,fichier);
                    strupr(chaine);
                    if (!strncmp(chaine,"VERTEX",6))	{
                        if (strncmp(chaine,"VERTEX LIST",11))	{
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
                                                                                
                                                                                if(x<xmin) xmin=x;
                                                                                    if(x>xmax) xmax=x;
                                                                                        if(y<ymin) ymin=y;
                                                                                            if(y>ymax) ymax=y;
                                                                                                if(z<zmin) zmin=z;
                                                                                                    if(z>zmax) zmax=z;
                                                                                                        
                                                                                                            nbp++;
                        }
                    }
                    else {
                        if (!strncmp(chaine,"FACE",4))	{
                            if (strncmp(chaine,"FACE LIST",9))	{
                                // Lecture d'une facette
                                i=j=4;
                                    while(chaine[i]!='A') i++;
                                        i+=2;
                                            while(chaine[i]==' ') i++;
                                                j=i;
                                                    while(chaine[j]!=' ') j++;
                                                        strncpy(temp,chaine+i,j-i);
                                                            temp[j-i]=0;
                                                            face[0][nbf]=atoi(temp)+decalage;
                                                            
                                                            while(chaine[i]!='B') i++;
                                                                i+=2;
                                                                    while(chaine[i]==' ') i++;
                                                                        j=i;
                                                                            while(chaine[j]!=' ') j++;
                                                                                strncpy(temp,chaine+i,j-i);
                                                                                    temp[j-i]=0;
                                                                                    face[1][nbf]=atoi(temp)+decalage;
                                                                                    
                                                                                    while(chaine[i]!='C') i++;
                                                                                        i+=2;
                                                                                            while(chaine[i]==' ') i++;
                                                                                                j=i;
                                                                                                    while(chaine[j]!=' ') j++;
                                                                                                        strncpy(temp,chaine+i,j-i);
                                                                                                            temp[j-i]=0;
                                                                                                            face[2][nbf]=atoi(temp)+decalage;
                                                                                                            
                                                                                                            nbf++;
                            }
                        }
                        else
                            if (!strncmp(chaine,"NAMED OBJECT",12)) {
                                StructName[NbStruct]=strdup(chaine+13);
                                char *_p = StructName[NbStruct];
                                while( _p[0] ) {
                                    if( _p[0] == 10 || _p[0] == 13 )
                                        _p[0] = 0;
                                    _p++;
                                }
                                decalage=nbp;
                                    StructD[NbStruct++]=nbf;
                            }
                    }
            } while(fin!=NULL);
            
                fclose(fichier);

                StructD[NbStruct]=nbf;

            OBJECT *cur = &obj;
            
                dx=-(xmin+xmax)*0.5f;
                dy=-(ymin+ymax)*0.5f;
                dz=-(zmin+zmax)*0.5f;
                xmin=xmax-xmin;
                ymin=ymax-ymin;
                zmin=zmax-zmin;
                xmax=sqrt(xmin*xmin+ymin*ymin+zmin*zmin);
                size=size/xmax;
                
                for(i=0;i<NbStruct;i++) {			// Crée les différentes parties de la meshe
                    if(i>0) {
                        cur->next= (OBJECT*) malloc(sizeof(OBJECT));
                        cur=cur->next;
                    }
                    cur->init();
                    cur->name=StructName[i];
                    cur->nb_prim=StructD[i+1]-StructD[i];
                    cur->nb_t_index=cur->nb_prim*3;
                    cur->nb_vtx=cur->nb_t_index;
                    cur->points = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
                    cur->t_index = (GLushort*) malloc(sizeof(GLushort)*cur->nb_t_index);
                    cur->tcoord = (float*) malloc(sizeof(float)*cur->nb_vtx<<1);

                    cur->surface.Flag=SURFACE_ADVANCED|SURFACE_GOURAUD|SURFACE_LIGHTED;
                    for(int k=0;k<4;k++)
                        cur->surface.Color[k]=cur->surface.RColor[k]=1.0f;
                    
                        int p_nbf=0;
                    int p_nbp=0;

                    for(int k=StructD[i];k<StructD[i+1];k++) {		// Compte et organise les points
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
                    }			// Fin de for(k=StructD[i];k<StructD[i+1];k++)
                }				// Fin de for(i=0;i<NbStruct;i++)

            cur=&obj;
            while(cur) {
                int removed=0;
                for(i=0;i<cur->nb_t_index;i++)				// Remove duplicate points
                    for(int e=0;e<i;e++)
                        if(cur->points[cur->t_index[i]].x==cur->points[cur->t_index[e]].x && cur->points[cur->t_index[i]].y==cur->points[cur->t_index[e]].y && cur->points[cur->t_index[i]].z==cur->points[cur->t_index[e]].z) {
                            cur->t_index[i]=cur->t_index[e];
                            removed++;
                            break;
                        }
                cur->nb_vtx-=removed;
                VECTOR *n_points = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
                int cur_pt=0;
                for(i=0;i<cur->nb_t_index;i++) {
                    bool ok=false;
                    for(int e=0;e<i;e++)
                        if(cur->points[cur->t_index[i]].x==n_points[cur->t_index[e]].x && cur->points[cur->t_index[i]].y==n_points[cur->t_index[e]].y && cur->points[cur->t_index[i]].z==n_points[cur->t_index[e]].z) {
                            cur->t_index[i]=cur->t_index[e];
                            ok=true;
                            break;
                        }
                    if(ok)	continue;
                    n_points[cur_pt]=cur->points[cur->t_index[i]];
                    cur->t_index[i]=cur_pt++;
                }
                free(cur->points);
                cur->points=n_points;
                for(i=0;i<cur->nb_vtx;i++) {				// Remove duplicate points
                    cur->points[i].x*=size;
                    cur->points[i].y*=size;
                    cur->points[i].z*=size;
                }

                cur->N = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);	// Calculate normals
                cur->F_N = new VECTOR[ cur->nb_t_index / 3 ];
                for(i=0;i<cur->nb_vtx;i++)
                    cur->N[i].x=cur->N[i].z=cur->N[i].y=0.0f;
                int e = 0;
                for(i=0;i<cur->nb_t_index;i+=3) {
                    VECTOR AB,AC,Normal;
                    AB=cur->points[cur->t_index[i+1]] - cur->points[cur->t_index[i]];
                    AC=cur->points[cur->t_index[i+2]] - cur->points[cur->t_index[i]];
                    Normal=AB*AC;	Normal.unit();
                    cur->F_N[ e++ ] = Normal;
                    for(int e=0;e<3;e++)
                        cur->N[cur->t_index[i+e]]=cur->N[cur->t_index[i+e]]+Normal;
                }
                for(i=0;i<cur->nb_vtx;i++)
                    cur->N[i].unit();
                cur=cur->next;
            }
            
                free(coor[0]);
                free(coor[1]);
                free(coor[2]);
                free(face[0]);
                free(face[1]);
                free(face[2]);
    }

    void OBJECT::save_3dm(FILE *dst, bool compressed)
    {
        uint8	len = strlen(name);
        fwrite(&len,sizeof(len),1,dst);		// Write the object name
        fwrite(name,len,1,dst);

        fwrite(&pos_from_parent.x,sizeof(pos_from_parent.x),1,dst);
        fwrite(&pos_from_parent.y,sizeof(pos_from_parent.y),1,dst);
        fwrite(&pos_from_parent.z,sizeof(pos_from_parent.z),1,dst);

        fwrite(&nb_vtx,sizeof(nb_vtx),1,dst);
        if(points!=NULL)
            fwrite(points,sizeof(VECTOR)*nb_vtx,1,dst);

        fwrite(sel,sizeof(GLushort)*4,1,dst);				// Selection primitive

        fwrite(&nb_p_index,sizeof(nb_p_index),1,dst);		// Write point data
        if(p_index!=NULL)
            fwrite(p_index,sizeof(GLushort)*nb_p_index,1,dst);

        fwrite(&nb_l_index,sizeof(nb_l_index),1,dst);		// Write line data
        if(l_index!=NULL)
            fwrite(l_index,sizeof(GLushort)*nb_l_index,1,dst);

        fwrite(&nb_t_index,sizeof(nb_t_index),1,dst);		// Write triangle data
        if(t_index!=NULL)
            fwrite(t_index,sizeof(GLushort)*nb_t_index,1,dst);

        fwrite(tcoord,sizeof(float)*nb_vtx<<1,1,dst);		// Write texture coordinates

        fwrite(surface.Color,sizeof(float)*4,1,dst);		// Write surface data
        fwrite(surface.RColor,sizeof(float)*4,1,dst);
        fwrite(&surface.Flag,sizeof(surface.Flag),1,dst);
        int tmp=surface.NbTex;
        if(!(surface.Flag&SURFACE_TEXTURED))
            surface.NbTex=0;
        else if( compressed )
            surface.NbTex = -surface.NbTex;			// For compatibility with older versions

        fwrite(&surface.NbTex,sizeof(surface.NbTex),1,dst);
        surface.NbTex=tmp;
        if(surface.Flag&SURFACE_TEXTURED)
            for(uint8 i=0;i<surface.NbTex;i++) {
                BITMAP *tex;
                GLint w,h;
                glBindTexture(GL_TEXTURE_2D,surface.gltex[i]);
                glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);
                glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
                tex=create_bitmap_ex(32,w,h);
                glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,tex->line[0]);

                if( !compressed ) {						// Store texture data without compression
                    fwrite(&tex->w,sizeof(tex->w),1,dst);
                    fwrite(&tex->h,sizeof(tex->h),1,dst);
                    for(int y=0;y<tex->h;y++)
                        for(int x=0;x<tex->w;x++)
                            fwrite(&((int*)(tex->line[y]))[x],4,1,dst);
                }
                else {									// Store texture data in JPG format (using two images, one for RGB and one for alpha, if needed -> check first that alpha isn't all 0xFF)
                    int buf_size = tex->w * tex->h * 5;
                    byte *buffer = new byte[ buf_size ];

                    int img_size = buf_size;
                    save_memory_jpg_ex( buffer, &img_size, tex, NULL, 85, JPG_SAMPLING_411 | JPG_OPTIMIZE, NULL );

                    fwrite( &img_size, sizeof( img_size ), 1, dst );		// Save the result
                    fwrite( buffer, img_size, 1, dst );

                    bool need_alpha = false;
                    for( int y = 0 ; y < tex->h ; y++ )
                        for( int x = 0 ; x < tex->w ; x++ ) {
                            int c = geta( getpixel( tex, x, y ) );
                            if( c != 255 )
                                need_alpha = true;
                            ((uint32*)(tex->line[y]))[x] = c * 0x01010101;
                        }

                    if( need_alpha ) {
                        putc( 1, dst );		// Alpha channel has to be stored
                        img_size = buf_size;
                        save_memory_jpg_ex( buffer, &img_size, tex, NULL, 100, JPG_GREYSCALE | JPG_OPTIMIZE, NULL );

                        fwrite( &img_size, sizeof( img_size ), 1, dst );		// Save the result
                        fwrite( buffer, img_size, 1, dst );
                    }
                    else
                        putc( 0, dst );		// No alpha channel stored

                    delete[] buffer;
                }

                destroy_bitmap(tex);
            }

        if(surface.Flag&SURFACE_GLSL) {		// Save the shader object
            fwrite(&surface.vert_shader_size,4,1,dst);
            fwrite(surface.vert_shader_src,surface.vert_shader_size,1,dst);
            fwrite(&surface.frag_shader_size,4,1,dst);
            fwrite(surface.frag_shader_src,surface.frag_shader_size,1,dst);
        }

        if( animation_data ) {		// Save animation data
            fputc(2,dst);
            fputc( animation_data->type, dst);
            fwrite( &(animation_data->angle_0), sizeof( VECTOR ), 1, dst );
            fwrite( &(animation_data->angle_1), sizeof( VECTOR ), 1, dst );
            fwrite( &(animation_data->angle_w), sizeof( float ), 1, dst );
            fwrite( &(animation_data->translate_0), sizeof( VECTOR ), 1, dst );
            fwrite( &(animation_data->translate_1), sizeof( VECTOR ), 1, dst );
            fwrite( &(animation_data->translate_w), sizeof( float ), 1, dst );
        }

        if(child) {
            fputc(1,dst);
            child->save_3dm(dst, compressed);
        }
        else
            fputc(0,dst);

        if(next) {
            fputc(1,dst);
            next->save_3dm(dst, compressed);
        }
        else
            fputc(0,dst);
    }

    inline byte *read_from_mem(void *buf,int len,byte *data)
    {
        memcpy(buf,data,len);
        return data+len;
    }

    byte *OBJECT::load_3dm(byte *data)
    {
        destroy();

        uint8	len = data[0];	data++;
        name = (char*) malloc(len+1);
        data=read_from_mem(name,len,data);
        name[len]=0;

        data=read_from_mem(&pos_from_parent.x,sizeof(pos_from_parent.x),data);
        data=read_from_mem(&pos_from_parent.y,sizeof(pos_from_parent.y),data);
        data=read_from_mem(&pos_from_parent.z,sizeof(pos_from_parent.z),data);

        data=read_from_mem(&nb_vtx,sizeof(nb_vtx),data);
        if(nb_vtx>0) {
            points = (VECTOR*) malloc(sizeof(VECTOR)*nb_vtx<<1);
            data=read_from_mem(points,sizeof(VECTOR)*nb_vtx,data);
        }
        else
            points=NULL;

        data=read_from_mem(sel,sizeof(GLushort)*4,data);

        data=read_from_mem(&nb_p_index,sizeof(nb_p_index),data);	// Read point data
        if(nb_p_index>0) {
            p_index = (GLushort*) malloc(sizeof(GLushort)*nb_p_index);
            data=read_from_mem(p_index,sizeof(GLushort)*nb_p_index,data);
        }
        else
            p_index=NULL;

        data=read_from_mem(&nb_l_index,sizeof(nb_l_index),data);	// Read line data
        if(nb_l_index>0) {
            l_index = (GLushort*) malloc(sizeof(GLushort)*nb_l_index);
            data=read_from_mem(l_index,sizeof(GLushort)*nb_l_index,data);
        }
        else
            l_index=NULL;

        data=read_from_mem(&nb_t_index,sizeof(nb_t_index),data);	// Read triangle data
        if(nb_t_index>0) {
            t_index = (GLushort*) malloc(sizeof(GLushort)*nb_t_index);
            data=read_from_mem(t_index,sizeof(GLushort)*nb_t_index,data);
        }
        else
            t_index=NULL;

        tcoord = (float*) malloc(sizeof(float)*nb_vtx<<1);
        data=read_from_mem(tcoord,sizeof(float)*nb_vtx<<1,data);

        data=read_from_mem(surface.Color,sizeof(float)*4,data);	// Read surface data
        data=read_from_mem(surface.RColor,sizeof(float)*4,data);
        data=read_from_mem(&surface.Flag,sizeof(surface.Flag),data);
        surface.NbTex=0;
        data=read_from_mem(&surface.NbTex,sizeof(surface.NbTex),data);
        bool compressed = surface.NbTex < 0;
        surface.NbTex = abs( surface.NbTex );
        for(uint8 i=0;i<surface.NbTex;i++) {
            BITMAP *tex;
            if( !compressed ) {
                int tex_w;
                int tex_h;
                data=read_from_mem(&tex_w,sizeof(tex_w),data);
                data=read_from_mem(&tex_h,sizeof(tex_h),data);

                tex = create_bitmap_ex(32,tex_w,tex_h);
                for(int y=0;y<tex->h;y++)
                    for(int x=0;x<tex->w;x++)
                        data=read_from_mem(&((int*)(tex->line[y]))[x],4,data);
            }
            else {
                int img_size = 0;
                data=read_from_mem(&img_size,sizeof(img_size),data);	// Read RGB data first
                byte *buffer = new byte[ img_size ];

                data=read_from_mem( buffer, img_size, data );

                set_color_depth( 32 );
                tex = load_memory_jpg( buffer, img_size, NULL );

                delete[] buffer;

                byte has_alpha;									// Read alpha channel if present
                data=read_from_mem( &has_alpha, 1, data );
                if( has_alpha ) {
                    data=read_from_mem(&img_size,sizeof(img_size),data);
                    buffer = new byte[ img_size ];

                    data=read_from_mem( buffer, img_size, data );

                    BITMAP *alpha = load_memory_jpg( buffer, img_size, NULL );

                    for( int y = 0 ; y < tex->h ; y++ )
                        for( int x = 0 ; x < tex->w ; x++ ) {
                            int c = getpixel( tex, x, y );
                            putpixel( tex, x, y, makeacol( getr(c), getg(c), getb(c), alpha->line[y][x<<2] ) );
                        }

                    destroy_bitmap( alpha );
                    delete[] buffer;
                }
                else
                    for( int y = 0 ; y < tex->h ; y++ )
                        for( int x = 0 ; x < tex->w ; x++ ) {
                            int c = getpixel( tex, x, y );
                            putpixel( tex, x, y, makeacol( getr(c), getg(c), getb(c), 0xFF ) );
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

        if(surface.Flag&SURFACE_GLSL) {					// Fragment & Vertex shaders
            data=read_from_mem(&surface.vert_shader_size,4,data);
            surface.vert_shader_src = (char*) malloc(surface.vert_shader_size+1);
            surface.vert_shader_src[surface.vert_shader_size]=0;
            data=read_from_mem(surface.vert_shader_src,surface.vert_shader_size,data);
            data=read_from_mem(&surface.frag_shader_size,4,data);
            surface.frag_shader_src = (char*) malloc(surface.frag_shader_size+1);
            surface.frag_shader_src[surface.frag_shader_size]=0;
            data=read_from_mem(surface.frag_shader_src,surface.frag_shader_size,data);
            surface.s_shader.load_memory(surface.frag_shader_src,surface.frag_shader_size,surface.vert_shader_src,surface.vert_shader_size);
        }

        N = (VECTOR*) malloc(sizeof(VECTOR)*nb_vtx<<1);	// Calculate normals
        if(nb_t_index>0 && t_index!=NULL)
        {
            F_N = new VECTOR[ nb_t_index / 3 ];
            for(int i=0;i<nb_vtx;i++)
                N[i].x=N[i].z=N[i].y=0.0f;
            int e = 0;
            for(int i=0;i<nb_t_index;i+=3)
            {
                VECTOR AB,AC,Normal;
                AB=points[t_index[i+1]] - points[t_index[i]];
                AC=points[t_index[i+2]] - points[t_index[i]];
                Normal=AB*AC;	Normal.unit();
                F_N[ e++ ] = Normal;
                for(int e=0;e<3;e++)
                    N[t_index[i+e]]=N[t_index[i+e]]+Normal;
            }
            for(int i=0;i<nb_vtx; ++i)
                N[i].unit();
        }

        byte link;
        data=read_from_mem(&link,1,data);

        if(link == 2) // Load animation data if present
        {
            animation_data = new ANIMATION;
            data = read_from_mem( &(animation_data->type), 1, data );
            data = read_from_mem( &(animation_data->angle_0), sizeof( VECTOR ), data );
            data = read_from_mem( &(animation_data->angle_1), sizeof( VECTOR ), data );
            data = read_from_mem( &(animation_data->angle_w), sizeof( float ), data );
            data = read_from_mem( &(animation_data->translate_0), sizeof( VECTOR ), data );
            data = read_from_mem( &(animation_data->translate_1), sizeof( VECTOR ), data );
            data = read_from_mem( &(animation_data->translate_w), sizeof( float ), data );

            data=read_from_mem(&link,1,data);
        }

        if(link)
        {
            child = (OBJECT*) malloc(sizeof(OBJECT));
            child->init();
            data=child->load_3dm(data);
        }
        else
            child=NULL;

        data=read_from_mem(&link,1,data);
        if(link)
        {
            next = (OBJECT*) malloc(sizeof(OBJECT));
            next->init();
            data=next->load_3dm(data);
        }
        else
            next=NULL;
        return data;
    }

    DRAWING_TABLE::~DRAWING_TABLE()
    {
        for( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for( List< RENDER_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                delete *e;
        hash_table.clear();
    }

    void DRAWING_TABLE::queue_instance( uint32 &model_id, INSTANCE instance )
    {
        uint32	hash = model_id & DRAWING_TABLE_MASK;
        for( List< RENDER_QUEUE* >::iterator i = hash_table[ hash ].begin() ; i != hash_table[ hash ].end() ; i++ )
            if( (*i)->model_id == model_id ) {		// We found an already existing render queue
                (*i)->queue.push_back( instance );
                return;
            }
        RENDER_QUEUE *render_queue = new RENDER_QUEUE( model_id );
        hash_table[ hash ].push_back( render_queue );
        render_queue->queue.push_back( instance );
    }

    void DRAWING_TABLE::draw_all()
    {
        for( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for( List< RENDER_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                (*e)->draw_queue();
    }

    void RENDER_QUEUE::draw_queue()
    {
        if( queue.size() == 0 )	return;
        glPushMatrix();

        if( model_manager.model[ model_id ].from_2d )
            glEnable(GL_ALPHA_TEST);

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        if( !model_manager.model[ model_id ].dlist ) {				// Build the display list if necessary
            model_manager.model[ model_id ].dlist = glGenLists (1);
            glNewList (model_manager.model[ model_id ].dlist, GL_COMPILE);
            model_manager.model[ model_id ].obj.draw_dl(NULL,false,0,true);
            glEndList();
        }

        for( List< INSTANCE >::iterator i = queue.begin() ; i != queue.end() ; i++ ) {
            glPopMatrix();
            glPushMatrix();
            glTranslatef( i->pos.x, i->pos.y, i->pos.z );
            glRotatef( i->angle, 0.0f, 1.0f, 0.0f );
            glColor4ubv( (GLubyte*) &i->col );
            glCallList( model_manager.model[ model_id ].dlist );
        }

        if( model_manager.model[ model_id ].from_2d )
            glDisable(GL_ALPHA_TEST);

        glPopMatrix();
    }

    QUAD_TABLE::~QUAD_TABLE()
    {
        for( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for( List< QUAD_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                delete *e;

        hash_table.clear();
    }

    void QUAD_TABLE::queue_quad( GLuint &texture_id, QUAD quad )
    {
        uint32	hash = texture_id & DRAWING_TABLE_MASK;
        for( List< QUAD_QUEUE* >::iterator i = hash_table[ hash ].begin() ; i != hash_table[ hash ].end() ; i++ )
            if( (*i)->texture_id == texture_id ) {		// We found an already existing render queue
                (*i)->queue.push_back( quad );
                return;
            }
        QUAD_QUEUE *quad_queue = new QUAD_QUEUE( texture_id );
        hash_table[ hash ].push_back( quad_queue );
        quad_queue->queue.push_back( quad );
    }

    void QUAD_TABLE::draw_all()
    {
        uint32	max_size = 0;
        for( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for( List< QUAD_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                max_size = max( max_size, (uint32)(*e)->queue.size() );

        VECTOR	*P = new VECTOR[ max_size << 2 ];
        uint32	*C = new uint32[ max_size << 2 ];
        GLfloat	*T = new GLfloat[ max_size << 3 ];

        int e = 0;
        for( int i = 0 ; i < max_size ; i++ ) {
            T[e<<1] = 0.0f;		T[(e<<1)+1] = 0.0f;
            e++;

            T[e<<1] = 1.0f;		T[(e<<1)+1] = 0.0f;
            e++;

            T[e<<1] = 1.0f;		T[(e<<1)+1] = 1.0f;
            e++;

            T[e<<1] = 0.0f;		T[(e<<1)+1] = 1.0f;
            e++;
        }


        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable( GL_TEXTURE_2D );
            glColorPointer(4,GL_UNSIGNED_BYTE,0,C);
        glVertexPointer( 3, GL_FLOAT, 0, P);
        glTexCoordPointer(2, GL_FLOAT, 0, T);

        for( uint16 i = 0 ; i < DRAWING_TABLE_SIZE ; i++ )
            for( List< QUAD_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; e++ )
                (*e)->draw_queue( P, C, T );

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            delete[] P;
        delete[] C;
        delete[] T;
    }

    void QUAD_QUEUE::draw_queue( VECTOR *P, uint32 *C, GLfloat	*T )
    {
        if( queue.size() == 0 )	return;
        glPushMatrix();

        int i = 0;
        for( List< QUAD >::iterator e = queue.begin() ; e != queue.end() ; e++ ) {
            P[i].x = e->pos.x - e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z - e->size_z;
            C[i] = e->col;
            i++;

            P[i].x = e->pos.x + e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z - e->size_z;
            C[i] = e->col;
            i++;

            P[i].x = e->pos.x + e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z + e->size_z;
            C[i] = e->col;
            i++;

            P[i].x = e->pos.x - e->size_x;
            P[i].y = e->pos.y;
            P[i].z = e->pos.z + e->size_z;
            C[i] = e->col;
            i++;
        }
        glBindTexture( GL_TEXTURE_2D, texture_id );

        glDrawArrays(GL_QUADS, 0, queue.size()<<2);		// draw those quads

        glPopMatrix();
    }
