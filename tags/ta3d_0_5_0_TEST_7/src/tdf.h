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
  |                               tdf.h                              |
  |   contient toutes les fonctions et classes permettant la gestion |
  | des fichiers TDF du jeu Total Annihilation qui contienne divers  |
  | éléments graphiques.                                             |
  \-----------------------------------------------------------------*/

#ifndef __TA3D_XX_TDF_H__
# define __TA3D_XX_TDF_H__

# include "threads/thread.h"
# include "gaf.h"
# include "gfx/particles/particles.h"
# include "3do.h"
# include "misc/camera.h"
# include <list>


namespace TA3D
{

    /*! \class FEATURE
    **
    ** \brief 
    */
    class FEATURE
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        FEATURE();
        //! Destructor
        ~FEATURE();
        //@}

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief
        */
        void convert();

    public:
        //!
        String	name;		// Nom
        //!
        String	world;
        //!
        String	description;
        //!
        String	category;

        //!
        bool	animating;
        //!
        int		footprintx;
        //!
        int		footprintz;
        //!
        int		height;
        //!
        String	filename;
        //!
        String	seqname;
        //!
        String	feature_dead;
        //!
        String	feature_burnt;
        //!
        String	feature_reclamate;
        //!
        bool	animtrans;
        //!
        bool	shadtrans;
        //!
        int		hitdensity;
        //!
        int		metal;
        //!
        int		energy;
        //!
        int		damage;
        //!
        bool	indestructible;
        //!
        Gaf::Animation anim;
        //!
        bool	vent;
        //!
        bool	m3d;
        //!
        MODEL	*model;
        //!
        bool	converted;		// Indique si l'objet a été converti en 3d depuis un sprite
        //!
        bool	reclaimable;
        //!
        bool	autoreclaimable;
        //!
        bool	blocking;
        //!
        bool	geothermal;

        //! \name Forest fires
        //@{
        //!
        bool	flamable;
        //!
        short	burnmin;
        //!
        short	burnmax;
        //!
        short	sparktime;			// Seems to be in seconds
        //!
        byte	spreadchance;
        //!
        String	burnweapon;
        //!
        bool	need_convert;
        //!
        bool	not_loaded;
        //@}

    }; // class FEATURE




    /*! \class FEATURE_MANAGER
    **
    ** \brief
    */
    class FEATURE_MANAGER
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        FEATURE_MANAGER();
        //! Destructor
        ~FEATURE_MANAGER();
        //@}

        /*!
        ** \brief Reset all data
        */
        void init();

        /*!
        ** \brief Release all data
        */
        void destroy();

        /*!
        ** \brief
        */
        void clean();

        /*!
        ** \brief
        ** \param name
        ** \return 
        */
        int add_feature(const String& name);


        /*!
        ** \brief Load a TDF file
        */
        void load_tdf(char* data, int size = 99999999);

        /*!
        ** \brief
        ** \param name
        ** \return The index of the feature (-1 means `not found`)
        */
        int get_feature_index(const String &name);

    public:
        //! Features' count
        int nb_features;
        //! All features
        FEATURE	*feature;

    private:
        //! hashtable used to speed up operations on FEATURE objects
        cHashTable<int>  feature_hashtable;

    private:
        char* get_line(const char *data);

    }; // class FEATURE_MANAGER



    //!
    extern FEATURE_MANAGER feature_manager;

    /*!
    ** \brief
    */
    void load_features(void (*progress)(float percent,const String& msg)=NULL);	// Charge tout les éléments


    /*! \class FEATURE_DATA
    **
    ** \brief
    */
    struct FEATURE_DATA
    {
        //! 
        Vector3D Pos;		// Position spatiale de l'élément
        //! 
        int	 type;		// Type d'élément
        //! 
        short frame;		// Pour l'animation
        //! 
        float dt;			// Pour la gestion du temps
        //! 
        float hp;
        //! 
        bool draw;		// Indique si l'objet est dessiné
        //! 
        bool grey;		// Tell if it is in the fog of war
        //! 
        float angle;		// Rotation angle to set orientation

        //! 
        bool burning;
        //! 
        float burning_time;
        //! 
        short time_to_burn;
        //! 
        uint32 px,py;
        //! Associated burning weapon
        sint32 BW_idx; 
        //! 
        byte weapon_counter;
        //! 
        float last_spread;

        //! Is that something sinking ?
        bool sinking; 
        //! 
        float dive_speed;
        //! 
        bool dive;
        //! Orientation angle
        float angle_x;

        //! Display list to speed up the shadow rendering
        GLuint shadow_dlist;
        //! 
        bool delete_shadow_dlist;

    }; // class FEATURE_DATA



    class MAP;




    /*! \class FEATURES
    **
    ** \brief 
    */
    class FEATURES : public ObjectSync	// Moteur de gestion des éléments graphiques
    {
       public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        FEATURES();
        //! Destructor
        ~FEATURES();
        //@}


        void init();
        void destroy();

        /*!
        ** \brief Set the wind direction
        ** \todo p_wind_dir should not be a pointer
        */
        void set_data(Vector3D& wind_dir) { p_wind_dir = &wind_dir; }

        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void compute_on_map_pos(const int idx);

        /*!
        **
        ** \param idx Index of the feature
        */
        void burn_feature(const int idx);
        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void sink_feature(const int idx);

        /*!
        ** \brief
        */
        int add_feature(const Vector3D& Pos, const int type);
        
        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void drawFeatureOnMap(const int idx);
        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void removeFeatureFromMap(const int idx);

        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void delete_feature(const int index);

        /*!
        ** \brief
        */
        void move(const float dt, MAP* map, bool clean = true);

        /*!
        ** \brief Simulates forest fires & tree reproduction (running in weapon thread,
        ** because to be synced with the rest of the engine)
        **
        ** \param dt Delta
        */
        void move_forest(const float dt);

        /*!
        ** \brief
        ** \param cam
        */
        void draw(Camera& cam);

        /*!
        ** \brief
        ** \param cam
        ** \param dir
        */
        void draw_shadow(Camera& cam, const Vector3D& Dir);

        /*!
        ** \brief
        ** \param idx Index of the feature
        */
        void display_info(const int idx) const;

        void resetListOfItemsToDisplay();

    public:
        //! \brief List of feature
        typedef std::list<uint32> FeaturesList;

    public:
        //!
        int nb_features;		// Nombre d'éléments à gérer
        //!
        int max_features;		// Quantité maximale d'éléments que l'on peut charger dans la mémoire allouée
        //!
        FEATURE_DATA* feature;			// Eléments

        //!
        int min_idx;			// Indices des premiers et derniers éléments du tableau à être affichés
        //!
        int max_idx;

        //!
        FeaturesList  burning_features;	// because it's faster that way
        //!
        FeaturesList  sinking_features;	// because it's faster that way

        //!
        int* list;				// Liste d'objets à afficher
        //!
        int list_size;

    protected:
        //!
        Vector3D* p_wind_dir;

    }; // class FEATURES

    extern FEATURES features;

} // namespace TA3D


#endif // __TA3D_XX_TDF_H__
