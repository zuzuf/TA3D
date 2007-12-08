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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"
//#include "fbi.h"
#include "EngineClass.h"
#include "UnitEngine.h"

UNIT_MANAGER unit_manager;

void UNIT_TYPE::show_info(float fade,GFX_FONT fnt)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.5f,0.5f,0.5f,fade);
	int x=gfx->SCREEN_W_HALF-160;
	int y=gfx->SCREEN_H_HALF-120;
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
	gfx->print(fnt,x+96,y+16,0.0f,format("%s: %s",TRANSLATE("Name").c_str(),name));
	gfx->print(fnt,x+96,y+28,0.0f,format("%s: %s",TRANSLATE("Internal name").c_str(),Unitname));
	gfx->print(fnt,x+96,y+40,0.0f,format("%s",Description));
	gfx->print(fnt,x+96,y+52,0.0f,format("%s: %d",TRANSLATE("HP").c_str(),MaxDamage));
	gfx->print(fnt,x+96,y+64,0.0f,format("%s: E %d M %d",TRANSLATE("Cost").c_str(),BuildCostEnergy,BuildCostMetal));
	gfx->print(fnt,x+16,y+100,0.0f,format("%s: %d",TRANSLATE("Build time").c_str(),BuildTime));
	gfx->print(fnt,x+16,y+124,0.0f,format("%s:",TRANSLATE("weapons").c_str()));
	int Y=y+136;
	if(weapon[0])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[0]->name,weapon_damage[0]));	Y+=12;	}
	if(weapon[1])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[1]->name,weapon_damage[1]));	Y+=12;	}
	if(weapon[2])	{	gfx->print(fnt,x+16,Y,0.0f,format("%s: %d",weapon[2]->name,weapon_damage[2]));	Y+=12;	}
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
		char *lang_name="name=";
		char *lang_desc="description=";
		switch(LANG)
		{
		case TA3D_LANG_FRENCH:
			lang_name="frenchname=";
			lang_desc="frenchdescription=";
			break;
		case TA3D_LANG_GERMAN:
			lang_name="germanname=";
			lang_desc="germandescription=";
			break;
		case TA3D_LANG_SPANISH:
			lang_name="spanishname=";
			lang_desc="spanishdescription=";
			break;
		case TA3D_LANG_ITALIAN:
			lang_name="italianname=";
			lang_desc="italiandescription=";
			break;
		};
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
			if(f=strstr(ligne,"unitname=")) {
				Unitname=strdup(f+9);
				if(strstr(Unitname,";"))
					*(strstr(Unitname,";"))=0;
				}
			else if(f=strstr(ligne,"version="))	version=f[8]-'0';
			else if(f=strstr(ligne,"side=")) {
				side=strdup(f+5);
				if(strstr(side,";"))
					*(strstr(side,";"))=0;
				}
			else if(f=strstr(ligne,"objectname=")) {
				ObjectName=strdup(f+11);
				if(strstr(ObjectName,";"))
					*(strstr(ObjectName,";"))=0;
				}
			else if(f=strstr(ligne,"designation=")) {
				Designation_Name=strdup(f+12);
				if(strstr(Designation_Name,";"))
					*(strstr(Designation_Name,";"))=0;
				}
			else if((f=strstr(ligne,lang_desc))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a'))) {
				if(Description)	free(Description);
				Description = strdup( f + strlen(lang_desc) );
				if(strstr(Description,";"))
					*(strstr(Description,";"))=0;
				}
			else if((f=strstr(ligne,lang_name))!=NULL && (f==ligne || (f>ligne && *(f-1)<'a'))) {
				if(name)	free(name);
				name = strdup( f + strlen(lang_name) );
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
			else if(f=strstr(ligne,"description=")) {		// Pour éviter de surcharger les logs
				}
			else if(f=strstr(ligne,"name=")) {
				}
			else if(f=strstr(ligne,"footprintx="))			FootprintX=atoi(f+11);
			else if(f=strstr(ligne,"footprintz="))			FootprintZ=atoi(f+11);
			else if(f=strstr(ligne,"buildcostenergy="))		BuildCostEnergy=atoi(f+16);
			else if(f=strstr(ligne,"buildcostmetal="))		BuildCostMetal=atoi(f+15);
			else if(f=strstr(ligne,"maxdamage="))			MaxDamage=atoi(f+10);
			else if(f=strstr(ligne,"maxwaterdepth="))		MaxWaterDepth=atoi(f+14);
			else if(f=strstr(ligne,"minwaterdepth="))	{
				MinWaterDepth=atoi(f+14);
				if(MaxWaterDepth==0)
					MaxWaterDepth=255;
				}
			else if(f=strstr(ligne,"energyuse="))			EnergyUse=atoi(f+10);
			else if(f=strstr(ligne,"buildtime="))
				BuildTime=atoi(f+10);
			else if(f=strstr(ligne,"workertime="))			WorkerTime=atoi(f+11);
			else if(f=strstr(ligne,"builder="))				Builder=(f[8]=='1');
			else if(f=strstr(ligne,"threed="))				ThreeD=(f[7]=='1');
			else if(f=strstr(ligne,"sightdistance="))		SightDistance=atoi(f+14)>>1;
			else if(f=strstr(ligne,"radardistance="))		RadarDistance=atoi(f+14)>>1;
			else if(f=strstr(ligne,"radardistancejam="))	RadarDistanceJam=atoi(f+17)>>1;
			else if(f=strstr(ligne,"soundcategory=")) {
				if(strstr(f,";"))
					*(strstr(f,";"))=0;
				soundcategory = strdup(f + 14);
				}
			else if(f=strstr(ligne,"wthi_badtargetcategory=")) {
				if( w_badTargetCategory[2] )	free( w_badTargetCategory[2] );		// To prevent memory leaks
				while( f[23] == ' ' )	f++;
				w_badTargetCategory[2] = strdup( f + 23 );
				}
			else if(f=strstr(ligne,"wsec_badtargetcategory=")) {
				if( w_badTargetCategory[1] )	free( w_badTargetCategory[1] );		// To prevent memory leaks
				while( f[23] == ' ' )	f++;
				w_badTargetCategory[1] = strdup( f + 23 );
				}
			else if(f=strstr(ligne,"wpri_badtargetcategory=")) {
				if( w_badTargetCategory[0] )	free( w_badTargetCategory[0] );		// To prevent memory leaks
				while( f[23] == ' ' )	f++;
				w_badTargetCategory[0] = strdup( f + 23 );
				}
			else if(f=strstr(ligne,"nochasecategory=")) {
				if( NoChaseCategory )	free( NoChaseCategory );		// To prevent memory leaks
				while( f[17] == ' ' )	f++;
				NoChaseCategory = strdup( f + 17 );
				}
			else if(f=strstr(ligne,"badtargetcategory=")) {
				if( BadTargetCategory )	free( BadTargetCategory );		// To prevent memory leaks
				while( f[18] == ' ' )	f++;
				BadTargetCategory = strdup( f + 18 );
				}
			else if(f=strstr(ligne,"category=")) {
				if( Category )	free( Category );		// To prevent memory leaks
				while( f[9] == ' ' )	f++;
				Category = strdup( f + 9 );
				fastCategory = 0;
				if( checkCategory( "kamikaze" ) )	fastCategory |= CATEGORY_KAMIKAZE;
				if( checkCategory( "notair" ) )		fastCategory |= CATEGORY_NOTAIR;
				if( checkCategory( "notsub" ) )		fastCategory |= CATEGORY_NOTSUB;
				if( checkCategory( "jam" ) )		fastCategory |= CATEGORY_JAM;
				if( checkCategory( "commander" ) )	fastCategory |= CATEGORY_COMMANDER;
				if( checkCategory( "weapon" ) )		fastCategory |= CATEGORY_WEAPON;
				if( checkCategory( "level3" ) )		fastCategory |= CATEGORY_LEVEL3;
				}
			else if(f=strstr(ligne,"unitnumber="))			UnitNumber=atoi(f+14);
			else if(f=strstr(ligne,"canmove="))				canmove=(f[8]=='1');
			else if(f=strstr(ligne,"canpatrol="))			canpatrol=(f[10]=='1');
			else if(f=strstr(ligne,"canstop="))				canstop=(f[8]=='1');
			else if(f=strstr(ligne,"canguard="))			canguard=(f[9]=='1');
			else if(f=strstr(ligne,"maxvelocity="))			MaxVelocity = atof(f+12)*16.0f;
			else if(f=strstr(ligne,"brakerate="))			BrakeRate=atof(f+10)*160.0f;
			else if(f=strstr(ligne,"acceleration="))		Acceleration=atof(f+13)*160.0f;
			else if(f=strstr(ligne,"turnrate="))			TurnRate = atof(f+9) * TA2DEG * 20.0f;
			else if(f=strstr(ligne,"candgun="))				candgun=(f[8]=='1');
			else if(f=strstr(ligne,"canattack="))			canattack=(f[10]=='1');
			else if(f=strstr(ligne,"canreclamate="))		CanReclamate=(f[13]=='1');
			else if(f=strstr(ligne,"energymake="))			EnergyMake=atoi(f+11);
			else if(f=strstr(ligne,"metalmake="))			MetalMake=atof(f+10);
			else if(f=strstr(ligne,"cancapture="))			CanCapture=(f[11]=='1');
			else if(f=strstr(ligne,"hidedamage="))			HideDamage=(f[11]=='1');
			else if(f=strstr(ligne,"healtime="))			HealTime=atoi(f+9)*30;		// To have it in seconds
			else if(f=strstr(ligne,"cloakcost="))			CloakCost=atoi(f+10);
			else if(f=strstr(ligne,"cloakcostmoving="))		CloakCostMoving=atoi(f+16);
			else if(f=strstr(ligne,"builddistance="))		BuildDistance=atoi(f+14);
			else if(f=strstr(ligne,"activatewhenbuilt="))	ActivateWhenBuilt=(f[18]=='1');
			else if(f=strstr(ligne,"immunetoparalyzer="))	ImmuneToParalyzer=(f[18]=='1');
			else if(f=strstr(ligne,"sonardistance="))		SonarDistance=atoi(f+14)>>1;
			else if(f=strstr(ligne,"sonardistancejam="))	SonarDistanceJam=atoi(f+17)>>1;
			else if(f=strstr(ligne,"copyright=")) {}
			else if(f=strstr(ligne,"maxslope="))			MaxSlope=atoi(f+9);
			else if(f=strstr(ligne,"steeringmode="))		SteeringMode=atoi(f+13);
			else if(f=strstr(ligne,"bmcode="))				BMcode=atoi(f+7);
			else if(f=strstr(ligne,"zbuffer=")) {}
			else if(f=strstr(ligne,"shootme="))				ShootMe=(f[8]=='1');
			else if(f=strstr(ligne,"upright="))				Upright=(f[8]=='1');
			else if(f=strstr(ligne,"norestrict="))			norestrict=(f[11]=='1');
			else if(f=strstr(ligne,"noautofire="))			AutoFire=(f[11]!='1');
			else if(f=strstr(ligne,"energystorage="))		EnergyStorage=atoi(f+14);
			else if(f=strstr(ligne,"metalstorage="))		MetalStorage=atoi(f+13);
			else if(f=strstr(ligne,"standingmoveorder="))	StandingMoveOrder=atoi(f+18);
			else if(f=strstr(ligne,"mobilestandorders="))	MobileStandOrders=atoi(f+18);
			else if(f=strstr(ligne,"standingfireorder="))	StandingFireOrder=atoi(f+18);
			else if(f=strstr(ligne,"firestandorders="))		FireStandOrders=atoi(f+16);
			else if(f=strstr(ligne,"waterline="))			WaterLine=atof(f+10);
			else if(f=strstr(ligne,"tedclass=")) {
				if(strstr(f,"water"))			TEDclass=CLASS_WATER;
				else if(strstr(f,"ship"))		TEDclass=CLASS_SHIP;
				else if(strstr(f,"energy"))		TEDclass=CLASS_ENERGY;
				else if(strstr(f,"vtol"))		TEDclass=CLASS_VTOL;
				else if(strstr(f,"kbot"))		TEDclass=CLASS_KBOT;
				else if(strstr(f,"plant"))		TEDclass=CLASS_PLANT;
				else if(strstr(f,"tank"))		TEDclass=CLASS_TANK;
				else if(strstr(f,"special"))	TEDclass=CLASS_SPECIAL;
				else if(strstr(f,"fort"))		TEDclass=CLASS_FORT;
				else if(strstr(f,"metal"))		TEDclass=CLASS_METAL;
				else if(strstr(f,"cnstr"))		TEDclass=CLASS_CNSTR;
				else if(strstr(f,"commander"))	TEDclass=CLASS_COMMANDER;
				else {
					printf("->tedclass id inconnu : %s\n",f);
					nb_inconnu++;
					}
				}
			else if(f=strstr(ligne,"noshadow="))			NoShadow=(f[9]=='1');
			else if(f=strstr(ligne,"antiweapons="))			antiweapons=(f[12]=='1');
			else if(f=strstr(ligne,"buildangle="))			BuildAngle=atoi(f+11);
			else if(f=strstr(ligne,"canfly="))				canfly=(f[7]=='1');
			else if(f=strstr(ligne,"canload="))				canload=(f[8]=='1');
			else if(f=strstr(ligne,"floater="))				Floater=(f[8]=='1');
			else if(f=strstr(ligne,"canhover="))			canhover=(f[9]=='1');
			else if(f=strstr(ligne,"bankscale="))			BankScale=atoi(f+10);
			else if(f=strstr(ligne,"tidalgenerator="))		TidalGenerator=(f[15]=='1');
			else if(f=strstr(ligne,"scale="))				Scale=1.0f;
			else if(f=strstr(ligne,"corpse=")) {
				char *nom=strdup(f+7);
				if(strstr(nom,";"))
					*(strstr(nom,";"))=0;
				Corpse=strdup(nom);
				free(nom);
				}
			else if(f=strstr(ligne,"windgenerator="))
				WindGenerator=atoi(f+14);
			else if(f=strstr(ligne,"onoffable="))			onoffable=(f[10]=='1');
			else if(f=strstr(ligne,"kamikaze="))			kamikaze=(f[9]=='1');
			else if(f=strstr(ligne,"kamikazedistance="))	kamikazedistance=atoi(f+17)>>1;
			else if(f=strstr(ligne,"weapon1=")) {
				char *weaponname=strdup(f+8);
				if(strstr(weaponname,";"))
					*(strstr(weaponname,";"))=0;
				Weapon1=weapon_manager.get_weapon_index(weaponname);
				free(weaponname);
				}
			else if(f=strstr(ligne,"weapon2=")) {
				char *weaponname=strdup(f+8);
				if(strstr(weaponname,";"))
					*(strstr(weaponname,";"))=0;
				Weapon2=weapon_manager.get_weapon_index(weaponname);
				free(weaponname);
				}
			else if(f=strstr(ligne,"weapon3=")) {
				char *weaponname=strdup(f+8);
				if(strstr(weaponname,";"))
					*(strstr(weaponname,";"))=0;
				Weapon3=weapon_manager.get_weapon_index(weaponname);
				free(weaponname);
				}
			else if(f=strstr(ligne,"yardmap="))	{
				f=strstr(dup_ligne,"=");
				if(strstr(f+1,";"))
					*(strstr(f+1,";"))=0;
				while(strstr(f," ")) {
					char *fm=strstr(f," ");
					memmove(fm,fm+1,strlen(fm+1)+1);
					}
				yardmap=strdup(f+1);
				}
			else if(f=strstr(ligne,"cruisealt="))			CruiseAlt=atoi(f+10);
			else if(f=strstr(ligne,"explodeas=")) {
				ExplodeAs=strdup(f+10);
				if(strstr(ExplodeAs,";"))
					*(strstr(ExplodeAs,";"))=0;
				}
			else if(f=strstr(ligne,"selfdestructas=")) {
				SelfDestructAs=strdup(f+15);
				if(strstr(SelfDestructAs,";"))
					*(strstr(SelfDestructAs,";"))=0;
				}
			else if(f=strstr(ligne,"maneuverleashlength="))	ManeuverLeashLength=atoi(f+20);
			else if(f=strstr(ligne,"defaultmissiontype=")) {
				if(strstr(f,"=standby;"))				DefaultMissionType=MISSION_STANDBY;
				else if(strstr(f,"=vtol_standby;"))		DefaultMissionType=MISSION_VTOL_STANDBY;
				else if(strstr(f,"=guard_nomove;"))		DefaultMissionType=MISSION_GUARD_NOMOVE;
				else if(strstr(f,"=standby_mine;"))		DefaultMissionType=MISSION_STANDBY_MINE;
				else {
					Console->AddEntry("->Unknown constant : %s\n",f);
					nb_inconnu++;
					}
				}
			else if(f=strstr(ligne,"transmaxunits="))		TransMaxUnits=TransportMaxUnits=atoi(f+14);
			else if(f=strstr(ligne,"transportmaxunits="))	TransMaxUnits=TransportMaxUnits=atoi(f+18);
			else if(f=strstr(ligne,"transportcapacity="))	TransportCapacity=atoi(f+18);
			else if(f=strstr(ligne,"transportsize="))		TransportSize=atoi(f+14);
			else if(f=strstr(ligne,"altfromsealevel="))		AltFromSeaLevel=atoi(f+16);
			else if(f=strstr(ligne,"movementclass=")) {
				MovementClass = strdup(f+14);
				if( strstr( MovementClass, ";") )
					*strstr( MovementClass, ";") = 0;
				}
			else if(f=strstr(ligne,"isairbase="))			IsAirBase=(f[10]=='1');
			else if(f=strstr(ligne,"commander="))			commander=(f[10]=='1');
			else if(f=strstr(ligne,"damagemodifier="))		DamageModifier=atof(f+15);
			else if(f=strstr(ligne,"makesmetal="))			MakesMetal=atof(f+11);
			else if(f=strstr(ligne,"sortbias="))			SortBias=atoi(f+9);
			else if(f=strstr(ligne,"extractsmetal="))		ExtractsMetal=atof(f+14);
			else if(f=strstr(ligne,"hoverattack="))			hoverattack=(f[12]=='1');
			else if(f=strstr(ligne,"isfeature="))			isfeature=(f[10]=='1');
			else if(f=strstr(ligne,"stealth="))				Stealth=atoi(f+8);
			else if(f=strstr(ligne,"attackrunlength="))		attackrunlength = atoi(f+16);
			else if(f=strstr(ligne,"selfdestructcountdown="))	selfdestructcountdown=atoi(f+22);
			else if(f=strstr(ligne,"downloadable=")) { }
			else if(f=strstr(ligne,"ovradjust=")) { }
			else if(f=strstr(ligne,"ai_limit=")) { }
			else if(f=strstr(ligne,"ai_weight=")) { }
			if(f==NULL && strstr(ligne,"}")==NULL && strlen( ligne ) > 0 ) {
				Console->AddEntry("FBI unknown variable: %s",ligne);
				nb_inconnu++;
				}
			if(dup_ligne)
				free(dup_ligne);
		}while(strstr(ligne,"}")==NULL && nb<1000 && pos<limit);
		delete[] ligne;
		if(Weapon1>-1)	{
			weapon[0]=&(weapon_manager.weapon[Weapon1]);
			weapon_damage[0] = weapon[0]->get_damage_for_unit( Unitname );
			}
		if(Weapon2>-1)	{
			weapon[1]=&(weapon_manager.weapon[Weapon2]);
			weapon_damage[1] = weapon[1]->get_damage_for_unit( Unitname );
			}
		if(Weapon3>-1)	{
			weapon[2]=&(weapon_manager.weapon[Weapon3]);
			weapon_damage[2] = weapon[2]->get_damage_for_unit( Unitname );
			}
		if(Unitname) {
			model=model_manager.get_model(ObjectName);
			if(model==NULL)
				Console->AddEntry("%s sans modèle 3D!",Unitname);
			}
		else
			Console->AddEntry("attention: unité sans nom!");
		return nb_inconnu;
	}

void UNIT_MANAGER::destroy()
{
	if(nb_unit>0 && unit_type!=NULL) {
		for(int i=0;i<nb_unit;i++)
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
		cTAFileParser parser( ta3d_sidedata.guis_dir + player_side + "MAIN.GUI" );
		gaf_img = parser.PullAsString( "gadget0.panel" );
		}
	catch( ... )
	{
		Console->AddEntry("ERROR: Unable to load %s", (ta3d_sidedata.guis_dir + player_side + "MAIN.GUI").c_str() );
		return;
	}

	set_color_depth( 32 );
	if(g_useTextureCompression)
		allegro_gl_set_texture_format( GL_COMPRESSED_RGB_ARB );
	else
		allegro_gl_set_texture_format( GL_RGB8 );
	int w,h;
	GLuint panel_tex = read_gaf_img( "anims\\" + player_side + "main.gaf", gaf_img, &w, &h, true );
	if( panel_tex == 0 ) {
		List< String >	file_list;
		HPIManager->GetFilelist( "anims\\*.gaf", &file_list );
		for( List< String >::iterator i = file_list.begin() ; i != file_list.end() && panel_tex == 0 ; i++ )
			panel_tex = read_gaf_img( *i, gaf_img, &w, &h, true );
		}
	panel.set( panel_tex );
	panel.width = w;	panel.height = h;

	paneltop.set( read_gaf_img( "anims\\" + intgaf + ".gaf", "PANELTOP", &w, &h ) );
	paneltop.width = w;		paneltop.height = h;
	panelbottom.set( read_gaf_img( "anims\\" + intgaf + ".gaf", "PANELBOT", &w, &h ) );
	panelbottom.width = w;	panelbottom.height = h;
}

int UNIT_MANAGER::unit_build_menu(int index,int omb,float &dt,int dec_x, int dec_y, bool GUI)				// Affiche et gère le menu des unités
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	bool nothing = index < -1 || index>=nb_unit;

	gfx->ReInitTexSys();
	glColor4f(1.0f,1.0f,1.0f,0.75f);
	if( GUI ) {
		if( panel.tex && nothing )
			gfx->drawtexture( panel.tex, 0.0f, 128.0f, 128.0f, 128.0f + panel.height );

		if( paneltop.tex ) {
			gfx->drawtexture( paneltop.tex, 128.0f, 0.0f, 128.0f + paneltop.width, paneltop.height );
			for( int k = 0 ; 128 + paneltop.width + panelbottom.width * k < SCREEN_W ; k++ )
				gfx->drawtexture( panelbottom.tex, 128.0f + paneltop.width + k * panelbottom.width, 0.0f, 128.0f + paneltop.width + panelbottom.width * (k+1), panelbottom.height );
			}

		if( panelbottom.tex )
			for( int k = 0 ; 128 + panelbottom.width * k < SCREEN_W ; k++ )
				gfx->drawtexture( panelbottom.tex, 128.0f + k * panelbottom.width, SCREEN_H - panelbottom.height, 128.0f + panelbottom.width * (k+1), SCREEN_H );

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
	for(int i=6*page;i<min((int)unit_type[index].nb_unit,6*page+6);i++) {		// Affiche les différentes images d'unités constructibles
		if(unit_type[index].BuildList[i]==-1)					// Weapon construction!
			gfx->drawtexture(unit_type[index].PicList[i],(i&1)*64+dec_x,(i-6*page>>1)*64+dec_y,(i&1)*64+64+dec_x,64+(i-6*page>>1)*64+dec_y);
		else
			gfx->drawtexture(unit_type[unit_type[index].BuildList[i]].glpic,(i&1)*64+dec_x,(i-6*page>>1)*64+dec_y,(i&1)*64+64+dec_x,64+(i-6*page>>1)*64+dec_y);

		if(mouse_x>=(i&1)*64+dec_x && mouse_x<(i&1)*64+64+dec_x && mouse_y>=(i-6*page>>1)*64+dec_y && mouse_y<64+(i-6*page>>1)*64+dec_y) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			glColor4f(1.0f,1.0f,1.0f,0.75f);
			if(unit_type[index].BuildList[i]==-1)					// Weapon construction!
				gfx->drawtexture(unit_type[index].PicList[i],(i&1)*64+dec_x,(i-6*page>>1)*64+dec_y,(i&1)*64+64+dec_x,64+(i-6*page>>1)*64+dec_y);
			else
				gfx->drawtexture(unit_type[unit_type[index].BuildList[i]].glpic,(i&1)*64+dec_x,(i-6*page>>1)*64+dec_y,(i&1)*64+64+dec_x,64+(i-6*page>>1)*64+dec_y);
			glDisable(GL_BLEND);
			sel=unit_type[index].BuildList[i];
			if(sel==-1)
				sel=-2;
			}

		if( ( unit_type[index].BuildList[i] == unit_type[index].last_click
			|| ( unit_type[index].last_click == -2 && unit_type[index].BuildList[i] == -1 ) )
			&& unit_type[index].click_time > 0.0f ) {
			glEnable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0f,1.0f,1.0f,unit_type[index].click_time);
			int mx = (i&1)*64+dec_x;
			int my = (i-6*page>>1)*64+dec_y;
			gfx->rectfill( mx,my,mx+64,my+64 );
			glColor4f(1.0f,1.0f,0.0f,0.75f);
			gfx->line( mx, my+64*unit_type[index].click_time, mx+64, my+64*unit_type[index].click_time );
			gfx->line( mx, my+64*(1.0f-unit_type[index].click_time), mx+64, my+64*(1.0f-unit_type[index].click_time) );
			gfx->line( mx+64*unit_type[index].click_time, my, mx+64*unit_type[index].click_time, my+64 );
			gfx->line( mx+64*(1.0f-unit_type[index].click_time), my, mx+64*(1.0f-unit_type[index].click_time), my+64 );
			glColor4f(1.0f,1.0f,1.0f,0.75f);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
			}
		}
	glColor4f(1.0f,1.0f,1.0f,1.0f);
//	glDisable(GL_BLEND);

	if( unit_type[index].last_click != -1 )
		unit_type[index].click_time -= dt;

	if(sel>-1) {
		set_uformat(U_ASCII);
		gfx->print(gfx->normal_font,ta3d_sidedata.side_int_data[ players.side_view ].Name.x1,ta3d_sidedata.side_int_data[ players.side_view ].Name.y1,0.0f,0xFFFFFFFF, format("%s M:%d E:%d HP:%d",unit_type[sel].name,unit_type[sel].BuildCostMetal,unit_type[sel].BuildCostEnergy,unit_type[sel].MaxDamage) );

		if(unit_type[sel].Description)
			gfx->print(gfx->normal_font,ta3d_sidedata.side_int_data[ players.side_view ].Description.x1,ta3d_sidedata.side_int_data[ players.side_view ].Description.y1,0.0f,0xFFFFFFFF,format("%s",unit_type[sel].Description) );
		glDisable(GL_BLEND);
		set_uformat(U_UTF8);
		}

	if( sel != -1 && mouse_b == 1 && omb != 1 ) {		// Click !!
		unit_type[index].last_click = sel;
		unit_type[index].click_time = 0.5f;		// One sec animation;
		}

	return sel;
}

int load_all_units()
{
	unit_manager.init();
	int nb_inconnu=0;
	List<String> file_list;
	HPIManager->GetFilelist( ta3d_sidedata.unit_dir + "*" + ta3d_sidedata.unit_ext,&file_list);

	for(List<String>::iterator i=file_list.begin();i!=file_list.end();i++) {
		char *nom=strdup(strstr(i->c_str(),"\\")+1);			// Vérifie si l'unité n'est pas déjà chargée
		*(strstr(nom,"."))=0;
		strupr(nom);

		if(unit_manager.get_unit_index(nom)==-1) {
			uint32 file_size=0;
			byte *data=HPIManager->PullFromHPI(*i,&file_size);
			nb_inconnu+=unit_manager.load_unit(data,file_size);
			if(unit_manager.unit_type[unit_manager.nb_unit-1].Unitname) {
				String nom_pcx = String("unitpics\\")+String(unit_manager.unit_type[unit_manager.nb_unit-1].Unitname)+String(".pcx");
				byte *dat=HPIManager->PullFromHPI(nom_pcx);
				if(dat) {
					unit_manager.unit_type[unit_manager.nb_unit-1].unitpic=load_memory_pcx(dat,pal);
					if(unit_manager.unit_type[unit_manager.nb_unit-1].unitpic) {
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
					free(dat);
					}
				else
					unit_manager.unit_type[unit_manager.nb_unit-1].unitpic=NULL;
				}
			free(data);
			Console->AddEntry("loading %s", nom);
			}
		free(nom);
		allegro_gl_flip();
		}

	for(uint32 i=0;i<unit_manager.nb_unit;i++)
		unit_manager.load_script_file(unit_manager.unit_type[i].Unitname);

	unit_manager.Identify();

	unit_manager.gather_all_build_data();

					// Correct some data given in the FBI file using data from the moveinfo.tdf file
	cTAFileParser parser( ta3d_sidedata.gamedata_dir + "moveinfo.tdf" );
	int n = 0;
	while( parser.PullAsString( format( "CLASS%d.name", n ) ) != "" )
		n++;

	for(uint32 i=0;i<unit_manager.nb_unit;i++) {
		if( unit_manager.unit_type[ i ].MovementClass != NULL ) {
			for( int e = 0 ; e < n ; e++ )
				if( parser.PullAsString( format( "CLASS%d.name", e ) ) == Uppercase( unit_manager.unit_type[ i ].MovementClass ) ) {
					unit_manager.unit_type[ i ].FootprintX = parser.PullAsInt( format( "CLASS%d.footprintx", e ), unit_manager.unit_type[ i ].FootprintX );
					unit_manager.unit_type[ i ].FootprintZ = parser.PullAsInt( format( "CLASS%d.footprintz", e ), unit_manager.unit_type[ i ].FootprintZ );
					unit_manager.unit_type[ i ].MinWaterDepth = parser.PullAsInt( format( "CLASS%d.minwaterdepth", e ), unit_manager.unit_type[ i ].MinWaterDepth );
					unit_manager.unit_type[ i ].MaxWaterDepth = parser.PullAsInt( format( "CLASS%d.maxwaterdepth", e ), unit_manager.unit_type[ i ].MaxWaterDepth );
					unit_manager.unit_type[ i ].MaxSlope = parser.PullAsInt( format( "CLASS%d.maxslope", e ), unit_manager.unit_type[ i ].MaxSlope );
					break;
					}
			}
		}
	return nb_inconnu;
}
