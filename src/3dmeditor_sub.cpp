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

#include "stdafx.h"
#include "TA3D_NameSpace.h"

#define TA3D_BASIC_ENGINE
#include "ta3dbase.h"		// Moteur
#include "TA3D_NameSpace.h"
#include "threads/cThread.h"
#include "logs/cLogger.h"
#include "gui.h"			// Interface utilisateur
#include "TA3D_hpi.h"		// Interface HPI requis pour 3do.h
#include "gfx/particles/particles.h"
#include "gaf.h"
#include "3do.h"			// Gestion des modèles 3D
#include "3ds.h"			// The 3DS model loader
#include "taconfig.h"		// Configuration
#include "3dmeditor.h"
#include "misc/paths.h"



int cur_part=0;
bool ClickOnExit=false;

float	player_color[30]={	0.11f,	0.28f,	0.91f,
							0.83f,	0.17f,	0.0f,
							1.0f,	1.0f,	1.0f,
							0.11f,	0.62f,	0.07f,
							0.03f,	0.12f,	0.48f,
							0.5f,	0.34f,	0.62f,
							1.0f,	1.0f,	0.0f,
							0.0f,	0.0f,	0.0f,
							0.61f,	0.8f,	0.87f,
							0.67f,	0.67f,	0.51f };
byte	player_color_map[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

/*---------------------------------------------------------------------------------------------------\
  |               Fonctions associées aux menus déroulant de la barre de menus de l'interface          |
  |   principale. Gère les actions liées aux options proposées par les menus.                          |
  \---------------------------------------------------------------------------------------------------*/
void mnu_file(int mnu_index)
{
    switch(mnu_index)
    {
        case 0:					// Nouveau
            TheModel->destroy();
            cur_part=0;
            init_surf_buf();
            break;
        case 1:					// Ouvrir
            {
                init_surf_buf();
                String filename = Dialog(TRANSLATE( "Ouvrir un modèle" ),"*.3dm");
                if( filename.c_str() && file_exists(filename.c_str(),FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_ARCH,NULL)) {
                    FILE *src = TA3D_OpenFile(filename.c_str(), "rb");
                    if(src) {
                        byte *data = new byte[FILE_SIZE(filename.c_str())];
                        fread(data,FILE_SIZE(filename.c_str()),1,src);
                        fclose(src);
                        TheModel->load_3dm(data);
                        delete[] data;
                        cur_part=0;
                    }
                    else
                        Console->AddEntry("Error : cannot open %s for reading",filename.empty() ? "NULL" : filename.c_str());
                }
            }
            break;
        case 2:					// Sauver
            {
                String filename=Dialog(TRANSLATE( "Sauvegarder un modèle" ),"*.3dm");
                TheModel->save_3dm((char*)filename.c_str(), WndAsk( TRANSLATE("compression"), TRANSLATE("comprimer les textures?") ));
            }
            break;
        case 3:					// Importer (*.asc)
            {
                init_surf_buf();
                String filename=Dialog(TRANSLATE( "Importer un modèle au format ASC" ),"*.asc");
                if(file_exists(filename.c_str(),FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_ARCH,NULL)) {
                    TheModel->load_asc((char*)filename.c_str(),30.0f);
                    convert_to_3dm();
                    cur_part=0;
                }
            }
            break;
        case 4:					// Importer (*.3do)
            {
                init_surf_buf();
                String filename=Dialog(TRANSLATE( "Importer un modèle au format 3DO" ),"*.3do");
                if(file_exists(filename.c_str(),FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_ARCH,NULL)) {
                    byte *data = new byte[FILE_SIZE(filename.c_str())];
                    FILE *src = TA3D_OpenFile(filename.c_str(),"rb");
                    fread(data,FILE_SIZE(filename.c_str()),1,src);
                    fclose(src);
                    TheModel->load_3do(data);
                    convert_to_3dm();
                    cur_part=0;
                    delete[] data;
                }
            }
            break;
        case 5:					// Importer (*.3ds)
            {
                init_surf_buf();
                String filename=Dialog(TRANSLATE( "Importer un modèle au format 3DS" ),"*.3ds");
                if(file_exists(filename.c_str(),FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_ARCH,NULL)) {
                    TheModel = load_3ds( filename );
                    cur_part=0;
                }
            }
            break;
        case 6:					// Quitter
            ClickOnExit=true;
            break;
    };
}

void mnu_surf(int mnu_index)
{
    switch(mnu_index)
    {
        case 0:						// Editer
            SurfEdit();		// Lance l'éditeur de surfaces
            init_surf_buf();
            break;
        case 1:						// Copier
            if(cur_part>=0 && cur_part<nb_obj())
                obj_surf = obj_table[cur_part]->surface;
            break;
        case 2:						// Coller
            if(cur_part>=0 && cur_part<nb_obj()) {
                for(int i = 0; i<obj_table[cur_part]->surface.NbTex;i++) {			// Remove old textures
                    bool current_copy=false;
                    for(int e=0 ; e<obj_surf.NbTex; e++)
                        if(obj_surf.gltex[e]==obj_table[cur_part]->surface.gltex[i]) {
                            current_copy=true;
                            break;
                        }
                    if(!current_copy)
                        glDeleteTextures(1,&obj_table[cur_part]->surface.gltex[i]);
                }
                obj_table[cur_part]->surface = obj_surf;			// copy surface properties
                for(int i = 0; i<obj_surf.NbTex; i++)				// copy the textures
                    obj_table[cur_part]->surface.gltex[i] = copy_tex(obj_surf.gltex[i]);
            }
            break;
        case 3:						// Réinitialiser
            if(obj_table[cur_part]->surface.NbTex>0)
                for(int i=0;i<obj_table[cur_part]->surface.NbTex;i++)
                    if(obj_table[cur_part]->surface.gltex[i])	glDeleteTextures(1,&(obj_table[cur_part]->surface.gltex[i]));
            obj_table[cur_part]->surface.Flag=0;
            for(int i=0;i<4;i++)
                obj_table[cur_part]->surface.Color[i]=obj_table[cur_part]->surface.RColor[i]=1.0f;
            obj_table[cur_part]->surface.NbTex=0;
            for(int i=0;i<8;i++)
                obj_table[cur_part]->surface.gltex[i]=0;
            init_surf_buf();
            break;
        case 4:						// Coller sur toutes
            if(nb_obj()>0)
                for(int e=0;e<nb_obj();e++) {
                    for(int i = 0; i<obj_table[e]->surface.NbTex;i++) {			// Remove old textures
                        bool current_copy=false;
                        for(int f=0 ; f<obj_surf.NbTex; f++)
                            if(obj_surf.gltex[f]==obj_table[cur_part]->surface.gltex[i]) {
                                current_copy=true;
                                break;
                            }
                        if(!current_copy)
                            glDeleteTextures(1,&obj_table[e]->surface.gltex[i]);
                    }
                    obj_table[e]->surface = obj_surf;			// copy surface properties
                    for(int i = 0; i<obj_surf.NbTex; i++)				// copy the textures
                        obj_table[e]->surface.gltex[i] = copy_tex(obj_surf.gltex[i]);
                }
            break;
    };
}

void mnu_selec(int mnu_index)
{
    cur_part=mnu_index;
}

int nb_sub_obj(OBJECT *obj,int *idx,int h)
{
    if(obj==NULL)	return 0;
    obj_table[*idx]=obj;
    h_table[*idx]=h;
    obj->script_index=(*idx)++;
    int n=1+nb_sub_obj(obj->child,idx,h+1);
    return n+nb_sub_obj(obj->next,idx,h);
}

int  nb_obj()						// Compte le nombre d'objets dans le modèle
{
    if(TheModel==NULL)	return 0;

    OBJECT *cur = &(TheModel->obj);
    int idx=0;
    return nb_sub_obj(cur,&idx,0);
}

void convert_to_3dm()
{
    int n=nb_obj();
    for(int i=0;i<n;i++)
        if((obj_table[i]->surface.Flag&SURFACE_ADVANCED)!=SURFACE_ADVANCED) {
            obj_table[i]->surface.Flag|=SURFACE_ADVANCED;
            obj_table[i]->surface.Flag|=SURFACE_LIGHTED;
            for(int e=0;e<4;e++)
                obj_table[i]->surface.Color[e]=obj_table[i]->surface.RColor[e]=1.0f;
            if(obj_table[i]->dtex) {
                obj_table[i]->surface.Flag|=SURFACE_TEXTURED;
                obj_table[i]->surface.NbTex=obj_table[i]->dtex;
                for(int e=0;e<obj_table[i]->dtex;e++)
                    obj_table[i]->surface.gltex[e]=obj_table[i]->gltex[e];
                obj_table[i]->dtex=0;
            }
        }
}

void button_rename(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    free(obj_table[cur_part]->name);
    obj_table[cur_part]->name = strdup( GetVal( TRANSLATE( "Nouveau nom d'objet" ) ).c_str() );
}

bool is_child(OBJECT *obj,int idx)
{
    if(obj->script_index==idx)	return true;
    if(obj->child)
        return is_child(obj->child,idx);
    return false;
}

void button_child(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    char *father_name = strdup( GetVal( TRANSLATE( "Nom du noeud père" ) ).c_str() );
    int father_id=-1;
    if( obj_table[cur_part] == &(TheModel->obj) ) {
        OBJECT obj = TheModel->obj;
        for( int i = 0 ; i < nb_obj() ; i++ )
            if( obj_table[i] == TheModel->obj.next ) {
                cur_part = i;
                break;
            }
        TheModel->obj = *(TheModel->obj.next);
        *(obj.next) = obj;
        obj.next->next = TheModel->obj.next;
        TheModel->obj.next = obj.next;
        obj.init();
    }
    for(int i=0;i<nb_obj();i++)
        if(strcmp(obj_table[i]->name,father_name)==0) {
            father_id=i;
            break;
        }
    free(father_name);

    if(is_child(obj_table[cur_part],father_id)) {
        Popup( TRANSLATE( "Action impossible" ), TRANSLATE( "Le noeud père est un fils du noeud séléctionné" ) );
        return;
    }

    int n=nb_obj();
    for(int i=0;i<n;i++)					// Libère l'objet
        if(obj_table[i]->child==obj_table[cur_part]) {
            obj_table[i]->child=obj_table[cur_part]->next;
            obj_table[cur_part]->next=NULL;
            break;
        }
        else if(obj_table[i]->next==obj_table[cur_part]) {
            obj_table[i]->next=obj_table[cur_part]->next;
            obj_table[cur_part]->next=NULL;
        }
    OBJECT *father = NULL;
    OBJECT *cur = NULL;
    bool child=true;
    if(father_id==-1) {
        father = &(TheModel->obj);
        cur = father->next;
        child = false;
    }
    else {
        father = obj_table[father_id];
        cur = obj_table[father_id]->child;
    }
    while(cur) {
        child=false;
        father=cur;
        cur=cur->next;
    }
    if(child)
        father->child=obj_table[cur_part];
    else
        father->next=obj_table[cur_part];

    cur = obj_table[cur_part];

    n = nb_obj();
    for(int i = 0; i<n;i++)
        if(obj_table[i]==cur) {
            cur_part=i;
            break;
        }
}

void button_remove(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    if( nb_obj() == 1 ) {			// Remove everything if there is only one object
        TheModel->destroy();
        cur_part=0;
        init_surf_buf();
        return;
    }

    OBJECT *old = obj_table[cur_part];
    for(int i=0;i<nb_obj();i++)
        if(obj_table[i]->child == old) {
            obj_table[i]->child = old -> next;
            break;
        }
        else if(obj_table[i]->next == old) {
            obj_table[i]->next = old -> next;
            break;
        }
    OBJECT *old2 = old->next;
    old->next=NULL;
    old->destroy();
    if(old != &(TheModel->obj))		// Si c'est quelque chose qu'on a alloué on le libère
        free(old);
    else {
        *old=*old2;
        free(old2);
    }
    nb_obj();
}

void button_scale(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    float scale_factor = atof( GetVal( TRANSLATE( "Echelle" ) ).c_str() );
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 )
    {
        if( cur == NULL ) {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ )
        {
            cur->points[i].x *= scale_factor;
            cur->points[i].y *= scale_factor;
            cur->points[i].z *= scale_factor;
        }
        if( cur != obj_table[ cur_part ] )
        {
            cur->pos_from_parent.x *= scale_factor;
            cur->pos_from_parent.y *= scale_factor;
            cur->pos_from_parent.z *= scale_factor;
        }
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
}

void button_mirror_x(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 ) {
        if( cur == NULL ) {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ ) {
            cur->points[i].x *= -1.0f;
            cur->N[i].x *= -1.0f;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 ) {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] )
            cur->pos_from_parent.x *= -1.0f;
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
}

void button_mirror_y(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 ) {
        if( cur == NULL ) {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ ) {
            cur->points[i].y *= -1.0f;
            cur->N[i].y *= -1.0f;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 ) {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] )
            cur->pos_from_parent.y *= -1.0f;
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
}

void button_mirror_z(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 ) {
        if( cur == NULL ) {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ ) {
            cur->points[i].z *= -1.0f;
            cur->N[i].z *= -1.0f;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 ) {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] )
            cur->pos_from_parent.z *= -1.0f;
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
}

void button_change_xy(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 ) {
        if( cur == NULL ) {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ ) {
            float x = cur->points[i].x;
            cur->points[i].x = cur->points[i].y;
            cur->points[i].y = x;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 ) {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] ) {
            float x = cur->pos_from_parent.x;
            cur->pos_from_parent.x = cur->pos_from_parent.y;
            cur->pos_from_parent.y = x;
        }
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
    for( int i = 0 ; i < nb_obj() ; i++ )		// MAJ normales
        obj_maj_normal( i );
}

void button_change_yz(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 )
    {
        if( cur == NULL )
        {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ )
        {
            float y = cur->points[i].y;
            cur->points[i].y = cur->points[i].z;
            cur->points[i].z = y;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 )
        {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] )
        {
            float y = cur->pos_from_parent.y;
            cur->pos_from_parent.y = cur->pos_from_parent.z;
            cur->pos_from_parent.z = y;
        }
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
    for( int i = 0 ; i < nb_obj() ; i++ )		// MAJ normales
        obj_maj_normal( i );
}

void button_change_zx(int mnu_index)
{
    if(cur_part<0 || cur_part>=nb_obj())	return;
    OBJECT *cur = obj_table[ cur_part ];
    std::list< OBJECT* >		stack;
    while( cur || stack.size() > 0 )
    {
        if( cur == NULL )
        {
            cur = stack.front();
            stack.pop_front();
        }
        for( int i = 0 ; i < cur->nb_vtx ; i++ )
        {
            float x = cur->points[i].x;
            cur->points[i].x = cur->points[i].z;
            cur->points[i].z = x;
        }
        for( int i = 0 ; i < cur->nb_t_index ; i+= 3 )
        {
            GLushort tmp = cur->t_index[ i ];
            cur->t_index[ i ] = cur->t_index[ i + 1 ];
            cur->t_index[ i + 1 ] = tmp;
        }
        if( cur != obj_table[ cur_part ] ) {
            float x = cur->pos_from_parent.x;
            cur->pos_from_parent.x = cur->pos_from_parent.z;
            cur->pos_from_parent.z = x;
        }
        if( cur != obj_table[ cur_part ] && cur->next )
            stack.push_front( cur->next );
        cur = cur->child;
    }
    for( int i = 0 ; i < nb_obj() ; i++ )		// MAJ normales
        obj_maj_normal( i );
}

GLuint copy_tex(GLuint gltex)
{
    BITMAP *tex;
    GLint w,h,ntex;
    glBindTexture(GL_TEXTURE_2D,gltex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
    tex=create_bitmap_ex(32,w,h);
    glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,tex->line[0]);

    allegro_gl_use_alpha_channel(true);
    ntex = allegro_gl_make_texture(tex);
    allegro_gl_use_alpha_channel(false);

    glBindTexture(GL_TEXTURE_2D,ntex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    destroy_bitmap(tex);
    return ntex;
}

BITMAP *read_tex(GLuint gltex)
{
    BITMAP *tex;
    GLint w,h;
    glBindTexture(GL_TEXTURE_2D,gltex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
    tex=create_bitmap_ex(32,w,h);
    glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,tex->line[0]);

    return tex;
}

BITMAP *read_tex_luminance(GLuint gltex)
{
    BITMAP *tex;
    GLint w,h;
    glBindTexture(GL_TEXTURE_2D,gltex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&h);
    tex=create_bitmap_ex(32,w<<1,h);
    glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_SHORT,tex->line[0]);

    return tex;
}

void obj_maj_normal(int idx)
{
    if(idx<0 || idx>=nb_obj())	return;
    OBJECT *cur = obj_table[idx];
    for(int i=0;i<cur->nb_vtx;i++)
        cur->N[i].x=cur->N[i].z=cur->N[i].y=0.0f;
    for(int i=0;i<cur->nb_t_index;i+=3) {
        VECTOR AB,AC,Normal;
        AB = cur->points[cur->t_index[i+1]] - cur->points[cur->t_index[i]];
        AC = cur->points[cur->t_index[i+2]] - cur->points[cur->t_index[i]];
        Normal=AB*AC;	Normal.unit();
        for(int e=0;e<3;e++)
            cur->N[cur->t_index[i+e]]=cur->N[cur->t_index[i+e]]+Normal;
    }
    for(int i=0;i<cur->nb_vtx;i++)
        cur->N[i].unit();
}

void obj_geo_optimize(int idx,bool notex)
{
    if(idx<0 || idx>=nb_obj())	return;
    obj_geo_split(idx);
    OBJECT *cur = obj_table[idx];
    int removed=0;
    for(int i=0;i<cur->nb_t_index;i++)				// Remove duplicate points
        for(int e=0;e<i;e++)
            if(cur->points[cur->t_index[i]].x==cur->points[cur->t_index[e]].x && cur->points[cur->t_index[i]].y==cur->points[cur->t_index[e]].y && cur->points[cur->t_index[i]].z==cur->points[cur->t_index[e]].z
               && cur->tcoord[cur->t_index[i]<<1]==cur->tcoord[cur->t_index[e]<<1] && cur->tcoord[(cur->t_index[i]<<1)+1]==cur->tcoord[(cur->t_index[e]<<1)+1]) {
                cur->t_index[i]=cur->t_index[e];
                removed++;
                break;
            }
    cur->nb_vtx-=removed;
    if( notex ) {
        VECTOR *n_points = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
        VECTOR *n_N = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
        int cur_pt=0;
        for(int i=0;i<cur->nb_t_index;i++) {
            bool ok=false;
            for(int e=0;e<cur_pt;e++)
                if(cur->points[cur->t_index[i]].x==n_points[e].x && cur->points[cur->t_index[i]].y==n_points[e].y && cur->points[cur->t_index[i]].z==n_points[e].z ) {
                    cur->t_index[i]=e;
                    ok=true;
                    break;
                }
            if(ok)	continue;
            n_points[cur_pt]=cur->points[cur->t_index[i]];
            n_N[cur_pt]=cur->N[cur->t_index[i]];
            cur->t_index[i]=cur_pt++;
        }
        free(cur->points);
        free(cur->N);
        cur->points=n_points;
        cur->N=n_N;
    }
    else {
        VECTOR *n_points = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
        VECTOR *n_N = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_vtx);
        float *n_tcoord = (float*) malloc(sizeof(float)*cur->nb_vtx<<1);
        int cur_pt=0;
        for(int i=0;i<cur->nb_t_index;i++) {
            bool ok=false;
            for(int e=0;e<cur_pt;e++)
                if(cur->points[cur->t_index[i]].x==n_points[e].x && cur->points[cur->t_index[i]].y==n_points[e].y && cur->points[cur->t_index[i]].z==n_points[e].z
                   && cur->tcoord[cur->t_index[i]<<1]==n_tcoord[e<<1] && cur->tcoord[(cur->t_index[i]<<1)+1]==n_tcoord[(e<<1)+1]) {
                    cur->t_index[i]=e;
                    ok=true;
                    break;
                }
            if(ok)	continue;
            n_points[cur_pt]=cur->points[cur->t_index[i]];
            n_N[cur_pt]=cur->N[cur->t_index[i]];
            n_tcoord[cur_pt<<1]=cur->tcoord[cur->t_index[i]<<1];
            n_tcoord[(cur_pt<<1)+1]=cur->tcoord[(cur->t_index[i]<<1)+1];
            cur->t_index[i]=cur_pt++;
        }
        free(cur->points);
        free(cur->N);
        free(cur->tcoord);
        cur->points=n_points;
        cur->N=n_N;
        cur->tcoord=n_tcoord;
    }
}

void obj_geo_split(int idx)
{
    if(idx<0 || idx>=nb_obj())	return;
    OBJECT *cur = obj_table[idx];
    VECTOR *n_points = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_t_index);
    VECTOR *n_N = (VECTOR*) malloc(sizeof(VECTOR)*cur->nb_t_index);
    float *n_tcoord = (float*) malloc(sizeof(float)*cur->nb_t_index<<1);
    for(int i=0;i<cur->nb_t_index;i++) {
        n_points[i]=cur->points[cur->t_index[i]];
        n_N[i]=cur->N[cur->t_index[i]];
        n_tcoord[i<<1]=cur->tcoord[cur->t_index[i]<<1];
        n_tcoord[(i<<1)+1]=cur->tcoord[(cur->t_index[i]<<1)+1];
        cur->t_index[i]=i;
    }
    free(cur->points);
    free(cur->N);
    free(cur->tcoord);
    cur->points=n_points;
    cur->N=n_N;
    cur->tcoord=n_tcoord;
    cur->nb_vtx=cur->nb_t_index;
}

int intersect(VECTOR O,VECTOR Dir,OBJECT *obj,VECTOR *PA,VECTOR *PB)	// Calcule l'intersection d'un rayon avec une partie de la meshe
{
    float mdist=1000000.0f;			// Distance du point de départ du rayon à l'objet
    int index=-1;					// -1 pour aucun triangle touché

    Dir.unit();		// S'assure que Dir est normalisé
    for(int i=0;i<obj->nb_t_index/3;i++) {			// Effectue l'opération pour chaque triangle
        VECTOR A,B,C,P;
        VECTOR AB,AC,N,AO;
        float dist,orient;

        A=obj->points[obj->t_index[i*3]];
        B=obj->points[obj->t_index[i*3+1]];
        C=obj->points[obj->t_index[i*3+2]];

        AB=B-A;	AC=C-A;
        N=AB*AC;								// Calcule un vecteur normal au triangle
        N.unit();			// Normalise ce vecteur

        orient=Dir%N;

        if(orient>=0.0f) continue;		// Si le triangle ne fait pas face au rayon, on le saute

        AO = O-A;

        dist=-(AO%N);		// Calcule la distance de O au triangle
        dist/=orient;		// Calcule la distance de 0 à P(point d'intersection avec le plan du triangle)

        if(dist>=mdist) continue;		// Pas la peinne de faire des calculs si ce triangle est trop loin

        P=O+dist*Dir;		// Calcule les coordonnées du point d'intersection

        if(Dir%(P-O)<0.0f) continue;		// Si le triangle est derrière, on le saute

        // Maintenant il faut vérifier que le point appartient bien au triangle
        float a,b,c;		// Coefficients pour que P soit le barycentre de A,B,C
        VECTOR AP=P-A;
        if(AC.y!=0.0f && AB.x*AC.y!=AB.y*AC.x) {
            b=(AP.x-AP.y*AC.x/AC.y)/(AB.x-AB.y*AC.x/AC.y);
            a=(AP.y-b*AB.y)/AC.y;
        }
        else if(AC.z!=0.0f && AB.x*AC.z!=AB.z*AC.x) {
            b=(AP.x-AP.z*AC.x/AC.z)/(AB.x-AB.z*AC.x/AC.z);
            a=(AP.z-b*AB.z)/AC.z;
        }
        else if(AC.x!=0.0f && AB.y*AC.x!=AB.x*AC.y) {
            b=(AP.y-AP.x*AC.y/AC.x)/(AB.y-AB.x*AC.y/AC.x);
            a=(AP.x-b*AB.x)/AC.x;
        }
        else if(AB.y!=0.0f && AC.x*AB.y!=AC.y*AB.x) {
            a=(AP.x-AP.y*AB.x/AB.y)/(AC.x-AC.y*AB.x/AB.y);
            b=(AP.y-a*AC.y)/AB.y;
        }
        else if(AB.z!=0.0f && AC.x*AB.z!=AC.z*AB.x) {
            a=(AP.x-AP.z*AB.x/AB.z)/(AC.x-AC.z*AB.x/AB.z);
            b=(AP.z-a*AC.z)/AB.z;
        }
        else if(AB.x!=0.0f && AC.y*AB.x!=AC.x*AB.y) {
            a=(AP.y-AP.x*AB.y/AB.x)/(AC.y-AC.x*AB.y/AB.x);
            b=(AP.x-a*AC.x)/AB.x;
        }
        else continue;		// Saute le point s'il n'est pas positionnable
        c=1.0f-a-b;
        if(a<0.0f || b<0.0f || c<0.0f) continue;		// Le point n'appartient pas au triangle

        mdist=dist;		// Mémorise la distance
        index=i;		// Et l'indice du triangle
        *PA=P;			// Ainsi que le point d'intersection
        PB->x=c;		// Et les coefficients
        PB->y=b;
        PB->z=a;
    }
    return index;			// Renvoie le triangle touché
}

/*---------------------------------------------------------------------------------\
  |                                   RGB *LoadPal(..)                               |
  |        Procédure qui charge la palette graphique d'un fichier pal                |
  \---------------------------------------------------------------------------------*/
RGB *LoadPal(const char *filename)
{
    RGB *pal=new RGB[256];		// Génère une nouvelle palette

    byte *palette=HPIManager->PullFromHPI(filename);

    if(palette) {
        for(int i=0;i<256;i++) {
            pal[i].r=palette[i<<2]>>2;
            pal[i].g=palette[(i<<2)+1]>>2;
            pal[i].b=palette[(i<<2)+2]>>2;
        }
        free(palette);
    }

    return pal;
}

void init()
{
    InterfaceManager = NULL;
    VARS::sound_manager = NULL;
    VARS::HPIManager = NULL;
    VARS::Console = NULL;
    VARS::gfx = NULL;

    try
    {
        InterfaceManager = new TA3D::IInterfaceManager();
    }
    catch( ... )
    {
        throw cError( "Init3DMEditor() - Constructing Interface manager", GETSYSERROR(), false );
        return;
    }

    String str = format( "3DMEditor %s initializing started:\n\n", TA3D_ENGINE_VERSION );
    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

    str = format("build info : %s , %s",__DATE__,__TIME__);
    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"Interface manager: succeded\n", NULL, NULL );
    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"Logging interface: succeded\n", NULL, NULL );


    set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters

#ifdef AGL_VERSION
    str = format( "\nInitializing Allegro (%s %s)(%s)\n",
                  ALLEGRO_VERSION_STR, ALLEGRO_DATE_STR, AGL_VERSION_STR );
#else
    str = format( "\nInitializing Allegro(%s %s)(unknown)\n ",
                  ALLEGRO_VERSION_STR, ALLEGRO_DATE_STR );
#endif
    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

    if( allegro_init() != 0 )
    {
        throw cError( "Init3DMEditor()", "allegro_init() yielded unexpected result.", true );
        return;
    }

    switch(os_type)
    {
        case OSTYPE_WIN3:         I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows 3.1\n", NULL, NULL);   break;
        case OSTYPE_WIN95:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows 95\n", NULL, NULL);      break;
        case OSTYPE_WIN98:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows 98\n", NULL, NULL);      break;
        case OSTYPE_WINME:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows ME\n", NULL, NULL);      break;
        case OSTYPE_WINNT:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows NT\n", NULL, NULL);      break;
        case OSTYPE_WIN2000:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows 2000\n", NULL, NULL);  break;
        case OSTYPE_WINXP:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Windows XP\n", NULL, NULL);      break;
        case OSTYPE_LINUX:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : Linux\n", NULL, NULL);        break;
        case OSTYPE_FREEBSD:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : FreeBSD\n", NULL, NULL);         break;
        case OSTYPE_NETBSD:     I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : NetBSD\n", NULL, NULL);         break;
        case OSTYPE_UNIX:      I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : UNIX\n", NULL, NULL);            break;
        default:               I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"OS : unknown\n", NULL, NULL);         break;
    };

#define CPU_MODEL_ATHLON64_N   15

    switch(cpu_family)
    {
        case CPU_FAMILY_I386:      str = "CPU: i386 ";      break;
        case CPU_FAMILY_I486:      str = "CPU: i486 ";      break;
        case CPU_FAMILY_I586:      str = "CPU: i586 ";      break;
        case CPU_FAMILY_I686:      str = "CPU: i686 ";      break;
        case CPU_FAMILY_ITANIUM:   str = "CPU: Itanium ";   break;
        case CPU_FAMILY_POWERPC:   str = "CPU: PPC ";         break;
        case CPU_FAMILY_EXTENDED:
                                   switch(cpu_model)
                                   {
                                       case CPU_MODEL_PENTIUMIV:   str = "CPU: Pentium4 ";     break;
                                       case CPU_MODEL_XEON:      str = "CPU: Xeon ";            break;
                                       case CPU_MODEL_ATHLON64_N:
                                       case CPU_MODEL_ATHLON64:   str = "CPU: Athlon64 ";      break;
                                       case CPU_MODEL_OPTERON:      str = "CPU: Opteron ";     break;
                                       default:               str = "CPU: ??? ";
                                   };
                                   break;
        case CPU_FAMILY_UNKNOWN:   str = "CPU: ??? ";                  break;
    };

    str += cpu_vendor;
    if(cpu_capabilities&CPU_AMD64)      str += "amd64 ";
    if(cpu_capabilities&CPU_IA64)      str += "i64 ";
    if(cpu_capabilities&CPU_ID)         str += "-cpuid";
    if(cpu_capabilities&CPU_FPU)      str += "-x87 FPU";
    if(cpu_capabilities&CPU_MMX)      str += "-MMX";
    if(cpu_capabilities&CPU_MMXPLUS)   str += "-MMX+";
    if(cpu_capabilities&CPU_SSE)      str += "-SSE";
    if(cpu_capabilities&CPU_SSE2)      str += "-SSE2";
    if(cpu_capabilities&CPU_3DNOW)      str += "-3DNow!";
    if(cpu_capabilities&CPU_ENH3DNOW)   str += "-Enhanced 3DNow!";
    if(cpu_capabilities&CPU_CMOV)      str += "-cmov";
    str += "\n";

    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"Starting Allegro timer.\n", NULL, NULL );
    if( install_timer() != 0 )
    {
        throw cError( "Init3DMEditor()", "install_timer() yielded unexpected result.", true );
        return;
    }

    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"Starting allegro mouse handler.\n ", NULL, NULL );
    if( install_mouse() == -1 )
    {
        throw cError( "Init3DMEditor()", "install_mouse() yielded unexpected result.", true );
        return;
    }

    I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void*)"Starting allegro keyboard handler.\n", NULL, NULL );
    if( install_keyboard() == -1 )
    {
        throw cError( "Init3DMEditor()", "install_mouse() yielded unexpected result.", true );
        return;
    }

    jpgalleg_init();

    TA3D::VARS::gfx = new TA3D::Interfaces::GFX;       // Creates the gfx object

    TA3D::VARS::Console=new TA3D::TA3D_DEBUG::cConsole(TA3D::Paths::Logs + "Console3DM.log" );   // Create console object, this will be dropped soon
    i18n.load_translations("3dmeditor.res");   // create translation object.
    TA3D::VARS::HPIManager = new TA3D::UTILS::HPI::cHPIHandler( GetClientPath() ); // create hpi manager object.

    set_window_title("3DMEditor - TA3D Project");
}
