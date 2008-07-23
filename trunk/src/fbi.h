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

    /*!
    ** \brief Add a unit to the list of units this unit can build
    ** \param index The index of the buildable unit
    ** \param px X coordinate in build menu
    ** \param py Y coordinate in build menu
    ** \param pw width
    ** \param ph height
    ** \param p menu ID
    ** \param Pic OpenGL texture ID
    */
	void AddUnitBuild(int index, int px, int py, int pw, int ph, int p, GLuint Pic = 0 );

    /*!
    ** \brief Fix conflicts in build menus
    */
	void FixBuild();

    /*!
    ** \brief Can the current unit build unit 'index' ?
    ** \param index The index of the buildable unit
    */
	inline bool canbuild(int index)
	{
		for(int i=0;i<nb_unit;i++)
			if(BuildList[i]==index)
				return true;
		return false;
	}

    /*!
    ** \brief Check if the unit belongs to the cat category
    ** \param cat The category to check
    */
	inline bool checkCategory( const char *cat )
	{
		if( Category == NULL || cat == NULL )	return false;
		return Category->exists(String::ToLower(cat));
	}

    /*!
    ** \brief Inits all the variables
    */
	void init();


    /*!
    ** \brief Constructor
    */
	UNIT_TYPE()
	{
		init();
	}

    /*!
    ** \brief Free memory and destroy the data contained in the object
    */
	void destroy();

    /*!
    ** \brief Destructor
    */
	~UNIT_TYPE()
	{
		destroy();
	}

private:
    /*!
    ** \brief Get next line of text from file buffer data
    */
	char *get_line(char *data);
public:

    /*!
    ** \brief Load units from file buffer data
    ** \param data The file buffer
    ** \param size File size
    */
	int load(char *data,int size=99999999);

    /*!
    ** \brief Load data contained in download/*dl.tdf files to build extra build menus
    */
	void load_dl();

    /*!
    ** \brief Everything is in the name ...
    */
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
		unit_hashtable.emptyHashTable();
		h_dl_data.emptyHashTable();
	}

	void load_panel_texture( const String &player_side, const String &intgaf );

	int load_unit(byte *data,int size=9999999);			// Ajoute une nouvelle unité

	inline int get_unit_index(const char *unit_name)		// Cherche l'indice de l'unité unit_name dans la liste d'unités
	{
		return ((unit_name) ? unit_hashtable.find(String::ToLower(unit_name)) - 1 : -1);
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

	void analyse2(char *data,int size=9999999);

    void gather_build_data();

    void gather_all_build_data();

    void load_script_file(char *unit_name);

    int unit_build_menu(int index,int omb,float &dt,bool GUI=false);				// Affiche et gère le menu des unités

    void Identify();			// Identifie les pièces aux quelles les scripts font référence
};

int load_all_units(void (*progress)(float percent,const String &msg)=NULL);

extern UNIT_MANAGER unit_manager;

#endif
