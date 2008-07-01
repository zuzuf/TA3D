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

/*-----------------------------------------------------------------\
  |                               tdf.cpp                            |
  |   contient toutes les fonctions et classes permettant la gestion |
  | des fichiers TDF du jeu Total Annihilation qui contienne divers  |
  | éléments graphiques.                                             |
  \-----------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "tdf.h"
#include "EngineClass.h"
#include "UnitEngine.h"
#include "network/TA3D_Network.h"

namespace TA3D
{


    FEATURE_MANAGER		feature_manager;
    FEATURES features;

    void FEATURE_MANAGER::load_tdf(char *data,int size)					// Charge un fichier tdf
    {
        char *pos=data;
        char *ligne=NULL;
        int nb=0;
        int index=0;
        int	first=nb_features;
        char *limit=data+size;
        do{
            do
            {
                nb++;
                if(ligne)
                    delete[] ligne;
                ligne=get_line(pos);
                strlwr(ligne);
                while(pos[0]!=0 && pos[0]!=13 && pos[0]!=10)	pos++;
                while(pos[0]==13 || pos[0]==10)	pos++;

                if( strstr( ligne, " " ) != NULL && strstr( ligne, "=" ) != NULL ) {			// remove useless spaces
                    int e = 0;
                    bool offset = true;
                    for( int i = 0 ; i <= strlen( ligne ) ; i++ ) {			// <= because of NULL termination!!
                        if( ligne[ i ] == '=' )	offset = false;
                        if( offset && ligne[ i ] == ' ' ) e++;
                        if( i >= e )
                            ligne[ i - e ] = ligne[ i ];
                    }
                }

                if(ligne[0]=='[') {
                    if(strstr(ligne,"]"))
                        *(strstr(ligne,"]"))=0;
                    index=add_feature(ligne+1);
                    feature[index].m3d=false;
                }
                else if(strstr(ligne,"world=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].world=strdup(strstr(ligne,"world=")+6);
                }
                else if(strstr(ligne,"description=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].description=strdup(strstr(ligne,"description=")+12);
                }
                else if(strstr(ligne,"category=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].category=strdup(strstr(ligne,"category=")+9);
                }
                else if(strstr(ligne,"object=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].filename=strdup(strstr(ligne,"object=")+7);
                    feature[index].m3d=true;
                }
                else if(strstr(ligne,"filename=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].filename=strdup(strstr(ligne,"filename=")+9);
                    feature_hashtable.Insert( Lowercase( feature[index].filename ), index + 1 );
                }
                else if(strstr(ligne,"seqname=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].seqname=strdup(strstr(ligne,"seqname=")+8);
                    feature_hashtable.Insert( Lowercase( feature[index].seqname ), index + 1 );
                }
                else if(strstr(ligne,"animating="))
                    feature[index].animating=(*(strstr(ligne,"animating=")+10)=='1');
                else if(strstr(ligne,"animtrans="))
                    feature[index].animtrans=feature[index].animating=(*(strstr(ligne,"animtrans=")+10)=='1');
                else if(strstr(ligne,"shadtrans="))
                    feature[index].shadtrans=(*(strstr(ligne,"shadtrans=")+10)=='1');
                else if(strstr(ligne,"indestructible="))
                    feature[index].indestructible=(*(strstr(ligne,"indestructible=")+15)=='1');
                else if(strstr(ligne,"height="))
                    feature[index].height=atoi(strstr(ligne,"height=")+7);
                else if(strstr(ligne,"hitdensity="))
                    feature[index].hitdensity=atoi(strstr(ligne,"hitdensity=")+11);
                else if(strstr(ligne,"metal="))
                    feature[index].metal=atoi(strstr(ligne,"metal=")+6);
                else if(strstr(ligne,"energy="))
                    feature[index].energy=atoi(strstr(ligne,"energy=")+7);
                else if(strstr(ligne,"damage="))
                    feature[index].damage=atoi(strstr(ligne,"damage=")+7);
                else if(strstr(ligne,"footprintx="))
                    feature[index].footprintx=atoi(strstr(ligne,"footprintx=")+11);
                else if(strstr(ligne,"footprintz="))
                    feature[index].footprintz=atoi(strstr(ligne,"footprintz=")+11);
                else if(strstr(ligne,"autoreclaimable="))
                    feature[index].autoreclaimable=(*(strstr(ligne,"autoreclaimable=")+16)=='1');
                else if(strstr(ligne,"reclaimable="))
                    feature[index].reclaimable=(*(strstr(ligne,"reclaimable=")+12)=='1');
                else if(strstr(ligne,"blocking="))
                    feature[index].blocking=(*(strstr(ligne,"blocking=")+9)=='1');
                else if(strstr(ligne,"flamable="))
                    feature[index].flamable=(*(strstr(ligne,"flamable=")+9)=='1');
                else if(strstr(ligne,"geothermal="))
                    feature[index].geothermal=(*(strstr(ligne,"geothermal=")+11)=='1');
                else if(strstr(ligne,"reproducearea="))	{}
                else if(strstr(ligne,"reproduce="))	{}
                else if(strstr(ligne,"featuredead=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_dead=strdup(strstr(ligne,"featuredead=")+12);
                }
                else if(strstr(ligne,"seqnameshad=")) {}
                else if(strstr(ligne,"seqnamedie=")) {}
                else if(strstr(ligne,"seqnamereclamate=")) {}
                else if(strstr(ligne,"permanent=")) {}
                else if(strstr(ligne,"nodisplayinfo=")) {}
                else if(strstr(ligne,"burnmin="))
                    feature[index].burnmin=atoi(strstr(ligne,"burnmin=")+8);
                else if(strstr(ligne,"burnmax="))
                    feature[index].burnmax=atoi(strstr(ligne,"burnmax=")+8);
                else if(strstr(ligne,"sparktime="))
                    feature[index].sparktime=atoi(strstr(ligne,"sparktime=")+10);
                else if(strstr(ligne,"spreadchance="))
                    feature[index].spreadchance=atoi(strstr(ligne,"spreadchance=")+13);
                else if(strstr(ligne,"burnweapon=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].burnweapon=strdup(strstr(ligne,"burnweapon=")+11);
                }
                else if(strstr(ligne,"seqnameburn=")) {}
                else if(strstr(ligne,"seqnameburnshad=")) {}
                else if(strstr(ligne,"featureburnt=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_burnt=strdup(strstr(ligne,"featureburnt=")+13);
                }
                else if(strstr(ligne,"featurereclamate=")) {
                    if(strstr(ligne,";"))	*(strstr(ligne,";"))=0;
                    feature[index].feature_reclamate=strdup(strstr(ligne,"featurereclamate=")+17);
                }
                else if(strstr(ligne,";"))
                    Console->AddEntry("(tdf)unknown: %s",ligne);

            }while(strstr(ligne,"}")==NULL && nb<10000 && pos<limit);
            delete[] ligne;
            ligne=NULL;
        }while(pos[0]=='[' && nb<10000 && pos<limit);
        if(g_useTextureCompression)
            allegro_gl_set_texture_format( GL_COMPRESSED_RGBA_ARB );
        else
            allegro_gl_set_texture_format( GL_RGBA8 );
        for(int i=first;i<nb_features;i++) {				// Charge les fichiers d'animation
            if(feature[i].category)
                feature[i].vent=(strstr(feature[i].category,"vents")!=NULL);
            if(feature[i].filename && feature[i].seqname && !feature[i].m3d) {
                if(model_manager.get_model((char*)(String(feature[i].filename)+"-"+String(feature[i].seqname)).c_str())!=NULL) {		// Check if there is a 3do version of it
                    feature[i].model=NULL;
                    feature[i].m3d=true;
                    feature[i].converted=false;
                }
                else {
                    String tmp = String("anims\\") + String(feature[i].filename) + String(".gaf");
                    byte *gaf=HPIManager->PullFromHPI(tmp);
                    if(gaf) {
                        int index=get_gaf_entry_index(gaf,feature[i].seqname);
                        if(index>=0)
                            feature[i].anim.load_gaf(gaf,get_gaf_entry_index(gaf,feature[i].seqname),true,feature[i].filename);
                        else
                            Console->AddEntry( "WARNING: %s has no picture to display!! Missing files?", feature[i].name );
                        free(gaf);
                        if(index>=0 && feature[i].height<=10.0f && feature[i].height>1.0f && feature[i].anim.nb_bmp>0 && feature[i].blocking
                           && feature[i].anim.bmp[0]->w>=16 && feature[i].anim.bmp[0]->h>=16 && strcasecmp(feature[i].description,"Metal")!=0) {			// Tente une conversion en 3d
                            char tmp[200];
                            tmp[0]=0;
                            strcat(tmp,feature[i].filename);
                            strcat(tmp,"-");
                            strcat(tmp,feature[i].seqname);
                            model_manager.create_from_2d(feature[i].anim.bmp[0],feature[i].footprintx*8,feature[i].footprintz*8,feature[i].height*H_DIV,tmp);
                            feature[i].model=NULL;
                            feature[i].m3d=true;
                            feature[i].converted=true;
                            feature[i].anim.destroy();
                            index=-1;
                        }
                        if(index < 0)
                            feature[i].need_convert = false;
                    }
                }
            }
            else if(feature[i].filename && feature[i].m3d)
                feature[i].model=NULL;
        }
    }

    void load_features(void (*progress)(float percent,const String &msg))				// Charge tout les éléments
    {
        std::list<String> file_list;
        HPIManager->getFilelist("features\\*.tdf", file_list);

        int n = 0;

        for(std::list<String>::iterator cur_file=file_list.begin();cur_file!=file_list.end();cur_file++)
        {
            if(progress!=NULL && !(n & 0xF))
                progress((200.0f+n*50.0f/(file_list.size()+1))/7.0f,TRANSLATE("Loading graphical features"));
            n++;

            uint32 file_size=0;
            byte *data=HPIManager->PullFromHPI(cur_file->c_str(),&file_size);
            if(data) {
                Console->AddEntry( "loading %s", cur_file->c_str() );
                feature_manager.load_tdf((char*)data,file_size);
                free(data);
            }
            else
                Console->AddEntry( "WARNING: loading %s failed", cur_file->c_str() );
        }

        for(int i=0;i<feature_manager.nb_features;i++)
            if(feature_manager.feature[i].m3d && feature_manager.feature[i].model==NULL && feature_manager.feature[i].filename!=NULL && feature_manager.feature[i].seqname!=NULL) {
                String tmp = String(feature_manager.feature[i].filename) + "-" + String(feature_manager.feature[i].seqname);
                feature_manager.feature[i].model=model_manager.get_model((char*)tmp.c_str());
                if(feature_manager.feature[i].model==NULL)
                    feature_manager.feature[i].model=model_manager.get_model((char*)(String("objects3d\\")+tmp).c_str());
            }
            else if(feature_manager.feature[i].m3d && feature_manager.feature[i].model==NULL && feature_manager.feature[i].filename!=NULL)
                feature_manager.feature[i].model=model_manager.get_model(feature_manager.feature[i].filename);
    }

    void FEATURES::draw(Camera* cam)
    {
        if(nb_features<=0)
            return;
        cam->setView();			// Positionne la caméra
        gfx->ReInitAllTex( true );
        glAlphaFunc(GL_GREATER,0.1);
        glEnable(GL_ALPHA_TEST);

        glDepthFunc( GL_LEQUAL );

        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        float sq2=1.0f/sqrt(2.0f);
        GLuint old = 0;
        bool texture_loaded=false;
        GLubyte index[]={	0, 1, 2, 3,
            4, 1, 2, 5,
            6, 1, 2, 7,
            8, 9,10,11,
            1,12,13, 2,
            1,14,15, 2,
            1,16,17, 2};

        float texcoord[]={	0.0f,	0.0f,
            0.5f,	0.0f,
            0.5f,	1.0f,
            0.0f,	1.0f,
            0.0f,	0.0f,
            0.0f,	1.0f,
            0.0f,	0.0f,
            0.0f,	1.0f,
            0.0f,	0.0f,
            1.0f,	0.0f,
            1.0f,	1.0f,
            0.0f,	1.0f,
            1.0f,	0.0f,
            1.0f,	1.0f,
            1.0f,	0.0f,
            1.0f,	1.0f,
            1.0f,	0.0f,
            1.0f,	1.0f};
        bool set=true;
        float points[]={	0.0f,		1.0f,		-1.0f,
            0.0f,		1.0f,		 0.0f,
            0.0f,		0.0f,		 0.0f,
            0.0f,		0.0f,		-1.0f,
            -sq2,		1.0f,		 -sq2,
            -sq2,		0.0f,		 -sq2,
            sq2,		1.0f,		 -sq2,
            sq2,		0.0f,		 -sq2,
            -1.0f,		1.0f,		 0.0f,
            1.0f,		1.0f,		 0.0f,
            1.0f,		0.0f,		 0.0f,
            -1.0f,		0.0f,		 0.0f,
            -sq2,		1.0f,		  sq2,
            -sq2,		0.0f,		  sq2,
            sq2,		1.0f,		  sq2,
            sq2,		0.0f,		  sq2,
            0.0f,		1.0f,		 1.0f,
            0.0f,		0.0f,		 1.0f};
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
        glVertexPointer( 3, GL_FLOAT, 0, points);

        float t = (float)units.current_tick / TICKS_PER_SEC;

        glPolygonOffset(-1.0f,-1.0f);

        DRAWING_TABLE	drawing_table;
        QUAD_TABLE		quad_table;

        pMutex.lock();
        for(int e=0;e<list_size;e++)
        {
            if( !(e & 15) ) {
                pMutex.unlock();
                pMutex.lock();
            }
            int i=list[e];
            if(feature[i].type<0)	continue;
            if(!feature[i].draw)	continue;

            if( cam->mirror && ((feature_manager.feature[feature[i].type].height>5.0f && feature_manager.feature[feature[i].type].m3d)			// Perform a small visibility check
                                || (feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL)) )
            {
                VECTOR Pos = feature[i].Pos;
                if( feature_manager.feature[feature[i].type].m3d )
                    Pos.y += feature_manager.feature[feature[i].type].model->size2;
                else
                    Pos.y += feature_manager.feature[feature[i].type].height*0.5f;

                float a = cam->rpos.y - units.map->sealvl;
                float b = Pos.y - units.map->sealvl;
                float c = a + b;
                if( c == 0.0f )
                    continue;
                Pos = (a / c) * Pos + (b / c) * cam->rpos;
                Pos.y = units.map->get_unit_h( Pos.x, Pos.z );

                if( Pos.y > units.map->sealvl )	// If it's not visible don't draw it
                    continue;
            }

            if(!feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].anim.nb_bmp>0)
            {
                feature_manager.feature[feature[i].type].convert();		// Convert texture data if needed

                feature[i].frame = (units.current_tick >> 1) % feature_manager.feature[feature[i].type].anim.nb_bmp;

                if(!texture_loaded || old!=feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame])
                {
                    old=feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame];
                    texture_loaded=true;
                    glBindTexture(GL_TEXTURE_2D,feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame]);
                }
                VECTOR Pos = feature[i].Pos;
                float	h=feature_manager.feature[feature[i].type].height*0.5f;
                float	dw=0.5f*feature_manager.feature[feature[i].type].anim.w[feature[i].frame];
                if(feature_manager.feature[feature[i].type].height>5.0f)
                {
                    dw *= h / feature_manager.feature[feature[i].type].anim.h[feature[i].frame];

                    if(feature[i].grey)
                        glColor4ub( 127, 127, 127, 255 );
                    else
                        glColor4ub( 255, 255, 255, 255 );

                    if (!set)
                    {
                        set=true;
                        glDisableClientState(GL_NORMAL_ARRAY);
                        glDisableClientState(GL_COLOR_ARRAY);
                        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
                        glVertexPointer( 3, GL_FLOAT, 0, points);
                    }

                    glPushMatrix();
                    glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
                    glScalef(dw,h,dw);
                    glDrawElements(GL_QUADS, 28,GL_UNSIGNED_BYTE,index);		// dessine le tout
                    glPopMatrix();
                }
                else
                    if( !cam->mirror ) 	// no need to draw things we can't see
                    {
                        dw *= 0.5f;
                        h = 0.25f*feature_manager.feature[feature[i].type].anim.h[feature[i].frame];

                        quad_table.queue_quad( feature_manager.feature[feature[i].type].anim.glbmp[feature[i].frame], QUAD( Pos, dw, h, feature[i].grey ? 0x7F7F7FFF : 0xFFFFFFFF ) );
                    }
            }
            else
                if(feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL)
                {

                    if( !feature_manager.feature[feature[i].type].model->animated && !feature[i].sinking )
                        drawing_table.queue_instance( feature_manager.feature[feature[i].type].model->id, INSTANCE( feature[i].Pos, feature[i].grey ? 0x7F7F7FFF : 0xFFFFFFFF, feature[i].angle ) );
                    else {
                        if(feature[i].grey)
                            glColor4ub( 127, 127, 127, 255 );
                        else
                            glColor4ub( 255, 255, 255, 255 );
                        glEnable(GL_LIGHTING);
                        glDisable(GL_BLEND);
                        if(!feature_manager.feature[feature[i].type].converted)				// To fix opacity with converted models
                            glDisable(GL_ALPHA_TEST);
                        glPushMatrix();
                        glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
                        glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
                        glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
                        feature_manager.feature[feature[i].type].model->draw(t,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,!feature[i].grey);

                        gfx->ReInitAllTex( true );

                        glPopMatrix();
                        glEnable(GL_BLEND);
                        if(!feature_manager.feature[feature[i].type].converted)				// To fix opacity with converted models
                            glEnable(GL_ALPHA_TEST);
                        glDisable(GL_LIGHTING);
                        glDisable(GL_CULL_FACE);
                        glEnable(GL_TEXTURE_2D);
                        texture_loaded=false;
                        set=false;
                    }
                }
        }
        pMutex.unlock();

        glColor4ub( 255, 255, 255, 255 );

        gfx->ReInitAllTex( true );

        glEnable(GL_POLYGON_OFFSET_FILL);
        quad_table.draw_all();
        glDisable(GL_POLYGON_OFFSET_FILL);

        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_CULL_FACE);

        drawing_table.draw_all();

        glDisable(GL_ALPHA_TEST);
        glDepthFunc( GL_LESS );
        glEnable(GL_TEXTURE_2D);
    }



    void FEATURES::draw_shadow(Camera* cam,VECTOR Dir)
    {
        if(nb_features<=0) return;
        cam->setView();
        float t = (float)units.current_tick / TICKS_PER_SEC;
        pMutex.lock();
        for (int e = 0; e < list_size; ++e)
        {
            pMutex.unlock();
            pMutex.lock();
            int i=list[e];
            if(feature[i].type<0)
                continue;
            if(!(!feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].anim.nb_bmp > 0))
            {
                if(feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL)
                {
                    if (!feature[i].draw)
                        continue;
                    if (feature[i].grey)
                        continue;				// No need to draw a texture here
                    if (feature_manager.feature[feature[i].type].converted)	// Quelques problèmes (graphiques et plantages) avec les modèles convertis
                        continue;

                    if (feature[ i ].delete_shadow_dlist && feature[ i ].shadow_dlist != 0 )
                    {
                        glDeleteLists( feature[ i ].shadow_dlist, 1 );
                        feature[ i ].shadow_dlist = 0;
                        feature[ i ].delete_shadow_dlist = false;
                    }

                    if( feature_manager.feature[feature[i].type].model->animated || feature[i].sinking || feature[i].shadow_dlist == 0 )
                    {
                        bool create_display_list = false;
                        if( !feature_manager.feature[feature[i].type].model->animated && !feature[i].sinking && feature[i].shadow_dlist == 0 )
                        {
                            feature[i].shadow_dlist = glGenLists (1);
                            glNewList( feature[i].shadow_dlist, GL_COMPILE_AND_EXECUTE);
                            create_display_list = true;
                            feature[ i ].delete_shadow_dlist = false;
                        }

                        glPushMatrix();
                        glTranslatef(feature[i].Pos.x,feature[i].Pos.y,feature[i].Pos.z);
                        glRotatef( feature[i].angle, 0.0f, 1.0f, 0.0f );
                        glRotatef( feature[i].angle_x, 1.0f, 0.0f, 0.0f );
                        VECTOR R_Dir = (sqrt(feature_manager.feature[feature[i].type].model->size)*2.0f+feature[i].Pos.y) * Dir * RotateY( -feature[i].angle * DEG2RAD ) * RotateX( -feature[i].angle_x * DEG2RAD );
                        if(g_useStencilTwoSide)													// Si l'extension GL_EXT_stencil_two_side est disponible
                            feature_manager.feature[feature[i].type].model->draw_shadow( R_Dir,t,NULL);
                        else
                            feature_manager.feature[feature[i].type].model->draw_shadow_basic( R_Dir,t,NULL);
                        glPopMatrix();

                        if( create_display_list )
                            glEndList();
                    }
                    else
                        glCallList( feature[i].shadow_dlist );
                }
            }
        }
        pMutex.unlock();
    }

    void FEATURES::move(float dt,MAP *map,bool clean)
    {
        if(nb_features<=0) return;

        pMutex.lock();

        for(int e=0;e<list_size;e++)
        {
            int i=list[e];
            if(feature[i].type<0)	continue;
            if(feature[i].hp<=0.0f && !feature[i].burning) {
                delete_feature(i);
                continue;
            }
            if(!feature_manager.feature[feature[i].type].vent && !feature[i].burning ) {
                feature[i].draw=false;
                continue;
            }
            feature[i].dt+=dt;
            if(feature[i].dt>0.2f && feature[i].draw) {
                if(feature[i].burning && feature[i].dt>0.3f) {
                    VECTOR t_mod;
                    bool random_vector = true;
                    if(feature_manager.feature[feature[i].type].m3d && feature_manager.feature[feature[i].type].model!=NULL ) {
                        OBJECT *obj = &(feature_manager.feature[feature[i].type].model->obj);
                        for( int base_n = rand_from_table(), n = 0 ; random_vector && n < obj->nb_sub_obj ; n++ )
                            random_vector = obj->random_pos( NULL, (base_n + n) % obj->nb_sub_obj, &t_mod );
                    }
                    else
                        random_vector = false;

                    if( random_vector )
                        t_mod = feature[i].Pos+t_mod;
                    else
                        t_mod = feature[i].Pos;

                    feature[i].dt=0.0f;
                    particle_engine.make_fire(t_mod,1,1,30.0f);
                }
                else if(!feature[i].burning) {
                    feature[i].dt=0.0f;
                    particle_engine.make_smoke(feature[i].Pos,0,1,5.0f,-1.0f,0.0f,0.25f);
                }
            }
            if(clean)
                feature[i].draw=false;
        }
        pMutex.unlock();
    }

    void FEATURES::compute_on_map_pos( int idx )
    {
        feature[ idx ].px = (int)(feature[idx].Pos.x) + the_map->map_w_d - 8 >> 3;
        feature[ idx ].py = (int)(feature[idx].Pos.z) + the_map->map_h_d - 8 >> 3;
    }

    void FEATURES::burn_feature( int idx )
    {
        pMutex.lock();

        if( idx >= 0 && idx < max_features && feature[ idx ].type >= 0
            && feature_manager.feature[ feature[ idx ].type ].flamable && !feature[ idx ].burning ) {		// We get something to burn !!
            feature[ idx ].burning = true;
            feature[ idx ].burning_time = 0.0f;
            int time_zone = abs( feature_manager.feature[ feature[idx].type ].burnmax - feature_manager.feature[ feature[idx].type ].burnmin ) + 1;
            feature[ idx ].time_to_burn = ( rand_from_table() % time_zone ) + feature_manager.feature[ feature[idx].type ].burnmin;		// How long it burns
            burning_features.push_back( idx );		// It's burning 8)

            // Start doing damages to things around
            if( feature_manager.feature[ feature[ idx ].type ].burnweapon ) {
                int w_idx = weapon_manager.get_weapon_index( feature_manager.feature[ feature[ idx ].type ].burnweapon );
                feature[ idx ].BW_idx = w_idx;
            }
            else
                feature[ idx ].BW_idx = -1;
            feature[ idx ].weapon_counter = 0;
        }

        pMutex.unlock();
    }

    void FEATURES::sink_feature( int idx )
    {
        pMutex.lock();
        // We get something to sink
        if( idx >= 0 && idx < max_features && feature[ idx ].type >= 0 && !feature[ idx ].sinking )
        {
            feature[ idx ].sinking = true;
            sinking_features.push_back( idx );		// It's burning 8)
        }
        pMutex.unlock();
    }

    void FEATURES::move_forest(const float &dt)			// Simulates forest fires & tree reproduction
    {
        pMutex.lock();

        VECTOR wind = 0.1f * *p_wind_dir;

        int wind_x = (int)(2.0f * wind.x + 0.5f);
        int wind_z = (int)(2.0f * wind.z + 0.5f);

        byte CS_count = 0;
        bool erased = false;
        for(std::list< uint32 >::iterator i = burning_features.begin() ; i != burning_features.end() ; ) {		// Makes fire spread 8)
            CS_count++;
            if( !CS_count )
            {
                pMutex.unlock();
                pMutex.lock();
            }
            feature[ *i ].burning_time += dt;
            if( feature[ *i ].burning_time >= feature[ *i ].time_to_burn ) // If we aren't burning anymore :(
            {
                if( network_manager.isServer() )
                    g_ta3d_network->sendFeatureDeathEvent( *i );

                feature[ *i ].burning = false;
                feature[ *i ].hp = 0.0f;

                int sx=((int)(feature[ *i ].Pos.x)+the_map->map_w_d-4)>>3;		// Delete the feature
                int sy=((int)(feature[ *i ].Pos.z)+the_map->map_h_d-4)>>3;
                // Remove it from map
                the_map->rect(sx-(feature_manager.feature[features.feature[*i].type].footprintx>>1),sy-(feature_manager.feature[features.feature[*i].type].footprintz>>1),feature_manager.feature[features.feature[*i].type].footprintx,feature_manager.feature[features.feature[*i].type].footprintz,-1);

                // Replace the feature if needed (with the burnt feature)
                if( feature_manager.feature[ feature[*i].type ].feature_burnt )
                {
                    int burnt_type = feature_manager.get_feature_index( feature_manager.feature[ feature[*i].type ].feature_burnt );
                    if( burnt_type >= 0 ) {
                        the_map->map_data[ sy ][ sx ].stuff = features.add_feature( feature[ *i ].Pos, burnt_type );
                        if( burnt_type != -1 && feature_manager.feature[ burnt_type ].blocking )
                            the_map->rect( sx-(feature_manager.feature[ burnt_type ].footprintx>>1), sy-(feature_manager.feature[ burnt_type ].footprintz>>1), feature_manager.feature[ burnt_type ].footprintx, feature_manager.feature[ burnt_type ].footprintz,-2-the_map->map_data[sy][sx].stuff );
                        if( network_manager.isServer() )
                            g_ta3d_network->sendFeatureDeathEvent( the_map->map_data[ sy ][ sx ].stuff );
                    }
                }

                delete_feature( *i );

                i = burning_features.erase( i );

                erased = true;
            }
            else {
                erased = false;												// Still there

                if( feature[ *i ].BW_idx >= 0 && !feature[ *i ].weapon_counter ) {										// Don't stop damaging things before the end!!
                    int w_idx = weapons.add_weapon( feature[ *i ].BW_idx, -1 );
                    if( w_idx >= 0 ) {
                        weapons.weapon[ w_idx ].just_explode = true;
                        weapons.weapon[ w_idx ].Pos = feature[ *i ].Pos;
                        weapons.weapon[ w_idx ].owner = 0xFF;
                        weapons.weapon[ w_idx ].local = true;
                    }
                }

                feature[ *i ].weapon_counter = ( feature[ *i ].weapon_counter + TICKS_PER_SEC - 1 ) % TICKS_PER_SEC;

                if( !network_manager.isConnected() || network_manager.isServer() ) {
                    feature[ *i ].last_spread += dt;
                    if( feature[ *i ].burning_time >= feature_manager.feature[ feature[ *i ].type ].sparktime && feature[ *i ].last_spread >= 0.1f ) {		// Can spread
                        feature[ *i ].last_spread = 0.0f;
                        int spread_score = rand_from_table() % 100;
                        if( spread_score < feature_manager.feature[ feature[ *i ].type ].spreadchance ) {		// It tries to spread :)
                            int rnd_x = feature[ *i ].px + (rand_from_table() % 12) - 6 + wind_x;		// Random pos in neighborhood, but affected by wind :)
                            int rnd_y = feature[ *i ].py + (rand_from_table() % 12) - 6 + wind_z;

                            if( rnd_x >= 0 && rnd_y >= 0 && rnd_x < the_map->bloc_w_db && rnd_y < the_map->bloc_h_db ) {		// Check coordinates are valid
                                burn_feature( units.map->map_data[ rnd_y ][ rnd_x ].stuff );			// Burn it if there is something to burn 8)
                                if( network_manager.isServer() )
                                    g_ta3d_network->sendFeatureFireEvent( units.map->map_data[ rnd_y ][ rnd_x ].stuff );
                            }
                        }
                    }
                }
            }

            if( !erased ) i++;			// We don't want to skip an element :) 
        }

        for(std::list< uint32 >::iterator i = sinking_features.begin() ; i != sinking_features.end() ; )		// A boat is sinking
            if( feature[*i].sinking ) {
                if( feature[*i].angle_x > -45.0f && !feature[*i].dive ) {
                    feature[*i].angle_x -= dt * 15.0f;
                    feature[*i].dive_speed = 0.0f;
                }
                else
                    feature[*i].dive = true;
                float sea_ground = the_map->get_unit_h( feature[*i].Pos.x, feature[*i].Pos.z );
                if( sea_ground < feature[*i].Pos.y ) {
                    if( sin( -feature[*i].angle_x * DEG2RAD ) * feature_manager.feature[feature[*i].type].footprintx * 8.0f > feature[*i].Pos.y - sea_ground ) {
                        feature[*i].angle_x = RAD2DEG * asin( ( sea_ground - feature[*i].Pos.y ) / ( feature_manager.feature[feature[*i].type].footprintx * 8.0f) );
                        feature[*i].dive = true;
                    }
                    feature[*i].dive_speed = ( feature[*i].dive_speed + 3.0f * dt ) * exp( -dt );
                    feature[*i].Pos.y -= feature[*i].dive_speed * dt;
                }
                else {
                    feature[*i].sinking = false;
                    feature[*i].dive_speed = 0.0f;
                    feature[*i].angle_x = 0.0f;
                }
                i++;
            }
            else
                sinking_features.erase( i++ );

        pMutex.unlock();
    }

    void FEATURES::display_info( int idx )
    {
        if( idx < 0 || idx >= max_features || feature[ idx ].type < 0 )	return;		// Nothing to display

        if(feature_manager.feature[ feature[ idx ].type ].description)
        {
            if( feature_manager.feature[ feature[ idx ].type ].reclaimable )
                gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Description.x1,ta3dSideData.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF, format("%s M:%d E:%d",TRANSLATE( feature_manager.feature[ feature[ idx ].type ].description ).c_str(),feature_manager.feature[ feature[ idx ].type ].metal,feature_manager.feature[ feature[ idx ].type ].energy) );
            else
                gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Description.x1,ta3dSideData.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF, TRANSLATE( feature_manager.feature[ feature[ idx ].type ].description ) );
        }

        glDisable(GL_BLEND);
    }


} // namespace TA3D

