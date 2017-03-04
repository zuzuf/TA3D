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
|                                        mesh.h                                      |
|  This module contains the base class for all meshes. The base mesh class is a      |
| virtual class so that a mesh type can be implemented separately                    |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __TA3D_MESH_H__
#define __TA3D_MESH_H__

#include <misc/string.h>
#include <misc/hash_table.h>
#include <ta3dbase.h>
#include <gaf.h>
#include <vector>
#include <misc/matrix.h>
#include <gfx/glfunc.h>
#include <gfx/shader.h>
#include <scripts/script.data.h>
#include <misc/progressnotifier.h>

namespace TA3D
{

    /*--------------Classes pour l'animation---------------------------------------------*/

#define FLAG_HIDE               0x01
#define FLAG_WAIT_FOR_TURN      0x02
#define FLAG_NEED_COMPUTE       0x04
#define FLAG_EXPLODE            0x08
#define FLAG_ANIMATE            0x10
#define FLAG_ANIMATED_TEXTURE   0x20
#define FLAG_DONT_SHADE         0x40

    // a few things needed to handle explosions properly

#define EXPLODE_SHATTER                 1               // The piece will shatter instead of remaining whole
#define EXPLODE_EXPLODE_ON_HIT          2               // The piece will explode when it hits the ground
#define EXPLODE_FALL                    4               // The piece will fall due to gravity instead of just flying off
#define EXPLODE_SMOKE                   8               // A smoke trail will follow the piece through the air
#define EXPLODE_FIRE                    16              // A fire trail will follow the piece through the air
#define EXPLODE_BITMAPONLY              32              // The piece will not fly off or shatter or anything.  Only a bitmap explosion will be rendered.

#define EXPLODE_BITMAP1                 256
#define EXPLODE_BITMAP2                 512
#define EXPLODE_BITMAP3                 1024
#define EXPLODE_BITMAP4                 2048
#define EXPLODE_BITMAP5                 4096
#define EXPLODE_BITMAPNUKE              8192

	class Axe
    {
    public:
        float	move_speed;
        float	move_distance;
        float	rot_angle;
        float	rot_speed;
        float	rot_accel;
        float	angle;
        float	pos;
		float	rot_target_speed;
		float	explode_time;
		bool	rot_limit;
        bool	rot_speed_limit;
        bool	is_moving;

        void reset()
        {
            move_speed = 0.0f;
            move_distance = 0.0f;
            pos = 0.0f;
            rot_angle = 0.0f;
            rot_speed = 0.0f;
            rot_accel = 0.0f;
            angle = 0.0f;
            rot_limit = true;
            rot_speed_limit = false;
            rot_target_speed = 0.0f;
            is_moving = false;
        }

        void reset_move() {move_speed = 0.0f;}

        void reset_rot()
        {
            rot_angle = 0.0f;
            rot_accel = 0.0f;
            rot_limit = true;
            rot_speed_limit = false;
            rot_target_speed = 0.0f;
        }
    };

	class AnimationData
    {
	public:
		struct Data
		{
			Axe axe[3];				// 3 axis (in following order : x,y,z)
			Vector3D tpos;
			Vector3D pos;
			Vector3D dir;			// Object orientation (when there is a single line in the model part)
			Matrix matrix;			// Local matrix
			short flag;
			short explosion_flag;
		};
    public:
		int nb_piece;
		typedef std::vector<Data> DataVector;
		DataVector data;
		float explode_time;
		bool explode;
		bool is_moving;


		inline AnimationData() {init();}

        void init();

        void destroy();

		inline ~AnimationData() {destroy();}

        void load(const int nb);

        void move(const float dt,const float g = 9.81f);
    };

    /*-----------------------------------------------------------------------------------*/

#define ROTATION				0x01
#define ROTATION_PERIODIC		0x02
#define ROTATION_COSINE			0x04		// Default calculation is linear
#define TRANSLATION				0x10
#define TRANSLATION_PERIODIC	0x20
#define TRANSLATION_COSINE		0x40

#define MESH_TYPE_TRIANGLES         GL_TRIANGLES
#define MESH_TYPE_TRIANGLE_STRIP    GL_TRIANGLE_STRIP

    class Animation : public zuzuf::ref_count				// Class used to set default animation to a model, this animation will play if no AnimationData is provided (ie for map features)
    {
	public:
        typedef zuzuf::smartptr<Animation> Ptr;
    public:
        byte	type;
        Vector3D	angle_0;
        Vector3D	angle_1;
        float	angle_w;
        Vector3D	translate_0;
        Vector3D	translate_1;
        float	translate_w;

		Animation()
            :type(0), angle_w(0.), translate_w(0.)
        {}

		void animate(const float t, Vector3D &R, Vector3D& T) const;
    };

	class Mesh                          // The basic mesh class
    {
		friend class Joins;
	public:
		typedef Mesh* Ptr;
    protected:
        short       nb_vtx;				// Nombre de points
        short       nb_prim;			// Nombre de primitives
        QString      name;				// Nom de l'objet / Object name
		Ptr			next;				// Objet suivant / Next object
		Ptr			child;				// Objet fils / Child object
        Vector3D    *points;			// Points composant l'objet / Vertices
        short       nb_p_index;			// Nombre d'indices de points
        short       nb_l_index;			// Nombre d'indices de lignes
        short       nb_t_index;			// Nombre d'indices de triangles
        Vector3D    pos_from_parent;	// Position par rapport à l'objet parent
        GLushort    *p_index;			// Tableau d'indices pour les points isolés
        GLushort    *l_index;			// Tableau d'indices pour les lignes
        GLushort    *t_index;			// Tableau d'indices pour les triangles
        short       *nb_index;			// Nombre d'indices par primitive
        Vector3D    *N;					// Tableau de normales pour les sommet
        Vector3D    *F_N;				// face normals
        std::vector<GfxTexture::Ptr> gltex;      // Texture pour le dessin de l'objet
        QStringList tex_cache_name; // Used for on-the-fly loading
        float       *tcoord;			// Tableau de coordonnées de texture
        GLushort    sel[4];				// Primitive de sélection
		sint32      script_index;		// Indice donné par le script associé à l'unité
        bool        emitter;			// This object can or has sub-objects which can emit particles
        bool        emitter_point;		// This object directly emits particles
        std::vector<GLuint> gl_dlist;   // Display lists to speed up the drawing process

		Animation::Ptr	animation_data;

        sint16      selprim;			// Polygone de selection

    protected:

        GLushort    *shadow_index;		// Pour la géométrie du volume d'ombre
        short   *t_line;				// Repère les arêtes
        short   *line_v_idx[2];
		int		nb_line;
        byte    *line_on;
        byte    *face_reverse;
        GLuint  type;                   // Tell which type of geometric data we have here (triangles, quads, triangle strips)
        Vector3D    last_dir;			// To speed up things when shadow has already been cast

        float   min_x, max_x;		// Used by hit_fast
        float   min_y, max_y;
        float   min_z, max_z;

		sint32  obj_id;				// Used to generate a random position on the object
	public:
		uint32  nb_sub_obj;
	protected:

		uint16  last_nb_idx;			// Remember how many things we have drawn last time
		bool    compute_min_max;
		bool    fixed_textures;

    public:

        void check_textures();
		virtual void load_texture_id(int id);

		sint32 set_obj_id( sint32 id );

		void compute_coord(AnimationData *data_s = NULL,
                           Vector3D *pos = NULL,
						   const bool c_part = false,
						   const int p_tex = 0,
						   const Vector3D *target = NULL,
                           Vector3D *upos = NULL,
                           Matrix *M = NULL,
						   const float size = 0.0f,
						   const Vector3D *center = NULL,
						   const bool reverse = false,
						   const Mesh *src = NULL,
						   const AnimationData *src_data = NULL) const;

		virtual bool draw(float t, AnimationData *data_s = NULL, bool sel_primitive = false, bool alset = false, bool notex = false, int side = 0, bool chg_col = true, bool exploding_parts = false) = 0;
        virtual bool draw_nodl(bool alset = false) = 0;

		int hit(Vector3D Pos, Vector3D Dir, AnimationData *data_s, Vector3D *I, Matrix M) const;

		bool hit_fast(Vector3D Pos, Vector3D Dir, AnimationData *data_s, Vector3D *I);

		int random_pos(const AnimationData *data_s, const int id, Vector3D *vec) const;

        bool compute_emitter();

		bool compute_emitter_point(const int obj_idx);

		virtual ~Mesh() { }

        void destroy();

        void init();

		void Identify(ScriptData *script);			// Identifie les pièces utilisées par le script

		void compute_center(Vector3D *center, const Vector3D &dec, int *coef) const;		// Calcule les coordonnées du centre de l'objet, objets liés compris

		float compute_size_sq(const Vector3D &center) const;		// Carré de la taille(on fera une racine après)

        float print_struct(const float Y, const float X, TA3D::Font *fnt);

		float compute_top( float top, const Vector3D &dec ) const;

		float compute_bottom( float bottom, const Vector3D &dec ) const;

		virtual bool has_animation_data() const;

		// Used by the buildpic renderer
		void hideFlares();
    };

	class Model					// Classe pour la gestion des modèles 3D
    {
    public:
		Model() {init();}
		~Model() {destroy();}

        void init();
        void destroy();


        /*!
        ** \brief
        */
        void compute_topbottom();

        /*!
        ** \brief
        */
        void create_from_2d(QImage bmp, float w, float h, float max_h);

        /*!
        ** \brief
        */
		void draw(float t, AnimationData* data_s = NULL, bool sel = false, bool notex = false,
				  bool c_part = false, int p_tex = 0, const Vector3D *target = NULL, Vector3D* upos = NULL,
				  Matrix* M = NULL, float Size = 0.0f, const Vector3D* Center = NULL, bool reverse = false,
				  int side = 0, bool chg_col = true, Mesh* src = NULL, AnimationData* src_data = NULL);

        /*!
        ** \brief
        */
		void compute_coord(AnimationData* data_s = NULL, Matrix* M = NULL) const;

        /*!
        ** \brief
        */
		int hit(const Vector3D &Pos, const Vector3D &Dir, AnimationData* data_s, Vector3D* I, const Matrix& M) const
        { return mesh->hit(Pos,Dir,data_s,I,M); }

        /*!
        ** \brief
        */
		bool hit_fast(const Vector3D& Pos, const Vector3D& Dir, AnimationData* data_s, Vector3D* I, const Matrix& M) const
        { return mesh->hit_fast(Pos,Dir*M,data_s,I); }

        /*!
        ** \brief
        */
        void Identify(ScriptData *script)
        { mesh->Identify(script); }

        /*!
        ** \brief
        */
        void print_struct(const float Y, const float X, TA3D::Font *fnt)
        { mesh->print_struct(Y, X, fnt); }

        /*!
        ** \brief
        */
        void check_textures()
        { mesh->check_textures(); }

		/*!
		** \brief
		*/
		void hideFlares()
		{
			mesh->hideFlares();
		}

    public:
        void postLoadComputations();

    public:
		Mesh::Ptr  mesh;		// Objet principal du modèle 3D
        Vector3D  center;			// Centre de l'objet pour des calculs d'élimination d'objets
        float	size;				// Square of the size of the sphere which contains the model
        float	size2;				// Same as above but it is its square root
        float	top;				// Max y coordinate found in the model
        float	bottom;				// Min y coordinate found in the model
        uint32	id;					// ID of the model in the model array

        GLuint	dlist;				// Display list to speed up drawings operations when no position data is given (trees, ...)
		uint32	nb_obj;
		bool	animated;
        bool	from_2d;
		bool	useDL;				// Are display lists safe ?
    }; // class MODEL



	class ModelManager	// Classe pour la gestion des modèles 3D du jeu
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default Constructor
		ModelManager();
        //! Destructor
		~ModelManager();
        //@}

        void init();
        void destroy();

		Model *get_model(const QString& name);

        /*!
        ** \brief
        */
		int load_all(ProgressNotifier *progress);

        /*!
        ** \brief
        */
        void compute_ids();

        /*!
        ** \brief
        */
        void create_from_2d(QImage bmp, float w, float h, float max_h, const QString& filename);

    public:
        int	 nb_models;     // Number of models
        std::vector<Model*> model;       // Model array
        QStringList name; // Array containing model names

    private:
        //! hashtable used to speed up operations on MODEL objects
		HashMap<int>::Dense model_hashtable;
		Mutex mInternals;
    }; // class MODEL_MANAGER


	extern ModelManager	model_manager;

	class MeshTypeManager
	{
		friend class ModelManager;
	public:
		typedef Model* (*MeshLoader)(const QString &filename);
	private:
		static std::vector<MeshLoader> *lMeshLoader;
        static QStringList *lMeshExtension;
	public:
		static void registerMeshLoader(MeshLoader loader);
		static void registerMeshExtension(const QString &ext);
		static Model *load(const QString &filename);
        static void getMeshList(QStringList &filelist);
	}; // class Archive

	//! A simple macro to auto register Mesh classes functionnalities :) (you don't even need to include the headers \o/)
#define REGISTER_MESH_TYPE(x) \
	class __class_register_mesh__##x \
	{\
	public:\
		inline __class_register_mesh__##x() \
		{\
			TA3D::MeshTypeManager::registerMeshLoader( x::load );\
			TA3D::MeshTypeManager::registerMeshExtension( x::getExt() );\
		}\
	};\
	__class_register_mesh__##x __my__##x##__registerer;      // Instantiate an object to have it fully functionnal :)

} // namespace TA3D

#endif
