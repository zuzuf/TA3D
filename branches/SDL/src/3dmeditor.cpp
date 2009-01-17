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
#include "threads/cThread.h"
#include "misc/math.h"
#include "gfx/shader.h"

#define TA3D_BASIC_ENGINE
#include "ta3d.h"			// Moteur
#include "gui.h"			// Interface utilisateur
#include "TA3D_hpi.h"		// Interface HPI requis pour 3do.h
#include "gfx/particles/particles.h"
#include "gaf.h"
#include "3do.h"			// Gestion des modèles 3D
#include "misc/paths.h"
#include "misc/resources.h"
#include "logs/logs.h"
#include "gfx/glfunc.h"
#include "3dmeditor.h"		// GUI functions for the editor
#include "misc/camera.h"
#include <vector>
#include "languages/i18n.h"
#include "gfx/gui/area.h"
#include "misc/application.h"
#include "misc/settings.h"
#include "animator/animator.h"


#define precision	MSEC_TO_TIMER(1)



volatile uint32 msec_timer = 0;

void Timer()            // procédure Timer
{
    msec_timer++;
}

END_OF_FUNCTION(Timer)

    int expected_players=1;
    int LANG = TA3D_LANG_ENGLISH;

    namespace TA3D
{
    namespace VARS
    {
        MODEL *TheModel;
        std::vector<OBJECT*>    obj_table;
        std::vector<int> h_table;
        OBJECT_SURFACE obj_surf;
    }
}


/*---------------------------------------------------------------------------------------------------\
  |                                              void main()                                           |
  |               Procédure principale,gère l'interface d'édition principale                           |
  \---------------------------------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Initialize all modules used by ta3d
    TA3D::Initialize(argc, argv, "3dmeditor");

    try
    {
        // this really should be safe, but we'll try catch it
        // anyhow just in case.
        TA3D::VARS::lp_CONFIG = new TA3D::TA3DCONFIG;
    }
    catch( ... )
    {
        // if we get here I have no clue what went wrong.
        //   most likely outa memory?

        // construct a error object.
        cError err( "main creating config structure.", GETSYSERROR(), false );
        err.DisplayError(); // show the error.

        exit(1); // were outa here.
    }

    init();

    TA3D::Settings::Load();
    lp_CONFIG->use_texture_cache = false;

    install_int_ex( Timer, precision);

    init_surf_buf();

    LOG_INFO(I18N::Translate("Initializing texture manager"));
    texture_manager.init();
    texture_manager.all_texture();

    TheModel = new MODEL;
    obj_table.clear();
    obj_table.push_back( &(TheModel->obj ));

    show_video_bitmap(screen);

    bool done=false;

    Camera DefCam;

    glClearColor(0.0f,0.0f,0.0f,0.0f);

    GLfloat LightAmbient[]=		{0.125f,0.125f,0.125f,1.0f};
    GLfloat LightDiffuse[]=		{1.0f,1.0f,1.0f,1.0f};
    GLfloat LightSpecular[]=	{1.0f,1.0f,1.0f,1.0f};
    GLfloat LightPosition[]=	{200.0f,200.0f,200.0f,1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);   // Setup The Diffuse Light
    glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);	// Position The Light
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);		// Attenuation

    glClearColor (0, 0, 0, 0);
    glShadeModel (GL_SMOOTH);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glPolygonMode (GL_BACK, GL_POINTS);
    glEnable (GL_DEPTH_TEST);
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glEnable (GL_LIGHT0);
    glEnable (GL_LIGHTING);
    glEnable (GL_COLOR_MATERIAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

    float r1,r2,r3;
    r1=r2=r3=0.0f;
    r1=30.0f;

    int counter=0;
    int FPS=0;
    int FPS_Timer = msec_timer;
    float Conv = 0.001f;

        // Load the interface from our area file:
    AREA main_area;
    main_area.load_tdf("gui/3dmeditor_main.area");
    main_area.set_action("edit.b_rename", button_rename);
    main_area.set_action("edit.b_child", button_child);
    main_area.set_action("edit.b_remove", button_remove);
    main_area.set_action("edit.b_scale", button_scale);
    main_area.set_action("edit.b_mirror_x", button_mirror_x);
    main_area.set_action("edit.b_mirror_y", button_mirror_y);
    main_area.set_action("edit.b_mirror_z", button_mirror_z);
    main_area.set_action("edit.b_change_xy", button_change_xy);
    main_area.set_action("edit.b_change_yz", button_change_yz);
    main_area.set_action("edit.b_change_zx", button_change_zx);

    int amx,amy,amz,amb;
    int IsOnGUI;

    float ScaleFactor=1.0f;
    uint32 timer = msec_timer;

    SCRIPT_DATA cur_data;
    cur_data.load(nb_obj());

    do
    {
        GUIOBJ *obj = main_area.get_object("edit.menu_selec");
        if (obj )
        {
        obj->Text.resize( nb_obj() + 1 );
        for(int i=0;i<nb_obj();i++)
            if (!obj_table[i]->name.empty())
            {
                String h="";
                for(int e=0;e<h_table[i];e++)	h+="-";
                obj->Text[i+1] = (h+obj_table[i]->name);
            }

        obj->Text[0] = obj->Text[cur_part+1];
        }

        bool key_is_pressed = false;
        do
        {
            amx = mouse_x;
            amy = mouse_y;
            amz = mouse_z;
            amb = mouse_b;

            key_is_pressed = keypressed();
            IsOnGUI = main_area.check();
            rest( 8 );
            if (!IsOnGUI && mouse_b)    break;
            if (msec_timer - timer >= 100)  break;
        } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !main_area.scrolling );
        timer = msec_timer;

        if (!IsOnGUI)                // Si la souris n'est pas sur un élément de l'interface utilisateur(si elle est sur la fenêtre 3D)
        {
            if (mouse_b==1)         // Appui sur le bouton gauche pour effecture une rotation
                r2+=mouse_x-amx;
            if (mouse_b==4)			// Appui sur le bouton du milieu pour regler le zoom
                ScaleFactor+=((mouse_y-amy)+(mouse_x-amx)+(mouse_z-amz))*0.1f;
            if (mouse_b==2 && cur_part>=0 && cur_part<nb_obj())             // Left-clic to move current object
            {
                Vector3D DP( 0.1f*(mouse_x-amx)*cosf(r2*DEG2RAD), -(mouse_y-amy)*0.1f, 0.1f*(mouse_x-amx)*sinf(r2*DEG2RAD) );
                obj_table[cur_part]->pos_from_parent = obj_table[cur_part]->pos_from_parent + DP;
                for( int i = 0 ; i < obj_table[cur_part]->nb_vtx ; i++ )
                    obj_table[cur_part]->points[ i ] = obj_table[cur_part]->points[ i ] - DP;
                if (obj_table[cur_part]->child)
                {
                    OBJECT *cur = obj_table[cur_part]->child;
                    while (cur)
                    {
                        cur->pos_from_parent = cur->pos_from_parent - DP;
                        cur = cur->next;
                    }
                }
            }
        }
        else
        {
            if (main_area.get_value("menu.file") >= 0)
                mnu_file( main_area.get_value("menu.file") );
            if (main_area.get_value("menu.surface") >= 0)
                mnu_surf( main_area.get_value("menu.surface") );
            if (main_area.get_value("edit.menu_selec") >= 0)
                mnu_selec( main_area.get_value("edit.menu_selec") );
            if (main_area.get_state("menu.animator"))
                Editor::Menus::Animator::Execute();
        }

        counter++;
        if (msec_timer-FPS_Timer>=50)			// Calcule du nombre d'images par secondes
        {
            FPS = (int)(counter/((msec_timer-FPS_Timer)*Conv));
            counter = 0;
            FPS_Timer = msec_timer;
        }

        show_mouse(NULL);					// Cache la souris
        if (key[KEY_ESC] || ClickOnExit) done=true;			// Quitte si on appuie sur echap ou clique sur quitter

        if (key[ KEY_X ] && !key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 0.0f;		r3 = 0.0f;	}
        if (key[ KEY_Y ] && !key[ KEY_LSHIFT ] ) {	r1 = 90.0f;		r2 = 0.0f;		r3 = 0.0f;	}
        if (key[ KEY_Z ] && !key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = -90.0f;		r3 = 0.0f;	}

        if (key[ KEY_X ] && key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 180.0f;	r3 = 0.0f;	}
        if (key[ KEY_Y ] && key[ KEY_LSHIFT ] ) {	r1 = -90.0f;	r2 = 0.0f;		r3 = 0.0f;	}
        if (key[ KEY_Z ] && key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 90.0f;	r3 = 0.0f;	}

        gfx->SetDefState();
        // Efface tout
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DefCam.setView();				// Fixe la caméra
        glTranslatef(ScaleFactor * DefCam.dir.x, ScaleFactor * DefCam.dir.y, ScaleFactor * DefCam.dir.z);

        glTranslatef(0.0f,-10.0f,0.0f);

        glRotatef(r1,1.0f,0.0f,0.0f);		// Rotations de l'objet
        glRotatef(r2,0.0f,1.0f,0.0f);
        glRotatef(r3,0.0f,0.0f,1.0f);

        glDisable(GL_LIGHTING);					// Dessine le repère
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
        glColor3f(1.0f,0.0f,0.0f);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(10.0f,0.0f,0.0f);
        glVertex3f(9.0f,1.0f,0.0f);
        glVertex3f(10.0f,0.0f,0.0f);
        glVertex3f(9.0f,-1.0f,0.0f);
        glVertex3f(10.0f,0.0f,0.0f);

        glColor3f(0.0f,1.0f,0.0f);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,10.0f,0.0f);
        glVertex3f(0.0f,9.0f,1.0f);
        glVertex3f(0.0f,10.0f,0.0f);
        glVertex3f(0.0f,9.0f,-1.0f);
        glVertex3f(0.0f,10.0f,0.0f);

        glColor3f(0.0f,0.0f,1.0f);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,0.0f,10.0f);
        glVertex3f(1.0f,0.0f,9.0f);
        glVertex3f(0.0f,0.0f,10.0f);
        glVertex3f(-1.0f,0.0f,9.0f);
        glVertex3f(0.0f,0.0f,10.0f);
        glEnd();
        glEnable(GL_LIGHTING);

        if (cur_data.nb_piece!=nb_obj())
        {
            cur_data.destroy();
            cur_data.load(nb_obj());
        }

        glDisable( GL_CULL_FACE );
        if (IsOnGUI && main_area.get_state("edit.menu_selec"))				// Affiche la partie prête à être sélectionnée
        {
            glColor3f(0.15f,0.15f,0.15f);
            for (int i=0; i < cur_data.nb_piece; ++i)
                cur_data.flag[i] = FLAG_ANIMATE;
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            gfx->ReInitAllTex(true);
            glEnable(GL_TEXTURE_2D);
            glClear(GL_DEPTH_BUFFER_BIT);
            for (int i = 0; i < cur_data.nb_piece; ++i)
                cur_data.flag[i] = ((i == main_area.get_object("edit.menu_selec")->Data) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
            glColor3f(1.0f,1.0f,1.0f);
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            glColor3f(1.0f,1.0f,1.0f);
            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
        }
		else
		{
			if (TheModel) // Affichage normal
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				for (int i = 0; i < cur_data.nb_piece; ++i)
					cur_data.flag[i]= ((i != cur_part) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
				TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
				gfx->ReInitAllTex( true );
				glEnable(GL_TEXTURE_2D);
				for(int i = 0; i < cur_data.nb_piece; ++i)
					cur_data.flag[i] = ((i == cur_part) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
				float col = cosf(msec_timer*0.002f)*0.375f+0.625f;
				glColor3f(col, col, col);
				TheModel->draw(msec_timer * 0.001f, &cur_data, false, false, false, 0,
					NULL, NULL, NULL, 0.0f, NULL, false, 0, false);
				glColor3f(1.0f, 1.0f, 1.0f);
				gfx->ReInitAllTex(true);
				glEnable(GL_TEXTURE_2D);
			}
		}
		glEnable(GL_CULL_FACE);

        //----------- draw the sphere that shows where is attached the current object ------------------

        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        glDisable( GL_LIGHTING );
        MATRIX_4x4 M = Scale( 1.0f );
        TheModel->compute_coord( &cur_data, &M );
        Vector3D P = cur_data.pos[ cur_part ];

        glPushMatrix();
        glDisable(GL_TEXTURE_2D);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );
        glColor4ub( 0xFF, 0x0, 0x0, 0x7F );
        glTranslatef( P.x, P.y, P.z );

        glBegin( GL_TRIANGLES );

        float s = 5.0f;

        for( int i = 0 ; i < 8 ; i++)
        {
            glVertex3f( (i & 1) ? s : -s, 0.0f, 0.0f );
            glVertex3f( 0.0f, (i & 2) ? s : -s, 0.0f );
            glVertex3f( 0.0f, 0.0f, (i & 4) ? s : -s );
        }

        glEnd();

        glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
        glEnable( GL_CULL_FACE );
        glEnable( GL_LIGHTING );
        glEnable( GL_DEPTH_TEST );

        //----------------------------------------------------------------------------------------------

        gfx->set_2D_mode();		// Passe en mode dessin allegro

        main_area.draw();

        glEnable(GL_TEXTURE_2D);			// Affiche le nombre d'images par secondes
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
        gfx->print(gfx->normal_font,SCREEN_W-80,3.0f,0.0f,0xFFFFFFFF,format("FPS: %d",FPS));
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
        show_mouse(screen);
        algl_draw_mouse();

        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

        // Affiche
        gfx->flip();

    }while(!done);

    cur_data.destroy();

    texture_manager.destroy();

    delete HPIManager;

    obj_table.clear();
    h_table.clear();

    delete TheModel;

    delete gfx;

    return 0;
}
END_OF_MAIN()

    /*------------------------------------------------------------------------------------\
      |                                  void SurfEdit()                                    |
      |         Cette fonction affiche une fenêtre permettant à l'utilisateur d'éditer la   |
      | surface de la partie sélectionnée ou de toute autre partie de la meshe.             |
      |                                                                                     |
      \------------------------------------------------------------------------------------*/

void S_MPart_Sel(int mnu_index)
{
    if (mnu_index>=0 && mnu_index<nb_obj())
        cur_part=mnu_index;
}

void SurfEdit()
{
    if (nb_obj()<=0) return;			// S'il n'y a rien à éditer, on quitte tout de suite la fonction, sinon ça plante
    if (nb_obj()==1 && TheModel->obj.name.empty())	return;

    SCRIPT_DATA cur_data;           // needed to render the preview
    cur_data.load(nb_obj());

    AREA surface_area;
    surface_area.load_tdf("gui/3dmeditor_surface.area");

    surface_area.set_caption("surface.t_mred", format("%d",(int)(obj_table[cur_part]->surface.Color[0]*255.0f)) );
    surface_area.set_caption("surface.t_mgreen", format("%d",(int)(obj_table[cur_part]->surface.Color[1]*255.0f)) );
    surface_area.set_caption("surface.t_mblue", format("%d",(int)(obj_table[cur_part]->surface.Color[2]*255.0f)) );
    surface_area.set_caption("surface.t_malpha", format("%d",(int)(obj_table[cur_part]->surface.Color[3]*255.0f)) );

    surface_area.set_state("surface.c_reflection", obj_table[cur_part]->surface.Flag&SURFACE_REFLEC );

    surface_area.set_caption("surface.t_rred", format("%d",(int)(obj_table[cur_part]->surface.RColor[0]*255.0f)) );
    surface_area.set_caption("surface.t_rgreen", format("%d",(int)(obj_table[cur_part]->surface.RColor[1]*255.0f)) );
    surface_area.set_caption("surface.t_rblue", format("%d",(int)(obj_table[cur_part]->surface.RColor[2]*255.0f)) );
    surface_area.set_caption("surface.t_ralpha", format("%d",(int)(obj_table[cur_part]->surface.RColor[3]*255.0f)) );

    surface_area.set_state("surface.c_gouraud", obj_table[cur_part]->surface.Flag&SURFACE_GOURAUD );
    surface_area.set_state("surface.c_lighting", obj_table[cur_part]->surface.Flag&SURFACE_LIGHTED );
    surface_area.set_state("surface.c_texturing", obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED );

    surface_area.set_action("surface.b_painting", SurfPaint);
    surface_area.set_action("surface.b_texture_mapping", TexturePosEdit);

    String::Vector Part_names(1+nb_obj());
    for(int i=0;i<nb_obj();i++)
        Part_names[i+1] = obj_table[i]->name;
    Part_names[0] = Part_names[cur_part+1];

    surface_area.set_entry("surface.m_part", Part_names);
    surface_area.set_action("surface.m_part", S_MPart_Sel);

    surface_area.set_state("surface.c_transparency", obj_table[cur_part]->surface.Flag&SURFACE_BLENDED );
    surface_area.set_state("surface.c_player_color", obj_table[cur_part]->surface.Flag&SURFACE_PLAYER_COLOR );
    surface_area.set_state("surface.c_glsl", obj_table[cur_part]->surface.Flag&SURFACE_GLSL );

    surface_area.set_state("surface.c_rotation", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION) );
    surface_area.set_state("surface.c_rperiodic", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION_PERIODIC) );
    surface_area.set_state("surface.c_rsinus", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION_COSINE) );

    surface_area.set_state("surface.c_translation", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION) );
    surface_area.set_state("surface.c_tperiodic", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION_PERIODIC) );
    surface_area.set_state("surface.c_tsinus", obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION_COSINE) );

    surface_area.set_caption("surface.t_angle0_x", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.x) : "0" );
    surface_area.set_caption("surface.t_angle0_y", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.y) : "0" );
    surface_area.set_caption("surface.t_angle0_z", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.z) : "0" );

    surface_area.set_caption("surface.t_angle1_x", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.x) : "0" );
    surface_area.set_caption("surface.t_angle1_y", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.y) : "0" );
    surface_area.set_caption("surface.t_angle1_z", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.z) : "0" );

    surface_area.set_caption("surface.t_translate0_x", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.x) : "0" );
    surface_area.set_caption("surface.t_translate0_y", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.y) : "0" );
    surface_area.set_caption("surface.t_translate0_z", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.z) : "0" );

    surface_area.set_caption("surface.t_translate1_x", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.x) : "0" );
    surface_area.set_caption("surface.t_translate1_y", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.y) : "0" );
    surface_area.set_caption("surface.t_translate1_z", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.z) : "0" );

    surface_area.set_caption("surface.t_angle_w", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_w) : "0" );
    surface_area.set_caption("surface.t_translate_w", obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_w) : "0" );

    bool done=false;

    int amx,amy,amb,amz;
    uint32 timer = msec_timer;
    float scaleFactor = 1.0f;

    do
    {
        bool key_is_pressed = false;
        do
        {
            amx = mouse_x;
            amy = mouse_y;
            amz = mouse_z;
            amb = mouse_b;

            key_is_pressed = keypressed();
            surface_area.check();
            rest( 8 );
            if (msec_timer - timer >= 50)  break;
        } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !surface_area.scrolling );
        timer = msec_timer;

        if (surface_area.get_state("surface.b_painting"))
            amz = mouse_z;

        if (surface_area.get_state("surface.b_glsl"))
        {
            glslEditor();
        }

        if (surface_area.get_state("surface.c_rotation"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION;
        }
        else if (obj_table[cur_part]->animation_data)
            obj_table[cur_part]->animation_data->type &= ~ROTATION;

        if (surface_area.get_state("surface.c_rperiodic"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION_PERIODIC;
        }
        else if (obj_table[cur_part]->animation_data)
            obj_table[cur_part]->animation_data->type &= ~ROTATION_PERIODIC;

        if (surface_area.get_state("surface.c_rsinus"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION_COSINE;
        }
        else if (obj_table[cur_part]->animation_data)
            obj_table[cur_part]->animation_data->type &= ~ROTATION_COSINE;

        if (surface_area.get_state("surface.c_translation"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION;
        }
        else if (obj_table[cur_part]->animation_data)
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION;

        if (surface_area.get_state("surface.c_tperiodic"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION_PERIODIC;
        }
        else if (obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION_PERIODIC;

        if (surface_area.get_state("surface.c_tsinus"))
        {
            if (obj_table[cur_part]->animation_data == NULL)	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION_COSINE;
        }
        else if (obj_table[cur_part]->animation_data)
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION_COSINE;

        if (obj_table[cur_part]->animation_data)		// Read other data
        {
            obj_table[cur_part]->animation_data->angle_0.x = atof( surface_area.get_caption("surface.t_angle0_x").c_str());
            obj_table[cur_part]->animation_data->angle_0.y = atof( surface_area.get_caption("surface.t_angle0_y").c_str());
            obj_table[cur_part]->animation_data->angle_0.z = atof( surface_area.get_caption("surface.t_angle0_z").c_str());

            obj_table[cur_part]->animation_data->angle_1.x = atof( surface_area.get_caption("surface.t_angle1_x").c_str());
            obj_table[cur_part]->animation_data->angle_1.y = atof( surface_area.get_caption("surface.t_angle1_y").c_str());
            obj_table[cur_part]->animation_data->angle_1.z = atof( surface_area.get_caption("surface.t_angle1_z").c_str());

            obj_table[cur_part]->animation_data->translate_0.x = atof( surface_area.get_caption("surface.t_translate0_x").c_str());
            obj_table[cur_part]->animation_data->translate_0.y = atof( surface_area.get_caption("surface.t_translate0_y").c_str());
            obj_table[cur_part]->animation_data->translate_0.z = atof( surface_area.get_caption("surface.t_translate0_z").c_str());

            obj_table[cur_part]->animation_data->translate_1.x = atof( surface_area.get_caption("surface.t_translate1_x").c_str());
            obj_table[cur_part]->animation_data->translate_1.y = atof( surface_area.get_caption("surface.t_translate1_y").c_str());
            obj_table[cur_part]->animation_data->translate_1.z = atof( surface_area.get_caption("surface.t_translate1_z").c_str());

            obj_table[cur_part]->animation_data->angle_w = atof( surface_area.get_caption("surface.t_angle_w").c_str());
            obj_table[cur_part]->animation_data->translate_w = atof( surface_area.get_caption("surface.t_translate_w").c_str());
        }

        if (surface_area.get_state("surface.b_optimize_geometry") && !(obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED))
        {
            obj_geo_optimize( cur_part, true );
            obj_maj_normal( cur_part );
        }

        if (surface_area.get_object("surface.m_part") && surface_area.get_caption("surface.m_part") != surface_area.get_object("surface.m_part")->Text[cur_part+1]) // Si l'utilisateur a sélectionné une partie
        {
            surface_area.set_state("surface.c_gouraud", obj_table[cur_part]->surface.Flag&SURFACE_GOURAUD);
            surface_area.set_state("surface.c_lighting", obj_table[cur_part]->surface.Flag&SURFACE_LIGHTED);
            surface_area.set_state("surface.c_texturing", obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED);
            surface_area.set_state("surface.c_reflection", obj_table[cur_part]->surface.Flag&SURFACE_REFLEC);
            surface_area.set_state("surface.c_transparency", obj_table[cur_part]->surface.Flag&SURFACE_BLENDED);
            surface_area.set_state("surface.c_player_color", obj_table[cur_part]->surface.Flag&SURFACE_PLAYER_COLOR);
            surface_area.set_state("surface.c_glsl", obj_table[cur_part]->surface.Flag&SURFACE_GLSL);
            surface_area.set_caption("surface.t_mred", format("%d",(int)(obj_table[cur_part]->surface.Color[0]*255.0f)));
            surface_area.set_caption("surface.t_mgreen", format("%d",(int)(obj_table[cur_part]->surface.Color[1]*255.0f)));
            surface_area.set_caption("surface.t_mblue", format("%d",(int)(obj_table[cur_part]->surface.Color[2]*255.0f)));
            surface_area.set_caption("surface.t_malpha", format("%d",(int)(obj_table[cur_part]->surface.Color[3]*255.0f)));

            surface_area.set_caption("surface.t_rred", format("%d",(int)(obj_table[cur_part]->surface.RColor[0]*255.0f)));
            surface_area.set_caption("surface.t_rgreen", format("%d",(int)(obj_table[cur_part]->surface.RColor[1]*255.0f)));
            surface_area.set_caption("surface.t_rblue", format("%d",(int)(obj_table[cur_part]->surface.RColor[2]*255.0f)));
            surface_area.set_caption("surface.t_ralpha", format("%d",(int)(obj_table[cur_part]->surface.RColor[3]*255.0f)));

            if (obj_table[cur_part]->animation_data)
            {
                surface_area.set_caption("surface.t_angle0_x", format( "%f", obj_table[cur_part]->animation_data->angle_0.x ) );
                surface_area.set_caption("surface.t_angle0_y", format( "%f", obj_table[cur_part]->animation_data->angle_0.y ) );
                surface_area.set_caption("surface.t_angle0_z", format( "%f", obj_table[cur_part]->animation_data->angle_0.z ) );

                surface_area.set_caption("surface.t_angle1_x", format( "%f", obj_table[cur_part]->animation_data->angle_1.x ) );
                surface_area.set_caption("surface.t_angle1_y", format( "%f", obj_table[cur_part]->animation_data->angle_1.y ) );
                surface_area.set_caption("surface.t_angle1_z", format( "%f", obj_table[cur_part]->animation_data->angle_1.z ) );

                surface_area.set_caption("surface.t_translate0_x", format( "%f", obj_table[cur_part]->animation_data->translate_0.x ) );
                surface_area.set_caption("surface.t_translate0_y", format( "%f", obj_table[cur_part]->animation_data->translate_0.y ) );
                surface_area.set_caption("surface.t_translate0_z", format( "%f", obj_table[cur_part]->animation_data->translate_0.z ) );

                surface_area.set_caption("surface.t_translate1_x", format( "%f", obj_table[cur_part]->animation_data->translate_1.x ) );
                surface_area.set_caption("surface.t_translate1_y", format( "%f", obj_table[cur_part]->animation_data->translate_1.y ) );
                surface_area.set_caption("surface.t_translate1_z", format( "%f", obj_table[cur_part]->animation_data->translate_1.z ) );

                surface_area.set_caption("surface.t_angle_w", format( "%f", obj_table[cur_part]->animation_data->angle_w ) );
                surface_area.set_caption("surface.t_translate_w", format( "%f", obj_table[cur_part]->animation_data->translate_w ) );

                surface_area.set_state("surface.c_rotation", obj_table[cur_part]->animation_data->type & ROTATION);
                surface_area.set_state("surface.c_rperiodic", obj_table[cur_part]->animation_data->type & ROTATION_PERIODIC);
                surface_area.set_state("surface.c_rsinus", obj_table[cur_part]->animation_data->type & ROTATION_COSINE);

                surface_area.set_state("surface.c_translation", obj_table[cur_part]->animation_data->type & TRANSLATION);
                surface_area.set_state("surface.c_tperiodic", obj_table[cur_part]->animation_data->type & TRANSLATION_PERIODIC);
                surface_area.set_state("surface.c_tsinus", obj_table[cur_part]->animation_data->type & TRANSLATION_COSINE);
            }
            else
            {
                surface_area.set_caption("surface.t_angle0_x", "0" );
                surface_area.set_caption("surface.t_angle0_y", "0" );
                surface_area.set_caption("surface.t_angle0_z", "0" );

                surface_area.set_caption("surface.t_angle1_x", "0" );
                surface_area.set_caption("surface.t_angle1_y", "0" );
                surface_area.set_caption("surface.t_angle1_z", "0" );

                surface_area.set_caption("surface.t_translate0_x", "0" );
                surface_area.set_caption("surface.t_translate0_y", "0" );
                surface_area.set_caption("surface.t_translate0_z", "0" );

                surface_area.set_caption("surface.t_translate1_x", "0" );
                surface_area.set_caption("surface.t_translate1_y", "0" );
                surface_area.set_caption("surface.t_translate1_z", "0" );

                surface_area.set_caption("surface.t_angle_w", "0" );
                surface_area.set_caption("surface.t_translate_w", "0" );

                surface_area.set_state("surface.c_rotation", false);
                surface_area.set_state("surface.c_rperiodic", false);
                surface_area.set_state("surface.c_rsinus", false);

                surface_area.set_state("surface.c_translation", false);
                surface_area.set_state("surface.c_tperiodic", false);
                surface_area.set_state("surface.c_tsinus", false);
            }
        }
        if (surface_area.get_object("surface.m_part"))
            surface_area.get_object("surface.m_part")->Text[0] = surface_area.get_object("surface.m_part")->Text[cur_part+1];

        // Modifie la couleur de matière
        obj_table[cur_part]->surface.Color[0]=atoi(surface_area.get_caption("surface.t_mred").c_str())/255.0f;
        obj_table[cur_part]->surface.Color[1]=atoi(surface_area.get_caption("surface.t_mgreen").c_str())/255.0f;
        obj_table[cur_part]->surface.Color[2]=atoi(surface_area.get_caption("surface.t_mblue").c_str())/255.0f;
        obj_table[cur_part]->surface.Color[3]=atoi(surface_area.get_caption("surface.t_malpha").c_str())/255.0f;
        // Modifie la couleur de réflexion
        if (surface_area.get_state("surface.c_reflection"))
        {
            obj_table[cur_part]->surface.Flag|=SURFACE_REFLEC;
            obj_table[cur_part]->surface.RColor[0]=atoi(surface_area.get_caption("surface.t_rred").c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[1]=atoi(surface_area.get_caption("surface.t_rgreen").c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[2]=atoi(surface_area.get_caption("surface.t_rblue").c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[3]=atoi(surface_area.get_caption("surface.t_ralpha").c_str())/255.0f;
        }
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_REFLEC;

        // Active désactive certaines options de surface
        if (surface_area.get_state("surface.c_glsl"))								// Utilisation de GLSL
            obj_table[cur_part]->surface.Flag|=SURFACE_GLSL;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_GLSL;
        if (surface_area.get_state("surface.c_player_color"))								// Utilisation de la couleur du joueur
            obj_table[cur_part]->surface.Flag|=SURFACE_PLAYER_COLOR;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_PLAYER_COLOR;
        if (surface_area.get_state("surface.c_transparency"))								// Utilisation de la transparence
            obj_table[cur_part]->surface.Flag|=SURFACE_BLENDED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_BLENDED;
        if (surface_area.get_state("surface.c_gouraud"))								// Utilisation de l'éclairage GOURAUD
            obj_table[cur_part]->surface.Flag|=SURFACE_GOURAUD;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_GOURAUD;
        if (surface_area.get_state("surface.c_lighting"))								// Utilisation de l'éclairage
            obj_table[cur_part]->surface.Flag|=SURFACE_LIGHTED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_LIGHTED;
        if (surface_area.get_state("surface.c_texturing"))								// Utilisation du texturage
            obj_table[cur_part]->surface.Flag|=SURFACE_TEXTURED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_TEXTURED;

        if (surface_area.get_state("surface.b_ok")) done=true;		// En cas de click sur "OK", on quitte la fenêtre

        show_mouse(NULL);					// Cache la souris / Hide cursor
        if (key[KEY_ESC])
        {
            reset_keyboard();
            done=true;			// Quitte si on appuie sur echap / Leave on ESC
        }

        if (surface_area.get_object("surface.preview"))
        {
            GUIOBJ *preview = surface_area.get_object("surface.preview");
            if ((GLuint)preview->Data == 0)
            {
                allegro_gl_set_texture_format(GL_RGB8);
                preview->Data = (uint32) gfx->create_texture( (int)(preview->x2 - preview->x1), (int)(preview->y2 - preview->y1), FILTER_LINEAR );
            }

            gfx->renderToTexture( (GLuint)preview->Data, true );

            gfx->SetDefState();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the texture

            Camera Cam;

            MATRIX_4x4 Rot;				// Oriente la caméra
            Rot = Scale(1.0f);
            Cam.setMatrix(Rot);

            Cam.setView();			// Positionne la caméra

            glRotatef(msec_timer*0.01f,0.0f,1.0f,0.0f);
            glRotatef(msec_timer*0.01333f,1.0f,0.0f,0.0f);
            glTranslatef( -obj_table[cur_part]->pos_from_parent.x, -obj_table[cur_part]->pos_from_parent.y, -obj_table[cur_part]->pos_from_parent.z );

            scaleFactor *= expf( 0.05f * ( amz - mouse_z ) );

            glScalef( scaleFactor, scaleFactor, scaleFactor );

            gfx->set_color( 0xFFFFFFFF );

            for(int i = 0; i < nb_obj(); ++i)
                cur_data.flag[i]= i==cur_part ? 0 : FLAG_HIDE;
            glDisable( GL_CULL_FACE );
            obj_table[cur_part]->draw_dl(&cur_data);		// Dessine la partie en cours d'édition de la meshe
            glEnable( GL_CULL_FACE );

            gfx->renderToTexture( 0 );
        }

        // Efface tout / Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gfx->set_2D_mode();		// Passe en mode dessin allegro

        // Update textures
        if (obj_table[cur_part]->surface.NbTex>0)
            for(int i=0;i<obj_table[cur_part]->surface.NbTex;i++)
                surface_area.set_data( format("surface.tex%d", i), obj_table[cur_part]->surface.gltex[i]);

        surface_area.draw();

        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
        show_mouse(screen);
        algl_draw_mouse();

        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

        int index = -1;
        for (int i = 0 ; i < 8 ; i++)
            if (surface_area.get_state( format("surface.tex%d",i)))
            {
                surface_area.set_state( format("surface.tex%d",i), false );
                index = i;
                break;
            }

        if (index!=-1) // L'utilisateur veut choisir une texture / the user is selecting a texture
        {
            String filename = Dialogf( I18N::Translate( "Load a texture" ).c_str(),"*.*");
            BITMAP *bmp_tex = filename.length()>0 ? gfx->load_image(filename) : NULL;
            if (bmp_tex) // Si il s'agit d'ajouter/modifier une texture / If we are adding/modifying a texture
            {
                if (bitmap_color_depth(bmp_tex)==24 || strstr(filename.c_str(),".jpg")!=NULL)
                {
                    BITMAP *tmp = create_bitmap_ex(32,bmp_tex->w,bmp_tex->h);
                    for(int y=0;y<tmp->h;y++)
                        for(int x=0;x<tmp->w;x++)
                            putpixel(tmp,x,y,getpixel(bmp_tex,x,y)|0xFF000000);
                    destroy_bitmap(bmp_tex);
                    bmp_tex=tmp;
                }
                if (index>=obj_table[cur_part]->surface.NbTex)      // Ajoute la texture / Add the texture
                {
                    index=obj_table[cur_part]->surface.NbTex;
                    allegro_gl_use_alpha_channel(true);
                    allegro_gl_set_texture_format(GL_RGBA8);
                    obj_table[cur_part]->surface.gltex[index]=allegro_gl_make_texture(bmp_tex);
                    allegro_gl_use_alpha_channel(false);
                    glBindTexture(GL_TEXTURE_2D,obj_table[cur_part]->surface.gltex[index]);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    obj_table[cur_part]->surface.NbTex++;
                    destroy_bitmap(bmp_tex);
                }
                else
                {
                    gfx->destroy_texture(obj_table[cur_part]->surface.gltex[index]);
                    allegro_gl_use_alpha_channel(true);
                    allegro_gl_set_texture_format(GL_RGBA8);
                    obj_table[cur_part]->surface.gltex[index] = allegro_gl_make_texture(bmp_tex);
                    allegro_gl_use_alpha_channel(false);
                    glBindTexture(GL_TEXTURE_2D,obj_table[cur_part]->surface.gltex[index]);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    destroy_bitmap(bmp_tex);
                }
            }
            else
            {
                if (index<obj_table[cur_part]->surface.NbTex && obj_table[cur_part]->surface.NbTex>0) // Sinon on enlève la texture / otherwise we remove the texture
                {
                    gfx->destroy_texture(obj_table[cur_part]->surface.gltex[index]);

                    for(int i=0;i<8;i++)                            // Reset IDs of displayed textures
                        surface_area.set_data( format("surface.tex%d", i), 0);

                    obj_table[cur_part]->surface.NbTex--;
                    if (index<obj_table[cur_part]->surface.NbTex)		// Décale les textures si nécessaire / Shift textures if needed
                        for(int i=index;i<obj_table[cur_part]->surface.NbTex;i++)
                            obj_table[cur_part]->surface.gltex[i]=obj_table[cur_part]->surface.gltex[i+1];
                }
            }
        }		// Fin de l'algorithme de sélection de texture / End of texture selection code

        // Affiche
        gfx->flip();
    } while(!done);

    // Clear texture values (we mustn't destroy those textures since they're part of the objects)
    for(int i=0;i<8;i++)
        surface_area.set_data( format("surface.tex%d", i), 0);
}					// Fin de l'éditeur de surfaces

/*---------------------------------------------------------------------------------------------------\
  |                                Editeur de texture de surface sur l'objet                           |
  \---------------------------------------------------------------------------------------------------*/

void SurfPaint(int index)
{
    // Vérifie s'il y a un pack de texture de sélectionné et si la surface est texturée
    if (!(obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED))
        return;

    AREA painter_area;
    painter_area.load_tdf("gui/3dmeditor_painting.area");

    bool done=false;

    int amx = mouse_x, amy = mouse_y, amb = mouse_b, amz = mouse_z;

    Camera Cam;
    Cam.znear=0.01f;
    Cam.zfar=1400.0f;

    int IsOnGUI;		// Variable indiquant si le curseur est sur un élément de l'interface utilisateur

    float r1=0.0f,r2=0.0f;

    int Focus=0;

    int RenderingMode=GL_FILL;

    enum { EDIT_NONE, EDIT_SELTRI, EDIT_PAINT };			// Constantes indiquant le mode d'édition

    int EditMode=EDIT_NONE;

    float	tool_tex_size=0.5f;
    GLuint	tool_tex_gl=0;

    enum { TOOL_POINT, TOOL_LINE, TOOL_FILL, TOOL_PEN, TOOL_TEX};	// Liste des outils disponibles

    int Tool=TOOL_POINT;
    unsigned int PColor=makeacol( 0,0,0, 255 );		// Couleur par défaut le noir avec alpha à 0xFF
    int au=0,av=0;					// Pour l'outil de traçage de lignes
    bool startline=true;		// Indique s'il faut démarrer une ligne

    int NbSel=0;				// Pour la selection de triangles
    int *Sel=new int[obj_table[cur_part]->nb_vtx];		// Tableau de sélection

    GLuint tex = obj_table[cur_part]->surface.gltex[0];

    GLuint CancelH[10];			// Pour revenir jusqu'à 10x en arrière
    int NbH=0;		// Profondeur de l'historique
    {
        for(int i=0;i<10;i++)
            CancelH[i]=0;
    }

    Shader shader_paint_u,shader_paint_v;
    shader_paint_u.load("shaders/3dmpaint_u.frag","shaders/3dmpaint.vert");
    shader_paint_v.load("shaders/3dmpaint_v.frag","shaders/3dmpaint.vert");

    SCRIPT_DATA	cur_data;
    cur_data.load(nb_obj());

    GLuint brush_U,brush_V,zbuf;
    GLuint brush_FBO;

    if (g_useProgram && g_useFBO)
    {
        glGenFramebuffersEXT(1,&brush_FBO);

        BITMAP *tmp=create_bitmap_ex(32,SCREEN_W,SCREEN_H);	// On ne peut pas utiliser screen donc on crée un BITMAP temporaire
        allegro_gl_set_texture_format(GL_RGBA16);
        brush_U=allegro_gl_make_texture(tmp);
        brush_V=allegro_gl_make_texture(tmp);
        allegro_gl_set_texture_format(GL_RGBA8);

        glGenRenderbuffersEXT(1,&zbuf);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, zbuf);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT24, SCREEN_W, SCREEN_H);
        destroy_bitmap(tmp);
        glBindTexture(GL_TEXTURE_2D,brush_U);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D,brush_V);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    }

    uint32 timer = msec_timer;

    do
    {
        bool key_is_pressed = false;
        do
        {
            amx = mouse_x;
            amy = mouse_y;
            amz = mouse_z;
            amb = mouse_b;

            key_is_pressed = keypressed();
            IsOnGUI = painter_area.check() != 0;
            rest( 8 );
            if (msec_timer - timer >= 50)  break;
        } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !painter_area.scrolling );
        timer = msec_timer;

        if (painter_area.get_state("tools.b_cancel")) // Clic sur annuler
        {
            glDeleteTextures(1,&tex);
            tex=CancelH[9];
            for(int i=9;i>10-NbH;i--)
                CancelH[i]=CancelH[i-1];
            CancelH[10-NbH]=0;
            NbH--;

            obj_table[cur_part]->surface.gltex[0]=tex;			// Recopie le BITMAP sur la texture
        }

        if (NbH>10) NbH=10;			// Limite la profondeur de l'historique

        if (painter_area.get_state("tools.b_points"))   Tool=TOOL_POINT;
        if (painter_area.get_state("tools.b_lines"))    Tool=TOOL_LINE;
        if (painter_area.get_state("tools.b_fill"))     Tool=TOOL_FILL;
        if (painter_area.get_state("tools.b_pencil"))   Tool=TOOL_PEN;
        if (painter_area.get_state("tools.b_texture"))  Tool=TOOL_TEX;

        switch(Tool)			// Indique quel outil est sélectionné
        {
            case TOOL_TEX:
                tool_tex_size+=(mouse_z-amz)*0.05f;
                if (tool_tex_size<0.01f)	tool_tex_size=0.01f;
                if (key[KEY_SPACE] || tool_tex_gl==0)
                {
                    String filename = Dialogf( I18N::Translate( "Load a texture" ).c_str(),"*.*");
                    BITMAP *bmp_tex = filename.length()>0 ? gfx->load_image(filename) : NULL;
                    if (bmp_tex) // Si il s'agit d'ajouter/modifier une texture
                    {
                        if (bitmap_color_depth(bmp_tex)!=32 || strstr(filename.c_str(),".jpg")!=NULL)
                        {
                            BITMAP *tmp = create_bitmap_ex(32,bmp_tex->w,bmp_tex->h);
                            for(int y=0;y<tmp->h;y++)
                                for(int x=0;x<tmp->w;x++)
                                {
                                    int c = getpixel(bmp_tex,x,y);
                                    int r = getr( c );
                                    int g = getg( c );
                                    int b = getb( c );
                                    if (bitmap_color_depth(bmp_tex)==16)
                                    {
                                        r <<= 3;
                                        g <<= 2;
                                        b <<= 3;
                                    }
                                    else if (bitmap_color_depth(bmp_tex)==8)
                                        r = g = b = r << 2;
                                    putpixel(tmp,x,y,makeacol(r,g,b,0xFF));
                                }
                            destroy_bitmap(bmp_tex);
                            bmp_tex=tmp;
                        }
                        bool need_alpha_fill = true;
                        for(int y=0;y<bmp_tex->h;y++)
                            for(int x=0;x<bmp_tex->w;x++)
                                if (geta( getpixel(bmp_tex,x,y) ))
                                {
                                    need_alpha_fill = false;
                                    break;
                                }
                        if (need_alpha_fill)
                            for(int y=0;y<bmp_tex->h;y++)
                                for(int x=0;x<bmp_tex->w;x++)
                                    putpixel(bmp_tex,x,y,getpixel(bmp_tex,x,y)|0xFF000000);
                        if (tool_tex_gl)
                            glDeleteTextures(1,&tool_tex_gl);
                        allegro_gl_use_alpha_channel(true);
                        allegro_gl_set_texture_format(GL_RGBA8);
                        tool_tex_gl=allegro_gl_make_texture(bmp_tex);
                        allegro_gl_use_alpha_channel(false);
                        glBindTexture(GL_TEXTURE_2D,tool_tex_gl);
                        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                        destroy_bitmap(bmp_tex);
                    }
                }
                break;
        }

        if (painter_area.get_state("paint.b_resolution")) 								// Change the texture size
        {
            String new_res = GetVal( I18N::Translate( "New texture resolution" ) );
            char *new_separator = strstr(new_res.c_str(),"x");
            if (new_separator)
            {
                *new_separator=0;
                int n_w = atoi(new_res.c_str());
                *new_separator='x';
                new_separator++;
                int n_h = atoi(new_separator);

                BITMAP *bmp_tex = read_tex(tex);
                glDeleteTextures(1,&tex);
                BITMAP *tmp = create_bitmap_ex(32,n_w,n_h);
                stretch_blit(bmp_tex,tmp,0,0,bmp_tex->w,bmp_tex->h,0,0,tmp->w,tmp->h);
                destroy_bitmap(bmp_tex);
                allegro_gl_use_alpha_channel(true);
                obj_table[cur_part]->surface.gltex[0] = tex = allegro_gl_make_texture(tmp);
                allegro_gl_use_alpha_channel(false);

                glBindTexture(GL_TEXTURE_2D,tex);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

                destroy_bitmap(tmp);
            }
            else
                Popup( I18N::Translate( "Error" ), I18N::Translate( "The resolution must be widthxheight" ));
        }
        if (painter_area.get_state("paint.b_cubic_mapping"))    CubeTexturing(cur_part);
        if (painter_area.get_state("paint.b_optimize"))         obj_geo_optimize(cur_part);
        if (painter_area.get_state("paint.b_normalize"))        obj_maj_normal(cur_part);
        if (painter_area.get_state("paint.b_disassemble"))      obj_geo_split(cur_part);

        if (painter_area.get_state("paint.b_wireframe"))    RenderingMode=GL_LINE;			// Défini le mode de rendu
        if (painter_area.get_state("paint.b_normal"))       RenderingMode=GL_FILL;

        if (painter_area.get_state("paint.b_selecting"))    EditMode=EDIT_SELTRI;				// Défini le mode d'édition
        if (painter_area.get_state("paint.b_painting"))     EditMode=EDIT_PAINT;

        glClearColor (0, 0, 0, 0);
        glShadeModel (GL_SMOOTH);
        glPolygonMode (GL_FRONT_AND_BACK, RenderingMode);
        glPolygonMode (GL_BACK, GL_POINTS);
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LEQUAL);
        glCullFace (GL_BACK);
        glEnable (GL_CULL_FACE);
        glEnable (GL_LIGHT0);
        glEnable (GL_LIGHTING);
        glEnable (GL_COLOR_MATERIAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

        if (painter_area.get_state("paint.b_cylindric_mapping"))
            CylinderTexturing(cur_part);

        if (painter_area.get_state("paint.b_painting"))
            EditMode = EDIT_PAINT;

        if (painter_area.get_state("paint.b_selecting"))
            EditMode = EDIT_SELTRI;

        if (IsOnGUI && mouse_b!=0)
            Focus=0;

        // Efface tout
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la mémoire tampon

        show_mouse(NULL);					// Cache la souris
        if (key[KEY_ESC] || painter_area.get_state("paint.b_ok"))
        {
            done = true;
            while( key[KEY_ESC] )
            {
                rest(8);
                poll_keyboard();
            }
        }

        if (!IsOnGUI && mouse_b==2)			// Rotation de la caméra à la souris par clic droit
        {
            r2-=(mouse_x-amx)*0.25f;
            r1-=(mouse_y-amy)*0.25f;
        }

        MATRIX_4x4 Rot;				// Oriente la caméra
        Rot = RotateX(r1*PI/180.0f) * RotateY(r2*PI/180.0f);

        Cam.setMatrix(Rot);

        if (!IsOnGUI && Tool!=TOOL_TEX)
            Cam.rpos = Cam.rpos - 0.5f * (mouse_z-amz) * Cam.dir; // Déplace la caméra si besoin

        Cam.setView();			// Positionne la caméra

        for(int i = 0; i < nb_obj(); ++i)
            cur_data.flag[i]= i==cur_part ? 0 : FLAG_HIDE;
        obj_table[cur_part]->draw_dl(&cur_data);		// Dessine la partie en cours d'édition de la meshe
        glDisable(GL_TEXTURE_2D);

        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);		// Restore les paramètres de remplissage
        glPolygonMode (GL_BACK, GL_POINTS);

        if (NbSel>0) // Affiche la sélection courante
        {
            glColor4f(0.0f,0.0f,1.0f,0.25f);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glTranslatef(obj_table[cur_part]->pos_from_parent.x,obj_table[cur_part]->pos_from_parent.y,obj_table[cur_part]->pos_from_parent.z);
            glBegin(GL_TRIANGLES);
            for(int i=0;i<NbSel;i++)
            {
                index=Sel[i];
                glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].z);
                glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].z);
                glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].z);
            }
            glEnd();
            glDisable(GL_BLEND);
        }

        if (!IsOnGUI)					// Code de l'édition
            switch(EditMode)
            {
                case EDIT_SELTRI:			// Code pour la sélection
                    startline=true;		// Pour l'outil de traçage de lignes
                    {
                        Vector3D A,B,O;
                        Vector3D Dir;
                        Cam.setView();
                        O = Cam.pos-obj_table[cur_part]->pos_from_parent;			// Origine du rayon=le point de vue de la caméra
                        Dir=Cam.dir+(mouse_x-(SCREEN_W>>1))/(SCREEN_W*0.5f)*Cam.side-0.75f*(mouse_y-(SCREEN_H>>1))/(SCREEN_H*0.5f)*Cam.up;
                        int index=intersect(O,Dir,obj_table[cur_part],&A,&B); 		// Obtient l'indice du triangle visé

                        if (index>=0) // Si l'indice est valable
                        {
                            glColor4f(1.0f,1.0f,1.0f,0.25f);
                            glDisable(GL_TEXTURE_2D);
                            glDisable(GL_LIGHTING);
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		// Affiche le triangle visé
                            glBegin(GL_TRIANGLES);
                            glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3]].z);
                            glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+1]].z);
                            glVertex3f(obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].x,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].y,obj_table[cur_part]->points[obj_table[cur_part]->t_index[index*3+2]].z);
                            glEnd();
                            glDisable(GL_BLEND);

                            if (mouse_b == 1) // Modifie la sélection
                            {
                                if (key[KEY_LSHIFT] || key[KEY_RSHIFT]) // Ajoute le triangle à la sélection
                                {
                                    bool already=false;		// Vérifie si le triangle n'est pas déjà présent
                                    if (NbSel>0)
                                        for(int i=0;i<NbSel;i++)
                                            if (Sel[i]==index)
                                            {
                                                already=true;
                                                break;
                                            }
                                    if (!already)		// L'ajoute si il n'y est pas déjà
                                        Sel[NbSel++]=index;
                                }
                                else
                                {
                                    if (key[KEY_CAPSLOCK]) 	// Retire le triangle de la sélection
                                    {
                                        int pos=-1;				// Cherche la position du triangle
                                        if (NbSel>0)
                                            for(int i=0;i<NbSel;i++)
                                                if (Sel[i]==index)
                                                {
                                                    pos=i;
                                                    break;
                                                }
                                        if (pos!=-1) 	// Si le triangle est présent
                                        {
                                            if (pos+1<NbSel)		// Si ce n'est pas le dernier
                                                for(int i=pos;i<NbSel-1;i++)	// Décale tout
                                                    Sel[i]=Sel[i+1];
                                            NbSel--;		// Décrémente le nombre de triangles sélectionnés
                                        }
                                    }
                                    else
                                    {											// Ne sélectionne que ce triangle
                                        NbSel=1;
                                        Sel[0]=index;
                                    }
                                }
                            }			// Fin de if (mouse_b==1) {
                        }		// Fin de if (index>=0) {
                    }
                    break;
                case EDIT_PAINT:			// Code pour le dessin
                    if (mouse_b==1 && NbSel>0)	// Si il y a une sélection
                    {
                        Vector3D A,B,O;
                        Vector3D Dir;
                        Cam.setView();
                        O=Cam.pos-obj_table[cur_part]->pos_from_parent;			// Origine du rayon=le point de vue de la caméra
                        Dir=Cam.dir+(mouse_x-(SCREEN_W>>1))/(SCREEN_W*0.5f)*Cam.side-0.75f*(mouse_y-(SCREEN_H>>1))/(SCREEN_H*0.5f)*Cam.up;
                        int index=intersect(O,Dir,obj_table[cur_part],&A,&B); 		// Obtient l'indice du triangle visé
                        bool Selected=false;
                        if (index>=0)					// Vérifie si le triangle est sélectionné
                            for(int i=0;i<NbSel;i++)
                                if (Sel[i]==index)
                                {
                                    Selected=true;
                                    break;
                                }
                        if (Selected) // Si le triangle est sélectionné
                        {
                            if (amb!=1)
                            {
                                if (CancelH[0])
                                    glDeleteTextures(1,CancelH);
                                for(int i=0;i<9;i++)		// Fait descendre l'historique
                                    CancelH[i]=CancelH[i+1];
                                CancelH[9]=copy_tex(tex);
                                NbH++;		// Sauvegarde la texture dans l'historique
                            }

                            float l = B.x + B.y + B.z;
                            B.x /= l;
                            B.y /= l;
                            B.z /= l;

                            float u,v;
                            int p1,p2,p3;
                            p1=obj_table[cur_part]->t_index[index*3];			// Indices des sommets du triangle
                            p2=obj_table[cur_part]->t_index[index*3+1];
                            p3=obj_table[cur_part]->t_index[index*3+2];

                            // Calcule les coordonnées correspondantes sur la texture
                            u=B.x*obj_table[cur_part]->tcoord[(p1<<1)]+B.y*obj_table[cur_part]->tcoord[(p2<<1)]+B.z*obj_table[cur_part]->tcoord[(p3<<1)];
                            v=B.x*obj_table[cur_part]->tcoord[(p1<<1)+1]+B.y*obj_table[cur_part]->tcoord[(p2<<1)+1]+B.z*obj_table[cur_part]->tcoord[(p3<<1)+1];

                            if (u>=0.0f && v>=0.0f && u<1.0f && v<1.0f)
                            {
                                GLint tex_w,tex_h;
                                BITMAP *n_tex = read_tex(tex);
                                tex_w=n_tex->w;
                                tex_h=n_tex->h;
                                u=u*tex_w+0.5f;
                                v=v*tex_h+0.5f;
                                switch(Tool)
                                {
                                    case TOOL_POINT:			// Dessine un point
                                        putpixel(n_tex,(int)u,(int)v,PColor);		// Pixel de couleur PColor
                                        break;
                                    case TOOL_LINE:				// Trace une ligne
                                        if (!startline)
                                            line(n_tex,au,av,(int)u,(int)v,PColor);		// Ligne de couleur PColor
                                        else
                                            putpixel(n_tex,(int)u,(int)v,PColor);		// Pixel de couleur PColor
                                        startline=false;
                                        au=(int)u;
                                        av=(int)v;
                                        break;
                                    case TOOL_FILL:				// Rempli la sélection
                                        {
                                            for(int i=0;i<NbSel;i++) {
                                                int u1,v1,u2,v2,u3,v3;
                                                u1=(int)(obj_table[cur_part]->tcoord[obj_table[cur_part]->t_index[Sel[i]*3]<<1]*n_tex->w+0.5f);
                                                v1=(int)(obj_table[cur_part]->tcoord[(obj_table[cur_part]->t_index[Sel[i]*3]<<1)+1]*n_tex->h+0.5f);
                                                u2=(int)(obj_table[cur_part]->tcoord[obj_table[cur_part]->t_index[Sel[i]*3+1]<<1]*n_tex->w+0.5f);
                                                v2=(int)(obj_table[cur_part]->tcoord[(obj_table[cur_part]->t_index[Sel[i]*3+1]<<1)+1]*n_tex->h+0.5f);
                                                u3=(int)(obj_table[cur_part]->tcoord[obj_table[cur_part]->t_index[Sel[i]*3+2]<<1]*n_tex->w+0.5f);
                                                v3=(int)(obj_table[cur_part]->tcoord[(obj_table[cur_part]->t_index[Sel[i]*3+2]<<1)+1]*n_tex->h+0.5f);
                                                triangle(n_tex,u1,v1,u2,v2,u3,v3,PColor);
                                            }
                                        }
                                        break;
                                    case TOOL_PEN:				// Dessine un cercle plein de rayon 2
                                        circlefill(n_tex,(int)u,(int)v,2,PColor);
                                        break;
                                    case TOOL_TEX:
                                        {
                                            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,brush_FBO);

                                            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,brush_U,0);
                                            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,zbuf);
                                            glViewport(0, 0, SCREEN_W, SCREEN_H);
                                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// First pass for U coordinate
                                            Cam.setView();			// Positionne la caméra
                                            for(int i=0;i<nb_obj();i++)
                                                cur_data.flag[i]= i==cur_part ? 0 : FLAG_HIDE;
                                            shader_paint_u.on();
                                            obj_table[cur_part]->draw(0.0f,&cur_data);		// Dessine la partie en cours d'édition de la meshe
                                            shader_paint_u.off();

                                            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,brush_V,0);
                                            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,zbuf);
                                            glViewport(0, 0, SCREEN_W, SCREEN_H);
                                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Second pass for V coordinate
                                            Cam.setView();			// Positionne la caméra
                                            shader_paint_v.on();
                                            obj_table[cur_part]->draw(0.0f,&cur_data);		// Dessine la partie en cours d'édition de la meshe
                                            shader_paint_v.off();

                                            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

                                            BITMAP *tex_U = read_tex_luminance(brush_U);
                                            BITMAP *tex_V = read_tex_luminance(brush_V);
                                            BITMAP *brush = read_tex(tool_tex_gl);
                                            for(int y=0;y<brush->h;y++)
                                            {
                                                int Y = SCREEN_H-1-(int)(mouse_y-32.0f*tool_tex_size+64.0f*tool_tex_size*y/brush->h);
                                                if (Y>=0 && Y<SCREEN_H)
                                                    for(int x=0;x<brush->w;x++)
                                                    {
                                                        int X = (int)(mouse_x-32.0f*tool_tex_size+64.0f*tool_tex_size*x/brush->w);
                                                        if (X>=0 && X<SCREEN_W)
                                                        {
                                                            int u = (int)(((unsigned short*)(tex_U->line[Y]))[(X<<2)+1]/65536.0f*n_tex->w);
                                                            int v = (int)(((unsigned short*)(tex_V->line[Y]))[(X<<2)+1]/65536.0f*n_tex->h);
                                                            int c1 = getpixel(brush,x,y);
                                                            int c2 = getpixel(n_tex,u,v);
                                                            int cr1 = getr(c1);
                                                            int cg1 = getg(c1);
                                                            int cb1 = getb(c1);
                                                            int ca1 = geta(c1);
                                                            int cr2 = getr(c2);
                                                            int cg2 = getg(c2);
                                                            int cb2 = getb(c2);
                                                            int ca2 = geta(c2);
                                                            int col = makeacol32((cr1*ca1+(255-ca1)*cr2)>>8,
                                                                                 (cg1*ca1+(255-ca1)*cg2)>>8,
                                                                                 (cb1*ca1+(255-ca1)*cb2)>>8,
                                                                                 (ca1*ca1+(255-ca1)*ca2)>>8);
                                                            putpixel(n_tex,u,v,col);
                                                        }
                                                    }
                                            }
                                            destroy_bitmap(tex_U);
                                            destroy_bitmap(tex_V);
                                            destroy_bitmap(brush);
                                        }
                                        break;
                                }

                                glDeleteTextures(1,&tex);
                                allegro_gl_use_alpha_channel(true);
                                tex=allegro_gl_make_texture(n_tex);
                                allegro_gl_use_alpha_channel(false);
                                glBindTexture(GL_TEXTURE_2D,tex);
                                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                                destroy_bitmap(n_tex);

                                obj_table[cur_part]->surface.gltex[0]=tex;
                            }
                            else
                                startline=true;
                        }
                        else
                            startline=true;
                    }
                    else
                        startline=true;
                    break;
            }

        gfx->set_2D_mode();		// Passe en mode dessin allegro

        painter_area.draw();

        if (EditMode==EDIT_PAINT)
        {
            int r = painter_area.get_caption("tools.red").toInt32();
            int g = painter_area.get_caption("tools.green").toInt32();
            int b = painter_area.get_caption("tools.blue").toInt32();
            int a = painter_area.get_caption("tools.alpha").toInt32();
            PColor = makeacol32(r,g,b,a);
            painter_area.set_data("tools.color", makeacol(r,g,b,a));

            switch(Tool)			// Indique quel outil est sélectionné
            {
                case TOOL_POINT:
                    glBlendFunc(GL_ONE,GL_ONE);
                    glEnable(GL_BLEND);
                    gfx->print(gfx->normal_font,0.0f,SCREEN_H-8.0f,0.0f,0xFFFFFFFF,I18N::Translate( "Dessin point par point" ));
                    glDisable(GL_BLEND);
                    break;
                case TOOL_LINE:
                    glBlendFunc(GL_ONE,GL_ONE);
                    glEnable(GL_BLEND);
                    gfx->print(gfx->normal_font,0.0f,SCREEN_H-8.0f,0.0f,0xFFFFFFFF,I18N::Translate( "Tracage de lignes" ));
                    glDisable(GL_BLEND);
                    break;
                case TOOL_FILL:
                    glBlendFunc(GL_ONE,GL_ONE);
                    glEnable(GL_BLEND);
                    gfx->print(gfx->normal_font,0.0f,SCREEN_H-8.0f,0.0f,0xFFFFFFFF,I18N::Translate( "Remplissage" ));
                    glDisable(GL_BLEND);
                    break;
                case TOOL_PEN:
                    glBlendFunc(GL_ONE,GL_ONE);
                    glEnable(GL_BLEND);
                    gfx->print(gfx->normal_font,0.0f,SCREEN_H-8.0f,0.0f,0xFFFFFFFF, I18N::Translate( "Crayon" ));
                    glDisable(GL_BLEND);
                    break;
                case TOOL_TEX:
                    glBlendFunc(GL_ONE,GL_ONE);
                    glEnable(GL_BLEND);
                    gfx->print(gfx->normal_font,0.0f,SCREEN_H-8.0f,0.0f,0xFFFFFFFF, I18N::Translate("Motif"));
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    if (!IsOnGUI)
                        gfx->drawtexture(tool_tex_gl,mouse_x-tool_tex_size*32.0f,mouse_y-tool_tex_size*32.0f,mouse_x+tool_tex_size*32.0f,mouse_y+tool_tex_size*32.0f);
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                    break;
            }
        }		// Fin de if (EditMode==EDIT_PAINT) {

        if (painter_area.get_state("mapper") && painter_area.get_object("mapper.texture"))       // If the window is visible then update the texture
        {
            GUIOBJ *preview = painter_area.get_object("mapper.texture");
            int w = (int)(preview->x2 - preview->x1);
            int h = (int)(preview->y2 - preview->y1);
            if ((GLuint)preview->Data == 0)
            {
                allegro_gl_set_texture_format(GL_RGB8);
                preview->Data = (uint32) gfx->create_texture( w, h, FILTER_LINEAR );
            }

            gfx->renderToTexture( (GLuint)preview->Data, true );

            gfx->SetDefState();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the texture

            gfx->unset_2D_mode();
            gfx->set_2D_mode();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,tex);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);	glVertex2f(0.0f,0.0f);
            glTexCoord2f(1.0f,0.0f);	glVertex2f(w-1.0f,0.0f);
            glTexCoord2f(1.0f,1.0f);	glVertex2f(w-1.0f,h-1.0f);
            glTexCoord2f(0.0f,1.0f);	glVertex2f(0.0f,h-1.0f);
            glEnd();
            glDisable(GL_TEXTURE_2D);

            gfx->set_color(0xFF,0xFF,0xFF,0xFF);

            glPushMatrix();
            glScalef(w-1.0f,h-1.0f,0.0f);

            glEnableClientState(GL_VERTEX_ARRAY);					// Les coordonnées des points
            glVertexPointer( 2, GL_FLOAT, 0, obj_table[cur_part]->tcoord);

            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

            glDrawElements(GL_TRIANGLES, obj_table[cur_part]->nb_t_index,GL_UNSIGNED_SHORT,obj_table[cur_part]->t_index);		// dessine les triangles sur la texture


            gfx->set_color(0,0,0xFF);
            glPointSize(3.0f);

            glDrawElements(GL_POINTS, obj_table[cur_part]->nb_t_index,GL_UNSIGNED_SHORT,obj_table[cur_part]->t_index);		// dessine les triangles sur la texture

            glPointSize(1.0f);

            glPopMatrix();
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
            glPolygonMode (GL_BACK, GL_POINTS);

            gfx->renderToTexture( 0 );      // Back to normal
            gfx->unset_2D_mode();
            gfx->set_2D_mode();
        }

        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
        show_mouse(screen);
        algl_draw_mouse();

        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

        // Affiche
        gfx->flip();
    } while(!done);

    if (g_useProgram && g_useFBO)
    {
        glDeleteFramebuffersEXT(1,&brush_FBO);
        glDeleteRenderbuffersEXT(1,&zbuf);
        glDeleteTextures(1,&brush_U);
        glDeleteTextures(1,&brush_V);
    }

    shader_paint_u.destroy();
    shader_paint_v.destroy();

    if (tool_tex_gl!=0)
        glDeleteTextures(1,&tool_tex_gl);

    cur_data.destroy();

    for(int i=0;i<10;i++) // Libère la mémoire allouée pour les bitmaps de l'historique
    {
        if (CancelH[i])
            glDeleteTextures(1,&(CancelH[i]));
    }

    delete[] Sel;							// Libère la mémoire du tableau de sélection
}

/*---------------------------------------------------------------------------------------------------\
    |                                      Editeur de plaquage de texture                                |
    \---------------------------------------------------------------------------------------------------*/

void TexturePosEdit(int index)
{
    CylinderTexturing(cur_part);
}

/*---------------------------------------------------------------------------------------------------\
    |                                     Plaquage cubique de la texture                                 |
    \---------------------------------------------------------------------------------------------------*/

void CubeTexturing(int part)
{
    if (part<0 || part>=nb_obj())	return;		// Quitte si l'indice n'est pas valable

    float COS45=0.5f*sqrtf(2.0f);		// Valeur à partir de laquelle on utilise les bords droit et gauche de la texture
    Vector3D I,J,K;
    I=J=K=I-I;
    I.x=1.0f;				// Vecteur de référence pour les calculs de repérage
    J.y=1.0f;
    K.z=1.0f;

    float CAymin=1000000.0f,CAymax=-1000000.0f;
    float CAzmin=1000000.0f,CAzmax=-1000000.0f;
    float CBymin=1000000.0f,CBymax=-1000000.0f;
    float CBzmin=1000000.0f,CBzmax=-1000000.0f;
    float CCxmin=1000000.0f,CCxmax=-1000000.0f;
    float CCzmin=1000000.0f,CCzmax=-1000000.0f;
    float CDxmin=1000000.0f,CDxmax=-1000000.0f;
    float CDzmin=1000000.0f,CDzmax=-1000000.0f;
    float CEymin=1000000.0f,CEymax=-1000000.0f;
    float CExmin=1000000.0f,CExmax=-1000000.0f;
    float CFymin=1000000.0f,CFymax=-1000000.0f;
    float CFxmin=1000000.0f,CFxmax=-1000000.0f;

    int i;
    for (i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse tous les points pour connaître les bornes de l'objet
    {
        float COS=obj_table[part]->N[i]%I;
        if (fabsf(COS)<COS45)			// Milieu
        {
            COS=obj_table[part]->N[i]%J;
            if (fabsf(COS)<COS45)
            {
                COS=obj_table[part]->N[i]%K;
                if (COS>0.0f)			// Coté E(bas droit)
                {
                    if (obj_table[part]->points[i].x>CExmax)	CExmax=obj_table[part]->points[i].x;
                    if (obj_table[part]->points[i].x<CExmin)	CExmin=obj_table[part]->points[i].x;
                    if (obj_table[part]->points[i].y>CEymax)	CEymax=obj_table[part]->points[i].y;
                    if (obj_table[part]->points[i].y<CEymin)	CEymin=obj_table[part]->points[i].y;
                }
                else					// Coté F(bas gauche)
                {
                    if (obj_table[part]->points[i].x>CFxmax)	CFxmax=obj_table[part]->points[i].x;
                    if (obj_table[part]->points[i].x<CFxmin)	CFxmin=obj_table[part]->points[i].x;
                    if (obj_table[part]->points[i].y>CFymax)	CFymax=obj_table[part]->points[i].y;
                    if (obj_table[part]->points[i].y<CFymin)	CFymin=obj_table[part]->points[i].y;
                }
            }
            else if (COS>0.0f)			// Coté C(milieu droit)
            {
                if (obj_table[part]->points[i].x>CCxmax)	CCxmax=obj_table[part]->points[i].x;
                if (obj_table[part]->points[i].x<CCxmin)	CCxmin=obj_table[part]->points[i].x;
                if (obj_table[part]->points[i].z>CCzmax)	CCzmax=obj_table[part]->points[i].z;
                if (obj_table[part]->points[i].z<CCzmin)	CCzmin=obj_table[part]->points[i].z;
            }
            else						// Coté D(milieu gauche)
            {
                if (obj_table[part]->points[i].x>CDxmax)	CDxmax=obj_table[part]->points[i].x;
                if (obj_table[part]->points[i].x<CDxmin)	CDxmin=obj_table[part]->points[i].x;
                if (obj_table[part]->points[i].z>CDzmax)	CDzmax=obj_table[part]->points[i].z;
                if (obj_table[part]->points[i].z<CDzmin)	CDzmin=obj_table[part]->points[i].z;
            }
        }
        else if (COS>0.0f)				// Coté A(haut droit)
        {
            if (obj_table[part]->points[i].y>CAymax)	CAymax=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].y<CAymin)	CAymin=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].z>CAzmax)	CAzmax=obj_table[part]->points[i].z;
            if (obj_table[part]->points[i].z<CAzmin)	CAzmin=obj_table[part]->points[i].z;
        }
        else							// Coté B(haut gauche)
        {
            if (obj_table[part]->points[i].y>CBymax)	CBymax=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].y<CBymin)	CBymin=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].z>CBzmax)	CBzmax=obj_table[part]->points[i].z;
            if (obj_table[part]->points[i].z<CBzmin)	CBzmin=obj_table[part]->points[i].z;
        }
    }

    if (CAymin==CAymax) CAymin--;
    if (CAzmin==CAzmax) CAzmin--;
    if (CBymin==CBymax) CBymin--;
    if (CBzmin==CBzmax) CBzmin--;

    if (CCxmin==CCxmax) CCxmin--;
    if (CCzmin==CCzmax) CCzmin--;
    if (CDxmin==CDxmax) CDxmin--;
    if (CDzmin==CDzmax) CDzmin--;

    if (CExmin==CExmax) CExmin--;
    if (CEymin==CEymax) CEymin--;
    if (CFxmin==CFxmax) CFxmin--;
    if (CFymin==CFymax) CFymin--;

    for(i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        float COS=obj_table[part]->N[i]%I;
        if (fabsf(COS)<COS45)			// Milieu
        {
            COS=obj_table[part]->N[i]%J;
            if (fabsf(COS)<COS45)
            {
                COS=obj_table[part]->N[i]%K;
                if (COS>0.0f)			// Coté E(bas droit)
                {
                    obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].x-CExmin)/(CExmax-CExmin);
                    obj_table[part]->tcoord[(i<<1)+1]=0.667f+0.333f*(obj_table[part]->points[i].y-CEymin)/(CEymax-CEymin);
                }
                else					// Coté F(bas gauche)
                {
                    obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].x-CFxmin)/(CFxmax-CFxmin);
                    obj_table[part]->tcoord[(i<<1)+1]=0.667f+0.333f*(obj_table[part]->points[i].y-CFymin)/(CFymax-CFymin);
                }
            }
            else if (COS>0.0f)			// Coté C(milieu droit)
            {
                obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].x-CCxmin)/(CCxmax-CCxmin);
                obj_table[part]->tcoord[(i<<1)+1]=0.333f+0.333f*(obj_table[part]->points[i].z-CCzmin)/(CCzmax-CCzmin);
            }
            else						// Coté D(milieu gauche)
            {
                obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].x-CDxmin)/(CDxmax-CDxmin);
                obj_table[part]->tcoord[(i<<1)+1]=0.333f+0.333f*(obj_table[part]->points[i].z-CDzmin)/(CDzmax-CDzmin);
            }
        }
        else if (COS>0.0f)				// Coté A(haut droit)
        {
            obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].y-CAymin)/(CAymax-CAymin);
            obj_table[part]->tcoord[(i<<1)+1]=0.333f*(obj_table[part]->points[i].z-CAzmin)/(CAzmax-CAzmin);
        }
        else							// Coté B(haut gauche)
        {
            obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].y-CBymin)/(CBymax-CBymin);
            obj_table[part]->tcoord[(i<<1)+1]=0.333f*(obj_table[part]->points[i].z-CBzmin)/(CBzmax-CBzmin);
        }
    }
    float txmin=1.0f;
    float txmax=0.0f;
    float tymin=1.0f;
    float tymax=0.0f;
    for (i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        if (obj_table[part]->tcoord[i<<1]<txmin)	txmin=obj_table[part]->tcoord[i<<1];
        if (obj_table[part]->tcoord[i<<1]>txmax)	txmax=obj_table[part]->tcoord[i<<1];
        if (obj_table[part]->tcoord[(i<<1)+1]<tymin)	tymin=obj_table[part]->tcoord[(i<<1)+1];
        if (obj_table[part]->tcoord[(i<<1)+1]>tymax)	tymax=obj_table[part]->tcoord[(i<<1)+1];
    }
    if (txmin==txmax)	txmin--;
    if (tymin==tymax)	tymin--;
    for(i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        obj_table[part]->tcoord[i<<1]=(obj_table[part]->tcoord[i<<1]-txmin)/(txmax-txmin);
        obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->tcoord[(i<<1)+1]-tymin)/(tymax-tymin);
    }
}

/*---------------------------------------------------------------------------------------------------\
    |                                   Plaquage cylindrique de la texture                               |
    \---------------------------------------------------------------------------------------------------*/

void CylinderTexturing(int part)
{
    if (part<0 || part>=nb_obj())	return;		// Quitte si l'indice n'est pas valable

    float COS45=0.5f*sqrtf(2.0f);		// Valeur à partir de laquelle on utilise les bords droit et gauche de la texture
    Vector3D I,J,K;
    I=J=K=I-I;
    I.x=1.0f;				// Vecteur de référence pour les calculs de repérage
    J.y=1.0f;
    K.z=1.0f;

    float xmin=1000000.0f,xmax=-1000000.0f;
    float CAymin=1000000.0f,CAymax=-1000000.0f;
    float CBymin=1000000.0f,CBymax=-1000000.0f;
    float CAzmin=1000000.0f,CAzmax=-1000000.0f;
    float CBzmin=1000000.0f,CBzmax=-1000000.0f;
    float ymin=1000000.0f,ymax=-1000000.0f;

    int i;
    for (i = 0; i < obj_table[part]->nb_vtx; ++i) // Analyse tous les points pour connaître les bornes de l'objet
    {
        float COS=obj_table[part]->N[i]%I;
        if (fabsf(COS) < COS45) // Milieu
        {
            if (obj_table[part]->points[i].x>xmax)	xmax=obj_table[part]->points[i].x;
            if (obj_table[part]->points[i].x<xmin)	xmin=obj_table[part]->points[i].x;
            Vector3D V;
            V=obj_table[part]->points[i];
            V=V-(V%I)*I;
            float angle=VAngle(J,V);
            if (K%V<0.0f)
                angle=2.0f*PI-angle;
            obj_table[part]->tcoord[(i<<1)+1]=angle*0.5f/PI;
            if (obj_table[part]->tcoord[(i<<1)+1]>ymax)	ymax=obj_table[part]->tcoord[(i<<1)+1];
            if (obj_table[part]->tcoord[(i<<1)+1]<ymin)	ymin=obj_table[part]->tcoord[(i<<1)+1];
        }
        else if (COS>0.0f)				// Coté A(droit)
        {
            if (obj_table[part]->points[i].y>CAymax)	CAymax=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].y<CAymin)	CAymin=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].z>CAzmax)	CAzmax=obj_table[part]->points[i].z;
            if (obj_table[part]->points[i].z<CAzmin)	CAzmin=obj_table[part]->points[i].z;
        }
        else							// Coté B(gauche)
        {
            if (obj_table[part]->points[i].y>CBymax)	CBymax=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].y<CBymin)	CBymin=obj_table[part]->points[i].y;
            if (obj_table[part]->points[i].z>CBzmax)	CBzmax=obj_table[part]->points[i].z;
            if (obj_table[part]->points[i].z<CBzmin)	CBzmin=obj_table[part]->points[i].z;
        }
    }

    if (xmin==xmax) xmin--;
    if (CAymin==CAymax) CAymin--;
    if (CAzmin==CAzmax) CAzmin--;
    if (CBymin==CBymax) CBymin--;
    if (CBzmin==CBzmax) CBzmin--;
    if (ymin==ymax) ymin--;

    for (i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        float COS=obj_table[part]->N[i]%I;
        if (fabsf(COS)<COS45)			// Milieu
        {
            Vector3D V = obj_table[part]->points[i];
            V=V-(V%I)*I;
            float angle=VAngle(J,V);
            if (K%V<0.0f)
                angle=2.0f*PI-angle;
            obj_table[part]->tcoord[i<<1]=0.25f+0.5f*(obj_table[part]->points[i].x-xmin)/(xmax-xmin);
            obj_table[part]->tcoord[(i<<1)+1]=(angle*0.5f/PI-ymin)/(ymax-ymin);
        }
        else if (COS>0.0f)				// Coté A(droit)
        {
            obj_table[part]->tcoord[i<<1]=0.75f+0.25f*(obj_table[part]->points[i].y-CAymin)/(CAymax-CAymin);
            obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->points[i].z-CAzmin)/(CAzmax-CAzmin);
        }
        else							// Coté B(gauche)
        {
            obj_table[part]->tcoord[i<<1]=0.25f*(obj_table[part]->points[i].y-CBymin)/(CBymax-CBymin);
            obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->points[i].z-CBzmin)/(CBzmax-CBzmin);
        }
    }

    float txmin=1.0f;
    float txmax=0.0f;
    float tymin=1.0f;
    float tymax=0.0f;
    for(i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        if (obj_table[part]->tcoord[i<<1]<txmin)	txmin=obj_table[part]->tcoord[i<<1];
        if (obj_table[part]->tcoord[i<<1]>txmax)	txmax=obj_table[part]->tcoord[i<<1];
        if (obj_table[part]->tcoord[(i<<1)+1]<tymin)	tymin=obj_table[part]->tcoord[(i<<1)+1];
        if (obj_table[part]->tcoord[(i<<1)+1]>tymax)	tymax=obj_table[part]->tcoord[(i<<1)+1];
    }
    if (txmin==txmax)	txmin--;
    if (tymin==tymax)	tymin--;
    for(i=0;i<obj_table[part]->nb_vtx;i++)		// Analyse toutes les normales et les points
    {
        obj_table[part]->tcoord[i<<1]=(obj_table[part]->tcoord[i<<1]-txmin)/(txmax-txmin);
        obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->tcoord[(i<<1)+1]-tymin)/(tymax-tymin);
    }
}

    /*------------------------------------------------------------------------------------\
      |                                 void glslEditor()                                   |
      |         This function is the GLSL fragment and vertex program editor. It displays   |
      | a fullscreen window with large text input widgets.                                  |
      |                                                                                     |
      \------------------------------------------------------------------------------------*/

void glslEditor()                  // Fragment and vertex programs editor
{
    if (nb_obj()<=0) return;			// S'il n'y a rien à éditer, on quitte tout de suite la fonction, sinon ça plante
    if (nb_obj()==1 && TheModel->obj.name.empty())	return;

    AREA glsl_area;
    glsl_area.load_tdf("gui/3dmeditor_glsl.area");

    if (!obj_table[cur_part]->surface.frag_shader_src.empty())
        glsl_area.set_caption("glsl.fragment program", obj_table[cur_part]->surface.frag_shader_src);
    else
        glsl_area.set_caption("glsl.fragment program", "");

    if (!obj_table[cur_part]->surface.vert_shader_src.empty())
        glsl_area.set_caption("glsl.vertex program", obj_table[cur_part]->surface.vert_shader_src);
    else
        glsl_area.set_caption("glsl.vertex program", "");

    bool done=false;

    int amx,amy,amb,amz;
    uint32 timer = msec_timer;

    do
    {
        bool key_is_pressed = false;
        do
        {
            amx = mouse_x;
            amy = mouse_y;
            amz = mouse_z;
            amb = mouse_b;

            key_is_pressed = keypressed();
            glsl_area.check();
            rest( 8 );
            if (msec_timer - timer >= 50)  break;
        } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !glsl_area.scrolling );
        timer = msec_timer;

        if (glsl_area.get_state("glsl.b_ok"))		// OK => save the buffers and leave the GLSL editor
        {
            obj_table[cur_part]->surface.frag_shader_src = glsl_area.get_caption("glsl.fragment program");
            obj_table[cur_part]->surface.vert_shader_src = glsl_area.get_caption("glsl.vertex program");
            obj_table[cur_part]->surface.s_shader.destroy();
            obj_table[cur_part]->surface.s_shader.load_memory(  obj_table[cur_part]->surface.frag_shader_src.c_str(),       // Builds the shader
                                                                obj_table[cur_part]->surface.frag_shader_src.size(),
                                                                obj_table[cur_part]->surface.vert_shader_src.c_str(),
                                                                obj_table[cur_part]->surface.vert_shader_src.size());
            done=true;
        }

        show_mouse(NULL);					// Cache la souris / Hide cursor
        if (key[KEY_ESC] || glsl_area.get_state("glsl.b_cancel"))
        {
            reset_keyboard();
            done=true;			// Quitte si on appuie sur echap / Leave on ESC
        }

        // Efface tout / Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gfx->set_2D_mode();		// Passe en mode dessin allegro

        glsl_area.draw();

        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
        show_mouse(screen);
        algl_draw_mouse();

        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

        // Affiche
        gfx->flip();
    } while(!done);
}					// End of GLSL editor

