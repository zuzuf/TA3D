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

#ifndef __CLASSES_TDF

#define __CLASSES_TDF

#include "threads/thread.h"
#include "gaf.h"			// Pour la gestion des fichiers GAF
#include "gfx/particles/particles.h"		// Pour l'accès au moteur à particules
#include "3do.h"			// Pour pouvoir utiliser des modèles 3D
#include "misc/camera.h"
#include <list>


namespace TA3D
{


    class FEATURE
    {
    public:
        char	*name;		// Nom
        char	*world;
        char	*description;
        char	*category;
        bool	animating;
        int		footprintx;
        int		footprintz;
        int		height;
        char	*filename;
        char	*seqname;
        char	*feature_dead;
        char	*feature_burnt;
        char	*feature_reclamate;
        bool	animtrans;
        bool	shadtrans;
        int		hitdensity;
        int		metal;
        int		energy;
        int		damage;
        bool	indestructible;
        ANIM	anim;
        bool	vent;
        bool	m3d;
        MODEL	*model;
        bool	converted;		// Indique si l'objet a été converti en 3d depuis un sprite
        bool	reclaimable;
        bool	autoreclaimable;
        bool	blocking;
        bool	geothermal;

        // Following variables are used to handle forest fires

        bool	flamable;
        short	burnmin;
        short	burnmax;
        short	sparktime;			// Seems to be in seconds
        byte	spreadchance;
        char	*burnweapon;
        bool	need_convert;

        void init()
        {
            need_convert=true;
            flamable=false;
            burnmin = 0;
            burnmax = 0;
            sparktime = 0;
            spreadchance = 0;
            burnweapon = NULL;

            feature_burnt = NULL;
                feature_reclamate = NULL;
                feature_dead=NULL;
            geothermal=false;
            blocking=false;
            reclaimable=false;
            autoreclaimable=false;
            energy=0;
            converted=false;
            m3d=false;
            model=NULL;
            vent=false;
            name=NULL;
            world=NULL;
            description=NULL;
            category=NULL;
            animating=false;
            footprintx=0;
            footprintz=0;
            height=0;
            filename=NULL;
            seqname=NULL;
            animtrans=false;
            shadtrans=false;
            hitdensity=0;
            metal=0;
            damage=100;
            indestructible=false;
            anim.init();
        }

        FEATURE()
        {
            init();
        }

        void destroy()
        {
            if(burnweapon)			free(burnweapon);
            if(feature_reclamate)	free(feature_reclamate);
            if(feature_burnt)		free(feature_burnt);
            if(feature_dead)		free(feature_dead);
            if(name)				free(name);
            if(world)				free(world);
            if(description)			free(description);
            if(category)			free(category);
            if(filename)			free(filename);
            if(seqname)				free(seqname);
            anim.destroy();
            init();
        }

        ~FEATURE()
        {
            destroy();
        }

        inline void convert()
        {
            if( need_convert ) {
                need_convert = false;
                anim.convert(false,true);
                anim.clean();
            }
        }
    };

    class FEATURE_MANAGER
    {
    public:
        int		nb_features;
        FEATURE	*feature;

    private:
        cHashTable< int >	feature_hashtable;		// hashtable used to speed up operations on FEATURE objects

    public:

        void init()
        {
            nb_features=0;
            feature=NULL;
        }

        FEATURE_MANAGER() : feature_hashtable()
        {
            init();
        }

        void destroy()
        {
            if(nb_features>0 && feature)			// Détruit les éléments
                for(int i=0;i<nb_features;i++)
                    feature[i].destroy();
            if(feature)
                free(feature);

            feature_hashtable.EmptyHashTable();
            feature_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

            init();
        }

        ~FEATURE_MANAGER()
        {
            destroy();
            feature_hashtable.EmptyHashTable();
        }

        void clean()
        {
            if(feature)
            {
                for(int i = 0; i < nb_features; ++i)
                {
                    if (!feature[i].need_convert)
                        feature[i].anim.clean();
                }
            }
        }

        int add_feature(const String& name)			// Ajoute un élément
        {
            nb_features++;
            FEATURE *n_feature=(FEATURE*) malloc(sizeof(FEATURE)*nb_features);
            if (feature && nb_features > 1)
            {
                for(int i = 0;i < nb_features-1; ++i)
                    n_feature[i]=feature[i];
            }
            if (feature)
                free(feature);
            feature = n_feature;
            feature[nb_features-1].init();
            feature[nb_features-1].name = strdup(name.c_str());
            feature_hashtable.Insert(Lowercase(name), nb_features);
            return nb_features-1;
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

        void load_tdf(char *data,int size=99999999);					// Charge un fichier tdf

        int get_feature_index(const char *name)
        {
            if(name==NULL)	return -1;
            if(nb_features<=0)	return -1;
            if(name == NULL)	return -1;
            return feature_hashtable.Find( Lowercase( name ) ) - 1;
        }
    };

    extern FEATURE_MANAGER		feature_manager;

    void load_features(void (*progress)(float percent,const String &msg)=NULL);				// Charge tout les éléments

    struct FEATURE_DATA
    {
        VECTOR	Pos;		// Position spatiale de l'élément
        int		type;		// Type d'élément
        short	frame;		// Pour l'animation
        float	dt;			// Pour la gestion du temps
        float	hp;
        bool	draw;		// Indique si l'objet est dessiné
        bool	grey;		// Tell if it is in the fog of war
        float	angle;		// Rotation angle to set orientation

        bool	burning;
        float	burning_time;
        short	time_to_burn;
        uint32	px,py;
        sint32	BW_idx;		// Associated burning weapon
        byte	weapon_counter;
        float	last_spread;

        bool	sinking;	// Is that something sinking ?
        float	dive_speed;
        bool	dive;
        float	angle_x;	// Orientation angle

        GLuint	shadow_dlist;		// Display list to speed up the shadow rendering
        bool	delete_shadow_dlist;
    };

    class MAP;

    class FEATURES : public ObjectSync	// Moteur de gestion des éléments graphiques
    {
    public:
        int				nb_features;		// Nombre d'éléments à gérer
        int				max_features;		// Quantité maximale d'éléments que l'on peut charger dans la mémoire allouée
        FEATURE_DATA	*feature;			// Eléments
        int				min_idx;			// Indices des premiers et derniers éléments du tableau à être affichés
        int				max_idx;
        int				*list;				// Liste d'objets à afficher
        int				list_size;
        std::list<uint32>	burning_features;	// because it's faster that way
        std::list<uint32>	sinking_features;	// because it's faster that way

    protected:
        VECTOR	*p_wind_dir;
    public:

        inline void set_data( VECTOR &wind_dir )	{	p_wind_dir = &wind_dir;	}

        inline void init()
        {
            p_wind_dir = NULL;
            nb_features=0;
            max_features=0;
            feature=NULL;
            min_idx=0;
            max_idx=0;
            list=NULL;
            list_size=0;
        }

        FEATURES() : burning_features(), sinking_features()
        {
            init();
        }

        inline void destroy()
        {
            if(feature) {
                for( int i = 0 ; i < max_features ; i++ )
                    if( feature[ i ].shadow_dlist ) {
                        glDeleteLists( feature[i].shadow_dlist, 1 );
                        feature[ i ].shadow_dlist = 0;
                    }
                free(feature);
            }
            if(list)
                free(list);
            init();
            burning_features.clear();
            sinking_features.clear();
        }

        ~FEATURES()
        {
            destroy();
        }

        void compute_on_map_pos( int idx );

        void burn_feature( int idx );
        void sink_feature( int idx );

        int add_feature(VECTOR Pos,int type);
        
        void drawFeatureOnMap(int idx);
        void removeFeatureFromMap(int idx);

        void delete_feature(int index);

        void move(float dt,MAP *map,bool clean=true);

        void move_forest(const float &dt);			// Simulates forest fires & tree reproduction (running in weapon thread, because to be synced with the rest of the engine)

        void draw(Camera *cam);

        void draw_shadow(Camera *cam,VECTOR Dir);

        void display_info( int idx );
    };

    extern FEATURES features;

} // namespace TA3D

#endif
