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

#ifndef __TA3D_XX_FBI_H__
# define __TA3D_XX_FBI_H__

# include "scripts/script.data.h"
# include <vector>
# include <map>
# include "ingame/weapons/weapons.h"
# include "ingame/sidedata.h"
# include "gfx/texture.h"
# include "misc/string.h"
# include "misc/grid.h"
# include "threads/thread.h"




# define SFORDER_HOLD_FIRE      0x0
# define SFORDER_RETURN_FIRE    0x1
# define SFORDER_FIRE_AT_WILL   0x2

# define SMORDER_HOLD_POSITION  0x0
# define SMORDER_MOVE           0x1
# define SMORDER_ROAM           0x2

# define CATEGORY_COMMANDER     0x00001
# define CATEGORY_WEAPON        0x00002
# define CATEGORY_NOTAIR        0x00004
# define CATEGORY_NOTSUB        0x00008
# define CATEGORY_JAM           0x00010
# define CATEGORY_KAMIKAZE      0x00020
# define CATEGORY_LEVEL3        0x00040

# define CLASS_UNDEF            0x0000
# define CLASS_WATER            0x0001
# define CLASS_SHIP             0x0002
# define CLASS_ENERGY           0x0004
# define CLASS_VTOL             0x0008
# define CLASS_KBOT             0x0010
# define CLASS_PLANT            0x0020
# define CLASS_TANK             0x0040
# define CLASS_SPECIAL          0x0080
# define CLASS_FORT             0x0100
# define CLASS_METAL            0x0200
# define CLASS_COMMANDER        0x0400
# define CLASS_CNSTR            0x0800



namespace TA3D
{


	struct DlDataPic
    {
		short x;
		short y;
		short w;
		short h;
	};

	typedef std::vector<DlDataPic>	DlData;

	struct AimData
    {
        Vector3D    dir;
        float       Maxangledif;
        bool        check;
    };



    class UnitType         // Structure pour la description des unités du jeu
    {
    public:
		ScriptData::Ptr script;	// Scripts de l'unité
		GLuint  glpic;					// Image de l'unité sous forme de texture OpenGl
		Model   *model;					// Modèle associé à l'unité
		SDL_Surface *unitpic;			// Image de l'unité / Unit picture
		bool    isfeature;				// tell if we must turn this unit into a feature
		byte    SortBias;
		short   AltFromSeaLevel;
		bool    Builder;
		bool    ThreeD;
		String  Unitname;
		byte    FootprintX;
		byte    FootprintZ;
		Grid<float> gRepulsion;
		HashSet<>::Dense Category;
		String::Vector categories;
		uint32  fastCategory;
		short   MaxSlope;
		byte    BMcode;
		bool    ShootMe;
		bool    norestrict;
		byte    StandingMoveOrder;
		byte    MobileStandOrders;
		byte    StandingFireOrder;
		byte    FireStandOrders;
		float   WaterLine;
		int     TEDclass;
		int     BuildAngle;
		short   CruiseAlt;
		short   ManeuverLeashLength;
		byte    DefaultMissionType;
		int     TransportSize;
		int     TransportCapacity;
		bool    IsAirBase;
		bool    hoverattack;
		bool    canresurrect;       // Can this unit resurrect wreckages
		bool    commander;          // Is that a commander unit ?
		String  name;              // Nom de l'unité
		byte    version;            // Version
		String  side;              // Camp de l'unité
		String  ObjectName;        // Nom du modèle 3D
		String  Designation_Name;  // Nom visible de l'unité
		String  Description;       // Description
		int     BuildCostEnergy;    // Energie nécessaire pour la construire
		int     BuildCostMetal;     // Metal nécessaire pour la construire
		int     MaxDamage;          // Points de dégats maximum que l'unité peut encaisser
		int     EnergyUse;          // Energie nécessaire pour faire quelque chose
		int     BuildTime;          // Temps de construction
		int     WorkerTime;         // Vitesse de construction
		bool    AutoFire;           // Tire automatique
		int     SightDistance;      // Distance maximale de vue de l'unité
		int     RadarDistance;      // Distance maximale de detection radar
		int     RadarDistanceJam;   // For Radar Jammers
		int     EnergyStorage;      // Quantité d'énergie stockable par l'unité
		int     MetalStorage;       // Quantité de metal stockable par l'unité
		String  ExplodeAs;         // Type d'explosion lorsque l'unité est détruite
		String  SelfDestructAs;    // Type d'explosion lors de l'autodestruction
		String  Corpse;            // Restes de l'unité
		short   UnitNumber;         // ID de l'unité
		bool    canmove;            // Indique si l'unité peut bouger
		bool    canpatrol;          // si elle peut patrouiller
		bool    canstop;            // si elle peut s'arrêter
		bool    canguard;           // si elle peut garder une autre unité
		float   MaxVelocity;        // Vitesse maximale
		float   BrakeRate;          // Vitesse de freinage
		float   Acceleration;       // Accélération
		float   TurnRate;           // Vitesse de tournage
		byte    SteeringMode;
		bool    canfly;             // si l'unité peut voler
		float   Scale;              // Echelle
		byte    BankScale;
		float   BuildDistance;      // Distance maximale de construction
		bool    CanReclamate;       // si elle peut récupérer
		short   EnergyMake;         // Production d'énergie de l'unité
		float   MetalMake;          // Production de métal de l'unité
		String  MovementClass;     // Type de mouvement
		bool    Upright;            // Si l'unité est debout
		std::vector<int>     WeaponID;            // Weapon IDs
		String  BadTargetCategory;     // Type d'unité non attaquable
		float   DamageModifier;
		bool    canattack;          // Si l'unité peut attaquer
		bool    ActivateWhenBuilt;  // L'unité s'active lorsqu'elle est achevée
		bool    onoffable;          // (Dés)activable
		short   MaxWaterDepth;      // Profondeur maximale où l'unité peut aller
		short   MinWaterDepth;      // Profondeur minimale où l'unité peut aller
		bool    NoShadow;           // Si l'unité n'a pas d'ombre
		bool    canload;            // Si elle peut charger d'autres unités
		String::Vector  w_badTargetCategory;    // Unités non ciblable par les armes
		bool    Floater;            // Si l'unité flotte
		bool    canhover;           // For hovercrafts
		String  NoChaseCategory;   // Type d'unité non chassable
		int     SonarDistance;      // Portée du sonar
		int     SonarDistanceJam;   // For Sonar Jammers
		bool    candgun;            // si l'unité peut utiliser l'arme ravage
		int     CloakCost;          // Coût en energie pour rendre l'unité invisible
		int     CloakCostMoving;    // Idem mais quand l'unité bouge
		int     HealTime;           // Temps nécessaire à l'unité pour se réparer (cf commandeurs)
		bool    CanCapture;         // Si elle peut capturer d'autres unités
		bool    HideDamage;         // Cache la vie de l'unité aux autres joueurs
		bool    ImmuneToParalyzer;  // Immunisation
		bool    Stealth;
		float   MakesMetal;         // Si l'unité produit du métal
		float   ExtractsMetal;      // métal extrait par l'unité
		bool    TidalGenerator;     // Si l'unité est une centrale marée-motrice
		int     TransportMaxUnits;  // Maximum d'unités transportables
		bool    kamikaze;           // Unité kamikaze
		uint16  kamikazedistance;   // Maximal distance from its target before self-destructing
		short   WindGenerator;      // Centrale de type Eolienne
		String  yardmap;           // To tell where the unit is on the map
		std::vector<WeaponDef*>  weapon;     // Weapons
		int     attackrunlength;    // Distance à laquelle l'unité commence une attaque (bombardiers)
		bool    antiweapons;
		bool    emitting_points_computed;   // Just to test if we need to get emitting point from script
		uint8   selfdestructcountdown;
		bool    init_cloaked;
		int     mincloakdistance;
		bool	bomber;				// Is this a bomber ?
		std::vector<AimData> aim_data;

        /*-----------------------------------------------------------------------*/

        String  soundcategory;     // Category of sounds to play for that unit

        /*-----------------------------------------------------------------------*/

        short   nb_unit;            // Nombre d'unités que cette unité peut construire
        std::vector<short>  BuildList;         // Liste des unités que cette unité peut construire
        std::vector<short>  Pic_x;             // Coordinates
        std::vector<short>  Pic_y;
        std::vector<short>  Pic_w;             // Size
        std::vector<short>  Pic_h;
        std::vector<short>  Pic_p;             // Page where the pic has to be shown
        std::vector<GLuint> PicList;
        short   nb_pages;

        DlData *dl_data;

        /*-----------------------------------------------------------------------*/

        byte    page;               // Pour le menu de construction
        float   click_time;         // To have a nice animation when click on a button :-)
        sint16  last_click;         // What was clicked

        /*-----------------------------------------------------------------------*/

        bool    not_used;           // Do we have the right to use this unit ? (campaign mode)

        /*-----------------------------------------------------------------------*/

        int     sweetspot_cached;

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
		void AddUnitBuild(int index, int px, int py, int pw, int ph, int p, GLuint Pic = 0);

        /*!
        ** \brief Can the current unit build unit 'index' ?
        ** \param index The index of the buildable unit
        */
        bool canBuild(const int index) const;

        /*!
        ** \brief Check if the unit belongs to the cat category
        ** \param cat The category to check
        */
		bool checkCategory(const String &cat) const
		{ return !cat.empty() && Category.count(ToLower(cat)) != 0; }

        /*!
        ** \brief Inits all the variables
        */
        void init();


        /*!
        ** \brief Constructor
        */
		UnitType();

        /*!
        ** \brief Free memory and destroy the data contained in the object
        */
        void destroy();

        /*!
        ** \brief Destructor
        */
        ~UnitType()
        {
            destroy();
        }

    public:

        /*!
        ** \brief Load units from file buffer data
        ** \param data The file buffer
        ** \param size File size
        */
        int load(const String &filename);

        /*!
        ** \brief Load data contained in `download/ *dl.tdf` files to build extra build menus
        */
        void load_dl();

        /*!
        ** \brief Everything is in the name ...
        */
		void show_info();

        /*!
        ** \brief Returns true if the units float on water
        */
		bool floatting() const;

		/*!
		** \brief Returns a String identifying the units movement properties
		*/
		String getMoveStringID() const;
    };

    class UnitManager          // Classe pour charger toutes les données relatives aux unités
    {
		friend class UnitDataLoader;
    public:
        typedef std::vector<UnitType*>  UnitList;
    public:
        int         nb_unit;        // Nombre d'unités
        UnitList  unit_type;     // Données sur l'unité
		String::Vector		animsList;		// the list of GAF files in anims/
		HashMap< String >::Dense name2gaf;	// a cache to speed up GAF search

    private:
        Interfaces::GfxTexture  panel;          // The texture used by the panel
        Interfaces::GfxTexture  paneltop,panelbottom;
		HashMap< int >::Dense   unit_hashtable;     // hashtable used to speed up operations on UnitType objects
		volatile bool ready;

		Mutex mInternals;

    public:

		HashMap< DlData* >::Dense      h_dl_data;      // To speed things up

		void init();

		UnitManager();

        void destroy();

        ~UnitManager()
        {
            destroy();
			unit_hashtable.clear();
			h_dl_data.clear();
        }

		void load_panel_texture( const String &intgaf );

		UnitType *load_unit(const String &filename);         // Ajoute une nouvelle unité

        inline int get_unit_index(const String &unit_name)        // Cherche l'indice de l'unité unit_name dans la liste d'unités
        {
			HashMap< int >::Dense::const_iterator it = unit_hashtable.find(ToLower(unit_name));
			if (it == unit_hashtable.end())
				return -1;
			return it.value();
        }

    private:
        inline char *get_line(char *data)
        {
            int pos=0;
            while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)   pos++;
            char *d=new char[pos+1];
            memcpy(d,data,pos);
            d[pos]=0;
            return d;
        }
    public:

		void analyse(const String &filename, const int unit_index);

		void analyse2(File *file);

        void gather_build_data();

		void start_threaded_stuffs();

		void gather_all_build_data();

        void load_script_file(const String &unit_name);

		void waitUntilReady() const;

		int unit_build_menu(int index,int omb,float &dt,int scrolling,bool GUI);                // Affiche et gère le menu des unités

        void Identify();            // Identifie les pièces aux quelles les scripts font référence

		int load_all_units(ProgressNotifier *progress = NULL);
	};

	class UnitDataLoader : public Thread
	{
		friend class UnitManager;
	protected:
		UnitDataLoader();

		virtual void proc(void*);
	};

    extern UnitManager unit_manager;


} // namespace TA3D

#endif // __TA3D_XX_FBI_H__
