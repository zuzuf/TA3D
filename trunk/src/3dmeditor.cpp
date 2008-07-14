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
#include "logs/cLogger.h"

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
#include "gfx/gui/wnd.h"
#include "misc/application.h"


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
        OBJECT **obj_table;
        int h_table[1000];
        OBJECT_SURFACE obj_surf;
    }
}

/*
 ** Function: LoadConfigFile
 **    Notes: This function will eventually load our config file if it exists.
 **             config files will be stored as 'tdf' format and thus loaded as text,
 **             using the cTAFileParser class.
 **           If something goes wrong you can safely throw a string for an error.
 **             The call to this function is tried, but it only catches exceptions
 **             and strings, ie throw( "LoadConfigFile: some error occured" );
 */
void LoadConfigFile( void )
{
    cTAFileParser *cfgFile;

    try { // we need to try catch this cause the config file may not exists
        // and if it don't exists it will throw an error on reading it, which
        // will be caught in our main function and the application will exit.
        cfgFile = new TA3D::UTILS::cTAFileParser(TA3D::Paths::ConfigFile);
    }
    catch( ... )
    {
        printf("Opening config file %s failed\n", (TA3D::Paths::ConfigFile).c_str() );
        return;
    }

    TA3D::VARS::lp_CONFIG->fps_limit = cfgFile->pullAsFloat( "TA3D.FPS Limit" );
    TA3D::VARS::lp_CONFIG->shadow_r  = cfgFile->pullAsFloat( "TA3D.Shadow R" );
    TA3D::VARS::lp_CONFIG->timefactor = cfgFile->pullAsFloat( "TA3D.Time Factor" );

    TA3D::VARS::lp_CONFIG->shadow_quality = cfgFile->pullAsInt( "TA3D.Shadow Quality" );
    TA3D::VARS::lp_CONFIG->priority_level = cfgFile->pullAsInt( "TA3D.Priority Level" );
    TA3D::VARS::lp_CONFIG->fsaa = cfgFile->pullAsInt( "TA3D.FSAA" );
    TA3D::VARS::lp_CONFIG->Lang = cfgFile->pullAsInt( "TA3D.Language" );
    TA3D::VARS::lp_CONFIG->water_quality = cfgFile->pullAsInt( "TA3D.Water Quality" );
    TA3D::VARS::lp_CONFIG->screen_width = cfgFile->pullAsInt( "TA3D.Screen Width" );
    TA3D::VARS::lp_CONFIG->screen_height = cfgFile->pullAsInt( "TA3D.Screen Height" );

    TA3D::VARS::lp_CONFIG->showfps = cfgFile->pullAsBool( "TA3D.Show FPS" );
    TA3D::VARS::lp_CONFIG->wireframe = cfgFile->pullAsBool( "TA3D.Show Wireframe" );
    TA3D::VARS::lp_CONFIG->particle = cfgFile->pullAsBool( "TA3D.Show particles" );
    TA3D::VARS::lp_CONFIG->waves = cfgFile->pullAsBool( "TA3D.Show Waves" );
    TA3D::VARS::lp_CONFIG->shadow = cfgFile->pullAsBool( "TA3D.Show Shadows" );
    TA3D::VARS::lp_CONFIG->height_line = cfgFile->pullAsBool( "TA3D.Show Height Lines" );
    TA3D::VARS::lp_CONFIG->fullscreen = cfgFile->pullAsBool( "TA3D.Show FullScreen" );
    TA3D::VARS::lp_CONFIG->detail_tex = cfgFile->pullAsBool( "TA3D.Detail Texture" );
    TA3D::VARS::lp_CONFIG->draw_console_loading = cfgFile->pullAsBool( "TA3D.Draw Console Loading" );

    delete cfgFile; 

    LANG = lp_CONFIG->Lang;
    I18N::Instance()->currentLanguage(lp_CONFIG->Lang); // refresh the language used by the i18n object
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

    LoadConfigFile();

    install_int_ex( Timer, precision);

    init_surf_buf();

    installOpenGLExtensions();

    HPIManager=new cHPIHandler("");

    /*RGB * */pal = new RGB[256];
    TA3D::UTILS::HPI::load_palette(pal); // Charge la palette graphique
    set_palette(pal);		// Active la palette chargée

    Console->AddEntry(I18N::Translate("Initializing texture manager"));
    texture_manager.init();
    texture_manager.all_texture();

    TheModel = new MODEL;
    obj_table = new OBJECT*[1000];

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

    int i;

    int counter=0;
    int FPS=0;
    int FPS_Timer = msec_timer;
    float Conv = 0.001f;

    // Pour la gestion de l'interface
    gfx->normal_font.copy(font);
    gui_font = gfx->normal_font;

    WND MainWnd;		// Fenêtre principale

    MainWnd.x=0;				MainWnd.y=0;
    MainWnd.width=SCREEN_W;		MainWnd.height=28;
    MainWnd.Title = I18N::Translate("Barre de menus").c_str();
    MainWnd.Lock=true;
    MainWnd.NbObj=2;
    MainWnd.Objets=new GUIOBJ[MainWnd.NbObj];

    // Menu fichier
    const char *mnu_fichier_list[] = {"Fichiers","Nouveau","Ouvrir","Sauver","Importer(*.ASC)","Importer(*.3DO)","Importer(*.3DS)","Quitter"};
    String::Vector mnu_fichier( mnu_fichier_list, &(mnu_fichier_list[8]));
    I18N::Translate(mnu_fichier);
    MainWnd.Objets[0].create_menu(3,12,83,24,mnu_fichier,mnu_file);

    // Menu surface
    const char *mnu_surface_list[] = {"Surface","Editer","Copier","Coller","Reinitialiser","Coller sur toutes"};
    String::Vector mnu_surface( mnu_surface_list, &(mnu_surface_list[6]));
    I18N::Translate(mnu_surface);
    MainWnd.Objets[1].create_menu(86,12,166,24,mnu_surface,mnu_surf);

    WND	EditWnd;		// Fenêtre d'édition
    EditWnd.Title = I18N::Translate("Edition").c_str();
    EditWnd.width=200;							EditWnd.height=240;
    EditWnd.x=SCREEN_W-EditWnd.width;			EditWnd.y=SCREEN_H-EditWnd.height;
    EditWnd.Lock=false;							EditWnd.NbObj=12;
    EditWnd.Objets=new GUIOBJ[EditWnd.NbObj];
    EditWnd.Objets[0].create_text(5,12, I18N::Translate("Parties :" ), 0xFFFFFF);
    String::Vector mnu_part;
    mnu_part.resize( nb_obj()+1 );
    for( i=0; i<nb_obj(); i++ )
        mnu_part[i+1] = format( I18N::Translate("Partie %d").c_str() , i);

    mnu_part[0]=mnu_part[1];
    EditWnd.Objets[1].create_menu(5,20,196,32,mnu_part,mnu_selec);

    mnu_part.clear();

    EditWnd.Objets[2].create_button(5,40,196,52, I18N::Translate("renommer").c_str(), button_rename);
    EditWnd.Objets[3].create_button(5,60,196,72, I18N::Translate("passer en fils").c_str(), button_child);
    EditWnd.Objets[4].create_button(5,80,196,92, I18N::Translate("effacer").c_str(),button_remove);
    EditWnd.Objets[5].create_button(5,100,196,112, I18N::Translate("échelle").c_str(),button_scale);
    EditWnd.Objets[6].create_button(5,120,196,132,I18N::Translate("mirroir/x").c_str(),button_mirror_x);
    EditWnd.Objets[7].create_button(5,140,196,152,I18N::Translate("mirroir/y").c_str(),button_mirror_y);
    EditWnd.Objets[8].create_button(5,160,196,172,I18N::Translate("mirroir/z").c_str(),button_mirror_z);
    EditWnd.Objets[9].create_button(5,180,196,192,I18N::Translate("x<->y").c_str(),button_change_xy);
    EditWnd.Objets[10].create_button(5,200,196,212,I18N::Translate("y<->z").c_str(),button_change_yz);
    EditWnd.Objets[11].create_button(5,220,196,232,I18N::Translate("z<->x").c_str(),button_change_zx);

    int amx,amy,amz,amb;
    int IsOnGUI;

    float ScaleFactor=1.0f;

    SCRIPT_DATA cur_data;
    cur_data.load(nb_obj());

    do
    {
        EditWnd.Objets[1].Text.resize( nb_obj() + 1 );
        for(int i=0;i<nb_obj();i++)
            if(obj_table[i]->name!=NULL) {
                String h="";
                for(int e=0;e<h_table[i];e++)	h+="-";
                EditWnd.Objets[1].Text[i+1] = (h+obj_table[i]->name);
            }

        EditWnd.Objets[1].Text[0] = EditWnd.Objets[1].Text[cur_part+1];
        amx=mouse_x;
        amy=mouse_y;
        amz=mouse_z;
        amb=mouse_b;

        rest(1);
        poll_mouse();		// obtient les coordonnées de la souris

        IsOnGUI=MainWnd.check(amx,amy,amz,amb);		// gestion de l'interface utilisateur graphique
        if(!IsOnGUI)
            IsOnGUI = EditWnd.check(amx,amy,amz,amb,counter==0);

        if(!IsOnGUI) {				// Si la souris n'est pas sur un élément de l'interface utilisateur(si elle est sur la fenêtre 3D)
            if(mouse_b==1) {		// Appui sur le bouton gauche pour effecture une rotation
                r2+=mouse_x-amx;
            }
            if(mouse_b==4)			// Appui sur le bouton du milieu pour regler le zoom
                ScaleFactor+=((mouse_y-amy)+(mouse_x-amx)+(mouse_z-amz))*0.1f;
            if(mouse_b==2 && cur_part>=0 && cur_part<nb_obj()) {			// Left-clic to move current object
                VECTOR DP( 0.1f*(mouse_x-amx)*cos(r2*DEG2RAD), -(mouse_y-amy)*0.1f, 0.1f*(mouse_x-amx)*sin(r2*DEG2RAD) );
                obj_table[cur_part]->pos_from_parent = obj_table[cur_part]->pos_from_parent + DP;
                for( int i = 0 ; i < obj_table[cur_part]->nb_vtx ; i++ )
                    obj_table[cur_part]->points[ i ] = obj_table[cur_part]->points[ i ] - DP;
                if( obj_table[cur_part]->child ) {
                    OBJECT *cur = obj_table[cur_part]->child;
                    while( cur ) {
                        cur->pos_from_parent = cur->pos_from_parent - DP;
                        cur = cur->next;
                    }
                }
            }
        }

        counter++;
        if(msec_timer-FPS_Timer>=50) {			// Calcule du nombre d'images par secondes
            FPS = (int)(counter/((msec_timer-FPS_Timer)*Conv));
            counter = 0;
            FPS_Timer = msec_timer;
        }

        show_mouse(NULL);					// Cache la souris
        if(key[KEY_ESC] || ClickOnExit) done=true;			// Quitte si on appuie sur echap ou clique sur quitter

        if( key[ KEY_X ] && !key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 0.0f;		r3 = 0.0f;	}
        if( key[ KEY_Y ] && !key[ KEY_LSHIFT ] ) {	r1 = 90.0f;		r2 = 0.0f;		r3 = 0.0f;	}
        if( key[ KEY_Z ] && !key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = -90.0f;		r3 = 0.0f;	}

        if( key[ KEY_X ] && key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 180.0f;	r3 = 0.0f;	}
        if( key[ KEY_Y ] && key[ KEY_LSHIFT ] ) {	r1 = -90.0f;	r2 = 0.0f;		r3 = 0.0f;	}
        if( key[ KEY_Z ] && key[ KEY_LSHIFT ] ) {	r1 = 0.0f;		r2 = 90.0f;	r3 = 0.0f;	}

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

        if(cur_data.nb_piece!=nb_obj()) {
            cur_data.destroy();
            cur_data.load(nb_obj());
        }

        glDisable( GL_CULL_FACE );
        if(IsOnGUI && EditWnd.Objets[1].Etat) {				// Affiche la partie prête à être sélectionnée
            glColor3f(0.15f,0.15f,0.15f);
            for(int i=0;i<cur_data.nb_piece;i++)
                cur_data.flag[i] = FLAG_ANIMATE;
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
            glClear(GL_DEPTH_BUFFER_BIT);
            for(int i=0 ; i < cur_data.nb_piece ; ++i)
                cur_data.flag[i] = ((i == EditWnd.Objets[1].Data) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
            glColor3f(1.0f,1.0f,1.0f);
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            glColor3f(1.0f,1.0f,1.0f);
            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
        }
        else if(TheModel) {					// Affichage normal
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            for(int i=0;i<cur_data.nb_piece;i++)
                cur_data.flag[i]= ((i != cur_part) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
            for(int i=0;i<cur_data.nb_piece;i++)
                cur_data.flag[i]= ((i == cur_part) ? 0 : FLAG_HIDE) | FLAG_ANIMATE;
            float col = cos(msec_timer*0.002f)*0.375f+0.625f;
            glColor3f(col,col,col);
            TheModel->draw(msec_timer * 0.001f,&cur_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);
            glColor3f(1.0f,1.0f,1.0f);
            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
        }
        glEnable( GL_CULL_FACE );

        //----------- draw the sphere that shows where is attached the current object ------------------

        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        glDisable( GL_LIGHTING );
        MATRIX_4x4 M = Scale( 1.0f );
        TheModel->compute_coord( &cur_data, &M );
        VECTOR P = cur_data.pos[ cur_part ];

        glPushMatrix();
        glDisable(GL_TEXTURE_2D);
        glColor4ub( 0xFF, 0x0, 0x0, 0xFF );
        glTranslatef( P.x, P.y, P.z );

        glBegin( GL_TRIANGLES );

        float s = 5.0f;

        for( int i = 0 ; i < 8 ; i++ ) {
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

        String help_msg = "";
        EditWnd.draw( help_msg );				// Dessine la fenêtre d'édition
        MainWnd.draw( help_msg );				// Dessine la fenêtre de la barre de menus

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

    delete[] obj_table;

    delete TheModel;

    delete gfx;

    delete Console;

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
    if(mnu_index>=0 && mnu_index<nb_obj())
        cur_part=mnu_index;
}

void SurfEdit()
{
    if(nb_obj()<=0) return;			// S'il n'y a rien à éditer, on quitte tout de suite la fonction, sinon ça plante
    if(nb_obj()==1 && TheModel->obj.name==NULL)	return;

    WND SEdit;				// Fenêtre d'édition des surfaces
    SEdit.Title=I18N::Translate( "Editeur de surfaces" ).c_str();
    SEdit.x=0;					SEdit.y=0;
    SEdit.width=SCREEN_W;		SEdit.height=SCREEN_H;
    SEdit.Lock=true;
    SEdit.NbObj=59;
    SEdit.Objets=new GUIOBJ[SEdit.NbObj];

    // Bouton OK
    SEdit.Objets[0].create_button(SEdit.width/2-16,SEdit.height-20,SEdit.width/2+16,SEdit.height-4,"OK",NULL);

    // Texte indiquant les éléments modifiant la couleur materielle de la surface
    SEdit.Objets[1].create_text(10,16,I18N::Translate( "Couleur de la matiere:" ).c_str(),Noir);
    // Barre de texte pour modifier la couleur de matière rouge(avec texte indicateur)
    SEdit.Objets[2].create_text(10,30,I18N::Translate("rouge->").c_str(),Rouge);
    SEdit.Objets[3].create_textbar(66,28,106,44,format("%d",(int)(obj_table[cur_part]->surface.Color[0]*255.0f)),4,NULL);
    // Barre de texte pour modifier la couleur de matière vert(avec texte indicateur)
    SEdit.Objets[4].create_text(10,50,I18N::Translate( "vert ->" ).c_str(),Vert);
    SEdit.Objets[5].create_textbar(66,48,106,64,format("%d",(int)(obj_table[cur_part]->surface.Color[1]*255.0f)),4,NULL);
    // Barre de texte pour modifier la couleur de matière bleu(avec texte indicateur)
    SEdit.Objets[6].create_text(10,70,I18N::Translate( "bleu ->" ).c_str(),Bleu);
    SEdit.Objets[7].create_textbar(66,68,106,84,format("%d",(int)(obj_table[cur_part]->surface.Color[2]*255.0f)),4,NULL);
    // Barre de texte pour modifier le canal alpha de la couleur de matière(avec texte indicateur)
    SEdit.Objets[8].create_text(10,90,I18N::Translate( "alpha->" ).c_str(),Blanc);
    SEdit.Objets[9].create_textbar(66,88,106,104,format("%d",(int)(obj_table[cur_part]->surface.Color[3]*255.0f)),4,NULL);

    // Case à cocher pour activer/désactiver l'effet de réflection
    SEdit.Objets[10].create_optionc(200,12,I18N::Translate( "Effet de reflection" ).c_str(),obj_table[cur_part]->surface.Flag&SURFACE_REFLEC,NULL);
    // Barre de texte pour modifier la couleur de reflection rouge(avec texte indicateur)
    SEdit.Objets[11].create_text(130,30,I18N::Translate( "rouge->" ).c_str(),Rouge);
    SEdit.Objets[12].create_textbar(186,28,226,44,format("%d",(int)(obj_table[cur_part]->surface.RColor[0]*255.0f)),4,NULL);
    // Barre de texte pour modifier la couleur de reflection vert(avec texte indicateur)
    SEdit.Objets[13].create_text(130,50,I18N::Translate( "vert ->" ).c_str(),Vert);
    SEdit.Objets[14].create_textbar(186,48,226,64,format("%d",(int)(obj_table[cur_part]->surface.RColor[1]*255.0f)),4,NULL);
    // Barre de texte pour modifier la couleur de reflection bleu(avec texte indicateur)
    SEdit.Objets[15].create_text(130,70,I18N::Translate( "bleu ->" ).c_str(),Bleu);
    SEdit.Objets[16].create_textbar(186,68,226,84,format("%d",(int)(obj_table[cur_part]->surface.RColor[2]*255.0f)),4,NULL);
    // Barre de texte pour modifier le canal alpha de la couleur de reflection(avec texte indicateur)
    SEdit.Objets[17].create_text(130,90,I18N::Translate( "alpha->" ).c_str(),Blanc);
    SEdit.Objets[18].create_textbar(186,88,226,104,format("%d",(int)(obj_table[cur_part]->surface.RColor[3]*255.0f)),4,NULL);

    // Case à cocher pour activer/désactiver le mode d'éclairage GOURAUD
    SEdit.Objets[19].create_optionc(10,120,I18N::Translate( "Rendu GOURAUD" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_GOURAUD)==SURFACE_GOURAUD,NULL);
    // Case à cocher pour activer/désactiver l'éclairage
    SEdit.Objets[20].create_optionc(10,140,I18N::Translate( "Calculs d'eclairage" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_LIGHTED)==SURFACE_LIGHTED,NULL);
    // Case à cocher pour activer/désactiver le texturage
    SEdit.Objets[21].create_optionc(10,160,I18N::Translate( "Texturage" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED)==SURFACE_TEXTURED,NULL);

    // Texte d'aide sur le dessin 3D de textures, directement sur l'objet
    SEdit.Objets[22].create_text(58,270,I18N::Translate( "Le bouton peinture de l'objet permet de creer une texture a appliquer a l'objet grace" ) );
    SEdit.Objets[23].create_text(10,278,I18N::Translate( "a un outil integre de dessin directement sur l'objet en 3D, et le bouton plaquage de texture" ) );
    SEdit.Objets[24].create_text(10,286,I18N::Translate( "permet egalement de modifier le plaquage de la texture sur l'objet." ) );

    // Boutons "peinture" et "plaquage de texture"
    SEdit.Objets[25].create_button(20,300,188,316,I18N::Translate( "peinture" ).c_str(),SurfPaint);
    SEdit.Objets[26].create_button(20,320,188,336,I18N::Translate( "plaquage de texture" ).c_str(),TexturePosEdit);

    // Menu déroulant de sélection de partie de la meshe et texte indicateur
    SEdit.Objets[27].create_text(58,340,I18N::Translate( "Partie en cours d'edition:" ).c_str() );
    String::Vector Part_names(1+nb_obj());
    for(int i=0;i<nb_obj();i++)
        Part_names[i+1] = obj_table[i]->name;
    Part_names[0] = Part_names[cur_part+1];

    SEdit.Objets[28].create_menu(10,350,170,362,Part_names,S_MPart_Sel);

    // Case à cocher pour activer/désactiver la transparence
    SEdit.Objets[29].create_optionc(186,120,I18N::Translate( "Transparence" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_BLENDED)==SURFACE_BLENDED,NULL);

    // Case à cocher pour activer/désactiver la couleur du joueur
    SEdit.Objets[30].create_optionc(186,140,I18N::Translate( "Couleur joueur" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_PLAYER_COLOR)==SURFACE_PLAYER_COLOR,NULL);

    // Case à cocher pour activer/désactiver les shaders GLSL
    SEdit.Objets[31].create_optionc(186,160,I18N::Translate( "GLSL" ).c_str(),(obj_table[cur_part]->surface.Flag&SURFACE_GLSL)==SURFACE_GLSL,NULL);

    // Partie concernant l'animation par défaut
    SEdit.Objets[32].create_optionc(320,32,I18N::Translate( "rotation" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION), NULL );
    SEdit.Objets[33].create_optionc(320,44,I18N::Translate( "périodique" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION_PERIODIC), NULL );
    SEdit.Objets[34].create_optionc(320,56,I18N::Translate( "sinusoïdal" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & ROTATION_COSINE), NULL );

    SEdit.Objets[35].create_optionc(480,32,I18N::Translate( "translation" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION), NULL );
    SEdit.Objets[36].create_optionc(480,44,I18N::Translate( "périodique" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION_PERIODIC), NULL );
    SEdit.Objets[37].create_optionc(480,56,I18N::Translate( "sinusoïdal" ), obj_table[cur_part]->animation_data != NULL && (obj_table[cur_part]->animation_data->type & TRANSLATION_COSINE), NULL );

    SEdit.Objets[38].create_text(320,70, "angle_0" );
    SEdit.Objets[39].create_textbar(320,80,384,96, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.x) : "0",8, NULL );
    SEdit.Objets[40].create_textbar(320,100,384,116, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.y) : "0",8, NULL );
    SEdit.Objets[41].create_textbar(320,120,384,136, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_0.z) : "0",8, NULL );

    SEdit.Objets[42].create_text(390,70, "angle_1" );
    SEdit.Objets[43].create_textbar(390,80,454,96, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.x) : "0",8, NULL );
    SEdit.Objets[44].create_textbar(390,100,454,116, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.y) : "0",8, NULL );
    SEdit.Objets[45].create_textbar(390,120,454,136, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_1.z) : "0",8, NULL );

    SEdit.Objets[46].create_text(460,70, "translate_0" );
    SEdit.Objets[47].create_textbar(460,80,524,96, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.x) : "0",8, NULL );
    SEdit.Objets[48].create_textbar(460,100,524,116, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.y) : "0",8, NULL );
    SEdit.Objets[49].create_textbar(460,120,524,136, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_0.z) : "0",8, NULL );

    SEdit.Objets[50].create_text(560,70, "translate_1" );
    SEdit.Objets[51].create_textbar(560,80,624,96, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.x) : "0",8, NULL );
    SEdit.Objets[52].create_textbar(560,100,624,116, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.y) : "0",8, NULL );
    SEdit.Objets[53].create_textbar(560,120,624,136, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_1.z) : "0",8, NULL );

    SEdit.Objets[54].create_text(360,140, "angle_w=" );
    SEdit.Objets[55].create_textbar(430,140,500,156, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->angle_w) : "0",8, NULL );

    SEdit.Objets[56].create_text(360,160, "translate_w=" );
    SEdit.Objets[57].create_textbar(460,160,530,176, obj_table[cur_part]->animation_data ? format( "%f", obj_table[cur_part]->animation_data->translate_w) : "0",8, NULL );

    SEdit.Objets[58].create_button(16,380,216,390, I18N::Translate("optimiser la géometrie"), NULL );

    bool done=false;

    int amx,amy,amb,amz;

    do
    {
        if( SEdit.Objets[32].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~ROTATION;

        if( SEdit.Objets[33].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION_PERIODIC;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~ROTATION_PERIODIC;

        if( SEdit.Objets[34].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= ROTATION_COSINE;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~ROTATION_COSINE;

        if( SEdit.Objets[35].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION;

        if( SEdit.Objets[36].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION_PERIODIC;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION_PERIODIC;

        if( SEdit.Objets[37].Etat ) {
            if( obj_table[cur_part]->animation_data == NULL )	obj_table[cur_part]->animation_data = new ANIMATION;
            obj_table[cur_part]->animation_data->type |= TRANSLATION_COSINE;
        }
        else if( obj_table[cur_part]->animation_data )
            obj_table[cur_part]->animation_data->type &= ~TRANSLATION_COSINE;

        if( obj_table[cur_part]->animation_data ) {		// Read other data
            obj_table[cur_part]->animation_data->angle_0.x = atof( SEdit.Objets[39].Text[0].c_str());
            obj_table[cur_part]->animation_data->angle_0.y = atof( SEdit.Objets[40].Text[0].c_str());
            obj_table[cur_part]->animation_data->angle_0.z = atof( SEdit.Objets[41].Text[0].c_str());

            obj_table[cur_part]->animation_data->angle_1.x = atof( SEdit.Objets[43].Text[0].c_str());
            obj_table[cur_part]->animation_data->angle_1.y = atof( SEdit.Objets[44].Text[0].c_str());
            obj_table[cur_part]->animation_data->angle_1.z = atof( SEdit.Objets[45].Text[0].c_str());

            obj_table[cur_part]->animation_data->translate_0.x = atof( SEdit.Objets[47].Text[0].c_str());
            obj_table[cur_part]->animation_data->translate_0.y = atof( SEdit.Objets[48].Text[0].c_str());
            obj_table[cur_part]->animation_data->translate_0.z = atof( SEdit.Objets[49].Text[0].c_str());

            obj_table[cur_part]->animation_data->translate_1.x = atof( SEdit.Objets[51].Text[0].c_str());
            obj_table[cur_part]->animation_data->translate_1.y = atof( SEdit.Objets[52].Text[0].c_str());
            obj_table[cur_part]->animation_data->translate_1.z = atof( SEdit.Objets[53].Text[0].c_str());

            obj_table[cur_part]->animation_data->angle_w = atof( SEdit.Objets[55].Text[0].c_str());
            obj_table[cur_part]->animation_data->translate_w = atof( SEdit.Objets[57].Text[0].c_str());
        }

        if( SEdit.Objets[58].Etat && !(obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED) )
        {
            obj_geo_optimize( cur_part, true );
            obj_maj_normal( cur_part );
        }

        if (SEdit.Objets[28].Text[0] != SEdit.Objets[28].Text[cur_part+1]) // Si l'utilisateur a sélectionné une partie
        {
            SEdit.Objets[19].Etat=obj_table[cur_part]->surface.Flag&SURFACE_GOURAUD;
            SEdit.Objets[20].Etat=obj_table[cur_part]->surface.Flag&SURFACE_LIGHTED;
            SEdit.Objets[21].Etat=obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED;
            SEdit.Objets[10].Etat=obj_table[cur_part]->surface.Flag&SURFACE_REFLEC;
            SEdit.Objets[29].Etat=obj_table[cur_part]->surface.Flag&SURFACE_BLENDED;
            SEdit.Objets[30].Etat=obj_table[cur_part]->surface.Flag&SURFACE_PLAYER_COLOR;
            SEdit.Objets[31].Etat=obj_table[cur_part]->surface.Flag&SURFACE_GLSL;
            SEdit.Objets[3].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.Color[0]*255.0f));
            SEdit.Objets[5].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.Color[1]*255.0f));
            SEdit.Objets[7].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.Color[2]*255.0f));
            SEdit.Objets[9].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.Color[3]*255.0f));

            SEdit.Objets[12].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.RColor[0]*255.0f));
            SEdit.Objets[14].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.RColor[1]*255.0f));
            SEdit.Objets[16].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.RColor[2]*255.0f));
            SEdit.Objets[18].Text[0] = format("%d",(int)(obj_table[cur_part]->surface.RColor[3]*255.0f));

            if( obj_table[cur_part]->animation_data )
            {
                SEdit.Objets[39].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_0.x ) );
                SEdit.Objets[40].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_0.y ) );
                SEdit.Objets[41].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_0.z ) );

                SEdit.Objets[43].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_1.x ) );
                SEdit.Objets[44].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_1.y ) );
                SEdit.Objets[45].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_1.z ) );

                SEdit.Objets[47].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_0.x ) );
                SEdit.Objets[48].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_0.y ) );
                SEdit.Objets[49].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_0.z ) );

                SEdit.Objets[51].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_1.x ) );
                SEdit.Objets[52].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_1.y ) );
                SEdit.Objets[53].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_1.z ) );

                SEdit.Objets[55].set_caption( format( "%f", obj_table[cur_part]->animation_data->angle_w ) );
                SEdit.Objets[57].set_caption( format( "%f", obj_table[cur_part]->animation_data->translate_w ) );

                SEdit.Objets[32].Etat = obj_table[cur_part]->animation_data->type & ROTATION;
                SEdit.Objets[33].Etat = obj_table[cur_part]->animation_data->type & ROTATION_PERIODIC;
                SEdit.Objets[34].Etat = obj_table[cur_part]->animation_data->type & ROTATION_COSINE;

                SEdit.Objets[35].Etat = obj_table[cur_part]->animation_data->type & TRANSLATION;
                SEdit.Objets[36].Etat = obj_table[cur_part]->animation_data->type & TRANSLATION_PERIODIC;
                SEdit.Objets[37].Etat = obj_table[cur_part]->animation_data->type & TRANSLATION_COSINE;
            }
            else
            {
                SEdit.Objets[39].set_caption( "0" );
                SEdit.Objets[40].set_caption( "0" );
                SEdit.Objets[41].set_caption( "0" );

                SEdit.Objets[43].set_caption( "0" );
                SEdit.Objets[44].set_caption( "0" );
                SEdit.Objets[45].set_caption( "0" );

                SEdit.Objets[47].set_caption( "0" );
                SEdit.Objets[48].set_caption( "0" );
                SEdit.Objets[49].set_caption( "0" );

                SEdit.Objets[51].set_caption( "0" );
                SEdit.Objets[52].set_caption( "0" );
                SEdit.Objets[53].set_caption( "0" );

                SEdit.Objets[55].set_caption( "0" );
                SEdit.Objets[57].set_caption( "0" );

                SEdit.Objets[32].Etat = false;
                SEdit.Objets[33].Etat = false;
                SEdit.Objets[34].Etat = false;

                SEdit.Objets[35].Etat = false;
                SEdit.Objets[36].Etat = false;
                SEdit.Objets[37].Etat = false;
            }
        }
        SEdit.Objets[28].Text[0] = SEdit.Objets[28].Text[cur_part+1];

        amx=mouse_x;
        amy=mouse_y;
        amz=mouse_z;
        amb=mouse_b;

        // Modifie la couleur de matière
        obj_table[cur_part]->surface.Color[0]=atoi(SEdit.Objets[3].Text[0].c_str())/255.0f;
        obj_table[cur_part]->surface.Color[1]=atoi(SEdit.Objets[5].Text[0].c_str())/255.0f;
        obj_table[cur_part]->surface.Color[2]=atoi(SEdit.Objets[7].Text[0].c_str())/255.0f;
        obj_table[cur_part]->surface.Color[3]=atoi(SEdit.Objets[9].Text[0].c_str())/255.0f;
        // Modifie la couleur de réflexion
        if(SEdit.Objets[10].Etat)
        {
            obj_table[cur_part]->surface.Flag|=SURFACE_REFLEC;
            obj_table[cur_part]->surface.RColor[0]=atoi(SEdit.Objets[12].Text[0].c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[1]=atoi(SEdit.Objets[14].Text[0].c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[2]=atoi(SEdit.Objets[16].Text[0].c_str())/255.0f;
            obj_table[cur_part]->surface.RColor[3]=atoi(SEdit.Objets[18].Text[0].c_str())/255.0f;
        }
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_REFLEC;

        // Active désactive certaines options de surface
        if(SEdit.Objets[31].Etat)								// Utilisation de GLSL
            obj_table[cur_part]->surface.Flag|=SURFACE_GLSL;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_GLSL;
        if(SEdit.Objets[30].Etat)								// Utilisation de la couleur du joueur
            obj_table[cur_part]->surface.Flag|=SURFACE_PLAYER_COLOR;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_PLAYER_COLOR;
        if(SEdit.Objets[29].Etat)								// Utilisation de la transparence
            obj_table[cur_part]->surface.Flag|=SURFACE_BLENDED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_BLENDED;
        if(SEdit.Objets[19].Etat)								// Utilisation de l'éclairage GOURAUD
            obj_table[cur_part]->surface.Flag|=SURFACE_GOURAUD;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_GOURAUD;
        if(SEdit.Objets[20].Etat)								// Utilisation de l'éclairage
            obj_table[cur_part]->surface.Flag|=SURFACE_LIGHTED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_LIGHTED;
        if(SEdit.Objets[21].Etat)								// Utilisation du texturage
            obj_table[cur_part]->surface.Flag|=SURFACE_TEXTURED;
        else
            obj_table[cur_part]->surface.Flag&=~SURFACE_TEXTURED;

        poll_mouse();		// obtient les coordonnées de la souris

        SEdit.check(amx,amy,amz,amb);		// gestion de l'interface utilisateur graphique

        if(SEdit.Objets[0].Etat) done=true;		// En cas de click sur "OK", on quitte la fenêtre

        show_mouse(NULL);					// Cache la souris
        if(key[KEY_ESC]) done=true;			// Quitte si on appuie sur echap
        // Efface tout
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gfx->set_2D_mode();		// Passe en mode dessin allegro

        String help_msg = "";
        SEdit.draw( help_msg );				// Dessine la fenêtre d'édition

        // Dessine les textures utilisées par la surface
        if(obj_table[cur_part]->surface.NbTex>0)
        {
            glEnable(GL_TEXTURE_2D);
            glColor3f(1.0f,1.0f,1.0f);
            for(int i=0;i<obj_table[cur_part]->surface.NbTex;i++) 
            {
                glBindTexture(GL_TEXTURE_2D,obj_table[cur_part]->surface.gltex[i]);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f,0.0f);	glVertex2f(200+64*i,200);
                glTexCoord2f(1.0f,0.0f);	glVertex2f(263+64*i,200);
                glTexCoord2f(1.0f,1.0f);	glVertex2f(263+64*i,263);
                glTexCoord2f(0.0f,1.0f);	glVertex2f(200+64*i,263);
                glEnd();
            }
            glDisable(GL_TEXTURE_2D);
        }
        glDisable(GL_TEXTURE_2D);
        int index=-1;
        if(mouse_y>=200 && mouse_y<=263)
        {
            if(mouse_x>=200 && mouse_x<=711)
            {
                index=(mouse_x-200)>>6;
                if(index>=8) index=-1;
            }
        }

        glBegin(GL_LINES);
        for(int i=0;i<8;i++)
        {
            if(i==index)
                glColor3f(1.0f,0.0f,0.0f);
            else
                glColor3f(1.0f,1.0f,1.0f);
            glVertex2f(200+64*i,200);	glVertex2f(263+64*i,200);
            glVertex2f(263+64*i,200);	glVertex2f(263+64*i,263);
            glVertex2f(263+64*i,263);	glVertex2f(200+64*i,263);
            glVertex2f(200+64*i,263);	glVertex2f(200+64*i,200);
        }
        glEnd();

        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
        show_mouse(screen);
        algl_draw_mouse();

        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

        if(index!=-1 && amb!=mouse_b && mouse_b==1) // L'utilisateur veut choisir une texture
        {
            String filename = Dialog( I18N::Translate( "Charger une texture" ).c_str(),"*.*");
            BITMAP *bmp_tex = filename.length()>0 ? load_bitmap(filename.c_str(),NULL) : NULL;
            if(bmp_tex) // Si il s'agit d'ajouter/modifier une texture
            {
                if(bitmap_color_depth(bmp_tex)==24 || strstr(filename.c_str(),".jpg")!=NULL)
                {
                    BITMAP *tmp = create_bitmap_ex(32,bmp_tex->w,bmp_tex->h);
                    for(int y=0;y<tmp->h;y++)
                        for(int x=0;x<tmp->w;x++)
                            putpixel(tmp,x,y,getpixel(bmp_tex,x,y)|0xFF000000);
                    destroy_bitmap(bmp_tex);
                    bmp_tex=tmp;
                }
                if(index>=obj_table[cur_part]->surface.NbTex)// Ajoute la texture
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
                    glDeleteTextures(1,&obj_table[cur_part]->surface.gltex[index]);
                    allegro_gl_use_alpha_channel(true);
                    allegro_gl_set_texture_format(GL_RGBA8);
                    obj_table[cur_part]->surface.gltex[index]=allegro_gl_make_texture(bmp_tex);
                    allegro_gl_use_alpha_channel(false);
                    glBindTexture(GL_TEXTURE_2D,obj_table[cur_part]->surface.gltex[index]);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    destroy_bitmap(bmp_tex);
                }
            }
            else
            {
                if(index<obj_table[cur_part]->surface.NbTex && obj_table[cur_part]->surface.NbTex>0) // Sinon on enlève la texture
                {
                    glDeleteTextures(1,&obj_table[cur_part]->surface.gltex[index]);

                    obj_table[cur_part]->surface.NbTex--;
                    if(index<obj_table[cur_part]->surface.NbTex)		// Décale les textures si nécessaire
                        for(int i=index;i<obj_table[cur_part]->surface.NbTex;i++)
                            obj_table[cur_part]->surface.gltex[i]=obj_table[cur_part]->surface.gltex[i+1];
                }
            }
        }		// Fin de l'algorithme de sélection de texture

        // Affiche
        gfx->flip();
    } while(!done);
}					// Fin de l'éditeur de surfaces

/*---------------------------------------------------------------------------------------------------\
  |                                Editeur de texture de surface sur l'objet                           |
  \---------------------------------------------------------------------------------------------------*/

void SurfPaint(int index)
{
    // Vérifie s'il y a un pack de texture de sélectionné et si la surface est texturée
    if (!(obj_table[cur_part]->surface.Flag&SURFACE_TEXTURED))
        return;

    WND SPaint;				// Fenêtre d'édition des textures
    SPaint.Title=I18N::Translate( "Peinture de l'objet" ).c_str();
    SPaint.x=0;					SPaint.y=0;
    SPaint.width=210;			SPaint.height=220;
    SPaint.Lock=false;
    SPaint.NbObj=13;
    SPaint.Objets=new GUIOBJ[SPaint.NbObj];

    // Bouton Ok
    SPaint.Objets[0].create_button(SPaint.width/2-16,SPaint.height-20,SPaint.width/2+16,SPaint.height-4,"Ok",NULL);

    // Bouton Créer une texture pour l'objet
    SPaint.Objets[1].create_button(10,20,154,32,I18N::Translate( "Nouvelle texture" ).c_str(),NULL);

    // Bouton Plaquage cylindrique
    SPaint.Objets[2].create_button(10,40,186,52,I18N::Translate( "Plaquage cylindrique" ).c_str(),NULL);

    // Bouton Plaqueur de texture
    SPaint.Objets[3].create_button(10,60,178,72,I18N::Translate( "Plaqueur de texture" ).c_str(),NULL);

    // Boutons Fil de fer et mode normal
    SPaint.Objets[4].create_button(10,80,106,92,I18N::Translate( "Fil de fer" ).c_str(),NULL);
    SPaint.Objets[5].create_button(110,80,200,92,I18N::Translate( "Normal" ).c_str(),NULL);

    // Boutons pour controler le mode d'édition
    SPaint.Objets[6].create_button(10,100,98,112,I18N::Translate( "Selection" ).c_str(),NULL);
    SPaint.Objets[7].create_button(100,100,180,112,I18N::Translate( "Peinture" ).c_str(),NULL);

    // Bouton desassembler les faces
    SPaint.Objets[8].create_button(10,120,122,132,I18N::Translate( "Desassembler" ).c_str(),NULL);

    // Bouton normaliser
    SPaint.Objets[9].create_button(10,140,106,152,I18N::Translate( "Normaliser" ).c_str(),NULL);

    // Bouton optimiser
    SPaint.Objets[10].create_button(10,160,98,172,I18N::Translate( "Optimiser" ).c_str(),NULL);

    // Bouton Plaquage cubique
    SPaint.Objets[11].create_button(10,180,154,192,I18N::Translate( "Plaquage cubique" ).c_str(),NULL);

    // Bouton résolution
    SPaint.Objets[12].create_button(112,160,200,172,I18N::Translate( "résolution" ).c_str(),NULL);

    WND SCoor;				// Fenêtre d'édition du plaquage de la texture
    SCoor.Title=I18N::Translate( "Peinture de l'objet" ).c_str();
    SCoor.width=310;				SCoor.height=340;
    SCoor.x=SCREEN_W-SCoor.width;	SCoor.y=SCREEN_H-SCoor.height;
    SCoor.Lock=false;
    SCoor.NbObj=1;
    SCoor.Objets=new GUIOBJ[SCoor.NbObj];

    // Bouton fermer
    SCoor.Objets[0].create_button(SCoor.width-64>>1,SCoor.height-18,SCoor.width+64>>1,SCoor.height-6,I18N::Translate( "Fermer" ).c_str(),NULL);

    WND STool;				// Fenêtre des outils de dessin
    STool.Title=I18N::Translate( "Outils" ).c_str();
    STool.width=124;				STool.height=150;
    STool.x=SCREEN_W-STool.width;	STool.y=0;
    STool.Lock=false;
    STool.NbObj=6;
    STool.Objets=new GUIOBJ[STool.NbObj];

    // Bouton point par point
    STool.Objets[0].create_button(10,16,114,28,I18N::Translate( "Points" ).c_str(),NULL);

    // Bouton lignes
    STool.Objets[1].create_button(10,32,114,44,I18N::Translate( "Lignes" ).c_str(),NULL);

    // Bouton remplissage
    STool.Objets[2].create_button(10,48,114,60,I18N::Translate( "Remplissage" ).c_str(),NULL);

    // Bouton crayon
    STool.Objets[3].create_button(10,64,114,76,I18N::Translate( "Crayon" ).c_str(),NULL);

    // Bouton texture
    STool.Objets[5].create_button(10,80,114,92,I18N::Translate( "Texture" ).c_str(),NULL);

    // Bouton annuler
    STool.Objets[4].create_button(10,96,114,108,I18N::Translate( "Annuler" ).c_str(),NULL);

    bool done=false;

    int amx = mouse_x, amy = mouse_y, amb = mouse_b, amz = mouse_z;

    Camera Cam;
    Cam.znear=0.01f;
    Cam.zfar=1400.0f;

    int IsOnGUI;		// Variable indiquant si le curseur est sur un élément de l'interface utilisateur

    float r1=0.0f,r2=0.0f;

    bool showcoorwindow=false;

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

    SHADER shader_paint_u,shader_paint_v;
    shader_paint_u.load("shaders/3dmpaint_u.frag","shaders/3dmpaint.vert");
    shader_paint_v.load("shaders/3dmpaint_v.frag","shaders/3dmpaint.vert");

    SCRIPT_DATA	cur_data;
    cur_data.load(nb_obj());

    GLuint brush_U,brush_V,zbuf;
    GLuint brush_FBO;

    if(g_useProgram && g_useFBO)
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

    do
    {
        if(amb!=1 && STool.Objets[4].Etat && NbH>0) // Clic sur annuler
        {
            glDeleteTextures(1,&tex);
            tex=CancelH[9];
            for(int i=9;i>10-NbH;i--)
                CancelH[i]=CancelH[i-1];
            CancelH[10-NbH]=0;
            NbH--;

            obj_table[cur_part]->surface.gltex[0]=tex;			// Recopie le BITMAP sur la texture
        }

        if(NbH>10) NbH=10;			// Limite la profondeur de l'historique

        if(STool.Objets[0].Etat)	Tool=TOOL_POINT;
        if(STool.Objets[1].Etat)	Tool=TOOL_LINE;
        if(STool.Objets[2].Etat)	Tool=TOOL_FILL;
        if(STool.Objets[3].Etat)	Tool=TOOL_PEN;
        if(STool.Objets[5].Etat)	Tool=TOOL_TEX;

        switch(Tool)			// Indique quel outil est sélectionné
        {
            case TOOL_POINT:
                STool.Objets[0].Focus=true;
                break;
            case TOOL_LINE:
                STool.Objets[1].Focus=true;
                break;
            case TOOL_FILL:
                STool.Objets[2].Focus=true;
                break;
            case TOOL_PEN:
                STool.Objets[3].Focus=true;
                break;
            case TOOL_TEX:
                STool.Objets[5].Focus=true;
                tool_tex_size+=(mouse_z-amz)*0.05f;
                if(tool_tex_size<0.01f)	tool_tex_size=0.01f;
                if(key[KEY_SPACE] || tool_tex_gl==0)
                {
                    String filename = Dialog( I18N::Translate( "Charger une texture" ).c_str(),"*.*");
                    BITMAP *bmp_tex = filename.length()>0 ? load_bitmap(filename.c_str(),NULL) : NULL;
                    if(bmp_tex) // Si il s'agit d'ajouter/modifier une texture
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
                        if(tool_tex_gl)
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

        if(SPaint.Objets[12].Etat) 								// Change the texture size
        {
            String new_res = GetVal( I18N::Translate( "Nouvelle résolution de la texture" ).c_str() );
            char *new_separator = strstr(new_res.c_str(),"x");
            if(new_separator)
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
                Popup( I18N::Translate( "Erreur" ), I18N::Translate( "La résolution doit être de la forme: largeurxhauteur" ));
        }
        if(SPaint.Objets[11].Etat) CubeTexturing(cur_part);
        if(SPaint.Objets[10].Etat) obj_geo_optimize(cur_part);
        if(SPaint.Objets[9].Etat) obj_maj_normal(cur_part);
        if(SPaint.Objets[8].Etat) obj_geo_split(cur_part);

        if(SPaint.Objets[4].Etat) RenderingMode=GL_LINE;			// Défini le mode de rendu
        if(SPaint.Objets[5].Etat) RenderingMode=GL_FILL;

        if(SPaint.Objets[6].Etat) EditMode=EDIT_SELTRI;				// Défini le mode d'édition
        if(SPaint.Objets[7].Etat) EditMode=EDIT_PAINT;

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

        if(SPaint.Objets[2].Etat)
        {
            SPaint.Objets[2].Etat=false;
            CylinderTexturing(cur_part);
            do
                poll_mouse();
            while(mouse_b!=0);
        }

        if(SCoor.Objets[0].Etat && showcoorwindow) // Cache la fenêtre SCoor
        {
            showcoorwindow=false;
            SCoor.Objets[0].Etat=false;
        }

        if(SPaint.Objets[3].Etat)						// Ouvre la fenêtre SCoor
            showcoorwindow=true;

        amx=mouse_x;
        amy=mouse_y;
        amz=mouse_z;
        amb=mouse_b;

        poll_mouse();		// obtient les coordonnées de la souris

        IsOnGUI=SPaint.check(amx,amy,amz,amb);		// gestion de l'interface utilisateur graphique

        if(IsOnGUI && mouse_b!=0)
            Focus=0;

        if(EditMode==EDIT_PAINT && !IsOnGUI)
        {
            IsOnGUI=STool.check(amx,amy,amz,amb);
            if(IsOnGUI && mouse_b!=0)
                Focus=2;
        }

        if(showcoorwindow && !IsOnGUI)
        {
            IsOnGUI=SCoor.check(amx,amy,amz,amb);
            if(IsOnGUI && mouse_b!=0)
                Focus=1;
        }

        // Efface tout
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la mémoire tampon

        show_mouse(NULL);					// Cache la souris
        if(key[KEY_ESC]) done=true;			// Quitte si on appuie sur echap

        if(!IsOnGUI && mouse_b==2) {			// Rotation de la caméra à la souris par clic droit
            r2-=(mouse_x-amx)*0.25f;
            r1-=(mouse_y-amy)*0.25f;
        }

        MATRIX_4x4 Rot;				// Oriente la caméra
        Rot = RotateX(r1*PI/180.0f) * RotateY(r2*PI/180.0f);

        Cam.setMatrix(Rot);

        if(!IsOnGUI && Tool!=TOOL_TEX)
            Cam.rpos = Cam.rpos - 0.5f * (mouse_z-amz) * Cam.dir; // Déplace la caméra si besoin

        Cam.setView();			// Positionne la caméra

        for(int i = 0; i < nb_obj(); ++i)
            cur_data.flag[i]= i==cur_part ? 0 : FLAG_HIDE;
        obj_table[cur_part]->draw(0.0f, &cur_data);		// Dessine la partie en cours d'édition de la meshe
        glDisable(GL_TEXTURE_2D);

        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);		// Restore les paramètres de remplissage
        glPolygonMode (GL_BACK, GL_POINTS);

        if(NbSel>0) // Affiche la sélection courante
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

        if(!IsOnGUI)					// Code de l'édition
            switch(EditMode)
            {
                case EDIT_SELTRI:			// Code pour la sélection
                    startline=true;		// Pour l'outil de traçage de lignes
                    {
                        VECTOR A,B,O;
                        VECTOR Dir;
                        Cam.setView();
                        O = Cam.pos-obj_table[cur_part]->pos_from_parent;			// Origine du rayon=le point de vue de la caméra
                        Dir=Cam.dir+(mouse_x-(SCREEN_W>>1))/(SCREEN_W*0.5f)*Cam.side-0.75f*(mouse_y-(SCREEN_H>>1))/(SCREEN_H*0.5f)*Cam.up;
                        int index=intersect(O,Dir,obj_table[cur_part],&A,&B); 		// Obtient l'indice du triangle visé

                        if(index>=0) // Si l'indice est valable
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

                            if(mouse_b == 1) // Modifie la sélection
                            {
                                if(key[KEY_LSHIFT] || key[KEY_RSHIFT]) // Ajoute le triangle à la sélection
                                {
                                    bool already=false;		// Vérifie si le triangle n'est pas déjà présent
                                    if(NbSel>0)
                                        for(int i=0;i<NbSel;i++)
                                            if(Sel[i]==index)
                                            {
                                                already=true;
                                                break;
                                            }
                                    if(!already)		// L'ajoute si il n'y est pas déjà
                                        Sel[NbSel++]=index;
                                }
                                else
                                {
                                    if(key[KEY_CAPSLOCK]) 	// Retire le triangle de la sélection
                                    {
                                        int pos=-1;				// Cherche la position du triangle
                                        if(NbSel>0)
                                            for(int i=0;i<NbSel;i++)
                                                if(Sel[i]==index)
                                                {
                                                    pos=i;
                                                    break;
                                                }
                                        if(pos!=-1) 	// Si le triangle est présent
                                        {
                                            if(pos+1<NbSel)		// Si ce n'est pas le dernier
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
                            }			// Fin de if(mouse_b==1) {
                        }		// Fin de if(index>=0) {
                        }
                        break;
                            case EDIT_PAINT:			// Code pour le dessin
                        if(mouse_b==1 && NbSel>0)	// Si il y a une sélection
                        {
                            VECTOR A,B,O;
                            VECTOR Dir;
                            Cam.setView();
                            O=Cam.pos-obj_table[cur_part]->pos_from_parent;			// Origine du rayon=le point de vue de la caméra
                            Dir=Cam.dir+(mouse_x-(SCREEN_W>>1))/(SCREEN_W*0.5f)*Cam.side-0.75f*(mouse_y-(SCREEN_H>>1))/(SCREEN_H*0.5f)*Cam.up;
                            int index=intersect(O,Dir,obj_table[cur_part],&A,&B); 		// Obtient l'indice du triangle visé
                            bool Selected=false;
                            if(index>=0)					// Vérifie si le triangle est sélectionné
                                for(int i=0;i<NbSel;i++)
                                    if(Sel[i]==index)
                                    {
                                        Selected=true;
                                        break;
                                    }
                            if (Selected) // Si le triangle est sélectionné
                            {
                                if (amb!=1)
                                {
                                    if(CancelH[0])
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

                                if(u>=0.0f && v>=0.0f && u<1.0f && v<1.0f)
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
                                            if(!startline)
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
                                                    if(Y>=0 && Y<SCREEN_H)
                                                        for(int x=0;x<brush->w;x++)
                                                        {
                                                            int X = (int)(mouse_x-32.0f*tool_tex_size+64.0f*tool_tex_size*x/brush->w);
                                                            if(X>=0 && X<SCREEN_W)
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
                                                                int col = makeacol32(cr1*ca1+(255-ca1)*cr2>>8,cg1*ca1+(255-ca1)*cg2>>8,cb1*ca1+(255-ca1)*cb2>>8,ca1*ca1+(255-ca1)*ca2>>8);
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

                        String help_msg = "";
                        SPaint.draw( help_msg, Focus==0);		// Dessine la fenêtre d'édition

                        if(EditMode==EDIT_PAINT)
                        {
                            STool.draw( help_msg, Focus==2);		// Dessine la boîte à outils

                            float r=getr32(PColor)/255.0f,g=getg32(PColor)/255.0f,b=getb32(PColor)/255.0f,a=geta32(PColor)/255.0f;
                            glBegin(GL_QUADS);			// Dessine les barres de sélection de couleur
                            glColor3f(0.0f,0.0f,0.0f);	glVertex2f(STool.x+10,STool.y+112);		// Barre Grise
                            glColor3f(1.0f,1.0f,1.0f);	glVertex2f(STool.x+80,STool.y+112);
                            glColor3f(1.0f,1.0f,1.0f);	glVertex2f(STool.x+80,STool.y+119);
                            glColor3f(0.0f,0.0f,0.0f);	glVertex2f(STool.x+10,STool.y+119);

                            glColor3f(0.0f,g,b);	glVertex2f(STool.x+10,STool.y+120);		// Barre Rouge
                            glColor3f(1.0f,g,b);	glVertex2f(STool.x+80,STool.y+120);
                            glColor3f(1.0f,g,b);	glVertex2f(STool.x+80,STool.y+127);
                            glColor3f(0.0f,g,b);	glVertex2f(STool.x+10,STool.y+127);

                            glColor3f(r,0.0f,b);	glVertex2f(STool.x+10,STool.y+128);		// Barre Vert
                            glColor3f(r,1.0f,b);	glVertex2f(STool.x+80,STool.y+128);
                            glColor3f(r,1.0f,b);	glVertex2f(STool.x+80,STool.y+135);
                            glColor3f(r,0.0f,b);	glVertex2f(STool.x+10,STool.y+135);

                            glColor3f(r,g,0.0f);	glVertex2f(STool.x+10,STool.y+136);		// Barre Bleu
                            glColor3f(r,g,1.0f);	glVertex2f(STool.x+80,STool.y+136);
                            glColor3f(r,g,1.0f);	glVertex2f(STool.x+80,STool.y+143);
                            glColor3f(r,g,0.0f);	glVertex2f(STool.x+10,STool.y+143);

                            if(IsOnGUI)			// Sélection de la couleur ?
                                if(mouse_x>=STool.x+10 && mouse_x<=STool.x+80 && mouse_y>=STool.y+112 && mouse_y<=STool.y+143)
                                {
                                    if(mouse_y>=STool.y+112 && mouse_y<=STool.y+119)
                                        a=(mouse_x-STool.x-10)/70.0f;
                                    if(mouse_y>=STool.y+120 && mouse_y<=STool.y+127)
                                        r=(mouse_x-STool.x-10)/70.0f;
                                    if(mouse_y>=STool.y+128 && mouse_y<=STool.y+135)
                                        g=(mouse_x-STool.x-10)/70.0f;
                                    if(mouse_y>=STool.y+136 && mouse_y<=STool.y+143)
                                        b=(mouse_x-STool.x-10)/70.0f;
                                    if(mouse_b==1)
                                        PColor=makeacol32((int)(r*255.0f),(int)(g*255.0f),(int)(b*255.0f),(int)(a*255.0f));
                                }

                            glColor3f(r,g,b);
                            glVertex2f(STool.x+90.0f,STool.y+120.0f);
                            glVertex2f(STool.x+114.0f,STool.y+120.0f);
                            glVertex2f(STool.x+114.0f,STool.y+143.0f);
                            glVertex2f(STool.x+90.0f,STool.y+143.0f);
                            glColor3f(a,a,a);
                            glVertex2f(STool.x+90.0f,STool.y+112.0f);
                            glVertex2f(STool.x+114.0f,STool.y+112.0f);
                            glVertex2f(STool.x+114.0f,STool.y+119.0f);
                            glVertex2f(STool.x+90.0f,STool.y+119.0f);
                            glEnd();

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
                                    if(!IsOnGUI)
                                        gfx->drawtexture(tool_tex_gl,mouse_x-tool_tex_size*32.0f,mouse_y-tool_tex_size*32.0f,mouse_x+tool_tex_size*32.0f,mouse_y+tool_tex_size*32.0f);
                                    glDisable(GL_TEXTURE_2D);
                                    glDisable(GL_BLEND);
                                    break;
                            }
                        }		// Fin de if(EditMode==EDIT_PAINT) {

                        if(showcoorwindow)	// Dessine la fenêtre de plaquage de texture
                        {
                            String help_msg = "";
                            SCoor.draw( help_msg, screen, Focus==1 );
                            gfx->unset_2D_mode();
                            gfx->set_2D_mode();
                            glEnable(GL_TEXTURE_2D);
                            glBindTexture(GL_TEXTURE_2D,tex);
                            glBegin(GL_QUADS);
                            glTexCoord2f(0.0f,0.0f);	glVertex2f(SCoor.x+5,SCoor.y+20);
                            glTexCoord2f(1.0f,0.0f);	glVertex2f(SCoor.x+SCoor.width-5,SCoor.y+20);
                            glTexCoord2f(1.0f,1.0f);	glVertex2f(SCoor.x+SCoor.width-5,SCoor.y+SCoor.height-20);
                            glTexCoord2f(0.0f,1.0f);	glVertex2f(SCoor.x+5,SCoor.y+SCoor.height-20);
                            glEnd();
                            glDisable(GL_TEXTURE_2D);

                            glColor4f(1.0f,1.0f,1.0f,1.0f);

                            glPushMatrix();
                            glTranslatef(SCoor.x+5,SCoor.y+20,0.0f);
                            glScalef(300.0f,300.0f,0.0f);

                            glEnableClientState(GL_VERTEX_ARRAY);					// Les coordonnées des points
                            glVertexPointer( 2, GL_FLOAT, 0, obj_table[cur_part]->tcoord);

                            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

                            glDrawElements(GL_TRIANGLES, obj_table[cur_part]->nb_t_index,GL_UNSIGNED_SHORT,obj_table[cur_part]->t_index);		// dessine les triangles sur la texture


                            glColor3f(0.0f,0.0f,1.0f);
                            glPointSize(3.0f);

                            glDrawElements(GL_POINTS, obj_table[cur_part]->nb_t_index,GL_UNSIGNED_SHORT,obj_table[cur_part]->t_index);		// dessine les triangles sur la texture

                            glPointSize(1.0f);

                            glPopMatrix();
                            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
                            glPolygonMode (GL_BACK, GL_POINTS);
                        }

                        glEnable(GL_TEXTURE_2D);			// Affiche le curseur
                        show_mouse(screen);
                        algl_draw_mouse();

                        if(SPaint.Objets[0].Etat)
                            done=true;		// En cas de click sur "Annuler", on quitte la fenêtre

                        gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

                        // Affiche
                        gfx->flip();
                    } while(!done);

                        if(g_useProgram && g_useFBO)
                        {
                            glDeleteFramebuffersEXT(1,&brush_FBO);
                            glDeleteRenderbuffersEXT(1,&zbuf);
                            glDeleteTextures(1,&brush_U);
                            glDeleteTextures(1,&brush_V);
                        }

                        shader_paint_u.destroy();
                        shader_paint_v.destroy();

                        if(tool_tex_gl!=0)
                            glDeleteTextures(1,&tool_tex_gl);

                        cur_data.destroy();

                        for(int i=0;i<10;i++) // Libère la mémoire allouée pour les bitmaps de l'historique
                        {
                            if(CancelH[i])
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
                        if(part<0 || part>=nb_obj())	return;		// Quitte si l'indice n'est pas valable

                        float COS45=0.5f*sqrt(2.0f);		// Valeur à partir de laquelle on utilise les bords droit et gauche de la texture
                        VECTOR I,J,K;
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
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse tous les points pour connaître les bornes de l'objet
                            float COS=obj_table[part]->N[i]%I;
                            if(fabs(COS)<COS45) {			// Milieu
                                COS=obj_table[part]->N[i]%J;
                                if(fabs(COS)<COS45) {
                                    COS=obj_table[part]->N[i]%K;
                                    if(COS>0.0f) {			// Coté E(bas droit)
                                        if(obj_table[part]->points[i].x>CExmax)	CExmax=obj_table[part]->points[i].x;
                                        if(obj_table[part]->points[i].x<CExmin)	CExmin=obj_table[part]->points[i].x;
                                        if(obj_table[part]->points[i].y>CEymax)	CEymax=obj_table[part]->points[i].y;
                                        if(obj_table[part]->points[i].y<CEymin)	CEymin=obj_table[part]->points[i].y;
                                    }
                                    else {					// Coté F(bas gauche)
                                        if(obj_table[part]->points[i].x>CFxmax)	CFxmax=obj_table[part]->points[i].x;
                                        if(obj_table[part]->points[i].x<CFxmin)	CFxmin=obj_table[part]->points[i].x;
                                        if(obj_table[part]->points[i].y>CFymax)	CFymax=obj_table[part]->points[i].y;
                                        if(obj_table[part]->points[i].y<CFymin)	CFymin=obj_table[part]->points[i].y;
                                    }
                                }
                                else if(COS>0.0f) {			// Coté C(milieu droit)
                                    if(obj_table[part]->points[i].x>CCxmax)	CCxmax=obj_table[part]->points[i].x;
                                    if(obj_table[part]->points[i].x<CCxmin)	CCxmin=obj_table[part]->points[i].x;
                                    if(obj_table[part]->points[i].z>CCzmax)	CCzmax=obj_table[part]->points[i].z;
                                    if(obj_table[part]->points[i].z<CCzmin)	CCzmin=obj_table[part]->points[i].z;
                                }
                                else {						// Coté D(milieu gauche)
                                    if(obj_table[part]->points[i].x>CDxmax)	CDxmax=obj_table[part]->points[i].x;
                                    if(obj_table[part]->points[i].x<CDxmin)	CDxmin=obj_table[part]->points[i].x;
                                    if(obj_table[part]->points[i].z>CDzmax)	CDzmax=obj_table[part]->points[i].z;
                                    if(obj_table[part]->points[i].z<CDzmin)	CDzmin=obj_table[part]->points[i].z;
                                }
                            }
                            else if(COS>0.0f) {				// Coté A(haut droit)
                                if(obj_table[part]->points[i].y>CAymax)	CAymax=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].y<CAymin)	CAymin=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].z>CAzmax)	CAzmax=obj_table[part]->points[i].z;
                                if(obj_table[part]->points[i].z<CAzmin)	CAzmin=obj_table[part]->points[i].z;
                            }
                            else {							// Coté B(haut gauche)
                                if(obj_table[part]->points[i].y>CBymax)	CBymax=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].y<CBymin)	CBymin=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].z>CBzmax)	CBzmax=obj_table[part]->points[i].z;
                                if(obj_table[part]->points[i].z<CBzmin)	CBzmin=obj_table[part]->points[i].z;
                            }
                        }

                        if(CAymin==CAymax) CAymin--;
                        if(CAzmin==CAzmax) CAzmin--;
                        if(CBymin==CBymax) CBymin--;
                        if(CBzmin==CBzmax) CBzmin--;

                        if(CCxmin==CCxmax) CCxmin--;
                        if(CCzmin==CCzmax) CCzmin--;
                        if(CDxmin==CDxmax) CDxmin--;
                        if(CDzmin==CDzmax) CDzmin--;

                        if(CExmin==CExmax) CExmin--;
                        if(CEymin==CEymax) CEymin--;
                        if(CFxmin==CFxmax) CFxmin--;
                        if(CFymin==CFymax) CFymin--;

                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            float COS=obj_table[part]->N[i]%I;
                            if(fabs(COS)<COS45) {			// Milieu
                                COS=obj_table[part]->N[i]%J;
                                if(fabs(COS)<COS45) {
                                    COS=obj_table[part]->N[i]%K;
                                    if(COS>0.0f) {			// Coté E(bas droit)
                                        obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].x-CExmin)/(CExmax-CExmin);
                                        obj_table[part]->tcoord[(i<<1)+1]=0.667f+0.333f*(obj_table[part]->points[i].y-CEymin)/(CEymax-CEymin);
                                    }
                                    else {					// Coté F(bas gauche)
                                        obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].x-CFxmin)/(CFxmax-CFxmin);
                                        obj_table[part]->tcoord[(i<<1)+1]=0.667f+0.333f*(obj_table[part]->points[i].y-CFymin)/(CFymax-CFymin);
                                    }
                                }
                                else if(COS>0.0f) {			// Coté C(milieu droit)
                                    obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].x-CCxmin)/(CCxmax-CCxmin);
                                    obj_table[part]->tcoord[(i<<1)+1]=0.333f+0.333f*(obj_table[part]->points[i].z-CCzmin)/(CCzmax-CCzmin);
                                }
                                else {						// Coté D(milieu gauche)
                                    obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].x-CDxmin)/(CDxmax-CDxmin);
                                    obj_table[part]->tcoord[(i<<1)+1]=0.333f+0.333f*(obj_table[part]->points[i].z-CDzmin)/(CDzmax-CDzmin);
                                }
                            }
                            else if(COS>0.0f) {				// Coté A(haut droit)
                                obj_table[part]->tcoord[i<<1]=0.5f+0.5f*(obj_table[part]->points[i].y-CAymin)/(CAymax-CAymin);
                                obj_table[part]->tcoord[(i<<1)+1]=0.333f*(obj_table[part]->points[i].z-CAzmin)/(CAzmax-CAzmin);
                            }
                            else {							// Coté B(haut gauche)
                                obj_table[part]->tcoord[i<<1]=0.5f*(obj_table[part]->points[i].y-CBymin)/(CBymax-CBymin);
                                obj_table[part]->tcoord[(i<<1)+1]=0.333f*(obj_table[part]->points[i].z-CBzmin)/(CBzmax-CBzmin);
                            }
                        }
                        float txmin=1.0f;
                        float txmax=0.0f;
                        float tymin=1.0f;
                        float tymax=0.0f;
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            if(obj_table[part]->tcoord[i<<1]<txmin)	txmin=obj_table[part]->tcoord[i<<1];
                            if(obj_table[part]->tcoord[i<<1]>txmax)	txmax=obj_table[part]->tcoord[i<<1];
                            if(obj_table[part]->tcoord[(i<<1)+1]<tymin)	tymin=obj_table[part]->tcoord[(i<<1)+1];
                            if(obj_table[part]->tcoord[(i<<1)+1]>tymax)	tymax=obj_table[part]->tcoord[(i<<1)+1];
                        }
                        if(txmin==txmax)	txmin--;
                        if(tymin==tymax)	tymin--;
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            obj_table[part]->tcoord[i<<1]=(obj_table[part]->tcoord[i<<1]-txmin)/(txmax-txmin);
                            obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->tcoord[(i<<1)+1]-tymin)/(tymax-tymin);
                        }
                    }

                    /*---------------------------------------------------------------------------------------------------\
                      |                                   Plaquage cylindrique de la texture                               |
                      \---------------------------------------------------------------------------------------------------*/

                    void CylinderTexturing(int part)
                    {
                        if(part<0 || part>=nb_obj())	return;		// Quitte si l'indice n'est pas valable

                        float COS45=0.5f*sqrt(2.0f);		// Valeur à partir de laquelle on utilise les bords droit et gauche de la texture
                        VECTOR I,J,K;
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
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse tous les points pour connaître les bornes de l'objet
                            float COS=obj_table[part]->N[i]%I;
                            if(fabs(COS)<COS45) {			// Milieu
                                if(obj_table[part]->points[i].x>xmax)	xmax=obj_table[part]->points[i].x;
                                if(obj_table[part]->points[i].x<xmin)	xmin=obj_table[part]->points[i].x;
                                VECTOR V;
                                V=obj_table[part]->points[i];
                                V=V-(V%I)*I;
                                float angle=VAngle(J,V);
                                if(K%V<0.0f)
                                    angle=2.0f*PI-angle;
                                obj_table[part]->tcoord[(i<<1)+1]=angle*0.5f/PI;
                                if(obj_table[part]->tcoord[(i<<1)+1]>ymax)	ymax=obj_table[part]->tcoord[(i<<1)+1];
                                if(obj_table[part]->tcoord[(i<<1)+1]<ymin)	ymin=obj_table[part]->tcoord[(i<<1)+1];
                            }
                            else if(COS>0.0f) {				// Coté A(droit)
                                if(obj_table[part]->points[i].y>CAymax)	CAymax=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].y<CAymin)	CAymin=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].z>CAzmax)	CAzmax=obj_table[part]->points[i].z;
                                if(obj_table[part]->points[i].z<CAzmin)	CAzmin=obj_table[part]->points[i].z;
                            }
                            else {							// Coté B(gauche)
                                if(obj_table[part]->points[i].y>CBymax)	CBymax=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].y<CBymin)	CBymin=obj_table[part]->points[i].y;
                                if(obj_table[part]->points[i].z>CBzmax)	CBzmax=obj_table[part]->points[i].z;
                                if(obj_table[part]->points[i].z<CBzmin)	CBzmin=obj_table[part]->points[i].z;
                            }
                        }

                        if(xmin==xmax) xmin--;
                        if(CAymin==CAymax) CAymin--;
                        if(CAzmin==CAzmax) CAzmin--;
                        if(CBymin==CBymax) CBymin--;
                        if(CBzmin==CBzmax) CBzmin--;
                        if(ymin==ymax) ymin--;

                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            float COS=obj_table[part]->N[i]%I;
                            if(fabs(COS)<COS45) {			// Milieu
                                VECTOR V = obj_table[part]->points[i];
                                V=V-(V%I)*I;
                                float angle=VAngle(J,V);
                                if(K%V<0.0f)
                                    angle=2.0f*PI-angle;
                                obj_table[part]->tcoord[i<<1]=0.25f+0.5f*(obj_table[part]->points[i].x-xmin)/(xmax-xmin);
                                obj_table[part]->tcoord[(i<<1)+1]=(angle*0.5f/PI-ymin)/(ymax-ymin);
                            }
                            else if(COS>0.0f) {				// Coté A(droit)
                                obj_table[part]->tcoord[i<<1]=0.75f+0.25f*(obj_table[part]->points[i].y-CAymin)/(CAymax-CAymin);
                                obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->points[i].z-CAzmin)/(CAzmax-CAzmin);
                            }
                            else {							// Coté B(gauche)
                                obj_table[part]->tcoord[i<<1]=0.25f*(obj_table[part]->points[i].y-CBymin)/(CBymax-CBymin);
                                obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->points[i].z-CBzmin)/(CBzmax-CBzmin);
                            }
                        }

                        float txmin=1.0f;
                        float txmax=0.0f;
                        float tymin=1.0f;
                        float tymax=0.0f;
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            if(obj_table[part]->tcoord[i<<1]<txmin)	txmin=obj_table[part]->tcoord[i<<1];
                            if(obj_table[part]->tcoord[i<<1]>txmax)	txmax=obj_table[part]->tcoord[i<<1];
                            if(obj_table[part]->tcoord[(i<<1)+1]<tymin)	tymin=obj_table[part]->tcoord[(i<<1)+1];
                            if(obj_table[part]->tcoord[(i<<1)+1]>tymax)	tymax=obj_table[part]->tcoord[(i<<1)+1];
                        }
                        if(txmin==txmax)	txmin--;
                        if(tymin==tymax)	tymin--;
                        for(i=0;i<obj_table[part]->nb_vtx;i++) {		// Analyse toutes les normales et les points
                            obj_table[part]->tcoord[i<<1]=(obj_table[part]->tcoord[i<<1]-txmin)/(txmax-txmin);
                            obj_table[part]->tcoord[(i<<1)+1]=(obj_table[part]->tcoord[(i<<1)+1]-tymin)/(tymax-tymin);
                        }
                    }
