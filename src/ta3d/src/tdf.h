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

# include <assert.h>
# include "stdafx.h"
# include "misc/string.h"
# include "threads/thread.h"
# include "gaf.h"
# include "gfx/particles/particles.h"
# include "mesh/mesh.h"
# include "misc/camera.h"
# include <list>
# include "EngineClass.h"
# include "misc/grid.h"
# include "misc/progressnotifier.h"


namespace TA3D
{

	// Forward declaration
	class MAP;






	/*!
	** \brief
	*/
	class Feature
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		Feature();
		//! Destructor
		~Feature();
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
		Grid<float> gRepulsion;
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
		Model	*model;
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

	}; // class Feature




	/*!
	** \brief
	*/
	class FeatureManager
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		FeatureManager();
		//! Destructor
		~FeatureManager();
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
		void load_tdf(File* data);

		/*!
		** \brief
		** \param name
		** \return The index of the feature (-1 means `not found`)
		*/
		int get_feature_index(const String &name);

		/*!
		** \brief returns a pointer to the feature at given index, index = -1 corresponds to no feature type
		** \param feature index
		** \return a pointer to the feature at index 'index' or NULL if index == -1
		*/
		Feature *getFeaturePointer(int index) const
		{
			if (index == -1)
				return NULL;
			assert(index >= 0 && index < (int)feature.size() && "Out of bounds");
			return feature[index];
		}

		inline int getNbFeatures() const
		{
			return nb_features;
		}

	private:
		//! Features' count
		int nb_features;
		//! All features
		std::vector<Feature*>   feature;

		//! A mutex to protect internal structures when loading data
		Mutex mInternals;

	private:
		//! hashtable used to speed up operations on Feature objects
		HashMap<int>::Dense  feature_hashtable;

	}; // class FeatureManager



	//!
	extern FeatureManager feature_manager;

	/*!
	** \brief
	*/
	void load_features(ProgressNotifier *progress = NULL);	// Charge tout les éléments


	/*! \class FeatureData
	**
	** \brief
	*/
	struct FeatureData
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

        //! Time reference to add some visual complexity to animations
        uint32 timeRef;

		//! Is this object referenced on map ?
		bool drawnOnMap;
	}; // class FeatureData






	/*!
	** \brief
	*/
	class Features : public ObjectSync	// Moteur de gestion des éléments graphiques
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		Features();
		//! Destructor
		virtual ~Features();
		//@}


		void init();
		void destroy(bool bInit = true);

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
		void move(const float dt, bool clean = true);

		/*!
		** \brief Simulates forest fires & tree reproduction (running in weapon thread,
		** because to be synced with the rest of the engine)
		**
		** \param dt Delta
		*/
		void move_forest(const float dt);

		/*!
		** \brief
		** \param no_flat
		*/
        void draw(float t, bool no_flat = false);

		/*!
		** \brief
		** \param dir
		*/
        void draw_shadow(float t, const Vector3D& Dir);

		/*!
		** \brief Draw icons for all features in symbolic_features
		*/
		void draw_icons();

		/*!
		** \brief
		** \param idx Index of the feature
		*/
		void display_info(const int idx) const;

		void resetListOfItemsToDisplay();

	public:
		//! \brief List of feature
		typedef std::vector<uint32> FeaturesList;
		//! \brief Set of feature
		typedef HashSet<uint32>::Dense FeaturesSet;

	public:
		//!
		int nb_features;		// Nombre d'éléments �  gérer
		//!
		int max_features;		// Quantité maximale d'éléments que l'on peut charger dans la mémoire allouée
		//!
		FeatureData* feature;			// Eléments

		//!
		FeaturesList  burning_features;	// because it's faster that way
		//!
		FeaturesList  sinking_features;	// because it's faster that way

		//!
		std::vector<int> list;				// Liste d'objets �  afficher

		//! features to render as icons in tactical mode
		FeaturesSet symbolic_features;

		//! icons
		Interfaces::GfxTexture icons[2];

	protected:
		//!
		Vector3D* p_wind_dir;

	}; // class Features




	extern Features features;





} // namespace TA3D


#endif // __TA3D_XX_TDF_H__
