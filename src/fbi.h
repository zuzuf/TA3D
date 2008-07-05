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
|                                         fbi.h                                      |
|  Ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers fbi du jeu totalannihilation qui sont les fichiers de données sur les |
| unités du jeu. Cela inclus les classes pour gérer les différents types d'unités et |
| le système de gestion de liens entre unités.                                       |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_UNIT
# define __CLASSE_UNIT

# include "scripts/cob.h" // Pour la gestion des scripts

# include "ingame/weapons/weapons.h"			// Pour la gestion des armes des unités
# include "ingame/sidedata.h"
# include <vector>
# include <list>

#define SFORDER_HOLD_FIRE		0x0
#define SFORDER_RETURN_FIRE		0x1
#define SFORDER_FIRE_AT_WILL	0x2

#define SMORDER_HOLD_POSITION	0x0
#define SMORDER_MOVE			0x1
#define SMORDER_ROAM			0x2

#define CATEGORY_COMMANDER			0x00001
#define CATEGORY_WEAPON				0x00002
#define CATEGORY_NOTAIR				0x00004
#define CATEGORY_NOTSUB				0x00008
#define CATEGORY_JAM				0x00010
#define CATEGORY_KAMIKAZE			0x00020
#define CATEGORY_LEVEL3				0x00040

#define CLASS_UNDEF			0x0000
#define CLASS_WATER			0x0001
#define CLASS_SHIP			0x0002
#define CLASS_ENERGY		0x0004
#define CLASS_VTOL			0x0008
#define CLASS_KBOT			0x0010
#define CLASS_PLANT			0x0020
#define CLASS_TANK			0x0040
#define CLASS_SPECIAL		0x0080
#define CLASS_FORT			0x0100
#define CLASS_METAL			0x0200
#define CLASS_COMMANDER		0x0400
#define CLASS_CNSTR			0x0800

#define MISSION_STANDBY			0x00		// Aucune mission
#define MISSION_VTOL_STANDBY	0x01
#define MISSION_GUARD_NOMOVE	0x02		// Patrouille immobile
#define MISSION_MOVE			0x03		// Déplacement de l'unité
#define MISSION_BUILD			0x04		// Création d'une unité
#define MISSION_BUILD_2			0x05		// Construction d'une unité
#define MISSION_STOP			0x06		// Arrêt des opérations en cours
#define MISSION_REPAIR			0x07		// Réparation d'une unité
#define MISSION_ATTACK			0x08		// Attaque une unité
#define MISSION_PATROL			0x09		// Patrouille
#define MISSION_GUARD			0x0A		// Surveille une unité
#define MISSION_RECLAIM			0x0B		// Récupère une unité/un cadavre
#define MISSION_LOAD			0x0C		// Load other units
#define MISSION_UNLOAD			0x0D		// Unload other units
#define MISSION_STANDBY_MINE	0x0E		// Mine mission, must explode when an enemy gets too close
#define MISSION_REVIVE			0x0F		// Resurrect a wreckage
#define MISSION_CAPTURE			0x10		// Capture an enemy unit
#define MISSION_GET_REPAIRED	0x20		// For aircrafts getting repaired by air repair pads

	// Specific campaign missions
#define MISSION_WAIT			0x21		// Wait for a specified time
#define MISSION_WAIT_ATTACKED	0x22		// Wait until a specified unit is attacked

#define MISSION_FLAG_AUTO		0x10000		// Mission is sent from UNIT::move so don't ignore it

using namespace TA3D;
using namespace TA3D::Interfaces;

class DL_DATA
{
public:
	short	dl_num;				// How many build pics
	short	*dl_x;
	short 	*dl_y;
	short	*dl_w;
	short	*dl_h;

	DL_DATA()
	{
		dl_num = 0;
		dl_x = dl_y = NULL;
		dl_w = dl_h = NULL;
	}
	
	~DL_DATA()
	{
		if( dl_x )	delete[] dl_x;
		if( dl_y )	delete[] dl_y;
		if( dl_w )	delete[] dl_w;
		if( dl_h )	delete[] dl_h;
	}
};

class UNIT_TYPE			// Structure pour la description des unités du jeu
{
public:
	SCRIPT	*script;			// Scripts de l'unité
	GLuint	glpic;				// Image de l'unité sous forme de texture OpenGl
	MODEL	*model;				// Modèle associé à l'unité
	BITMAP	*unitpic;			// Image de l'unité
	bool	isfeature;			// tell if we must turn this unit into a feature
	byte	SortBias;
	short	AltFromSeaLevel;
	bool	Builder;
	bool	ThreeD;
	char	*Unitname;
	byte	FootprintX;
	byte	FootprintZ;
	cHashTable< int >	*Category;
	std::vector< String >	*categories;
	uint32	fastCategory;
	short	MaxSlope;
	byte	BMcode;
	bool	ShootMe;
	bool	norestrict;
	byte	StandingMoveOrder;
	byte	MobileStandOrders;
	byte	StandingFireOrder;
	byte	FireStandOrders;
	float	WaterLine;
	int		TEDclass;
	int		BuildAngle;
	short	CruiseAlt;
	short	ManeuverLeashLength;
	byte	DefaultMissionType;
	byte	TransportSize;
	byte	TransportCapacity;
	bool	IsAirBase;
	bool	hoverattack;
	bool	canresurrect;		// Can this unit resurrect wreckages
	bool	commander;			// Is that a commander unit ?
	char	*name;				// Nom de l'unité
	byte	version;			// Version
	char	*side;				// Camp de l'unité
	char	*ObjectName;		// Nom du modèle 3D
	char	*Designation_Name;	// Nom visible de l'unité
	char	*Description;		// Description
	int		BuildCostEnergy;	// Energie nécessaire pour la construire
	int		BuildCostMetal;		// Metal nécessaire pour la construire
	int		MaxDamage;			// Points de dégats maximum que l'unité peut encaisser
	int		EnergyUse;			// Energie nécessaire pour faire quelque chose
	int		BuildTime;			// Temps de construction
	int		WorkerTime;			// Vitesse de construction
	bool	AutoFire;			// Tire automatique
	int		SightDistance;		// Distance maximale de vue de l'unité
	int		RadarDistance;		// Distance maximale de detection radar
	int		RadarDistanceJam;	// For Radar Jammers
	int		EnergyStorage;		// Quantité d'énergie stockable par l'unité
	int		MetalStorage;		// Quantité de metal stockable par l'unité
	char	*ExplodeAs;			// Type d'explosion lorsque l'unité est détruite
	char	*SelfDestructAs;	// Type d'explosion lors de l'autodestruction
	char	*Corpse;			// Restes de l'unité
	short	UnitNumber;			// ID de l'unité
	bool	canmove;			// Indique si l'unité peut bouger
	bool	canpatrol;			// si elle peut patrouiller
	bool	canstop;			// si elle peut s'arrêter
	bool	canguard;			// si elle peut garder une autre unité
	float	MaxVelocity;		// Vitesse maximale
	float	BrakeRate;			// Vitesse de freinage
	float	Acceleration;		// Accélération
	float	TurnRate;			// Vitesse de tournage
	byte	SteeringMode;
	bool	canfly;				// si l'unité peut voler
	float	Scale;				// Echelle
	byte	BankScale;
	float	BuildDistance;		// Distance maximale de construction
	bool	CanReclamate;		// si elle peut récupérer
	short	EnergyMake;			// Production d'énergie de l'unité
	float	MetalMake;			// Production de métal de l'unité
	char	*MovementClass;		// Type de mouvement
	bool	Upright;			// Si l'unité est debout
	int		Weapon1;			// Arme 1
	char	*BadTargetCategory;		// Type d'unité non attaquable
	float	DamageModifier;
	bool	canattack;			// Si l'unité peut attaquer
	bool	ActivateWhenBuilt;	// L'unité s'active lorsqu'elle est achevée
	bool	onoffable;			// (Dés)activable
	short	MaxWaterDepth;		// Profondeur maximale où l'unité peut aller
	short	MinWaterDepth;		// Profondeur minimale où l'unité peut aller
	bool	NoShadow;			// Si l'unité n'a pas d'ombre
	byte	TransMaxUnits;		// Maximum d'unités portables
	bool	canload;			// Si elle peut charger d'autres unités
	int		Weapon2;			// Arme 2
	char	*w_badTargetCategory[3];	// Unités non ciblable par les armes
	bool	Floater;			// Si l'unité flotte
	bool	canhover;			// For hovercrafts
	char	*NoChaseCategory;	// Type d'unité non chassable
	int		Weapon3;			// Arme 3
	int		SonarDistance;		// Portée du sonar
	int		SonarDistanceJam;	// For Sonar Jammers
	bool	candgun;			// si l'unité peut utiliser l'arme ravage
	int		CloakCost;			// Coût en energie pour rendre l'unité invisible
	int		CloakCostMoving;	// Idem mais quand l'unité bouge
	int		HealTime;			// Temps nécessaire à l'unité pour se réparer (cf commandeurs)
	bool	CanCapture;			// Si elle peut capturer d'autres unités
	bool	HideDamage;			// Cache la vie de l'unité aux autres joueurs
	bool	ImmuneToParalyzer;	// Immunisation
	bool	Stealth;
	float	MakesMetal;			// Si l'unité produit du métal
	float	ExtractsMetal;		// métal extrait par l'unité
	bool	TidalGenerator;		// Si l'unité est une centrale marée-motrice
	byte	TransportMaxUnits;	// Maximum d'unités transportables
	bool	kamikaze;			// Unité kamikaze
	uint16	kamikazedistance;	// Maximal distance from its target before self-destructing
	short	WindGenerator;		// Centrale de type Eolienne
	char	*yardmap;			// To tell where the unit is on the map
	WEAPON_DEF	*weapon[3];		// Weapons
	uint32	weapon_damage[3];	// Damage made by weapons fired from this unit
	int		attackrunlength;	// Distance à laquelle l'unité commence une attaque (bombardiers)
	bool	antiweapons;
	bool	emitting_points_computed;	// Just to test if we need to get emitting point from script
	uint8	selfdestructcountdown;
	bool	init_cloaked;
	int		mincloakdistance;

/*-----------------------------------------------------------------------*/

	char	*soundcategory;		// Category of sounds to play for that unit

/*-----------------------------------------------------------------------*/

	short	nb_unit;			// Nombre d'unités que cette unité peut construire
	short	*BuildList;			// Liste des unités que cette unité peut construire
	short	*Pic_x;				// Coordinates
	short	*Pic_y;
	short	*Pic_w;				// Size
	short	*Pic_h;
	short	*Pic_p;				// Page where the pic has to be shown
	GLuint	*PicList;
	short	nb_pages;

	DL_DATA *dl_data;
	
/*-----------------------------------------------------------------------*/

	byte	page;				// Pour le menu de construction
	float	click_time;			// To have a nice animation when click on a button :-)
	sint16	last_click;			// What was clicked

/*-----------------------------------------------------------------------*/

	bool	not_used;			// Do we have the right to use this unit ? (campaign mode)

	#define SWAP( a, b ) { sint32 tmp = a; a = b; b = tmp; }

	void AddUnitBuild(int index, int px, int py, int pw, int ph, int p, GLuint Pic = 0 );

	void FixBuild();

	inline bool canbuild(int index)
	{
		for(int i=0;i<nb_unit;i++)
			if(BuildList[i]==index)
				return true;
		return false;
	}

	inline bool checkCategory( const char *cat )
	{
		if( Category == NULL || cat == NULL )	return false;
		return Category->Exists( Lowercase( cat ) );
	}

	inline void init()
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

	UNIT_TYPE()
	{
		init();
	}

	inline void destroy()
	{
		if(MovementClass)			free(MovementClass);
		if(soundcategory)			free(soundcategory);
		if(ExplodeAs) free(ExplodeAs);
		if(SelfDestructAs) free(SelfDestructAs);
		if(script) {
			script->destroy();
			free(script);
			}

		for( int i = 0 ; i < 3 ; i++ )
			if( w_badTargetCategory[i] )	free( w_badTargetCategory[i] );
		if( BadTargetCategory )			free( BadTargetCategory );
		if( NoChaseCategory )			free( NoChaseCategory );

		if(BuildList)		free(BuildList);
		if(Pic_x)		free(Pic_x);
		if(Pic_y)		free(Pic_y);
		if(Pic_w)		free(Pic_w);
		if(Pic_h)		free(Pic_h);
		if(Pic_p)		free(Pic_p);

		if(PicList) {
			for( int i = 0 ; i < nb_unit ; i++ )
				gfx->destroy_texture( PicList[i] );
			free(PicList);
			}

		if(yardmap)	free(yardmap);
		if(model)
			model=NULL;
		if(unitpic) {
			destroy_bitmap(unitpic);
			glDeleteTextures(1,&glpic);
			}
		if(Corpse)		free(Corpse);
		if(Unitname)		free(Unitname);
		if(name)		free(name);
		if(side)	free(side);
		if(ObjectName)	free(ObjectName);
		if(Designation_Name)	free(Designation_Name);
		if(Description)	free(Description);
		if(Category)	delete Category;
		if(categories)	delete categories;

		init();
	}

	~UNIT_TYPE()
	{
		destroy();
	}

private:
	inline char *get_line(char *data)
	{
		int pos=0;
		while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)	pos++;
		char *d=new char[pos+1];
		memcpy(d,data,pos);
		d[pos]=0;
		return d;
	}
public:

	int load(char *data,int size=99999999);

	void load_dl();

	void show_info(float fade,GfxFont fnt);
};

class UNIT_MANAGER			// Classe pour charger toutes les données relatives aux unités
{
public:
	int			nb_unit;		// Nombre d'unités
	UNIT_TYPE	*unit_type;		// Données sur l'unité

private:
	GfxTexture	panel;			// The texture used by the panel
	GfxTexture	paneltop,panelbottom;
	cHashTable< int >	unit_hashtable;		// hashtable used to speed up operations on UNIT_TYPE objects

public:

    std::list< DL_DATA* >		l_dl_data;		// To clean things at the end
	cHashTable< DL_DATA* >	h_dl_data;		// To speed things up
	
	inline void init()
	{
		nb_unit=0;
		unit_type=NULL;
		panel.init();
		paneltop.init();
		panelbottom.init();
	}

	UNIT_MANAGER() : unit_hashtable(), l_dl_data(), h_dl_data()
	{
		init();
	}

	void destroy();

	~UNIT_MANAGER()
	{
		destroy();
		unit_hashtable.EmptyHashTable();
		h_dl_data.EmptyHashTable();
	}

	void load_panel_texture( const String &player_side, const String &intgaf );

	inline int load_unit(byte *data,int size=9999999)			// Ajoute une nouvelle unité
	{
		UNIT_TYPE	*n_type=(UNIT_TYPE*) malloc(sizeof(UNIT_TYPE)*(nb_unit+1));
		int i;
		if(unit_type!=NULL) {
			for(i=0;i<nb_unit;i++)
				n_type[i]=unit_type[i];
			free(unit_type);
			}
		unit_type=n_type;
		unit_type[nb_unit].init();
		int result =  unit_type[nb_unit].load((char*)data,size);
		if( unit_type[ nb_unit ].Unitname )
			unit_hashtable.Insert( Lowercase( unit_type[ nb_unit ].Unitname ), nb_unit + 1 );
		if( unit_type[ nb_unit ].name )
			unit_hashtable.Insert( Lowercase( unit_type[ nb_unit ].name ), nb_unit + 1 );
		if( unit_type[ nb_unit ].ObjectName )
			unit_hashtable.Insert( Lowercase( unit_type[ nb_unit ].ObjectName ), nb_unit + 1 );
		if( unit_type[ nb_unit ].Description )
			unit_hashtable.Insert( Lowercase( unit_type[ nb_unit ].Description ), nb_unit + 1 );
		if( unit_type[ nb_unit ].Designation_Name )
			unit_hashtable.Insert( Lowercase( unit_type[ nb_unit ].Designation_Name ), nb_unit + 1 );
		nb_unit++;
		return result;
	}

	inline int get_unit_index(const char *unit_name)		// Cherche l'indice de l'unité unit_name dans la liste d'unités
	{
		if( unit_name )
			return unit_hashtable.Find( Lowercase( unit_name ) ) - 1;
		else
			return -1;
	}

private:
	inline char *get_line(char *data)
	{
		int pos=0;
		while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)	pos++;
		char *d=new char[pos+1];
		memcpy(d,data,pos);
		d[pos]=0;
		return d;
	}
public:

	void analyse(String filename,int unit_index);

	inline void analyse2(char *data,int size=9999999)
	{
		char *pos=data;
		char *ligne=NULL;
		char *limit=data+size;
		int nb=0;
		do {
			char *unitmenu=NULL;
			char *unitname=NULL;

			do
			{
				nb++;
				if(ligne)
					delete[] ligne;
				ligne=get_line(pos);
				strlwr(ligne);
				while(pos[0]!=0 && pos[0]!=13 && pos[0]!=10)	pos++;
				while(pos[0]==13 || pos[0]==10)	pos++;

				if(strstr(ligne,"unitmenu=")) {		// Obtient le nom de l'unité dont le menu doit être completé
					unitmenu=strstr(ligne,"unitmenu=")+9;
					if(strstr(unitmenu,";"))
						*(strstr(unitmenu,";"))=0;
					strupr(unitmenu);
					unitmenu=strdup(unitmenu);
					}
				if(strstr(ligne,"unitname=")) {		// Obtient le nom de l'unité à ajouter
					unitname=strstr(ligne,"unitname=")+9;
					if(strstr(unitname,";"))
						*(strstr(unitname,";"))=0;
					strupr(unitname);
					unitname=strdup(unitname);
					}

			}while(strstr(ligne,"}")==NULL && nb<2000 && data<limit);
			delete[] ligne;
			ligne=NULL;
			if(unitmenu==NULL || unitname==NULL) break;
			int unit_index=get_unit_index(unitmenu);
			if(unit_index==-1) continue;		// Au cas où l'unité n'existerait pas
			int idx=get_unit_index(unitname);
			if(idx>=0 && idx<nb_unit && unit_type[idx].unitpic)
				unit_type[unit_index].AddUnitBuild(idx, -1, -1, 64, 64, -1);
			}while(pos[0]=='[' && nb<2000 && data<limit);
	}

    inline void gather_build_data()
    {
        uint32 file_size=0;
        std::list<String> file_list;
        HPIManager->getFilelist( ta3dSideData.download_dir + "*.tdf", file_list);

        for(std::list<String>::iterator file=file_list.begin();file!=file_list.end();file++) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            byte *data=HPIManager->PullFromHPI(*file,&file_size);		// Lit le fichier

            if(data)
            {
                analyse2((char*)data,file_size);

                delete[] data;
            }
        }
    }

    inline void gather_all_build_data()
    {
        cTAFileParser sidedata_parser( ta3dSideData.gamedata_dir + "sidedata.tdf", false, true );
        for (short i = 0 ; i < nb_unit ; ++i)
        {
            int n = 1;
            String canbuild = sidedata_parser.PullAsString( Lowercase( format( "canbuild.%s.canbuild%d", unit_type[ i ].Unitname, n ) ) );
            while( canbuild != "" ) {
                int idx = get_unit_index( (char*)canbuild.c_str() );
                if(idx>=0 && idx<nb_unit && unit_type[idx].unitpic)
                    unit_type[ i ].AddUnitBuild(idx, -1, -1, 64, 64, -1);
                n++;
                canbuild = sidedata_parser.PullAsString( format( "canbuild.%s.canbuild%d", unit_type[ i ].Unitname, n ) );
            }
        }

        gather_build_data();			// Read additionnal build data

        std::list<String> file_list;
        HPIManager->getFilelist( ta3dSideData.guis_dir + "*.gui", file_list);

        for(std::list<String>::iterator file=file_list.begin();file!=file_list.end();file++) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            char *f=NULL;
            for (int i=0;i<nb_unit; ++i)
            {
                if ((f = strstr((char*)Uppercase(*file).c_str(), unit_type[i].Unitname)))
                    if(f[strlen(unit_type[i].Unitname)]=='.'
                       ||(f[strlen(unit_type[i].Unitname)]>='0' && f[strlen(unit_type[i].Unitname)]<='9'))
                        analyse(*file,i);
            }
        }

        for (int i = 0 ; i < nb_unit ; ++i)
            unit_type[i].FixBuild();
    }

    inline void load_script_file(char *unit_name)
    {
        strupr(unit_name);
        int unit_index=get_unit_index(unit_name);
        if(unit_index==-1) return;		// Au cas où l'unité n'existerait pas
        char *uprname=strdup(unit_name);
        strupr(uprname);

        std::list<String> file_list;
        HPIManager->getFilelist( format( "scripts\\%s.cob", unit_name ), file_list);

        for(std::list<String>::iterator file=file_list.begin();file!=file_list.end();file++) // Cherche un fichier pouvant contenir des informations sur l'unité unit_name
        {
            if(strstr(TA3D::Uppercase(*file).c_str(),uprname)) 	// A trouvé un fichier qui convient
            {
                byte *data=HPIManager->PullFromHPI(*file);		// Lit le fichier

                unit_type[unit_index].script=(SCRIPT*) malloc(sizeof(SCRIPT));
                unit_type[unit_index].script->init();
                unit_type[unit_index].script->load_cob(data);

                // Don't delete[] data here because the script keeps a reference to it.

                break;
            }
        }

        free(uprname);
    }

    int unit_build_menu(int index,int omb,float &dt,bool GUI=false);				// Affiche et gère le menu des unités

    inline void Identify()			// Identifie les pièces aux quelles les scripts font référence
    {
        for(int i=0;i<nb_unit;i++)
            if(unit_type[i].script && unit_type[i].model)
                unit_type[i].model->Identify(unit_type[i].script->nb_piece,unit_type[i].script->piece_name);
    }
};

int load_all_units(void (*progress)(float percent,const String &msg)=NULL);

extern UNIT_MANAGER unit_manager;

#endif
