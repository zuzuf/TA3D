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
  |                      EngineClass.cpp                      |
  |    Contient toutes les classes nécessaires au moteur 3D   |
  | et au moteur physique.                                    |
  |                                                           |
  \----------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "scripts/cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"
#include "UnitEngine.h"
#include "gfx/fx.h"
#include "misc/lzw.h"					// Support for LZW compression
#include <list>
#include "misc/math.h"
#include "logs/logs.h"

byte player_color_map[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };


float	player_color[30]=
{	0.11f,	0.28f,	0.91f,
    0.83f,	0.17f,	0.0f,
    1.0f,	1.0f,	1.0f,
    0.11f,	0.62f,	0.07f,
    0.03f,	0.12f,	0.48f,
    0.5f,	0.34f,	0.62f,
    1.0f,	1.0f,	0.0f,
    0.0f,	0.0f,	0.0f,
    0.61f,	0.8f,	0.87f,
    0.67f,	0.67f,	0.51f
};





namespace TA3D
{

    PLAYERS	players;		// Objet contenant les données sur les joueurs

    int NB_PLAYERS;

    MAP *the_map=NULL;

    void MAP::clean_map()		// Used to remove all objects when loading a saved game
    {
        for( int y = 0 ; y < bloc_h_db ; y++ )
            for( int x = 0 ; x < bloc_w_db ; x++ ) {
                map_data[y][x].stuff = -1;
                map_data[y][x].unit_idx = -1;
            }
    }

    void MAP::destroy()
    {
        /*------------- Experimental: code for new map format -----------------------*/
        /*	if(macro_bloc) {
            for(uint32 y=0;y<macro_h;y++)
            for(uint32 x=0;x<macro_w;x++)
            macro_bloc[y][x].destroy();
            delete[] macro_bloc[0];
            delete[] macro_bloc;
            macro_w=0;
            macro_h=0;
            }*/
        /*---------------------------------------------------------------------------*/

        if( view_map )		destroy_bitmap( view_map );
        if( sight_map )		destroy_bitmap( sight_map );
        if( radar_map )		destroy_bitmap( radar_map );
        if( sonar_map )		destroy_bitmap( sonar_map );

        detail_shader.destroy();
        gfx->destroy_texture( details_tex );

        if(low_vtx)			free(low_vtx);
        if(low_vtx_flat)	free(low_vtx_flat);
        if(low_tcoord)		free(low_tcoord);
        if(low_col)			free(low_col);
        if(low_index)		free(low_index);
        if(low_tex)			glDeleteTextures(1,&low_tex);

        ota_data.destroy();
        gfx->destroy_texture( lava_map );
        if(path && bloc_w && bloc_h) {
            free(path[0]);
            free(path);
        }
        if(view && bloc_w && bloc_h) {
            free(view[0]);
            free(view);
        }
        if(map_data && bloc_w && bloc_h) {
            free(map_data[0]);
            free(map_data);
        }
        if(ph_map && bloc_w && bloc_h) {
            free(ph_map[0]);		// la carte est allouée d'un seul bloc
            free(ph_map);
        }
        if(ph_map_2 && bloc_w && bloc_h) {
            free(ph_map_2[0]);		// la carte est allouée d'un seul bloc
            free(ph_map_2);
        }
        if(h_map && bloc_w && bloc_h) {
            free(h_map[0]);		// la carte est allouée d'un seul bloc
            free(h_map);
        }
        if(bmap && bloc_w && bloc_h) {
            free(bmap[0]);		// la carte est allouée d'un seul bloc
            free(bmap);
        }
        if(ntex>0) {
            for(int i=0;i<ntex;i++)
                gfx->destroy_texture( tex[i] );
            free(tex);
        }
        if(lvl) {
            for(int i=0;i<bloc_h*bloc_w;i++)
                free(lvl[i]);
            free(lvl);
        }
        if(bloc && nbbloc>0) {
            for(int i=0;i<nbbloc;i++) {
                bloc[i].point=NULL;
                bloc[i].destroy();
            }
            free(bloc);
        }
        if(mini) {
            gfx->destroy_texture( glmini );
            destroy_bitmap(mini);
        }
        init();
        detail_shader.destroy();		// Because init will reload it

        the_map = NULL;
    }

    void MAP::clear_FOW( sint8 FOW_flags )
    {
        if( FOW_flags < 0 )	FOW_flags = fog_of_war;
        fog_of_war = FOW_flags;

        if( fog_of_war & FOW_BLACK )
            memset( view_map->line[0], 0, view_map->w * view_map->h );
        else
            memset( view_map->line[0], 0xFF, view_map->w * view_map->h );
        if( fog_of_war & FOW_GREY )
            memset( sight_map->line[0], 0, sight_map->w * sight_map->h );
        else
            memset( sight_map->line[0], 0xFF, sight_map->w * sight_map->h );

        if( fog_of_war == FOW_DISABLED ) {
            memset( radar_map->line[0], 0xFF, radar_map->w * radar_map->h );
            memset( sonar_map->line[0], 0xFF, sonar_map->w * sonar_map->h );
        }
    }

    void MAP::load_details_texture( const String &filename )
    {
        set_color_depth( 32 );
        BITMAP *tex = load_bitmap( filename.c_str(), NULL );
        if( tex ) {
            uint32 average = 0;
            for( int y = 0 ; y < tex->h ; y++ )
                for( int x = 0 ; x < tex->w ; x++ )
                    average += tex->line[y][(x<<2)] + tex->line[y][(x<<2)+1] + tex->line[y][(x<<2)+2];
            average /= tex->w*tex->h*3;
            if( average == 0 )	average = 1;
            color_factor = 255.0f / average;
            details_tex = gfx->make_texture( tex, FILTER_TRILINEAR, false );
            destroy_bitmap( tex );
        }
        else {
            details_tex = 0;
            color_factor = 1.0f;
        }
    }

    void MAP_OTA::load( String filename )
    {
        uint32 ota_file_size = 0;
        byte *data = HPIManager->PullFromHPI( filename, &ota_file_size );
        if( data ) {
            load( (char*)data, ota_file_size );
            delete[] data;
        }
    }

    void MAP_OTA::load(char *data,int ota_size)
    {
        destroy();
        set_uformat(U_ASCII);
        char *pos=data;
        char *ligne=NULL;
        int nb=0;
        int index=0;
        char *limit=data+ota_size;
        char *f;
        int n_pos=0;
        do {
            nb++;
            if(ligne)
                delete[] ligne;
            ligne=get_line(pos);
            strlwr(ligne);
            while(pos[0]!=0 && pos[0]!=13 && pos[0]!=10)	{	pos++;	n_pos++;	}
            while(pos[0]==13 || pos[0]==10)	{	pos++;	n_pos++;	}

            if(strstr(ligne,"//") || strstr(ligne,"/*") || strstr(ligne,"{")) { }		// Saute les commentaires
            else if((f=strstr(ligne,"missionname="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                missionname=strdup(f+12);
            }
            else if((f=strstr(ligne,"planet="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                planet=strdup(f+7);
            }
            else if((f=strstr(ligne,"glamour="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                glamour=strdup(f+8);
            }
            else if((f=strstr(ligne,"missiondescription="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                missiondescription=strdup(f+19);
                char *prec=missiondescription;
                char *cur=prec;
                int nb=0;
                while(cur[0]) {
                    if(cur[0]=='\t')	cur[0] = ' ';
                    if(cur[0]==' ')		prec=cur;
                    nb++;
                    if(nb>34) {
                        prec[0]='\n';
                        nb=cur-prec;
                    }
                    cur++;
                }
            }
            else if((f=strstr(ligne,"tidalstrength="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                tidalstrength=atoi(f+14);
            }
            else if((f=strstr(ligne,"solarstrength="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                solarstrength=atoi(f+14);
            }
            else if((f=strstr(ligne,"lavaworld="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                lavaworld=(f[10]=='1');
            }
            else if((f=strstr(ligne,"killmul="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                killmul=atoi(f+8);
            }
            else if((f=strstr(ligne,"minwindspeed="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                minwindspeed=atoi(f+13);
            }
            else if((f=strstr(ligne,"maxwindspeed="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                maxwindspeed=atoi(f+13);
            }
            else if((f=strstr(ligne,"gravity="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                gravity=atoi(f+8)*0.1f;
            }
            else if((f=strstr(ligne,"numplayers="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                numplayers=strdup(f+11);
            }
            else if((f=strstr(ligne,"size="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                map_size=strdup(f+5);
            }
            else if((f=strstr(ligne,"surfacemetal="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                SurfaceMetal=atoi(f+13);
            }
            else if((f=strstr(ligne,"mohometal="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                MohoMetal=atoi(f+10);
            }
            else if((f=strstr(ligne,"specialwhat=startpos"))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                index=atoi(f+20)-1;
            }
            else if((f=strstr(ligne,"xpos="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                startX[index]=atoi(f+5);
            }
            else if((f=strstr(ligne,"zpos="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                startZ[index]=atoi(f+5);
            }
            else if((f=strstr(ligne,"waterdoesdamage="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                waterdoesdamage=atoi(f+16)>0;
            }
            else if((f=strstr(ligne,"waterdamage="))) {
                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
                waterdamage=atoi(f+12);
            }
            else if((f=strstr(ligne,"missionhint="))) {}
            else if((f=strstr(ligne,"brief="))) {}
            else if((f=strstr(ligne,"narration="))) {}
            else if((f=strstr(ligne,"glamour="))) {}
            else if((f=strstr(ligne,"lineofsight="))) {}
            else if((f=strstr(ligne,"mapping="))) {}
            else if((f=strstr(ligne,"timemul="))) {}
            else if((f=strstr(ligne,"memory="))) {}
            else if((f=strstr(ligne,"useonlyunits="))) {}
            else if((f=strstr(ligne,"schemacount="))) {}
            else if((f=strstr(ligne,"type="))) {
                if( strstr( f+5, "network") )
                    network = true;
            }
            else if((f=strstr(ligne,"aiprofile="))) {}
            else if((f=strstr(ligne,"humanmetal="))) {}
            else if((f=strstr(ligne,"computermetal="))) {}
            else if((f=strstr(ligne,"humanenergy="))) {}
            else if((f=strstr(ligne,"computerenergy="))) {}
            else if((f=strstr(ligne,"meteorweapon="))) {}
            else if((f=strstr(ligne,"meteorradius="))) {}
            else if((f=strstr(ligne,"meteordensity="))) {}
            else if((f=strstr(ligne,"meteorduration="))) {}
            else if((f=strstr(ligne,"meteorinterval="))) {}
            else if((f=strstr(ligne,"killenemycommander="))) {}
            else if((f=strstr(ligne,"destroyallunits="))) {}
            else if((f=strstr(ligne,"allunitskilled="))) {}
            else if((f=strstr(ligne,"featurename="))) {}

        } while(nb<1000 && pos<limit);
        delete[] ligne;
        ligne=NULL;
        if(waterdamage==0)
            waterdoesdamage=false;
    }

    void MAP::draw_mini(int x1,int y1,int w,int h, Camera* cam, byte player_mask)			// Draw the mini-map
    {
        if(!mini) return;		// Check if it exists

        gfx->set_color( 0xFFFFFFFF );

        int rw=w*mini_w/252;
        int rh=h*mini_h/252;
        x1+=w-rw>>1;
        y1+=h-rh>>1;
        float lw=mini_w/252.0f;
        float lh=mini_h/252.0f;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,glmini);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);	glVertex2f(x1,y1);
        glTexCoord2f(lw,0.0f);		glVertex2f(x1+rw,y1);
        glTexCoord2f(lw,lh);		glVertex2f(x1+rw,y1+rh);
        glTexCoord2f(0.0f,lh);		glVertex2f(x1,y1+rh);
        glEnd();

        if( fog_of_war != FOW_DISABLED )
        {
            glEnable( GL_BLEND );
            glBlendFunc( GL_ZERO, GL_SRC_COLOR );			// Special blending function
            glDisable( GL_TEXTURE_2D );
            glBegin( GL_LINES );

            int MY = 0;
            int DY = 0x10000 * ( bloc_h_db - 2 ) / rh;
            int DX = 0x10000 * ( bloc_w_db - 2 ) / rw;

            gfx->lock();

            for( int y = 0 ; y < rh ; y++ )
            {
                int my = MY >> 17;
                MY += DY;
                int old_col = -1;
                int old_x = -1;
                int MX = 0;
                for( int x = 0 ; x < rw ; x++ )
                {
                    int mx = MX >> 17;
                    MX += DX;
                    if(!(view_map->line[my][mx]&player_mask))
                    {
                        if( old_col != 0 )
                        {
                            if( old_x != -1 )
                            {
                                glVertex2i( x1+old_x, y1+y );
                                glVertex2i( x1+x, y1+y );
                            }
                            glColor3f( 0.0f, 0.0f, 0.0f );
                            old_col = 0;
                            old_x = x;
                        }
                    }
                    else
                        if(!(sight_map->line[my][mx] & player_mask))
                        {
                            if( old_col != 1 )
                            {
                                if( old_x != -1 )
                                {
                                    glVertex2i( x1+old_x, y1+y );
                                    glVertex2i( x1+x, y1+y );
                                }
                                glColor3f( 0.5f, 0.5f, 0.5f );
                                old_x = x;
                                old_col = 1;
                            }
                        }
                        else 
                            if( old_col != 2 )
                            {
                                if( old_x != -1 )
                                {
                                    glVertex2i( x1+old_x, y1+y );
                                    glVertex2i( x1+x, y1+y );
                                }
                                old_x = -1;
                                old_col = 2;
                            }
                }
                if( old_x != -1 )
                {
                    glVertex2i( x1+old_x, y1+y );
                    glVertex2i( x1+rw, y1+y );
                }
            }

            glEnd();
            glDisable( GL_BLEND );

            gfx->unlock();
        }

        if(!cam)
            return;

        Vector3D A,B,C,D,P;
        A=cam->dir + cam->widthFactor*(-cam->side)-0.75f*cam->up;
        B=cam->dir + cam->widthFactor*(cam->side)-0.75f*cam->up;
        C=cam->dir + cam->widthFactor*(cam->side)+0.75f*cam->up;
        D=cam->dir + cam->widthFactor*(-cam->side)+0.75f*cam->up;
        const int nmax=64;
        float cx[4*nmax+4],cy[4*nmax+4];
        if(A.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(A.y)*A;	cx[0]=P.x;	cy[0]=P.z; }
        else {
            P=cam->pos+10000.0f*A;	cx[0]=P.x;	cy[0]=P.z; }
        if(B.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(B.y)*B;	cx[1]=P.x;	cy[1]=P.z; }
        else {
            P=cam->pos+10000.0f*B;	cx[1]=P.x;	cy[1]=P.z; }
        if(C.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(C.y)*C;	cx[2]=P.x;	cy[2]=P.z; }
        else {
            P=cam->pos+10000.0f*C;	cx[2]=P.x;	cy[2]=P.z; }
        if(D.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(D.y)*D;	cx[3]=P.x;	cy[3]=P.z; }
        else {
            P=cam->pos+10000.0f*D;	cx[3]=P.x;	cy[3]=P.z; }

        int i;
        for(i=0;i<4;i++)
        {
            cx[i]=(cx[i]+0.5f*map_w)*rw/map_w;
            cy[i]=(cy[i]+0.5f*map_h)*rh/map_h;
        }
        for(i=0;i<4;i++)
        {
            for(int e=0;e<nmax;e++)
            {
                cx[i*nmax+e+4]=(cx[i]*(nmax-e)+cx[(i+1)%4]*(e+1))/(nmax+1);
                cy[i*nmax+e+4]=(cy[i]*(nmax-e)+cy[(i+1)%4]*(e+1))/(nmax+1);
            }
        }
        for (i=0;i<4+(nmax<<2); ++i)
        {
            if(cx[i]<0.0f) cx[i]=0.0f;
            else
                if(cx[i]>rw) cx[i]=rw;
            if(cy[i]<0.0f) cy[i]=0.0f;
            else
                if(cy[i]>rh) cy[i]=rh;
        }

        glDisable(GL_TEXTURE_2D);
        glColor3f(0.9f,0.9f,0.4f);
        glBegin(GL_LINE_STRIP);
        for(i=0;i<4;i++)
        {
            glVertex2f(cx[i]+x1,cy[i]+y1);
            for(int e=0;e<nmax;e++)
                glVertex2f(x1+cx[i*nmax+e+4],y1+cy[i*nmax+e+4]);
        }
        glVertex2f(cx[0]+x1,cy[0]+y1);
        glEnd();
        glColor3f(1.0f,1.0f,1.0f);
        glEnable(GL_TEXTURE_2D);
    }


    void MAP::update_player_visibility( int player_id, int px, int py, int r, int rd, int sn, int rd_j, int sn_j, bool jamming, bool black )
    {
        gfx->lock();

        px >>= 1;
        py >>= 1;
        sn >>= 1;
        r >>= 1;
        rd >>= 1;
        rd_j >>= 1;
        sn_j >>= 1;

        // Update jamming maps
        if( jamming )
        {
            if( rd_j > 0 )
                circlefill( radar_map, px, py, rd_j, 0 );
            if( sn_j > 0 )
                circlefill( sonar_map, px, py, sn_j, 0 );
        }
        else
        {
            byte mask = 1 << player_id;
            uint32 mask32 = 0x01010101 << player_id;
            int r2 = r * r;
            int rd2 = rd * rd;
            int sn2 = sn * sn;
            // Update detector maps
            if( sn > 0 )
                for( int y=0 ; y <= sn; y++ ) {			// Update sonar data
                    int x=(int)(0.5f+sqrt((float)(sn2-y*y)));
                    int ry=py-y;
                    if(ry>=0 && ry<sonar_map->h) {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=sonar_map->w)	lx=sonar_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            sonar_map->line[ry][rx] |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            ((uint32*)(sonar_map->line[ry]))[rx] |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            sonar_map->line[ry][rx] |= mask;
                    }
                    if(y!=0) {
                        ry=py+y;
                        if(ry>=0 && ry<sonar_map->h) {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=sonar_map->w)	lx=sonar_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                sonar_map->line[ry][rx] |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                ((uint32*)(sonar_map->line[ry]))[rx] |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                sonar_map->line[ry][rx] |= mask;
                        }
                    }
                }
            if( rd > 0 )
                for( int y=0 ; y <= rd; y++ ) {			// Update radar data
                    int x=(int)(0.5f+sqrt((float)(rd2-y*y)));
                    int ry=py-y;
                    if(ry>=0 && ry<radar_map->h) {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=radar_map->w)	lx=radar_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            radar_map->line[ry][rx] |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            ((uint32*)(radar_map->line[ry]))[rx] |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            radar_map->line[ry][rx] |= mask;
                    }
                    if(y!=0) {
                        ry=py+y;
                        if(ry>=0 && ry<radar_map->h) {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=radar_map->w)	lx=radar_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                radar_map->line[ry][rx] |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                ((uint32*)(radar_map->line[ry]))[rx] |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                radar_map->line[ry][rx] |= mask;
                        }
                    }
                }
            if( fog_of_war & FOW_GREY )
                for(int y=0;y<=r;y++) {			// Update view data
                    int x=(int)(0.5f+sqrt((float)(r2-y*y)));
                    int ry=py-y;
                    if(ry>=0 && ry<sight_map->h) {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=sight_map->w)	lx=sight_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            sight_map->line[ry][rx] |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            ((uint32*)(sight_map->line[ry]))[rx] |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            sight_map->line[ry][rx] |= mask;
                    }
                    if(y!=0) {
                        ry=py+y;
                        if(ry>=0 && ry<sight_map->h) {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=sight_map->w)	lx=sight_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                sight_map->line[ry][rx] |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                ((uint32*)(sight_map->line[ry]))[rx] |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                sight_map->line[ry][rx] |= mask;
                        }
                    }
                }
            if( black && (fog_of_war & FOW_BLACK) )
                for(int y=0;y<=r;y++) {			// Update view data
                    int x=(int)(0.5f+sqrt((float)(r2-y*y)));
                    int ry=py-y;
                    if(ry>=0 && ry<view_map->h) {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=view_map->w)	lx=view_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            view_map->line[ry][rx] |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            ((uint32*)(view_map->line[ry]))[rx] |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            view_map->line[ry][rx] |= mask;
                    }
                    if(y!=0) {
                        ry=py+y;
                        if(ry>=0 && ry<view_map->h) {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=view_map->w)	lx=view_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                view_map->line[ry][rx] |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                ((uint32*)(view_map->line[ry]))[rx] |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                view_map->line[ry][rx] |= mask;
                        }
                    }
                }
        }
        gfx->unlock();
    }

    inline float sq( float a )	{	return a * a;	}


    void MAP::draw(Camera* cam,byte player_mask,bool FLAT,float niv,float t,float dt,bool depth_only,bool check_visibility,bool draw_uw)
    {
        cam->setView();
        if(FLAT && !water)	return;

        gfx->lock();

        if(FLAT)
            glTranslatef(0.0f,0.0f,sea_dec);
        int i,x,y;
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        if(!FLAT)
        {
            if(ntex>0)
                gfx->ReInitAllTex( true );
            else
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        Vector3D A,B,C,D,P;
        A=cam->dir + cam->widthFactor * (-cam->side)-0.75f*cam->up;
        B=cam->dir + cam->widthFactor * (cam->side)-0.75f*cam->up;
        C=cam->dir + cam->widthFactor * (cam->side)+0.75f*cam->up;
        D=cam->dir + cam->widthFactor * (-cam->side)+0.75f*cam->up;
        float cx[4],cy[4];
        if(A.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(A.y)*A;	cx[0]=P.x;	cy[0]=P.z; }
        else {
            P=cam->pos+10000.0f*A;	cx[0]=P.x;	cy[0]=P.z; }
        if(B.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(B.y)*B;	cx[1]=P.x;	cy[1]=P.z; }
        else {
            P=cam->pos+10000.0f*B;	cx[1]=P.x;	cy[1]=P.z; }
        if(C.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(C.y)*C;	cx[2]=P.x;	cy[2]=P.z; }
        else {
            P=cam->pos+10000.0f*C;	cx[2]=P.x;	cy[2]=P.z; }
        if(D.y<0.0f) {
            P=cam->pos+cam->pos.y/fabs(D.y)*D;	cx[3]=P.x;	cy[3]=P.z; }
        else {
            P=cam->pos+10000.0f*D;	cx[3]=P.x;	cy[3]=P.z; }

        int minx=bloc_w<<1,maxx=0;
        int miny=bloc_h<<1,maxy=0;
        for(i=0;i<4;i++)
        {
            cx[i]=(cx[i]+0.5f*map_w)/16.0f;
            cy[i]=(cy[i]+0.5f*map_h)/16.0f;
            if(cx[i]<minx) minx=(int)cx[i];
            if(cx[i]>maxx) maxx=(int)cx[i];
            if(cy[i]<miny) miny=(int)cy[i];
            if(cy[i]>maxy) maxy=(int)cy[i];
        }
        int y1=bloc_h,y2=0,x1=bloc_w,x2=0,mx,my;
        float limit=cam->zfar*sqrt(2.0f);
        x1=(int)((cam->pos.x+0.5f*map_w-limit)/16.0f);
        y1=(int)((cam->pos.z+0.5f*map_h-limit)/16.0f);
        x2=(int)((cam->pos.x+0.5f*map_w+limit)/16.0f);
        y2=(int)((cam->pos.z+0.5f*map_h+limit)/16.0f);
        mx=(int)((cam->pos.x+0.5f*map_w)/16.0f);
        my=(int)((cam->pos.z+0.5f*map_h)/16.0f);
        if(x1<minx)
            x1=minx;
        if(x2>maxx)
            x2=maxx;
        if(y1<miny)
            y1=miny;
        if(y2>maxy)
            y2=maxy;
        if(x1>mx) x1=mx;
        if(y1>my) y1=my;
        if(x2<mx) x2=mx;
        if(y2<my) y2=my;

        if((abs(my-y2)<<4)>cam->zfar+64.0f)
            y2 = my>y2 ? my - (int)(cam->zfar/16.0f)-4 : my + 4 + (int)(cam->zfar/16.0f);
        if((abs(my-y1)<<4)>cam->zfar+64.0f)
            y1 = my>y1 ? my - (int)(cam->zfar/16.0f)-4 : my + 4 + (int)(cam->zfar/16.0f);
        if((abs(mx-x2)<<4)>cam->zfar+64.0f)
            x2 = mx>x2 ? mx - (int)(cam->zfar/16.0f)-4 : mx + 4 + (int)(cam->zfar/16.0f);
        if((abs(mx-x1)<<4)>cam->zfar+64.0f)
            x1 = mx>x1 ? mx - (int)(cam->zfar/16.0f)-4 : mx + 4 + (int)(cam->zfar/16.0f);

        if(y1<0) y1=0;
        if(y2<0) y2=0;
        if(y1>=bloc_h) y1=bloc_h-1;
        if(y2>=bloc_h) y2=bloc_h-1;
        if(x1<0) x1=0;
        if(x2<0) x2=0;
        if(x1>=bloc_w) x1=bloc_w-1;
        if(x2>=bloc_w) x2=bloc_w-1;

        y1-=3;
        if(y1<0) y1=0;

        A=(cam->dir+0.75f*cam->up-cam->widthFactor*cam->side);	A.unit();
        float ref = sq( 0.95f*(A%cam->dir) );
        float dhm=0.5f*map_h;
        float dwm=0.5f*map_w;

        if(!FLAT)
            glColor4f(1.0f,1.0f,1.0f,1.0f);

        Vector3D flat[9];
        if(FLAT)
        {
            flat[0].x=0.0f;		flat[0].y=niv+cos(t)*0.5f;			flat[0].z=0.0f;
            flat[1].x=8.0f;		flat[1].y=niv+cos(t+1.0f)*0.5f;		flat[1].z=0.0f;
            flat[2].x=16.0f;	flat[2].y=flat[0].y;				flat[2].z=0.0f;
            flat[3].x=0.0f;		flat[3].y=niv+cos(t+1.5f)*0.5f;		flat[3].z=8.0f;
            flat[4].x=8.0f;		flat[4].y=niv+cos(t+2.5f)*0.5f;		flat[4].z=8.0f;
            flat[5].x=16.0f;	flat[5].y=flat[3].y;				flat[5].z=8.0f;
            flat[6].x=0.0f;		flat[6].y=flat[0].y;				flat[6].z=16.0f;
            flat[7].x=8.0f;		flat[7].y=flat[1].y;				flat[7].z=16.0f;
            flat[8].x=16.0f;	flat[8].y=flat[0].y;				flat[8].z=16.0f;
        }

        bool enable_details = !cam->mirror;

        if(ntex>0 && !depth_only)
        {
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            if( lp_CONFIG->detail_tex && !FLAT && enable_details )
            {
                glClientActiveTextureARB(GL_TEXTURE1_ARB);
                glActiveTextureARB(GL_TEXTURE1_ARB );
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, details_tex );
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glActiveTextureARB(GL_TEXTURE0_ARB );
            }
        }

        if(FLAT)
            glTranslatef(cos(t),0.0f,sin(t));
        GLuint old_tex=bloc[0].tex;
        if(!depth_only)
            glBindTexture(GL_TEXTURE_2D,old_tex);
        if(!FLAT && check_visibility) {
            for(y=oy1;y<=oy2;y++)
                memset(view[y]+ox1,0,ox2-ox1+1);
            features.min_idx=features.nb_features-1;
            features.max_idx=0;
            features.list_size=0;
            ox1=x1;	ox2=x2;
            oy1=y1;	oy2=y2;
        }
        else if(!check_visibility) {
            x1=ox1;	x2=ox2;
            y1=oy1;	y2=oy2;
            for(int i=0;i<features.list_size;i++)
                features.feature[features.list[i]].draw=true;
        }
        int lavaprob=(int)(1000*dt);
        Vector3D	buf_p[4500];				// Tampon qui accumule les blocs pour les dessiner en chaîne
        float	buf_t[9000];
        uint8	buf_c[18000];
        short	buf_size=0;				// in blocs
        uint16	index_size=0;
        bool	was_flat=false;
        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
        glColorPointer(4,GL_UNSIGNED_BYTE,0,buf_c);
        glVertexPointer( 3, GL_FLOAT, 0, buf_p);

        if( lp_CONFIG->detail_tex && !FLAT && enable_details ) {
            detail_shader.on();
            detail_shader.setvar1f( "coef", color_factor );
            detail_shader.setvar1i( "details", 1 );
        }

        glClientActiveTextureARB(GL_TEXTURE0_ARB );
        glTexCoordPointer(2, GL_FLOAT, 0, buf_t);

        int	ox=x1;

        bool low_def_view = cam->rpos.y>gfx->low_def_limit;		// Low detail map for mega zoom
        if(low_def_view) {							// draw the low detail map
            detail_shader.off();
            glActiveTextureARB(GL_TEXTURE1_ARB );
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE0_ARB );

            i=0;
            for(y=0;y<=low_h;y++) {
                int Y=y*(bloc_h_db-2)/low_h;
                for(x=0;x<=low_w;x++) {
                    int X=x*(bloc_w_db-2)/low_w;
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    if(!(view_map->line[Z>>1][X>>1]&player_mask))	low_col[i<<2]=low_col[(i<<2)+1]=low_col[(i<<2)+2]=low_col[(i<<2)+3]=0;
                    else {
                        low_col[(i<<2)+3] = 255;
                        if(!(sight_map->line[Z>>1][X>>1]&player_mask))				low_col[i<<2]=low_col[(i<<2)+1]=low_col[(i<<2)+2]=127;
                        else									low_col[i<<2]=low_col[(i<<2)+1]=low_col[(i<<2)+2]=255;
                    }
                    i++;
                }
            }
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4,GL_UNSIGNED_BYTE,0,low_col);
            if(FLAT) {
                glTranslatef(0.0f,niv,0.0f);
                glVertexPointer( 3, GL_FLOAT, 0, low_vtx_flat);
            }
            else
                glVertexPointer( 3, GL_FLOAT, 0, low_vtx);
            glTexCoordPointer(2, GL_FLOAT, 0, low_tcoord);
            glBindTexture(GL_TEXTURE_2D,low_tex);
            glDrawElements(GL_TRIANGLE_STRIP, low_nb_idx,GL_UNSIGNED_INT,low_index);		// draw this map
        }

        if(cam->rpos.y>=900.0f) {
            memset(view[0],1,bloc_w*bloc_h);
            ox1=0;
            ox2=bloc_w-1;
            oy1=0;
            oy2=bloc_h-1;
        }

        Vector3D T;
        Vector3D	V;
        if(cam->rpos.y<900.0f)
            for(y=y1;y<=y2;y++) // Balaye les blocs susceptibles d'être visibles pour dessiner ceux qui le sont
            {
                int pre_y=y<<4;
                int Y=y<<1;
                int pre_y2=y*bloc_w;
                T.x=-dwm;	T.y=0.0f;	T.z=pre_y-dhm;
                buf_size=0;
                ox=x1;
                bool was_clean = false;

                int rx1 = x1;
                int rx2 = x2;

                if( !FLAT && check_visibility )
                {
                    for( ; rx1 <= x2 ; rx1++ )
                    {
                        int X = rx1<<1;
                        V.x = (rx1<<4) - dwm;
                        V.y = ph_map[Y|1][X|1];
                        V.z = pre_y - dhm + get_zdec_notest(X|1,Y|1);
                        V = V - cam->pos;
                        if(fabs(V%cam->dir) > cam->zfar)
                        {
                            view[y][rx1] = 0;
                            continue;
                        }
                        float d = V.sq();
                        if(d > 16384.0f)
                            if(sq(V % cam->dir) < ref * d)
                            {
                                view[y][rx1] = 0;
                                continue;
                            }
                        break;
                    }
                    for( ; rx2 >= rx1 ; rx2-- )
                    {
                        int X = rx2<<1;
                        V.x = (rx2<<4) - dwm;
                        V.y = ph_map[Y|1][X|1];
                        V.z = pre_y - dhm + get_zdec_notest(X|1,Y|1);
                        V = V - cam->pos;
                        if(fabs(V % cam->dir) > cam->zfar) {
                            view[y][rx2] = 0;
                            continue;
                        }
                        float d = V.sq();
                        if(d > 16384.0f)
                            if(sq(V % cam->dir) < ref * d) {
                                view[y][rx2] = 0;
                                continue;
                            }
                        break;
                    }
                }

                for(x=rx1;x<=rx2;x++)
                {
                    int X=x<<1;
                    if(!FLAT && check_visibility)
                    {
                        if(!(view_map->line[y][x]&player_mask))
                        {
                            if(water)
                            {
                                if(map_data[Y][X].underwater && map_data[Y|1][X].underwater && map_data[Y][X|1].underwater && map_data[Y|1][X|1].underwater)
                                    view[y][x]=2;
                                else
                                    view[y][x]=3;
                            }
                        }
                        else
                        {
                            if(!(sight_map->line[y][x]&player_mask))
                            {
                                if(map_data[Y][X].underwater || map_data[Y|1][X].underwater || map_data[Y][X|1].underwater || map_data[Y|1][X|1].underwater)
                                    view[y][x]=2;
                                else
                                    view[y][x]=3;
                            }
                            else
                                view[y][x]=1;
                            if(map_data[Y][X].stuff>=0 && map_data[Y][X].stuff<features.max_features) // Indique comme affichables les objets présents sur le bloc
                            {
                                if(features.feature[map_data[Y][X].stuff].type<0)
                                    map_data[Y][X].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y][X].stuff].draw=true;
                                    features.feature[map_data[Y][X].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y][X].stuff;
                                }
                            }
                            if(map_data[Y][X|1].stuff>=0 && map_data[Y][X|1].stuff<features.max_features)
                            {
                                if(features.feature[map_data[Y][X|1].stuff].type<0)
                                    map_data[Y][X|1].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y][X|1].stuff].draw=true;
                                    features.feature[map_data[Y][X|1].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y][X|1].stuff;
                                }
                            }
                            if(map_data[Y|1][X].stuff>=0 && map_data[Y|1][X].stuff<features.max_features)
                            {
                                if(features.feature[map_data[Y|1][X].stuff].type<0)
                                    map_data[Y|1][X].stuff=-1;
                                else {
                                    features.feature[map_data[Y|1][X].stuff].draw=true;
                                    features.feature[map_data[Y|1][X].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y|1][X].stuff;
                                }
                            }
                            if(map_data[Y|1][X|1].stuff>=0 && map_data[Y|1][X|1].stuff<features.max_features) {
                                if(features.feature[map_data[Y|1][X|1].stuff].type<0)
                                    map_data[Y|1][X|1].stuff=-1;
                                else {
                                    features.feature[map_data[Y|1][X|1].stuff].draw=true;
                                    features.feature[map_data[Y|1][X|1].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y|1][X|1].stuff;
                                }
                            }
                        }
                    }
                    else {
                        if(view[y][x]==0)
                            continue;
                        if(view[y][x]==2 && !draw_uw)	continue;		// Jump this if it is under water and don't have to be drawn
                        if(view[y][x]==3)
                            view[y][x]=2;
                        if(view[y][x]==2 && FLAT)
                            view[y][x]=0;
                        if( cam->mirror && map_data[Y][X].flat )	continue;
                    }
                    if(low_def_view)	continue;
                    // Si le joueur ne peut pas voir ce morceau, on ne le dessine pas en clair
                    T.x+=x<<4;
                    i=bmap[y][x];
                    if(FLAT) {
                        bloc[i].point=lvl[pre_y2+x];
                        if(bloc[i].point==NULL || bloc[i].point[0].y<niv || bloc[i].point[1].y<niv || bloc[i].point[2].y<niv ||
                           bloc[i].point[3].y<niv || bloc[i].point[4].y<niv || bloc[i].point[5].y<niv ||
                           bloc[i].point[6].y<niv || bloc[i].point[7].y<niv || bloc[i].point[8].y<niv)
                            bloc[i].point=flat;
                        else {
                            T.x-=x<<4;
                            continue;
                        }
                    }
                    else
                    {
                        if( check_visibility )
                        {
                            bool under_water = (h_map[Y|1][X|1] < sealvl && h_map[Y][X|1] < sealvl && h_map[Y|1][X] < sealvl && h_map[Y][X] < sealvl);

                            if( (bloc[i].lava || (under_water && ota_data.lavaworld) ) && !lp_CONFIG->pause
                                && (rand_from_table()%1000000)<=lavaprob) {		// Lava emiting code moved here because of lava effect using fragment program
                                Vector3D POS( (x<<4) - dwm + 8.0f, sealvl - 5.0f, pre_y - dhm + 8.0f );
                                V.x = ((rand_from_table()%201)-100);
                                V.y = ((rand_from_table()%51)+50);
                                V.z = ((rand_from_table()%201)-100);
                                V.unit();
                                particle_engine.emit_lava(POS,V,1,10,(rand_from_table()%1000)*0.01f+30.0f);
                            }
                            else if( !map_data[ Y ][ X ].lava && water && !ota_data.lavaworld && !under_water && !lp_CONFIG->pause &&										// A wave
                                     (h_map[Y|1][X|1] < sealvl || h_map[Y][X|1] < sealvl || h_map[Y|1][X] < sealvl || h_map[Y][X] < sealvl) &&
                                     (h_map[Y|1][X|1] >= sealvl || h_map[Y|1][X] >= sealvl || h_map[Y][X|1] >= sealvl || h_map[Y][X] >= sealvl) &&
                                     (rand_from_table()%4000)<=lavaprob &&
                                     (view_map->line[y][x]&player_mask) && lp_CONFIG->waves ) {
                                Vector3D POS;
                                POS.x=(x<<4)-dwm+8.0f;
                                POS.z=pre_y-dhm+8.0f;
                                POS.y=sealvl + 0.1f;
                                Vector3D grad;
                                grad.y = 0.0f;
                                grad.x = 0.0f;
                                grad.z = 0.0f;
                                if( h_map[Y][X] >= sealvl ) {
                                    grad.x -= h_map[Y][X] - sealvl;
                                    grad.z += h_map[Y][X] - sealvl;
                                }
                                if( h_map[Y|1][X] >= sealvl ) {
                                    grad.x -= h_map[Y|1][X] - sealvl;
                                    grad.z -= h_map[Y|1][X] - sealvl;
                                }
                                if( h_map[Y][X|1] >= sealvl ) {
                                    grad.x += h_map[Y][X|1] - sealvl;
                                    grad.z += h_map[Y][X|1] - sealvl;
                                }
                                if( h_map[Y|1][X|1] >= sealvl ) {
                                    grad.x += h_map[Y|1][X|1] - sealvl;
                                    grad.z -= h_map[Y|1][X|1] - sealvl;
                                }
                                float grad_len = grad.sq();
                                if( grad_len > 0.0f ) {
                                    grad = (1.0f / sqrt( grad_len )) * grad;
                                    fx_manager.addWave( POS, RAD2DEG * ( (grad.x >= 0.0f) ? -acos( grad.z ) : acos( grad.z ) ) );
                                }
                            }
                        }
                        bloc[i].point=lvl[pre_y2+x];
                        if(bloc[i].point==NULL) {
                            lvl[pre_y2+x]=bloc[i].point=(Vector3D*) malloc(sizeof(Vector3D)*9);
                            if(tnt) {
                                bloc[i].point[0].x=T.x;			bloc[i].point[0].z=get_zdec(X,Y)+T.z;
                                bloc[i].point[1].x=8.0f+T.x;	bloc[i].point[1].z=get_zdec(X|1,Y)+T.z;
                                bloc[i].point[2].x=16.0f+T.x;	bloc[i].point[2].z=get_zdec(X+2,Y)+T.z;
                                bloc[i].point[3].x=T.x;			bloc[i].point[3].z=8.0f+get_zdec(X,Y|1)+T.z;
                                bloc[i].point[4].x=8.0f+T.x;	bloc[i].point[4].z=8.0f+get_zdec(X|1,Y|1)+T.z;
                                bloc[i].point[5].x=16.0f+T.x;	bloc[i].point[5].z=8.0f+get_zdec(X+2,Y|1)+T.z;
                                bloc[i].point[6].x=T.x;			bloc[i].point[6].z=16.0f+get_zdec(X,Y+2)+T.z;
                                bloc[i].point[7].x=8.0f+T.x;	bloc[i].point[7].z=16.0f+get_zdec(X|1,Y+2)+T.z;
                                bloc[i].point[8].x=16.0f+T.x;	bloc[i].point[8].z=16.0f+get_zdec(X+2,Y+2)+T.z;
                                bloc[i].point[0].y=get_nh(X,Y);
                                bloc[i].point[1].y=get_nh(X|1,Y);
                                bloc[i].point[2].y=get_nh(X+2,Y);
                                bloc[i].point[3].y=get_nh(X,Y|1);
                                bloc[i].point[4].y=get_nh(X|1,Y|1);
                                bloc[i].point[5].y=get_nh(X+2,Y|1);
                                bloc[i].point[6].y=get_nh(X,Y+2);
                                bloc[i].point[7].y=get_nh(X|1,Y+2);
                                bloc[i].point[8].y=get_nh(X+2,Y+2);
                            }
                            else {
                                bloc[i].point[0].x=T.x;			bloc[i].point[0].z=T.z;
                                bloc[i].point[1].x=8.0f+T.x;	bloc[i].point[1].z=T.z;
                                bloc[i].point[2].x=16.0f+T.x;	bloc[i].point[2].z=T.z;
                                bloc[i].point[3].x=T.x;			bloc[i].point[3].z=8.0f+T.z;
                                bloc[i].point[4].x=8.0f+T.x;	bloc[i].point[4].z=8.0f+T.z;
                                bloc[i].point[5].x=16.0f+T.x;	bloc[i].point[5].z=8.0f+T.z;
                                bloc[i].point[6].x=T.x;			bloc[i].point[6].z=16.0f+T.z;
                                bloc[i].point[7].x=8.0f+T.x;	bloc[i].point[7].z=16.0f+T.z;
                                bloc[i].point[8].x=16.0f+T.x;	bloc[i].point[8].z=16.0f+T.z;
                                bloc[i].point[0].y=get_h(X,Y);
                                bloc[i].point[1].y=get_h(X|1,Y);
                                bloc[i].point[2].y=get_h(X+2,Y);
                                bloc[i].point[3].y=get_h(X,Y|1);
                                bloc[i].point[4].y=get_h(X|1,Y|1);
                                bloc[i].point[5].y=get_h(X+2,Y|1);
                                bloc[i].point[6].y=get_h(X,Y+2);
                                bloc[i].point[7].y=get_h(X|1,Y+2);
                                bloc[i].point[8].y=get_h(X+2,Y+2);
                            }
                            map_data[Y][X].flat = true;
                            for( byte f = 1 ; f < 9 ; f++ )			// Check if it's flat
                                if( bloc[i].point[0].y != bloc[i].point[f].y ) {
                                    map_data[Y][X].flat = false;
                                    break;
                                }
                        }
                    }

                    if(bloc[i].tex!=old_tex || buf_size>=500 || ox+1<x) {
                        if( buf_size > 0 )
                            glDrawElements(GL_TRIANGLE_STRIP, index_size,GL_UNSIGNED_SHORT,buf_i);		// dessine le tout
                        buf_size=0;
                        index_size=0;
                        was_flat = false;
                        if( old_tex != bloc[i].tex ) {
                            old_tex=bloc[i].tex;
                            glBindTexture(GL_TEXTURE_2D,bloc[i].tex);
                        }
                    }
                    ox=x;

                    uint16 buf_pos=buf_size*9;
                    if(!FLAT) {
                        for(byte e=0;e<9;e++)					// Copie le bloc
                            buf_p[buf_pos+e]=bloc[i].point[e];
                    }
                    else
                        for(byte e=0;e<9;e++) {					// Copie le bloc
                            buf_p[buf_pos+e].x=flat[e].x+T.x;
                            buf_p[buf_pos+e].y=flat[e].y;
                            buf_p[buf_pos+e].z=flat[e].z+T.z;
                        }

                    uint8 *color=buf_c+(buf_pos<<2);
                    if( FLAT )
                        for(int e=0;e<36;e+=4) {
                            color[e] = color[e|1] = color[e|2] = 255;
                            color[e|3] = 192;
                        }
                    else
                        for(int e=0;e<36;e+=4)
                            color[e]=color[e|1]=color[e|2]=color[e|3]=255;

                    bool is_clean = true;
                    if( fog_of_war != FOW_DISABLED ) {
                        int Z;
                        int grey = 0;
                        int black = 0;
                        Z=Y+get_zdec_notest(X,Y);									if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                        if(!(view_map->line[Z>>1][x]&player_mask))				{	color[0]=color[1]=color[2]=0;	black++;	}
                        else if(!(sight_map->line[Z>>1][x]&player_mask))		{	color[0]=color[1]=color[2]=127;	grey++;		}
                        if( X + 2 < bloc_w_db ) {
                            Z=Y+get_zdec_notest(X+2,Y);								if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                            if(!(view_map->line[Z>>1][x+1]&player_mask))		{	color[8]=color[9]=color[10]=0;		black++;	}
                            else if(!(sight_map->line[Z>>1][x+1]&player_mask))	{	color[8]=color[9]=color[10]=127;	grey++;		}
                        }
                        if( Y + 2 < bloc_h_db ) {
                            Z=Y+2+get_zdec_notest(X,Y+2);							if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                            if(!(view_map->line[Z>>1][x]&player_mask))			{	color[24]=color[25]=color[26]=0;	black++;	}
                            else if(!(sight_map->line[Z>>1][x]&player_mask))	{	color[24]=color[25]=color[26]=127;	grey++;		}
                            if( X + 2 < bloc_w_db ) {
                                Z=Y+2+get_zdec_notest(X+2,Y+2);							if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                                if(!(view_map->line[Z>>1][x+1]&player_mask))		{	color[32]=color[33]=color[34]=0;	black++;	}
                                else if(!(sight_map->line[Z>>1][x+1]&player_mask))	{	color[32]=color[33]=color[34]=127;	grey++;		}
                            }
                        }
                        is_clean = grey == 4 || black == 4 || ( grey == 0 && black == 0 );
                        if( !FLAT && !map_data[Y][X].flat ) {
                            color[4]=color[5]=color[6]= (color[0] + color[8]) >> 1;
                            color[12]=color[13]=color[14]= (color[0] + color[24]) >> 1;
                            color[20]=color[21]=color[22]= (color[8] + color[32]) >> 1;
                            color[16]=color[17]=color[18]= (color[12] + color[20]) >> 1;
                            color[28]=color[29]=color[30]= (color[24] + color[32]) >> 1;
                        }
                    }

                    //#define DEBUG_UNIT_POS

#ifndef DEBUG_UNIT_POS
                    if( FLAT || map_data[Y][X].flat ) {
                        if( was_flat && bloc[i].tex_x == bloc[ bmap[y][x-1] ].tex_x + 1 && is_clean && was_clean ) {
                            buf_i[ index_size-4 ] = 2+buf_pos;
                            buf_i[ index_size-2 ] = 8+buf_pos;
                            buf_i[ index_size-1 ] = 2+buf_pos;
                        }
                        else {
                            buf_i[ index_size++ ] = buf_pos;
                            buf_i[ index_size++ ] = 2+buf_pos;
                            buf_i[ index_size++ ] = 6+buf_pos;
                            buf_i[ index_size++ ] = 8+buf_pos;
                            buf_i[ index_size++ ] = 2+buf_pos;
                            was_flat = true;
                        }
                    }
                    else {
#endif
                        was_flat = false;
                        buf_i[ index_size++ ] = buf_pos;
                        buf_i[ index_size++ ] = 1+buf_pos;
                        buf_i[ index_size++ ] = 3+buf_pos;
                        buf_i[ index_size++ ] = 4+buf_pos;
                        buf_i[ index_size++ ] = 6+buf_pos;
                        buf_i[ index_size++ ] = 7+buf_pos;
                        buf_i[ index_size++ ] = 7+buf_pos;
                        buf_i[ index_size++ ] = 8+buf_pos;
                        buf_i[ index_size++ ] = 4+buf_pos;
                        buf_i[ index_size++ ] = 5+buf_pos;
                        buf_i[ index_size++ ] = 1+buf_pos;
                        buf_i[ index_size++ ] = 2+buf_pos;
#ifndef DEBUG_UNIT_POS
                    }
#endif
                    was_clean = is_clean;
                    T.x-=x<<4;
                    memcpy(buf_t+(buf_pos<<1),bloc[i].texcoord,72);		// texture

#ifdef DEBUG_UNIT_POS
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    Z&=0xFFFFFE;
                    X&=0xFFFFFE;
                    if(map_data[Z][X].unit_idx!=-1 ) {		// Shows unit's pos on map
                        color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
                        if(map_data[Z][X].unit_idx>=0 )		// Shows unit's pos on map
                            color[0]=color[4]=color[12]=color[16]=255;
                        else		// It's a feature
                            color[1]=color[5]=color[13]=color[17]=255;
                    }
                    else if( !map_data[Z][X].air_idx.isEmpty() ) {		// Shows unit's pos on map
                        color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
                        color[2]=color[6]=color[14]=color[18]=255;
                    }
                    if(map_data[Z][X+1].unit_idx!=-1 ) {		// Shows unit's pos on map
                        color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
                        if(map_data[Z][X+1].unit_idx>=0 )		// Shows unit's pos on map
                            color[8]=color[20]=255;
                        else
                            color[9]=color[21]=255;
                    }
                    else if( !map_data[Z][X+1].air_idx.isEmpty() ) {		// Shows unit's pos on map
                        color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
                        color[10]=color[22]=255;
                    }
                    if(map_data[Z+1][X].unit_idx!=-1 ) {		// Shows unit's pos on map
                        color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
                        if(map_data[Z+1][X].unit_idx>=0 )		// Shows unit's pos on map
                            color[24]=color[28]=255;
                        else
                            color[25]=color[29]=255;
                    }
                    else if( !map_data[Z+1][X].air_idx.isEmpty() ) {		// Shows unit's pos on map
                        color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
                        color[26]=color[30]=255;
                    }
                    if(map_data[Z+1][X+1].unit_idx!=-1 ) {		// Shows unit's pos on map
                        color[32]=color[33]=color[34]=color[35]=0;
                        if(map_data[Z+1][X+1].unit_idx>=0 )		// Shows unit's pos on map
                            color[32]=255;
                        else
                            color[33]=255;
                    }
                    else if( !map_data[Z+1][X+1].air_idx.isEmpty() ) {		// Shows unit's pos on map
                        color[32]=color[33]=color[34]=color[35]=0;
                        color[34]=255;
                    }
#elif defined DEBUG_RADAR_MAP
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    Z&=0xFFFFFE;
                    X&=0xFFFFFE;
                    if( (radar_map->line[Z>>1][X>>1] & player_mask) )		// Shows unit's pos on map
                        for(i=0;i<9;i++) {
                            color[i<<2]=color[(i<<2)+1]=color[(i<<2)+2]=color[(i<<2)+3]=0;
                            color[(i<<2)]=255;
                        }
                    else if( (sonar_map->line[Z>>1][X>>1] & player_mask) )		// Shows unit's pos on map
                        for(i=0;i<9;i++) {
                            color[i<<2]=color[(i<<2)+1]=color[(i<<2)+2]=color[(i<<2)+3]=0;
                            color[(i<<2)+2]=255;
                        }
#endif
                    ++buf_size;
                }
                if(buf_size>0) {
                    glDrawElements(GL_TRIANGLE_STRIP, index_size,GL_UNSIGNED_SHORT,buf_i);		// dessine le tout
                    was_flat = false;
                    index_size=0;
                    buf_size=0;
                }
            }
        glDisableClientState(GL_COLOR_ARRAY);		// Couleurs(pour le brouillard de guerre)

        detail_shader.off();
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        gfx->unlock();
    }

    Vector3D MAP::hit(Vector3D Pos,Vector3D Dir,bool water, float length, bool allow_out)			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
    {
        if(Dir.x==0.0f && Dir.z==0.0f) // Solution triviale
        {
            Vector3D P=Pos;
            P.y=get_unit_h(P.x,P.z);
            return P;
        }
        if(get_unit_h(Pos.x,Pos.z) > Pos.y)		// Cas non traité
            return Pos;
        float step=1.0f;
        if(Dir.x!=0.0f && Dir.z!=0.0f) {
            if(fabs(Dir.x)<fabs(Dir.z))
                step=1.0f/fabs(Dir.x);
            else
                step=1.0f/fabs(Dir.z);
        }
        int nb=0;
        int nb_limit = (int)(Pos.y) + 1000;
        float dwm=map_w_d;
        float dhm=map_h_d;
        Dir=(1.0f*step)*Dir;
        float len_step = Dir.norm();
        while(((sealvl<Pos.y && water) || !water) && get_max_h((int)(Pos.x+map_w_d)>>3,(int)(Pos.z+map_h_d)>>3)<Pos.y) {
            if(nb >= nb_limit || length<0.0f)
                return Pos;
            length-=len_step;
            nb++;
            Pos=Pos+Dir;
            if( (fabs(Pos.x)>dwm || fabs(Pos.z)>dhm) && !allow_out )		// Pas de résultat
                return Pos;
        }
        length+=len_step;
        Pos=Pos-Dir;
        while(((sealvl<Pos.y && water) || !water) && get_unit_h(Pos.x,Pos.z)<Pos.y) {
            if(nb >= nb_limit || length<0.0f)
                return Pos;
            length-=len_step;
            nb++;
            Pos=Pos+Dir;
            if( (fabs(Pos.x)>dwm || fabs(Pos.z)>dhm) && !allow_out )		// Pas de résultat
                return Pos;
        }
        for(byte i=0;i<7;i++) {
            length+=len_step;
            Pos=Pos-Dir;		// On recommence la dernière opération mais avec plus de précision
            Dir=0.5f*Dir;
            len_step *= 0.5f;
            nb=0;
            while(((sealvl<Pos.y && water) || !water) && get_unit_h(Pos.x,Pos.z)<Pos.y) {
                if(nb>=2 || length<0.0f)
                    return Pos;
                length-=len_step;
                nb++;
                Pos=Pos+Dir;
            }
        }
        return Pos;		// Meilleure solution approximative trouvée
    }

    int MAP::check_metal(int x1, int y1, int unit_idx )
    {
        if( unit_idx < 0 || unit_idx >= unit_manager.nb_unit )	return 0;

        int w = unit_manager.unit_type[ unit_idx ].FootprintX;
        int h = unit_manager.unit_type[ unit_idx ].FootprintZ;
        int metal_base = 0;
        int end_y = y1 + ( h >> 1 );
        int end_x = x1 + ( w >> 1 );
        int start_x = x1 - ( w >> 1 );
        for( int ry = y1 - ( h >> 1 ) ; ry <= end_y ; ry++ )
            if( ry >= 0 && ry < bloc_h_db )
                for( int rx = start_x ; rx <= end_x ; rx++ )
                    if( rx >= 0 && rx < bloc_w_db )
                        if( map_data[ry][rx].stuff >=0 )
                            metal_base += feature_manager.feature[ features.feature[ map_data[ry][rx].stuff ].type ].metal;
        if( metal_base == 0 )
            metal_base = ota_data.SurfaceMetal;
        return metal_base;
    }

    void WATER::draw(float t,float X,float Y,bool shaded)
    {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glTranslatef(cos(t),0.0f,sin(t));
        if(shaded) {
            glBegin(GL_QUADS);
            glTexCoord2f(-map_w/w+0.5f,-map_h/w+0.5f);		glVertex3f(-map_w,0.0f,-map_h);
            glTexCoord2f(map_w/w+0.5f,-map_h/w+0.5f);		glVertex3f(map_w,0.0f,-map_h);
            glTexCoord2f(map_w/w+0.5f,map_h/w+0.5f);		glVertex3f(map_w,0.0f,map_h);
            glTexCoord2f(-map_w/w+0.5f,map_h/w+0.5f);		glVertex3f(-map_w,0.0f,map_h);
            glEnd();
            return;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);		glVertex3f(-map_w*0.5f,0.0f,-map_h*0.5f);
        glTexCoord2f(1.0f,0.0f);		glVertex3f(map_w*0.5f,0.0f,-map_h*0.5f);
        glTexCoord2f(1.0f,1.0f);		glVertex3f(map_w*0.5f,0.0f,map_h*0.5f);
        glTexCoord2f(0.0f,1.0f);		glVertex3f(-map_w*0.5f,0.0f,map_h*0.5f);

        glDisable( GL_TEXTURE_2D );
        glColor4f(0.0f,0.0f,0.0f,1.0f);

        glVertex3f(-map_w,0.0f,-map_h);
        glVertex3f(map_w,0.0f,-map_h);
        glVertex3f(map_w,0.0f,-map_h*0.5f);
        glVertex3f(-map_w,0.0f,-map_h*0.5f);

        glVertex3f(-map_w,0.0f,map_h*0.5f);
        glVertex3f(map_w,0.0f,map_h*0.5f);
        glVertex3f(map_w,0.0f,map_h);
        glVertex3f(-map_w,0.0f,map_h);

        glVertex3f(-map_w,0.0f,-map_h*0.5f);
        glVertex3f(-map_w*0.5f,0.0f,-map_h*0.5f);
        glVertex3f(-map_w*0.5f,0.0f,map_h*0.5f);
        glVertex3f(-map_w,0.0f,map_h*0.5f);

        glVertex3f(map_w*0.5f,0.0f,-map_h*0.5f);
        glVertex3f(map_w,0.0f,-map_h*0.5f);
        glVertex3f(map_w,0.0f,map_h*0.5f);
        glVertex3f(map_w*0.5f,0.0f,map_h*0.5f);
        glEnd();
    }

    void SKY::build(int d,float size, bool full_sphere)
    {
        destroy();

        full = full_sphere;

        s = full_sphere ? (d << 1) : d;
        w = size;

        nb_vtx=(s+1)*((s<<1)+1);
        nb_idx = s*(s*2+1)*2;				// We'll use GL_TRIANGLE_STRIP

        point=(Vector3D*) malloc(sizeof(Vector3D)*nb_vtx);
        texcoord=(float*) malloc(sizeof(float)*nb_vtx*2);
        index=(GLushort*) malloc(sizeof(GLushort)*nb_idx);

        int i=0;
        for(int y=0;y<=s;y++)
            for(int x=0;x<=s*2;x++) {
                if( full_sphere ) {
                    point[i].x=size*cos(x/(2.0f*s)*PI*2.0f)*cos((float)y/s*PI - PI*0.5f);
                    point[i].y=size*sin((float)y/s*PI - PI*0.5f);
                    point[i].z=-size*sin(x/(2.0f*s)*PI*2.0f)*cos((float)y/s*PI - PI*0.5f);
                }
                else {
                    point[i].x=size*cos(x/(2.0f*s)*PI*2.0f)*cos(0.5f*y/s*PI);
                    point[i].y=size*sin(0.5f*y/s*PI);
                    point[i].z=-size*sin(x/(2.0f*s)*PI*2.0f)*cos(0.5f*y/s*PI);
                }
                texcoord[i<<1]=(float)x/(s*2);
                texcoord[(i<<1)+1]=1.0f-(float)y/s;
                i++;
            }
        i=0;
        for(int y=0;y<s;y++)				// We'll use GL_TRIANGLE_STRIP
            for(int x=0;x<=s*2;x++) {
                index[i++]=y*(s*2+1)+x;
                index[i++]=(y+1)*(s*2+1)+x;
            }
    }

    void SKY::draw()
    {
        glDisableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
        glVertexPointer( 3, GL_FLOAT, 0, point);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoord);

        glDrawElements(GL_TRIANGLE_STRIP, nb_idx,GL_UNSIGNED_SHORT,index);		// dessine le tout
    }

    int PLAYERS::add(char *NOM,char *SIDE,byte _control,int E,int M,byte AI_level)
    {
        if(nb_player>=10)	return -1;		// Trop de joueurs déjà
        metal_u[nb_player]=0;
        energy_u[nb_player]=0;
        metal_t[nb_player]=0;
        energy_t[nb_player]=0;
        kills[nb_player]=0;
        losses[nb_player]=0;
        nom[nb_player]=strdup(NOM);
        control[nb_player]=_control;
        nb_unit[nb_player]=0;
        energy_total[nb_player]=0.0f;
        metal_total[nb_player]=0.0f;

        LOG_INFO("Adding a new player: `" << NOM << "` (" << nb_player << ") of `" << SIDE << "` with E=" << E << ", M=" << M);

        com_metal[nb_player] = M;
        com_energy[nb_player] = E;
        energy[nb_player] = E;
        metal[nb_player] = M;
        energy_s[nb_player] = E;
        metal_s[nb_player] = M;

        side[nb_player++]=strdup(SIDE);
        if (_control == PLAYER_CONTROL_LOCAL_HUMAN)
        {
            local_human_id = NB_PLAYERS;
            for (int i = 0; i < ta3dSideData.nb_side ; ++i)
                if (String::ToLower(ta3dSideData.side_name[i]) == String::ToLower(SIDE))
                {
                    side_view = i;
                    break;
                }
        }
        if (_control==PLAYER_CONTROL_LOCAL_AI)
        {
            char filename[100];
            filename[0]=0;
            strcat(filename,"ai/");
            strcat(filename,NOM);
            strcat(filename,".ai");
            if(file_exists(filename,FA_RDONLY | FA_ARCH,NULL))						// Charge un joueur s'il existe
                ai_command[NB_PLAYERS].load(filename,NB_PLAYERS);
            else													// Sinon crée un nouveau joueur
                ai_command[NB_PLAYERS].change_name(NOM);
            ai_command[ NB_PLAYERS ].player_id = NB_PLAYERS;
            ai_command[ NB_PLAYERS ].AI_type = AI_level;
        }
        return NB_PLAYERS++;
    }

    void PLAYERS::show_resources()
    {
        units.lock();

        int _id = (local_human_id != -1) ? local_human_id : 0;
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
        char buf[100];
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalNum.x1,ta3dSideData.side_int_data[ players.side_view ].MetalNum.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,format("%d",(int)metal[_id]));
        uszprintf(buf,100,"%f",metal_t[_id]);
        if( strstr(buf,".") )	*(strstr(buf,".")+2)=0;
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalProduced.x1,ta3dSideData.side_int_data[ players.side_view ].MetalProduced.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,buf);
        uszprintf(buf,100,"%f",metal_u[_id]);
        if( strstr(buf,".") )	*(strstr(buf,".")+2)=0;
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalConsumed.x1,ta3dSideData.side_int_data[ players.side_view ].MetalConsumed.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,buf);
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].Metal0.x1,ta3dSideData.side_int_data[ players.side_view ].Metal0.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,"0");
        gfx->print_right(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalMax.x1,ta3dSideData.side_int_data[ players.side_view ].MetalMax.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color, format( "%d", metal_s[_id] ) );
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyNum.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyNum.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,format("%d",(int)energy[_id]));
        uszprintf(buf,100,"%f",energy_t[_id]);
        if( strstr(buf,".") )	*(strstr(buf,".")+2)=0;
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyProduced.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyProduced.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,buf);
        uszprintf(buf,100,"%f",energy_u[_id]);
        if( strstr(buf,".") )	*(strstr(buf,".")+2)=0;
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyConsumed.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyConsumed.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,buf);
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].Energy0.x1,ta3dSideData.side_int_data[ players.side_view ].Energy0.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,"0");
        gfx->print_right(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyMax.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyMax.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color, format( "%d", energy_s[_id] ) );

        glDisable(GL_TEXTURE_2D);

        glDisable(GL_BLEND);
        glBegin(GL_QUADS);			// Dessine les barres de metal et d'énergie
        gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].metal_color );

        if(metal_s[0])
        {
            float metal_percent = metal_s[_id] ? metal[_id] / metal_s[_id] : 0.0f;
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1, ta3dSideData.side_int_data[ players.side_view ].MetalBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1 + metal_percent * (ta3dSideData.side_int_data[ players.side_view ].MetalBar.x2-ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1), ta3dSideData.side_int_data[ players.side_view ].MetalBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1 + metal_percent * (ta3dSideData.side_int_data[ players.side_view ].MetalBar.x2-ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1), ta3dSideData.side_int_data[ players.side_view ].MetalBar.y2 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1, ta3dSideData.side_int_data[ players.side_view ].MetalBar.y2 );
        }

        gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].energy_color );
        if(energy_s[0])
        {
            float energy_percent = energy_s[_id] ? energy[_id] / energy_s[_id] : 0.0f;
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1, ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1 + energy_percent * (ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x2-ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1), ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1 + energy_percent * (ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x2-ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1), ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y2 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1, ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y2 );
        }
        glEnd();
        glColor4f(1.0f,1.0f,1.0f,1.0f);

        units.unlock();
    }

    inline bool need_sync( struct sync &a, struct sync &b )
    {
        return fabs( a.x - b.x ) > 0.001f
            ||	fabs( a.y - b.y ) > 1.0f
            ||	fabs( a.z - b.z ) > 0.001f
            ||	fabs( a.vx - b.vx ) > 0.001f
            ||	fabs( a.vz - b.vz ) > 0.001f
            ||	a.hp != b.hp
            ||	a.orientation != b.orientation
            ||	a.build_percent_left != b.build_percent_left;
    }

    void PLAYERS::player_control()
    {
        for( byte i = 0 ; i < nb_player ; i++ )
            if( control[ i ] == PLAYER_CONTROL_LOCAL_AI && ai_command )
                ai_command[ i ].monitor();

        if( (units.current_tick % 3) == 0 && last_ticksynced != units.current_tick && network_manager.isConnected() )
        {
            last_ticksynced = units.current_tick;
            
            uint32  nbTCP(0), nbTotal(0);

            units.lock();
            for( int e = 0 ; e < units.nb_unit ; e++ )
            {
                int i = units.idx_list[ e ];
                if( i < 0 || i >= units.max_unit )	continue;		// Error !!
                units.unlock();

                units.unit[ i ].lock();
                if( !(units.unit[ i ].flags & 1) )
                {
                    units.unit[ i ].unlock();
                    units.lock();
                    continue;
                }
                if( units.unit[ i ].local )
                {
                    struct sync sync;
                    sync.timestamp = units.current_tick;
                    sync.unit = i;
                    sync.flags = 0;
                    if( units.unit[ i ].flying )
                        sync.flags |= SYNC_FLAG_FLYING;
                    if( units.unit[ i ].cloaking )
                        sync.flags |= SYNC_FLAG_CLOAKING;
                    sync.x = units.unit[ i ].Pos.x;
                    sync.y = units.unit[ i ].Pos.y;
                    sync.z = units.unit[ i ].Pos.z;
                    sync.vx = units.unit[ i ].V.x;
                    sync.vz = units.unit[ i ].V.z;
                    float angle = units.unit[ i ].Angle.y;
                    while( angle < 0.0f )	angle += 360.0f;
                    sync.orientation = (uint16)(angle * 65535.0f / 360.0f);
                    sync.hp = (uint16)units.unit[ i ].hp;
                    sync.build_percent_left = (uint8)(units.unit[ i ].build_percent_left * 2.55f);

                    uint32 latest_sync = units.current_tick;
                    for( int f = 0 ; f < NB_PLAYERS ; ++f)
                        if( g_ta3d_network->isRemoteHuman(f) )
                            latest_sync = Math::Min(latest_sync, units.unit[ i ].last_synctick[f]);

                    nbTotal++;

                    if( g_ta3d_network->isTCPonly()
                        || latest_sync < units.unit[i].previous_sync.timestamp - 10
                        || units.unit[i].previous_sync.flags != sync.flags
                        || units.unit[i].previous_sync.hp != sync.hp
                        || ( units.unit[i].previous_sync.build_percent_left != sync.build_percent_left && sync.build_percent_left == 0.0f ) )
                    {		// We have to sync now
                        network_manager.sendSyncTCP( &sync );
                        units.unit[i].previous_sync = sync;
                        if( latest_sync < units.unit[i].previous_sync.timestamp - 10 )
                            nbTCP++;
                        //					printf("sending TCP sync packet!\n");
                    }
                    else
                    {
                        if( need_sync( sync, units.unit[i].previous_sync ) )
                        {			// Don't send what isn't needed
                            network_manager.sendSync( &sync );
                            units.unit[i].previous_sync = sync;
                        }
                    }
                }
                units.unit[ i ].unlock();

                units.lock();
            }
            units.unlock();

            if( !g_ta3d_network->isTCPonly() && nbTCP * 10 > nbTotal )
            {
                network_manager.sendAll("TCPONLY");             // Tell everyone UDP is not enough reliable and switch to TCP only mode
                g_ta3d_network->switchToTCPonly();
            }

        }
    }

    int PLAYERS::Run()
    {
        if( thread_is_running )	return 0;

        thread_is_running = true;

        players_thread_sync = 0;
        last_ticksynced = 9999;

        while( !thread_ask_to_stop ) {
            players.player_control();

            /*---------------------- handle Network events ------------------------------*/

            if( ta3d_network )
                ta3d_network->check();

            /*---------------------- end of Network events ------------------------------*/

            ThreadSynchroniser->lock();
            ThreadSynchroniser->unlock();

            players_thread_sync = 1;

            while( players_thread_sync && !thread_ask_to_stop )	rest( 1 );			// Wait until other thread sync with this one
        }

        thread_is_running = false;

        return 0;
    }

    void PLAYERS::SignalExitThread()
    {
        thread_ask_to_stop = true;
        while( thread_is_running )	rest( 1 );
        thread_ask_to_stop = false;
    }

    void SKY_DATA::load_tdf(const String& filename)
    {
        cTAFileParser parser(filename);

        def = parser.pullAsBool( "sky.default", false );
        spherical = parser.pullAsBool( "sky.spherical" );
        full_sphere = parser.pullAsBool( "sky.full sphere" );
        rotation_speed = parser.pullAsFloat( "sky.rotation speed" );
        rotation_offset = parser.pullAsFloat( "sky.rotation offset" );
        texture_name = parser.pullAsString( "sky.texture name" );
        ReadVectorString(planet, parser.pullAsString( "sky.planet" ) );
        FogColor[0] = parser.pullAsFloat( "sky.fog R" );
        FogColor[1] = parser.pullAsFloat( "sky.fog G" );
        FogColor[2] = parser.pullAsFloat( "sky.fog B" );
        FogColor[3] = parser.pullAsFloat( "sky.fog A" );
        ReadVectorString(MapName, parser.pullAsString( "sky.map" ) );
    }

    SKY_DATA* choose_a_sky( const String& mapname, const String& planet)
    {
        std::list<SKY_DATA*> sky_list;
        sky_list.clear();

        String::List file_list;
        HPIManager->getFilelist( "sky\\*.tdf",  file_list);
        uint32	nb_sky = 0;

        for (String::List::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
        {
            SKY_DATA *sky_data = new SKY_DATA;
            sky_data->load_tdf(*it);

            bool keep = false;
            for (unsigned int i = 0 ; i < sky_data->MapName.size() ; ++i)
                if( sky_data->MapName[i] == mapname)
                {
                    keep = true;
                    break;
                }
            if( !keep )
                for(unsigned int i = 0 ; i < sky_data->planet.size() ; ++i)
                    if (sky_data->planet[ i ] == planet)
                    {
                        keep = true;
                        break;
                    }
            if( keep )
            {
                sky_list.push_back( sky_data );
                nb_sky++;
            }
            else
                delete sky_data;
        }

        if( nb_sky == 0 )// Look for a default sky
        {
            for (String::List::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
            {
                SKY_DATA *sky_data = new SKY_DATA;
                sky_data->load_tdf(*it);

                bool keep = sky_data->def;
                if (keep)
                {
                    sky_list.push_back(sky_data);
                    ++nb_sky;
                }
                else
                    delete sky_data;
            }
        }

        SKY_DATA *selected_sky = NULL;

        if( nb_sky > 0 )
        {
            int select = TA3D_RAND() % nb_sky;
            for (std::list<SKY_DATA*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it, --select)
                if( select == 0 )
                {
                    selected_sky = *it;
                    *it = NULL;
                    break;
                }
        }

        for (std::list<SKY_DATA*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it)
        {
            if( *it != NULL )
                delete *it;
        }
        sky_list.clear();

        return selected_sky;
    }

} // namespace TA3D

