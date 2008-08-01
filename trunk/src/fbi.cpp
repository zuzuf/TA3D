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
  |                                         fbi.cpp                                    |
  |  Ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers fbi du jeu totalannihilation qui sont les fichiers de données sur les |
  | unités du jeu. Cela inclus les classes pour gérer les différents types d'unités et |
  | le système de gestion de liens entre unités.                                       |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include <vector>
#include <list>
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"
#include "EngineClass.h"
#include "UnitEngine.h"
#include "languages/i18n.h"
#include "misc/math.h"
#include "logs/logs.h"
#include "converters/pcx.h"
#include "ingame/players.h"

#define SWAP(a, b) { sint32 tmp = a; a = b; b = tmp; }



namespace TA3D
{



    UNIT_MANAGER unit_manager;

    DL_DATA::~DL_DATA()
    {
        if (dl_x) delete[] dl_x;
        if (dl_y) delete[] dl_y;
        if (dl_w) delete[] dl_w;
        if (dl_h) delete[] dl_h;
    }



    void UNIT_TYPE::AddUnitBuild(int index, int px, int py, int pw, int ph, int p, GLuint Pic )
    {
        if (index < -1)
            return;

        if (BuildList && nb_unit > 0)		// Vérifie si l'unité n'est pas déjà répertoriée / check if not already there
        {
            for (int i = 0; i < nb_unit; ++i)
            {
                if (BuildList[i] == index && Pic_p[i] < 0) // Update the data we have
                {
                    if( Pic != 0 )
                    {
                        gfx->destroy_texture(PicList[i]);
                        PicList[i] = Pic;
                    }
                    Pic_x[ i ] = px;
                    Pic_y[ i ] = py;
                    Pic_w[ i ] = pw;
                    Pic_h[ i ] = ph;
                    Pic_p[ i ] = p;
                    return;
                }
            }
        }

        ++nb_unit;
        if (BuildList == NULL)
            nb_unit = 1;
        short *Blist=(short*) malloc(sizeof(short)*nb_unit);
        short *Px=(short*) malloc(sizeof(short)*nb_unit);
        short *Py=(short*) malloc(sizeof(short)*nb_unit);
        short *Pw=(short*) malloc(sizeof(short)*nb_unit);
        short *Ph=(short*) malloc(sizeof(short)*nb_unit);
        short *Pp=(short*) malloc(sizeof(short)*nb_unit);
        GLuint *Plist=(GLuint*) malloc(sizeof(GLuint)*nb_unit);
        if (BuildList && nb_unit > 1)
        {
            for (int i = 0; i < nb_unit - 1; ++i)
            {
                Blist[i] = BuildList[i];
                Plist[i] = PicList[i];
                Px[i] = Pic_x[i];
                Py[i] = Pic_y[i];
                Pw[i] = Pic_w[i];
                Ph[i] = Pic_h[i];
                Pp[i] = Pic_p[i];
            }
        }
        Blist[nb_unit-1]=index;
        Plist[nb_unit-1]=Pic;
        Px[nb_unit-1]=px;
        Py[nb_unit-1]=py;
        Pw[nb_unit-1]=pw;
        Ph[nb_unit-1]=ph;
        Pp[nb_unit-1]=p;
        if(BuildList)	free(BuildList);
        if(PicList)		free(PicList);
        if(Pic_x)		free(Pic_x);
        if(Pic_y)		free(Pic_y);
        if(Pic_p)		free(Pic_p);
        BuildList=Blist;
        PicList=Plist;
        Pic_x = Px;
        Pic_y = Py;
        Pic_w = Pw;
        Pic_h = Ph;
        Pic_p = Pp;
    }


    void UNIT_MANAGER::analyse(String filename,int unit_index)
    {
        cTAFileParser gui_parser( filename, false, false, true );

        String number = filename.substr(0, filename.size() - 4);
        int first = number.size() - 1;
        while (first >= 0 && number[first] >= '0' && number[first] <= '9')
            --first;
        ++first;
        number = number.substr( first, number.size() - first );

        int page = atoi( number.c_str() ) - 1;		// Extract the page number

        int NbObj = gui_parser.pullAsInt( "gadget0.totalgadgets" );

        int x_offset = gui_parser.pullAsInt( "gadget0.common.xpos" );
        int y_offset = gui_parser.pullAsInt( "gadget0.common.ypos" );

        for (int i = 1; i <= NbObj; ++i)
        {
            int attribs = gui_parser.pullAsInt( format( "gadget%d.common.commonattribs", i ) );
            if (attribs & 4) // Unit Build Pic
            {
                int x = gui_parser.pullAsInt( format( "gadget%d.common.xpos", i ) ) + x_offset;
                int y = gui_parser.pullAsInt( format( "gadget%d.common.ypos", i ) ) + y_offset;
                int w = gui_parser.pullAsInt( format( "gadget%d.common.width", i ) );
                int h = gui_parser.pullAsInt( format( "gadget%d.common.height", i ) );
                String name = gui_parser.pullAsString( format( "gadget%d.common.name", i ) );
                int idx = get_unit_index( name.c_str() );

                if( idx >= 0 )
                {
                    String name = gui_parser.pullAsString( format( "gadget%d.common.name", i ) );

                    byte *gaf_file = HPIManager->PullFromHPI( format( "anims\\%s%d.gaf", unit_type[unit_index].Unitname, page + 1 ).c_str() );
                    if( gaf_file ) {
                        BITMAP *img = read_gaf_img( gaf_file, get_gaf_entry_index( gaf_file, (char*)name.c_str() ), 0 );

                        GLuint tex = 0;
                        if( img )
                        {
                            w = img->w;
                            h = img->h;
                            tex = gfx->make_texture( img );
                            destroy_bitmap( img );
                        }

                        delete[] gaf_file;

                        unit_type[unit_index].AddUnitBuild(idx, x, y, w, h, page, tex);
                    }
                    else
                        unit_type[unit_index].AddUnitBuild(idx, x, y, w, h, page);
                }
            }
            else
                if( attribs & 8 ) 	// Weapon Build Pic
                {
                    int x = gui_parser.pullAsInt( format( "gadget%d.common.xpos", i ) ) + x_offset;
                    int y = gui_parser.pullAsInt( format( "gadget%d.common.ypos", i ) ) + y_offset;
                    int w = gui_parser.pullAsInt( format( "gadget%d.common.width", i ) );
                    int h = gui_parser.pullAsInt( format( "gadget%d.common.height", i ) );
                    String name = gui_parser.pullAsString( format( "gadget%d.common.name", i));

                    byte* gaf_file = HPIManager->PullFromHPI( format( "anims\\%s%d.gaf", unit_type[unit_index].Unitname, page + 1 ).c_str() );
                    if (gaf_file)
                    {
                        BITMAP *img = read_gaf_img( gaf_file, get_gaf_entry_index( gaf_file, (char*)name.c_str() ), 0 );

                        GLuint tex = 0;
                        if( img )
                        {
                            w = img->w;
                            h = img->h;
                            tex = gfx->make_texture( img );
                            destroy_bitmap( img );
                        }

                        delete[] gaf_file;
                        unit_type[unit_index].AddUnitBuild(-1, x, y, w, h, page, tex);
                    }
                    else
                        unit_type[unit_index].AddUnitBuild(-1, x, y, w, h, page);
                }
        }
    }

    void UNIT_MANAGER::analyse2(char *data,int size)
    {
        char *pos=data;
        char *ligne=NULL;
        char *limit=data+size;
        int nb=0;
        do
        {
            char *unitmenu=NULL;
            char *unitname=NULL;

            do
            {
                ++nb;
                if (ligne)
                    delete[] ligne;
                ligne=get_line(pos);
                strlwr(ligne);
                while (pos[0]!=0 && pos[0]!=13 && pos[0]!=10)
                    ++pos;
                while (pos[0]==13 || pos[0]==10)
                    ++pos;

                if (strstr(ligne,"unitmenu="))          // Obtient le nom de l'unité dont le menu doit être completé
                {
                    unitmenu=strstr(ligne,"unitmenu=")+9;
                    if (strstr(unitmenu,";"))
                        *(strstr(unitmenu,";"))=0;
                    strupr(unitmenu);
                    unitmenu=strdup(unitmenu);
                }
                if (strstr(ligne,"unitname="))          // Obtient le nom de l'unité à ajouter
                {
                    unitname=strstr(ligne,"unitname=")+9;
                    if(strstr(unitname,";"))
                        *(strstr(unitname,";"))=0;
                    strupr(unitname);
                    unitname=strdup(unitname);
                }

            } while (strstr(ligne,"}")==NULL && nb<2000 && data<limit);
            delete[] ligne;
            ligne=NULL;
            if (unitmenu==NULL || unitname==NULL) break;
            int unit_index=get_unit_index(unitmenu);
            if (unit_index==-1) continue;		// Au cas où l'unité n'existerait pas
            int idx=get_unit_index(unitname);
            if (idx>=0 && idx<nb_unit && unit_type[idx].unitpic)
                unit_type[unit_index].AddUnitBuild(idx, -1, -1, 64, 64, -1);
        } while (pos[0]=='[' && nb<2000 && data<limit);
    }



    int UNIT_MANAGER::load_unit(byte *data,int size)			// Ajoute une nouvelle unité
    {
        UNIT_TYPE* n_type = (UNIT_TYPE*) malloc(sizeof(UNIT_TYPE)*(nb_unit+1));
        int i;
        if (unit_type != NULL)
        {
            for(i=0;i<nb_unit;i++)
                n_type[i]=unit_type[i];
            free(unit_type);
        }
        unit_type=n_type;
        unit_type[nb_unit].init();
        int result =  unit_type[nb_unit].load((char*)data,size);
        if (unit_type[ nb_unit ].Unitname)
            unit_hashtable.insert(String::ToLower(unit_type[nb_unit].Unitname ), nb_unit + 1 );
        if (unit_type[ nb_unit ].name)
            unit_hashtable.insert(String::ToLower(unit_type[nb_unit].name ), nb_unit + 1 );
        if (unit_type[ nb_unit ].ObjectName)
            unit_hashtable.insert(String::ToLower(unit_type[nb_unit].ObjectName ), nb_unit + 1 );
        if (unit_type[ nb_unit ].Description)
            unit_hashtable.insert(String::ToLower(unit_type[nb_unit].Description ), nb_unit + 1 );
        if (unit_type[ nb_unit ].Designation_Name)
            unit_hashtable.insert(String::ToLower(unit_type[nb_unit].Designation_Name ), nb_unit + 1 );
        nb_unit++;
        return result;
    }



    void UNIT_MANAGER::gather_build_data()
    {
        uint32 file_size=0;
        String::List file_list;
        HPIManager->getFilelist( ta3dSideData.download_dir + "*.tdf", file_list);

        for (String::List::const_iterator file = file_list.begin(); file != file_list.end(); ++file) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            byte* data = HPIManager->PullFromHPI(*file, &file_size);		// Lit le fichier
            if (data)
            {
                analyse2((char*)data,file_size);
                delete[] data;
            }
        }
    }


    void UNIT_MANAGER::gather_all_build_data()
    {
        cTAFileParser sidedata_parser( ta3dSideData.gamedata_dir + "sidedata.tdf", false, true );
        for (int i = 0 ; i < nb_unit ; ++i)
        {
            int n = 1;
            String canbuild = sidedata_parser.pullAsString(String::ToLower(format( "canbuild.%s.canbuild%d", unit_type[ i ].Unitname, n ) ) );
            while (!canbuild.empty())
            {
                int idx = get_unit_index( (char*)canbuild.c_str() );
                if (idx >= 0 && idx < nb_unit && unit_type[idx].unitpic)
                    unit_type[i].AddUnitBuild(idx, -1, -1, 64, 64, -1);
                ++n;
                canbuild = sidedata_parser.pullAsString( format( "canbuild.%s.canbuild%d", unit_type[ i ].Unitname, n ) );
            }
        }

        gather_build_data();			// Read additionnal build data

        String::List file_list;
        HPIManager->getFilelist( ta3dSideData.guis_dir + "*.gui", file_list);

        for (String::List::iterator file = file_list.begin(); file != file_list.end(); ++file) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            char *f=NULL;
            for (int i = 0; i < nb_unit; ++i)
            {
                if ((f = strstr((char*)String::ToUpper(*file).c_str(), unit_type[i].Unitname)))
                {
                    if(f[strlen(unit_type[i].Unitname)]=='.'
                       ||(f[strlen(unit_type[i].Unitname)]>='0' && f[strlen(unit_type[i].Unitname)]<='9'))
                        analyse(*file,i);
                }
            }
        }

        for (int i = 0 ; i < nb_unit ; ++i)
            unit_type[i].FixBuild();
    }


    void UNIT_MANAGER::load_script_file(char *unit_name)
    {
        strupr(unit_name);
        int unit_index=get_unit_index(unit_name);
        if (unit_index == -1) 
            return;

        char *uprname = strdup(unit_name);
        strupr(uprname);

        String::List file_list;
        HPIManager->getFilelist( format( "scripts\\%s.cob", unit_name ), file_list);

        for (String::List::iterator file = file_list.begin();file != file_list.end(); ++file) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            if (strstr(String::ToUpper(*file).c_str(),uprname)) 	// A trouvé un fichier qui convient
            {
                byte* data=HPIManager->PullFromHPI(*file);		// Lit le fichier
                unit_type[unit_index].script = new SCRIPT;
                unit_type[unit_index].script->load_cob(data);
                // Don't delete[] data here because the script keeps a reference to it.
                break;
            }
        }
        free(uprname);
    }


    void UNIT_MANAGER::Identify()			// Identifie les pièces aux quelles les scripts font référence
    {
        for (int i = 0; i < nb_unit; ++i)
        {
            if (unit_type[i].script && unit_type[i].model)
                unit_type[i].model->Identify(unit_type[i].script->nb_piece,unit_type[i].script->piece_name);
        }
    }

    char *UNIT_TYPE::get_line(char *data)
    {
        int pos=0;
        while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)	pos++;
        char *d=new char[pos+1];
        memcpy(d,data,pos);
        d[pos]=0;
        return d;
    }

    void UNIT_TYPE::destroy()
    {
        if(MovementClass)
            free(MovementClass);
        if(soundcategory)
            free(soundcategory);
        if(ExplodeAs)
            free(ExplodeAs);
        if(SelfDestructAs)
            free(SelfDestructAs);
        if(script)
            delete script;

        for (short int i = 0; i < 3; ++i)
        {
            if (w_badTargetCategory[i])
                free(w_badTargetCategory[i]);
        }
        if( BadTargetCategory)
            free(BadTargetCategory);
        if( NoChaseCategory)
            free(NoChaseCategory);

        if(BuildList)
            free(BuildList);
        if(Pic_x)
            free(Pic_x);
        if(Pic_y)
            free(Pic_y);
        if(Pic_w)
            free(Pic_w);
        if(Pic_h)
            free(Pic_h);
        if(Pic_p)
            free(Pic_p);

        if (PicList)
        {
            for (int i = 0; i < nb_unit; ++i)
                gfx->destroy_texture(PicList[i]);
            free(PicList);
        }

        if(yardmap)
            free(yardmap);
        if(model)
            model = NULL;
        if(unitpic)
        {
            destroy_bitmap(unitpic);
            glDeleteTextures(1,&glpic);
        }
        if(Corpse)
            free(Corpse);
        if(Unitname)
            free(Unitname);
        if(name)
            free(name);
        if(side)
            free(side);
        if(ObjectName)
            free(ObjectName);
        if(Designation_Name)
            free(Designation_Name);
        if(Description)
            free(Description);
        DELETEANDNIL(Category);
        DELETEANDNIL(categories);

        init();
    }

    void UNIT_TYPE::init()
    {
        not_used = false;
        commander = false;
        selfdestructcountdown = 5;

        last_click = -1;
        click_time = 0.0f;

        emitting_points_computed = false;
        soundcategory = strdup("");

        isfeature=false;
        antiweapons=false;

        weapon[0]=weapon[1]=weapon[2]=NULL;		// Pas d'armes

        script=NULL;		// Aucun script

        page=0;

        nb_pages = 0;
        nb_unit=0;
        BuildList=NULL;
        PicList=NULL;
        Pic_x=NULL;				// Coordinates
        Pic_y=NULL;
        Pic_w=NULL;				// Coordinates
        Pic_h=NULL;
        Pic_p=NULL;				// Page where the pic has to be shown

        dl_data = NULL;

        init_cloaked = false;
        mincloakdistance = 10;
        DefaultMissionType=MISSION_STANDBY;
        attackrunlength=0;
        yardmap=NULL;
        model=NULL;
        unitpic=NULL;
        hoverattack=false;
        SortBias=0;
        IsAirBase=false;
        AltFromSeaLevel=0;
        TransportSize=0;
        TransportCapacity=0;
        ManeuverLeashLength=640;
        CruiseAlt=0;
        TEDclass=CLASS_UNDEF;
        WaterLine=0.0f;
        StandingMoveOrder=1;
        MobileStandOrders=1;
        StandingFireOrder=1;
        FireStandOrders=1;
        ShootMe=false;
        ThreeD=true;
        Builder=false;
        Unitname=NULL;
        name=NULL;
        version=0;
        side=NULL;
        ObjectName=NULL;
        FootprintX=0;
        FootprintZ=0;
        Category = NULL;
        categories = NULL;
        fastCategory=0;
        MaxSlope=255;
        BMcode=0;
        norestrict=false;
        BuildAngle=10;
        canresurrect=false;
        Designation_Name=NULL;	// Nom visible de l'unité
        Description=NULL;		// Description
        BuildCostEnergy=0;		// Energie nécessaire pour la construire
        BuildCostMetal=0;		// Metal nécessaire pour la construire
        MaxDamage=10;			// Points de dégats maximum que l'unité peut encaisser
        EnergyUse=0;			// Energie nécessaire pour faire quelque chose
        BuildTime=0;			// Temps de construction
        WorkerTime=1;			// Vitesse de construction
        AutoFire=false;			// Tire automatique
        SightDistance=50;		// Distance maximale de vue de l'unité
        RadarDistance=0;		// Distance maximale de detection radar
        RadarDistanceJam=0;		// For Radar jammers
        EnergyStorage=0;		// Quantité d'énergie stockable par l'unité
        MetalStorage=0;			// Quantité de metal stockable par l'unité
        ExplodeAs=NULL;			// Type d'explosion lorsque l'unité est détruite
        SelfDestructAs=NULL;	// Type d'explosion lors de l'autodestruction
        Corpse=NULL;			// Restes de l'unité
        UnitNumber=0;			// ID de l'unité
        canmove=false;			// Indique si l'unité peut bouger
        canpatrol=false;		// si elle peut patrouiller
        canstop=false;			// si elle peut s'arrêter
        canguard=false;			// si elle peut garder une autre unité
        MaxVelocity=1;			// Vitesse maximale
        BrakeRate=1;			// Vitesse de freinage
        Acceleration=1;			// Accélération
        TurnRate=1;				// Vitesse de tournage
        SteeringMode=0;
        canfly=false;			// si l'unité peut voler
        Scale=1.0f;				// Echelle
        BankScale=0;
        BuildDistance=0.0f;		// Distance maximale de construction
        CanReclamate=false;		// si elle peut récupérer
        EnergyMake=0;			// Production d'énergie de l'unité
        MetalMake=0.0f;			// Production de métal de l'unité
        MovementClass=NULL;		// Type de mouvement
        Upright=false;			// Si l'unité est debout
        Weapon1=-1;				// Arme 1
        w_badTargetCategory[0]=NULL;	// Type d'unité non ciblable par les armes
        w_badTargetCategory[1]=NULL;	// Type d'unité non ciblable par les armes
        w_badTargetCategory[2]=NULL;	// Type d'unité non ciblable par les armes
        BadTargetCategory=NULL;	// Type d'unité non attacable
        DamageModifier=1.0f;	// How much of the weapon damage it takes
        canattack=false;			// Si l'unité peut attaquer
        ActivateWhenBuilt=false;// L'unité s'active lorsqu'elle est achevée
        onoffable=false;		// (Dés)activable
        MaxWaterDepth=0;		// Profondeur maximale où l'unité peut aller
        MinWaterDepth=-0xFFF;	// Profondeur minimale où l'unité peut aller
        NoShadow=false;			// Si l'unité n'a pas d'ombre
        TransMaxUnits=0;		// Maximum d'unités portables
        canload=false;			// Si elle peut charger d'autres unités
        Weapon2=-1;				// Arme 2
        Floater=false;			// Si l'unité flotte
        canhover=false;			// For hovercrafts
        NoChaseCategory=NULL;		// Type d'unité non chassable
        Weapon3=-1;				// Arme 3
        SonarDistance=0;		// Portée du sonar
        SonarDistanceJam=0;		// For Sonar jammers
        candgun=false;			// si l'unité peut utiliser l'arme ravage
        CloakCost = 0;			// Coût en energie pour rendre l'unité invisible
        CloakCostMoving = 0;	// Idem mais quand l'unité bouge
        HealTime=0;				// Temps nécessaire à l'unité pour se réparer
        CanCapture=false;		// Si elle peut capturer d'autres unités
        HideDamage=false;		// Cache la vie de l'unité aux autres joueurs
        ImmuneToParalyzer=false;	// Immunisation
        Stealth=false;
        MakesMetal=0;			// production de métal de l'unité
        ExtractsMetal=0.0f;		// métal extrait par l'unité
        TidalGenerator=false;	// Si l'unité est une centrale marée-motrice
        TransportMaxUnits=0;	// Maximum d'unités transportables
        kamikaze=false;			// Unité kamikaze
        kamikazedistance=0;
        WindGenerator=0;		// Centrale de type Eolienne
    }


    void UNIT_TYPE::show_info(float fade, GfxFont fnt)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.5f,0.5f,0.5f,fade);
        int x = gfx->SCREEN_W_HALF - 160;
        int y = gfx->SCREEN_H_HALF - 120;
        glBegin(GL_QUADS);
        glVertex2i(x,y);
        glVertex2i(x+320,y);
        glVertex2i(x+320,y+240);
        glVertex2i(x,y+240);
        glEnd();
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        glEnable(GL_TEXTURE_2D);
        glColor4f(1.0f,1.0f,1.0f,fade);
        gfx->drawtexture(glpic,x+16,y+16,x+80,y+80);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        gfx->print(fnt,x+96,y+16,0.0f,format("%s: %s",I18N::Translate("Name").c_str(),name));
        gfx->print(fnt,x+96,y+28,0.0f,format("%s: %s",I18N::Translate("Internal name").c_str(),Unitname));
        gfx->print(fnt,x+96,y+40,0.0f,format("%s",Description));
        gfx->print(fnt,x+96,y+52,0.0f,format("%s: %d",I18N::Translate("HP").c_str(),MaxDamage));
        gfx->print(fnt,x+96,y+64,0.0f,format("%s: E %d M %d",I18N::Translate("Cost").c_str(),BuildCostEnergy,BuildCostMetal));
        gfx->print(fnt,x+16,y+100,0.0f,format("%s: %d",I18N::Translate("Build time").c_str(),BuildTime));
        gfx->print(fnt,x+16,y+124,0.0f,format("%s:",I18N::Translate("weapons").c_str()));
        int Y=y+136;
        if(weapon[0])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[0]->name.c_str(),weapon_damage[0]));	Y+=12;	}
        if(weapon[1])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[1]->name.c_str(),weapon_damage[1]));	Y+=12;	}
        if(weapon[2])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[2]->name.c_str(),weapon_damage[2]));	Y+=12;	}
        glDisable(GL_BLEND);
    }

    int UNIT_TYPE::load(char *data,int size)
    {
        set_uformat(U_ASCII);
        destroy();
        char *pos=data;
        char *f;
        char *ligne=NULL;
        int nb=0;
        int nb_inconnu=0;
        char *limit=data+size;
        while(*pos!='{') pos++;
        pos=strstr(pos,"\n")+1;
        String lang_name = I18N::Translate("UNITTYPE_NAME", "name");
        String lang_desc = I18N::Translate("UNITTYPE_DESCRIPTION", "description");
        do
        {
            nb++;
            if(ligne)
                delete[] ligne;
            ligne=get_line(pos);
            char *dup_ligne=strdup(ligne);
            strlwr(ligne);
            while(pos[0]!=0 && pos[0]!=13 && pos[0]!=10)	pos++;
            while(pos[0]==13 || pos[0]==10)	pos++;

            f=NULL;
            if((f=strstr(ligne,"unitname="))) {
                Unitname=strdup(f+9);
                if(strstr(Unitname,";"))
                    *(strstr(Unitname,";"))=0;
            }
            else if((f=strstr(ligne,"version=")))	version=f[8]-'0';
            else if((f=strstr(ligne,"side="))) {
                side=strdup(f+5);
                if(strstr(side,";"))
                    *(strstr(side,";"))=0;
            }
            else if((f=strstr(ligne,"objectname="))) {
                ObjectName=strdup(f+11);
                if(strstr(ObjectName,";"))
                    *(strstr(ObjectName,";"))=0;
            }
            else if((f=strstr(ligne,"designation="))) {
                Designation_Name=strdup(f+12);
                if(strstr(Designation_Name,";"))
                    *(strstr(Designation_Name,";"))=0;
            }
            else if((f=strstr(ligne,lang_desc.c_str()))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a'))) {
                if(Description)	free(Description);
                Description = strdup( f + lang_desc.length() + 1 );
                if(strstr(Description,";"))
                    *(strstr(Description,";"))=0;
            }
            else if((f=strstr(ligne,lang_name.c_str()))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a'))) {
                if(name)	free(name);
                name = strdup( f + lang_name.length() + 1 );
                if(strstr(name,";"))
                    *(strstr(name,";"))=0;
            }
            else if((f=strstr(ligne,"description="))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a')) && Description==NULL) {
                Description = strdup( f + 12 );
                if(strstr(Description,";"))
                    *(strstr(Description,";"))=0;
            }
            else if((f=strstr(ligne,"name="))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a')) && name==NULL) {
                name = strdup( f + 5 );
                if(strstr(name,";"))
                    *(strstr(name,";"))=0;
            }
            else if((f=strstr(ligne,"description="))) {		// Pour éviter de surcharger les logs
            }
            else if((f=strstr(ligne,"name="))) {
            }
            else if((f=strstr(ligne,"footprintx=")))			FootprintX=atoi(f+11);
            else if((f=strstr(ligne,"footprintz=")))			FootprintZ=atoi(f+11);
            else if((f=strstr(ligne,"buildcostenergy=")))		BuildCostEnergy=atoi(f+16);
            else if((f=strstr(ligne,"buildcostmetal=")))		BuildCostMetal=atoi(f+15);
            else if((f=strstr(ligne,"maxdamage=")))			MaxDamage=atoi(f+10);
            else if((f=strstr(ligne,"maxwaterdepth=")))		MaxWaterDepth=atoi(f+14);
            else if((f=strstr(ligne,"minwaterdepth=")))	{
                MinWaterDepth=atoi(f+14);
                if(MaxWaterDepth==0)
                    MaxWaterDepth=255;
            }
            else if((f=strstr(ligne,"energyuse=")))			EnergyUse=atoi(f+10);
            else if((f=strstr(ligne,"buildtime=")))
                BuildTime=atoi(f+10);
            else if((f=strstr(ligne,"workertime=")))			WorkerTime=atoi(f+11);
            else if((f=strstr(ligne,"builder=")))				Builder=(f[8]=='1');
            else if((f=strstr(ligne,"threed=")))				ThreeD=(f[7]=='1');
            else if((f=strstr(ligne,"sightdistance=")))		SightDistance=atoi(f+14)>>1;
            else if((f=strstr(ligne,"radardistance=")))		RadarDistance=atoi(f+14)>>1;
            else if((f=strstr(ligne,"radardistancejam=")))	RadarDistanceJam=atoi(f+17)>>1;
            else if((f=strstr(ligne,"soundcategory="))) {
                if(strstr(f,";"))
                    *(strstr(f,";"))=0;
                soundcategory = strdup(f + 14);
            }
            else if((f=strstr(ligne,"wthi_badtargetcategory="))) {
                if( w_badTargetCategory[2] )	free( w_badTargetCategory[2] );		// To prevent memory leaks
                while( f[23] == ' ' )	f++;
                w_badTargetCategory[2] = strdup( f + 23 );
            }
            else if((f=strstr(ligne,"wsec_badtargetcategory="))) {
                if( w_badTargetCategory[1] )	free( w_badTargetCategory[1] );		// To prevent memory leaks
                while( f[23] == ' ' )	f++;
                w_badTargetCategory[1] = strdup( f + 23 );
            }
            else if((f=strstr(ligne,"wpri_badtargetcategory="))) {
                if( w_badTargetCategory[0] )	free( w_badTargetCategory[0] );		// To prevent memory leaks
                while( f[23] == ' ' )	f++;
                w_badTargetCategory[0] = strdup( f + 23 );
            }
            else if((f=strstr(ligne,"nochasecategory="))) {
                if( NoChaseCategory )	free( NoChaseCategory );		// To prevent memory leaks
                while( f[17] == ' ' )	f++;
                NoChaseCategory = strdup( f + 17 );
            }
            else if((f=strstr(ligne,"badtargetcategory="))) {
                if( BadTargetCategory )	free( BadTargetCategory );		// To prevent memory leaks
                while( f[18] == ' ' )	f++;
                BadTargetCategory = strdup( f + 18 );
            }
            else if((f=strstr(ligne,"category="))) {
                while( f[9] == ' ' )	f++;
                if(strstr(f,";"))
                    *(strstr(f,";"))=0;
                if( Category )		delete Category;
                if( categories )	delete categories;
                Category = new cHashTable< int >(128);
                categories = new String::Vector;
                ReadVectorString(*categories, f + 9, " " );
                for(String::Vector::const_iterator i = categories->begin(); i != categories->end(); ++i)
                    Category->insertOrUpdate(String::ToLower(*i), 1);
                fastCategory = 0;
                if( checkCategory( "kamikaze" ) )	fastCategory |= CATEGORY_KAMIKAZE;
                if( checkCategory( "notair" ) )		fastCategory |= CATEGORY_NOTAIR;
                if( checkCategory( "notsub" ) )		fastCategory |= CATEGORY_NOTSUB;
                if( checkCategory( "jam" ) )		fastCategory |= CATEGORY_JAM;
                if( checkCategory( "commander" ) )	fastCategory |= CATEGORY_COMMANDER;
                if( checkCategory( "weapon" ) )		fastCategory |= CATEGORY_WEAPON;
                if( checkCategory( "level3" ) )		fastCategory |= CATEGORY_LEVEL3;
            }
            else if((f=strstr(ligne,"unitnumber=")))			UnitNumber=atoi(f+14);
            else if((f=strstr(ligne,"canmove=")))				canmove=(f[8]=='1');
            else if((f=strstr(ligne,"canpatrol=")))			canpatrol=(f[10]=='1');
            else if((f=strstr(ligne,"canstop=")))				canstop=(f[8]=='1');
            else if((f=strstr(ligne,"canguard=")))			canguard=(f[9]=='1');
            else if((f=strstr(ligne,"maxvelocity=")))			MaxVelocity = atof(f+12)*16.0f;
            else if((f=strstr(ligne,"brakerate=")))			BrakeRate=atof(f+10)*160.0f;
            else if((f=strstr(ligne,"acceleration=")))		Acceleration=atof(f+13)*160.0f;
            else if((f=strstr(ligne,"turnrate=")))			TurnRate = atof(f+9) * TA2DEG * 20.0f;
            else if((f=strstr(ligne,"candgun=")))				candgun=(f[8]=='1');
            else if((f=strstr(ligne,"canattack=")))			canattack=(f[10]=='1');
            else if((f=strstr(ligne,"canreclamate=")))		CanReclamate=(f[13]=='1');
            else if((f=strstr(ligne,"energymake=")))			EnergyMake=atoi(f+11);
            else if((f=strstr(ligne,"metalmake=")))			MetalMake=atof(f+10);
            else if((f=strstr(ligne,"cancapture=")))			CanCapture=(f[11]=='1');
            else if((f=strstr(ligne,"hidedamage=")))			HideDamage=(f[11]=='1');
            else if((f=strstr(ligne,"healtime=")))			HealTime=atoi(f+9)*30;		// To have it in seconds
            else if((f=strstr(ligne,"cloakcost=")))			CloakCost=atoi(f+10);
            else if((f=strstr(ligne,"cloakcostmoving=")))		CloakCostMoving=atoi(f+16);
            else if((f=strstr(ligne,"init_cloaked=")))		init_cloaked=f[13]=='1';
            else if((f=strstr(ligne,"mincloakdistance=")))	mincloakdistance=atoi(f+17)>>1;
            else if((f=strstr(ligne,"builddistance=")))		BuildDistance=atoi(f+14);
            else if((f=strstr(ligne,"activatewhenbuilt=")))	ActivateWhenBuilt=(f[18]=='1');
            else if((f=strstr(ligne,"immunetoparalyzer=")))	ImmuneToParalyzer=(f[18]=='1');
            else if((f=strstr(ligne,"sonardistance=")))		SonarDistance=atoi(f+14)>>1;
            else if((f=strstr(ligne,"sonardistancejam=")))	SonarDistanceJam=atoi(f+17)>>1;
            else if((f=strstr(ligne,"copyright="))) {}
            else if((f=strstr(ligne,"maxslope=")))			MaxSlope=atoi(f+9);
            else if((f=strstr(ligne,"steeringmode=")))		SteeringMode=atoi(f+13);

            else if((f=strstr(ligne,"bmcode=")))				BMcode=atoi(f+7);
            else if((f=strstr(ligne,"zbuffer="))) {}
            else if((f=strstr(ligne,"shootme=")))				ShootMe=(f[8]=='1');
            else if((f=strstr(ligne,"upright=")))				Upright=(f[8]=='1');
            else if((f=strstr(ligne,"norestrict=")))			norestrict=(f[11]=='1');
            else if((f=strstr(ligne,"noautofire=")))			AutoFire=(f[11]!='1');
            else if((f=strstr(ligne,"energystorage=")))		EnergyStorage=atoi(f+14);
            else if((f=strstr(ligne,"metalstorage=")))		MetalStorage=atoi(f+13);
            else if((f=strstr(ligne,"standingmoveorder=")))	StandingMoveOrder=atoi(f+18);
            else if((f=strstr(ligne,"mobilestandorders=")))	MobileStandOrders=atoi(f+18);
            else if((f=strstr(ligne,"standingfireorder=")))	StandingFireOrder=atoi(f+18);
            else if((f=strstr(ligne,"firestandorders=")))		FireStandOrders=atoi(f+16);
            else if((f=strstr(ligne,"waterline=")))			WaterLine=atof(f+10);
            else if((f=strstr(ligne,"tedclass="))) {
                if(strstr(f,"water"))			TEDclass=CLASS_WATER;
                else if((strstr(f,"ship")))		TEDclass=CLASS_SHIP;
                else if((strstr(f,"energy")))		TEDclass=CLASS_ENERGY;
                else if((strstr(f,"vtol")))		TEDclass=CLASS_VTOL;
                else if((strstr(f,"kbot")))		TEDclass=CLASS_KBOT;
                else if((strstr(f,"plant")))		TEDclass=CLASS_PLANT;
                else if((strstr(f,"tank")))		TEDclass=CLASS_TANK;
                else if((strstr(f,"special")))	TEDclass=CLASS_SPECIAL;
                else if((strstr(f,"fort")))		TEDclass=CLASS_FORT;
                else if((strstr(f,"metal")))		TEDclass=CLASS_METAL;
                else if((strstr(f,"cnstr")))		TEDclass=CLASS_CNSTR;
                else if((strstr(f,"commander")))	TEDclass=CLASS_COMMANDER;
                else {
                    printf("->tedclass id inconnu : %s\n",f);
                    nb_inconnu++;
                }
            }
            else if((f=strstr(ligne,"noshadow=")))			NoShadow=(f[9]=='1');
            else if((f=strstr(ligne,"antiweapons=")))			antiweapons=(f[12]=='1');
            else if((f=strstr(ligne,"buildangle=")))			BuildAngle=atoi(f+11);
            else if((f=strstr(ligne,"canfly=")))				canfly=(f[7]=='1');
            else if((f=strstr(ligne,"canload=")))				canload=(f[8]=='1');
            else if((f=strstr(ligne,"floater=")))				Floater=(f[8]=='1');
            else if((f=strstr(ligne,"canhover=")))			canhover=(f[9]=='1');
            else if((f=strstr(ligne,"bankscale=")))			BankScale=atoi(f+10);
            else if((f=strstr(ligne,"tidalgenerator=")))		TidalGenerator=(f[15]=='1');
            //			else if(f=strstr(ligne,"scale="))				Scale=atof(f+6);
            else if((f=strstr(ligne,"scale=")))				Scale=1.0f;
            else if((f=strstr(ligne,"corpse="))) {
                char *nom=strdup(f+7);
                if(strstr(nom,";"))
                    *(strstr(nom,";"))=0;
                Corpse=strdup(nom);
                free(nom);
            }
            else if((f=strstr(ligne,"windgenerator=")))
                WindGenerator=atoi(f+14);
            else if((f=strstr(ligne,"onoffable=")))			onoffable=(f[10]=='1');
            else if((f=strstr(ligne,"kamikaze=")))			kamikaze=(f[9]=='1');
            else if((f=strstr(ligne,"kamikazedistance=")))	kamikazedistance=atoi(f+17)>>1;
            else if((f=strstr(ligne,"weapon1="))) {
                char *weaponname=strdup(f+8);
                if(strstr(weaponname,";"))
                    *(strstr(weaponname,";"))=0;
                Weapon1=weapon_manager.get_weapon_index(weaponname);
                free(weaponname);
            }
            else if((f=strstr(ligne,"weapon2="))) {
                char *weaponname=strdup(f+8);
                if(strstr(weaponname,";"))
                    *(strstr(weaponname,";"))=0;
                Weapon2=weapon_manager.get_weapon_index(weaponname);
                free(weaponname);
            }
            else if((f=strstr(ligne,"weapon3="))) {
                char *weaponname=strdup(f+8);
                if(strstr(weaponname,";"))
                    *(strstr(weaponname,";"))=0;
                Weapon3=weapon_manager.get_weapon_index(weaponname);
                free(weaponname);
            }
            else if((f=strstr(ligne,"yardmap=")))	{
                f=strstr(dup_ligne,"=");
                if(strstr(f+1,";"))
                    *(strstr(f+1,";"))=0;
                while(strstr(f," ")) {
                    char *fm=strstr(f," ");
                    memmove(fm,fm+1,strlen(fm+1)+1);
                }
                yardmap=strdup(f+1);
            }
            else if((f=strstr(ligne,"cruisealt=")))			CruiseAlt=atoi(f+10);
            else if((f=strstr(ligne,"explodeas="))) {
                ExplodeAs=strdup(f+10);
                if(strstr(ExplodeAs,";"))
                    *(strstr(ExplodeAs,";"))=0;
            }
            else if((f=strstr(ligne,"selfdestructas="))) {
                SelfDestructAs=strdup(f+15);
                if(strstr(SelfDestructAs,";"))
                    *(strstr(SelfDestructAs,";"))=0;
            }
            else if((f=strstr(ligne,"maneuverleashlength=")))	ManeuverLeashLength=atoi(f+20);
            else if((f=strstr(ligne,"defaultmissiontype="))) {
                if(strstr(f,"=standby;"))				DefaultMissionType=MISSION_STANDBY;
                else if(strstr(f,"=vtol_standby;"))		DefaultMissionType=MISSION_VTOL_STANDBY;
                else if(strstr(f,"=guard_nomove;"))		DefaultMissionType=MISSION_GUARD_NOMOVE;
                else if(strstr(f,"=standby_mine;"))		DefaultMissionType=MISSION_STANDBY_MINE;
                else {
                    LOG_ERROR("Unknown constant: `" << f << "`");
                    ++nb_inconnu;
                }
            }
            else if((f=strstr(ligne,"transmaxunits=")))		TransMaxUnits=TransportMaxUnits=atoi(f+14);
            else if((f=strstr(ligne,"transportmaxunits=")))	TransMaxUnits=TransportMaxUnits=atoi(f+18);
            else if((f=strstr(ligne,"transportcapacity=")))	TransportCapacity=atoi(f+18);
            else if((f=strstr(ligne,"transportsize=")))		TransportSize=atoi(f+14);
            else if((f=strstr(ligne,"altfromsealevel=")))		AltFromSeaLevel=atoi(f+16);
            else if((f=strstr(ligne,"movementclass="))) {
                MovementClass = strdup(f+14);
                if( strstr( MovementClass, ";") )
                    *strstr( MovementClass, ";") = 0;
            }
            else if((f=strstr(ligne,"isairbase=")))			IsAirBase=(f[10]=='1');
            else if((f=strstr(ligne,"commander=")))			commander=(f[10]=='1');
            else if((f=strstr(ligne,"damagemodifier=")))		DamageModifier=atof(f+15);
            else if((f=strstr(ligne,"makesmetal=")))			MakesMetal=atof(f+11);
            else if((f=strstr(ligne,"sortbias=")))			SortBias=atoi(f+9);
            else if((f=strstr(ligne,"extractsmetal=")))		ExtractsMetal=atof(f+14);
            else if((f=strstr(ligne,"hoverattack=")))			hoverattack=(f[12]=='1');
            else if((f=strstr(ligne,"isfeature=")))			isfeature=(f[10]=='1');
            else if((f=strstr(ligne,"stealth=")))				Stealth=atoi(f+8);
            else if((f=strstr(ligne,"attackrunlength=")))		attackrunlength = atoi(f+16);
            else if((f=strstr(ligne,"selfdestructcountdown=")))	selfdestructcountdown=atoi(f+22);
            else if((f=strstr(ligne,"canresurrect=")))		canresurrect = (f[13] == '1');
            else if((f=strstr(ligne,"resurrect=")))			canresurrect = (f[10] == '1');
            else if((f=strstr(ligne,"downloadable="))) { }
            else if((f=strstr(ligne,"ovradjust="))) { }
            else if((f=strstr(ligne,"ai_limit="))) { }
            else if((f=strstr(ligne,"ai_weight="))) { }
            if(f==NULL && strstr(ligne,"}")==NULL && strlen( ligne ) > 0 ) {
                LOG_ERROR("[FBI] Unknown variable: `" << ligne << "`");
                ++nb_inconnu;
            }
            if(dup_ligne)
                free(dup_ligne);
        }while(strstr(ligne,"}")==NULL && nb<1000 && pos<limit);
        delete[] ligne;
        if( canresurrect && BuildDistance == 0.0f )
            BuildDistance = SightDistance;
        if(Weapon1>-1)
            weapon[0]=&(weapon_manager.weapon[Weapon1]);
        if(Weapon2>-1)
            weapon[1]=&(weapon_manager.weapon[Weapon2]);
        if(Weapon3>-1)
            weapon[2]=&(weapon_manager.weapon[Weapon3]);
        if(Unitname) {
            model=model_manager.get_model(ObjectName);
            if(model==NULL)
                LOG_ERROR("`" << Unitname << "` without a 3D model");
        }
        else
            LOG_WARNING("The unit does not have a name");
        if (canfly == 1)
            TurnRate = TurnRate * 3; // A hack thanks to Doors
        load_dl();
        return nb_inconnu;
    }

    void UNIT_TYPE::load_dl()
    {
        if (side == NULL)
            return;
        dl_data = unit_manager.h_dl_data.find(String::ToLower(side));

        if (dl_data)
            return;			// Ok it's already loaded

        int side_id = -1;
        for( int i = 0 ; i < ta3dSideData.nb_side && side_id == -1 ; i++ )
            if( strcasecmp( ta3dSideData.side_name[ i ], side ) == 0 )
                side_id = i;
        if( side_id == -1 )		return;

        dl_data = new DL_DATA;

        try
        {
            cTAFileParser dl_parser( ta3dSideData.guis_dir + ta3dSideData.side_pref[ side_id ] + "dl.gui", false, false, true );

            int NbObj = dl_parser.pullAsInt( "gadget0.totalgadgets" );

            int x_offset = dl_parser.pullAsInt( "gadget0.common.xpos" );
            int y_offset = dl_parser.pullAsInt( "gadget0.common.ypos" );

            dl_data->dl_num = 0;

            for( int i = 1 ; i <= NbObj ; i++ )
                if( dl_parser.pullAsInt( format( "gadget%d.common.attribs", i ) ) == 32 )
                    dl_data->dl_num++;

            dl_data->dl_x = (short*) malloc( sizeof(short) * dl_data->dl_num );
            dl_data->dl_y = (short*) malloc( sizeof(short) * dl_data->dl_num );
            dl_data->dl_w = (short*) malloc( sizeof(short) * dl_data->dl_num );
            dl_data->dl_h = (short*) malloc( sizeof(short) * dl_data->dl_num );

            int e = 0;
            for( int i = 1 ; i <= NbObj ; i++ )
                if( dl_parser.pullAsInt( format( "gadget%d.common.attribs", i ) ) == 32 ) {
                    dl_data->dl_x[e] = dl_parser.pullAsInt( format( "gadget%d.common.xpos", i ) ) + x_offset;
                    dl_data->dl_y[e] = dl_parser.pullAsInt( format( "gadget%d.common.ypos", i ) ) + y_offset;
                    dl_data->dl_w[e] = dl_parser.pullAsInt( format( "gadget%d.common.width", i ) );
                    dl_data->dl_h[e] = dl_parser.pullAsInt( format( "gadget%d.common.height", i ) );
                    e++;
                }

            unit_manager.l_dl_data.push_back( dl_data );		// Put it there so it'll be deleted when finished

            unit_manager.h_dl_data.insert(String::ToLower(side), dl_data);
        }
        catch(...)
        {
            LOG_WARNING("`dl.gui` file is missing");
            delete dl_data;
            dl_data = NULL;
        }
    }

    inline bool overlaps( int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2 )
    {
        int w = w1 + w2;
        int h = h1 + h2;
        int X1 = Math::Min(x1, x2);
        int Y1 = Math::Min(y1, y2);
        int X2 = Math::Max(x1 + w1, x2 + w2);
        int Y2 = Math::Max(y1 + h1, y2 + h2);
        return X2 - X1 < w && Y2 - Y1 < h;
    }

    void UNIT_TYPE::FixBuild()
    {
        if (dl_data && dl_data->dl_num > 0)
        {
            for (int i = 0 ; i < nb_unit - 1; ++i)		// Ok it's O(N²) but we don't need something fast
            {
                for (int e = i + 1; e < nb_unit; ++e)
                {
                    if( (Pic_p[e] < Pic_p[i] && Pic_p[e] != -1) || Pic_p[i] == -1 )
                    {
                        SWAP( Pic_p[e], Pic_p[i] )
                            SWAP( Pic_x[e], Pic_x[i] )
                            SWAP( Pic_y[e], Pic_y[i] )
                            SWAP( Pic_w[e], Pic_w[i] )
                            SWAP( Pic_h[e], Pic_h[i] )
                            SWAP( PicList[e], PicList[i] )
                            SWAP( BuildList[e], BuildList[i] )
                    }
                }
            }
        }

        int next_id = 0;
        bool filled = true;
        int last = -2;

        bool first_time = true;
        nb_pages = -1;
        for( int i = 0 ; i < nb_unit ; i++ )		// We can't trust Pic_p data, because sometimes we get >= 10000 !! so only order is important
            if( Pic_p[ i ] != -1 ) {
                if( last == Pic_p[ i ] && !first_time )
                    Pic_p[ i ] = nb_pages;
                else {
                    last = Pic_p[ i ];
                    Pic_p[ i ] = ++nb_pages;
                }
                first_time = false;
            }

        if( nb_pages == -1 )	nb_pages = 0;

        if( dl_data && dl_data->dl_num > 0 )
            for( int i = 0 ; i < nb_unit ; i++ )
                if( Pic_p[ i ] == -1 ) {
                    Pic_p[ i ] = nb_pages;
                    Pic_x[ i ] = dl_data->dl_x[ next_id ];
                    Pic_y[ i ] = dl_data->dl_y[ next_id ];
                    Pic_w[ i ] = dl_data->dl_w[ next_id ];
                    Pic_h[ i ] = dl_data->dl_h[ next_id ];
                    next_id = (next_id + 1) % dl_data->dl_num;
                    filled = true;
                    if( next_id == 0 ) {
                        nb_pages++;
                        filled = false;
                    }
                }
        if( !filled )	nb_pages--;
        for( int i = nb_unit - 1 ; i > 0 ; i-- ) {
            for( int e = i - 1 ; e >= 0 ; e-- )
                if( Pic_p[ e ] == Pic_p[ i ] && overlaps( Pic_x[ e ], Pic_y[ e ], Pic_w[ e ], Pic_h[ e ], Pic_x[ i ], Pic_y[ i ], Pic_w[ i ], Pic_h[ i ] ) ) {
                    Pic_p[ i ]++;
                    e = i;
                }
        }
        for( int i = 0 ; i < nb_unit - 1 ; i++ )  		// Ok it's O(N²) but we don't need something fast
            for( int e = i + 1 ; e < nb_unit ; e++ )
                if( Pic_p[e] < Pic_p[i] ) {
                    SWAP( Pic_p[e], Pic_p[i] )
                        SWAP( Pic_x[e], Pic_x[i] )
                        SWAP( Pic_y[e], Pic_y[i] )
                        SWAP( Pic_w[e], Pic_w[i] )
                        SWAP( Pic_h[e], Pic_h[i] )
                        SWAP( PicList[e], PicList[i] )
                        SWAP( BuildList[e], BuildList[i] )
                }
        for (int i = 0; i < nb_unit; ++i)
            nb_pages = Math::Max(nb_pages, Pic_p[i]);
        nb_pages++;
    }

    void UNIT_MANAGER::destroy()
    {
        unit_hashtable.emptyHashTable();
        unit_hashtable.initTable( __DEFAULT_HASH_TABLE_SIZE );

        h_dl_data.emptyHashTable();
        h_dl_data.initTable( __DEFAULT_HASH_TABLE_SIZE );

        for( std::list< DL_DATA* >::iterator i = l_dl_data.begin() ; i != l_dl_data.end() ; i++ )
            delete	*i;

        l_dl_data.clear();

        if (nb_unit > 0 && unit_type != NULL)
        {
            for (int i = 0; i < nb_unit; ++i)
                unit_type[i].destroy();
            free(unit_type);
        }
        panel.destroy();
        paneltop.destroy();
        panelbottom.destroy();
        init();
    }

    void UNIT_MANAGER::load_panel_texture( const String &player_side, const String &intgaf )
    {
        panel.destroy();
        String gaf_img;
        try {
            cTAFileParser parser( ta3dSideData.guis_dir + player_side + "MAIN.GUI" );
            gaf_img = parser.pullAsString( "gadget0.panel" );
        }
        catch( ... )
        {
            LOG_ERROR("Unable to load `"<< (ta3dSideData.guis_dir + player_side + "MAIN.GUI") << "`");
            return;
        }

        set_color_depth( 32 );
        if(g_useTextureCompression)
            allegro_gl_set_texture_format( GL_COMPRESSED_RGB_ARB );
        else
            allegro_gl_set_texture_format( GL_RGB8 );
        int w,h;
        GLuint panel_tex = read_gaf_img( "anims\\" + player_side + "main.gaf", gaf_img, &w, &h, true );
        if (panel_tex == 0)
        {
            String::List file_list;
            HPIManager->getFilelist( "anims\\*.gaf", file_list);
            for (String::List::const_iterator i = file_list.begin(); i != file_list.end() && panel_tex == 0 ; ++i)
                panel_tex = read_gaf_img(*i, gaf_img, &w, &h, true);
        }
        panel.set( panel_tex );
        panel.width = w;	panel.height = h;

        paneltop.set( read_gaf_img( "anims\\" + intgaf + ".gaf", "PANELTOP", &w, &h ) );
        paneltop.width = w;		paneltop.height = h;
        panelbottom.set( read_gaf_img( "anims\\" + intgaf + ".gaf", "PANELBOT", &w, &h ) );
        panelbottom.width = w;	panelbottom.height = h;
    }

    int UNIT_MANAGER::unit_build_menu(int index,int omb,float &dt, bool GUI)				// Affiche et gère le menu des unités
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        bool nothing = index < -1 || index>=nb_unit;

        gfx->ReInitTexSys();
        glColor4f(1.0f,1.0f,1.0f,0.75f);
        if (GUI)
        {
            if( panel.tex && nothing )
                gfx->drawtexture( panel.tex, 0.0f, 128.0f, 128.0f, 128.0f + panel.height );

            if( paneltop.tex )
            {
                gfx->drawtexture( paneltop.tex, 128.0f, 0.0f, 128.0f + paneltop.width, paneltop.height );
                for (int k = 0 ; 128 + paneltop.width + panelbottom.width * k < SCREEN_W ; ++k)
                {
                    gfx->drawtexture(panelbottom.tex, 128.0f + paneltop.width + k * panelbottom.width, 0.0f,
                                     128.0f + paneltop.width + panelbottom.width * (k+1), panelbottom.height );
                }
            }

            if (panelbottom.tex)
            {
                for(int k = 0 ; 128 + panelbottom.width * k < SCREEN_W ; ++k)
                {
                    gfx->drawtexture( panelbottom.tex, 128.0f + k * panelbottom.width,
                                      SCREEN_H - panelbottom.height, 128.0f + panelbottom.width * (k+1), SCREEN_H );
                }
            }

            glDisable(GL_TEXTURE_2D);
            glColor4f(0.0f,0.0f,0.0f,0.75f);
            glBegin(GL_QUADS);
            glVertex2f(0.0f,0.0f);			// Barre latérale gauche
            glVertex2f(128.0f,0.0f);
            glVertex2f(128.0f,128.0f);
            glVertex2f(0.0f,128.0f);

            glVertex2f(0.0f,128.0f + panel.height);			// Barre latérale gauche
            glVertex2f(128.0f,128.0f + panel.height);
            glVertex2f(128.0f,SCREEN_H);
            glVertex2f(0.0f,SCREEN_H);
            glEnd();
            glColor4f(1.0f,1.0f,1.0f,0.75f);
            return 0;
        }

        glEnable(GL_TEXTURE_2D);

        if(index<0 || index>=nb_unit) return -1;		// L'indice est incorrect

        int page=unit_type[index].page;

        int sel=-1;

        glDisable(GL_BLEND);
        for( int i = 0 ; i < unit_type[index].nb_unit ; i++ ) {		// Affiche les différentes images d'unités constructibles
            if( unit_type[index].Pic_p[i] != page )	continue;
            int px = unit_type[index].Pic_x[ i ];
            int py = unit_type[index].Pic_y[ i ];
            int pw = unit_type[index].Pic_w[ i ];
            int ph = unit_type[index].Pic_h[ i ];
            bool unused = unit_type[index].BuildList[i] >= 0 && unit_type[unit_type[index].BuildList[i]].not_used;
            if( unused )
                glColor4f(0.3f,0.3f,0.3f,1.0f);			// Make it darker
            else
                glColor4f(1.0f,1.0f,1.0f,1.0f);

            if(unit_type[index].PicList[i])							// If a texture is given use it
                gfx->drawtexture(unit_type[index].PicList[i],px,py,px+pw,py+ph);
            else
                gfx->drawtexture(unit_type[unit_type[index].BuildList[i]].glpic,px,py,px+pw,py+ph);

            if(mouse_x>=px && mouse_x<px+pw && mouse_y>=py && mouse_y<py+ph && !unused) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glColor4f(1.0f,1.0f,1.0f,0.75f);
                if(unit_type[index].PicList[i])							// If a texture is given use it
                    gfx->drawtexture(unit_type[index].PicList[i],px,py,px+pw,py+ph);
                else
                    gfx->drawtexture(unit_type[unit_type[index].BuildList[i]].glpic,px,py,px+pw,py+ph);
                glDisable(GL_BLEND);
                sel = unit_type[index].BuildList[i];
                if(sel == -1)
                    sel = -2;
            }

            if( ( unit_type[index].BuildList[i] == unit_type[index].last_click
                  || ( unit_type[index].last_click == -2 && unit_type[index].BuildList[i] == -1 ) )
                && unit_type[index].click_time > 0.0f ) {
                glEnable(GL_BLEND);
                glDisable(GL_TEXTURE_2D);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(1.0f,1.0f,1.0f,unit_type[index].click_time);
                int mx = px;
                int my = py;
                gfx->rectfill( mx,my,mx+pw,my+ph );
                glColor4f(1.0f,1.0f,0.0f,0.75f);
                gfx->line( mx, my+ph*unit_type[index].click_time, mx+pw, my+ph*unit_type[index].click_time );
                gfx->line( mx, my+ph*(1.0f-unit_type[index].click_time), mx+pw, my+ph*(1.0f-unit_type[index].click_time) );
                gfx->line( mx+pw*unit_type[index].click_time, my, mx+pw*unit_type[index].click_time, my+ph );
                gfx->line( mx+pw*(1.0f-unit_type[index].click_time), my, mx+pw*(1.0f-unit_type[index].click_time), my+ph );
                glColor4f(1.0f,1.0f,1.0f,0.75f);
                glEnable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
            }
        }
        glColor4f(1.0f,1.0f,1.0f,1.0f);

        if( unit_type[index].last_click != -1 )
            unit_type[index].click_time -= dt;

        if(sel>-1) {
            set_uformat(U_ASCII);
            gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Name.x1,ta3dSideData.side_int_data[ players.side_view ].Name.y1,0.0f,0xFFFFFFFF, format("%s M:%d E:%d HP:%d",unit_type[sel].name,unit_type[sel].BuildCostMetal,unit_type[sel].BuildCostEnergy,unit_type[sel].MaxDamage) );

            if(unit_type[sel].Description)
                gfx->print(gfx->normal_font,ta3dSideData.side_int_data[ players.side_view ].Description.x1,ta3dSideData.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF,format("%s",unit_type[sel].Description) );
            glDisable(GL_BLEND);
            set_uformat(U_UTF8);
        }

        if( sel != -1 && mouse_b == 1 && omb != 1 ) {		// Click !!
            unit_type[index].last_click = sel;
            unit_type[index].click_time = 0.5f;		// One sec animation;
        }

        return sel;
    }

    int load_all_units(void (*progress)(float percent,const String &msg))
    {
        unit_manager.init();
        int nb_inconnu=0;
        String::List file_list;
        HPIManager->getFilelist( ta3dSideData.unit_dir + "*" + ta3dSideData.unit_ext, file_list);

        int n = 0;

        for (String::List::iterator i = file_list.begin(); i != file_list.end(); ++i)
        {
            if (progress != NULL && !(n & 0xF))
                progress((300.0f+n*50.0f/(file_list.size()+1))/7.0f, I18N::Translate("Loading units"));
            ++n;

            char *nom=strdup(strstr(i->c_str(),"\\")+1);			// Vérifie si l'unité n'est pas déjà chargée
            *(strstr(nom,"."))=0;
            strupr(nom);

            if (unit_manager.get_unit_index(nom) == -1)
            {
                uint32 file_size=0;
                byte *data = HPIManager->PullFromHPI(*i, &file_size);
                nb_inconnu += unit_manager.load_unit(data, file_size);
                if (unit_manager.unit_type[unit_manager.nb_unit-1].Unitname)
                {
                    String nom_pcx;
                    nom_pcx << "unitpics\\" << unit_manager.unit_type[unit_manager.nb_unit-1].Unitname << ".pcx";
                    unit_manager.unit_type[unit_manager.nb_unit-1].unitpic = Converters::PCX::FromHPIToBitmap(nom_pcx);

                    if (unit_manager.unit_type[unit_manager.nb_unit-1].unitpic)
                    {
                        allegro_gl_use_alpha_channel(false);
                        if(g_useTextureCompression)
                            allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
                        else
                            allegro_gl_set_texture_format(GL_RGB8);
                        unit_manager.unit_type[unit_manager.nb_unit-1].glpic=allegro_gl_make_texture(unit_manager.unit_type[unit_manager.nb_unit-1].unitpic);
                        glBindTexture(GL_TEXTURE_2D,unit_manager.unit_type[unit_manager.nb_unit-1].glpic);
                        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    }
                }
                delete[] data;
                LOG_DEBUG("Loading `" << nom << "`...");
            }
            free(nom);
        }

        for (int i = 0;i < unit_manager.nb_unit; ++i)
            unit_manager.load_script_file(unit_manager.unit_type[i].Unitname);

        unit_manager.Identify();

        unit_manager.gather_all_build_data();

        // Correct some data given in the FBI file using data from the moveinfo.tdf file
        cTAFileParser parser( ta3dSideData.gamedata_dir + "moveinfo.tdf" );
        n = 0;
        while( parser.pullAsString( format( "CLASS%d.name", n ) ) != "" )
            n++;

        for (int i=0;i<unit_manager.nb_unit; ++i)
        {
            if( unit_manager.unit_type[ i ].MovementClass != NULL )
            {
                for (int e = 0; e < n; ++e)
                    if (parser.pullAsString( format( "CLASS%d.name", e ) ) == String::ToUpper(unit_manager.unit_type[i].MovementClass))
                    {
                        unit_manager.unit_type[ i ].FootprintX = parser.pullAsInt( format( "CLASS%d.footprintx", e ), unit_manager.unit_type[ i ].FootprintX );
                        unit_manager.unit_type[ i ].FootprintZ = parser.pullAsInt( format( "CLASS%d.footprintz", e ), unit_manager.unit_type[ i ].FootprintZ );
                        unit_manager.unit_type[ i ].MinWaterDepth = parser.pullAsInt( format( "CLASS%d.minwaterdepth", e ), unit_manager.unit_type[ i ].MinWaterDepth );
                        unit_manager.unit_type[ i ].MaxWaterDepth = parser.pullAsInt( format( "CLASS%d.maxwaterdepth", e ), unit_manager.unit_type[ i ].MaxWaterDepth );
                        unit_manager.unit_type[ i ].MaxSlope = parser.pullAsInt( format( "CLASS%d.maxslope", e ), unit_manager.unit_type[ i ].MaxSlope );
                        break;
                    }
            }
        }
        return nb_inconnu;
    }

    bool UNIT_TYPE::canBuild(const int index) const
    {
        for (int i = 0; i < nb_unit; ++i)
            if (BuildList[i] == index)
                return true;
        return false;
    }



}
