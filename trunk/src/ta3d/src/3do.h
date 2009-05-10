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
|                                         3do.h                                      |
|  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers 3do de total annihilation qui sont les fichiers contenant les modèles |
| 3d des objets du jeu.                                                              |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_3DO
#define __CLASSE_3DO

# include "stdafx.h"
# include "misc/string.h"
# include "misc/hash_table.h"
# include "ta3dbase.h"
# include "gaf.h"
# include <vector>
# include <list>
# include <string.h>
# include "misc/matrix.h"
# include "gfx/glfunc.h"
# include "gfx/shader.h"
# include "scripts/script.data.h"

namespace TA3D
{
    namespace INSTANCING
    {
        extern bool water;
        extern float sealvl;
    };

    //	Classe pour la gestion des textures du jeu
    class TEXTURE_MANAGER
    {
    public:
        int	 nbtex;			// Nombre de textures
        Gaf::Animation* tex;			// Textures
        cHashTable<int> tex_hashtable;  // To speed up texture search

        TEXTURE_MANAGER() :nbtex(0), tex(NULL), tex_hashtable(__DEFAULT_HASH_TABLE_SIZE) {}
        ~TEXTURE_MANAGER() {destroy();}

        void init();

        void destroy();


        int get_texture_index(const String& texture_name);

        GLuint get_gl_texture(const String& texture_name, const int frame = 0);

        SDL_Surface *get_bmp_texture(const String& texture_name, const int frame = 0);

        int load_gaf(byte *data, bool logo);

        int all_texture();

    }; // class TEXTURE_MANAGER



    extern TEXTURE_MANAGER	texture_manager;

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

    class AXE
    {
    public:
        float	move_speed;
        float	move_distance;
        float	rot_angle;
        float	rot_speed;
        float	rot_accel;
        float	angle;
        float	pos;
        bool	rot_limit;
        bool	rot_speed_limit;
        float	rot_target_speed;
        bool	is_moving;
        float	explode_time;

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

    class ANIMATION_DATA
    {
    public:
        int         nb_piece;
        AXE	        *axe[3];			// 3 axes (dans l'ordre x,y,z)
        short       *flag;
        short       *explosion_flag;
        float       explode_time;
        bool        explode;
        Vector3D    *pos;
        Vector3D    *dir;			// Orientation des objets (quand il n'y a qu'une ligne)
        Matrix      *matrix;		// Store local matrixes
        bool        is_moving;


        ANIMATION_DATA() {init();}

        void init();

        void destroy();

        ~ANIMATION_DATA() {destroy();}

        void load(const int nb);

        const void move(const float dt,const float g = 9.81f);
    };

    /*-----------------------------------------------------------------------------------*/

#define SURFACE_ADVANCED		0x01		// Tell it is not a 3Do surface
#define	SURFACE_REFLEC			0x02		// Reflection
#define SURFACE_LIGHTED			0x04		// Lighting
#define SURFACE_TEXTURED		0x08		// Texturing
#define SURFACE_GOURAUD			0x10		// Gouraud shading
#define SURFACE_BLENDED			0x20		// Alpha Blending
#define SURFACE_PLAYER_COLOR	0x40		// The color is the owner's color
#define SURFACE_GLSL			0x80		// Use a shader to create a surface effect

	struct OBJECT_SURFACE
	{
	public:
		OBJECT_SURFACE()
			:Flag(0), NbTex(0), frag_shader_src(), vert_shader_src(), s_shader()
		{
			memset(&this->Color,  0, sizeof(float) * 4);
			memset(&this->RColor, 0, sizeof(float) * 4);
			memset(&this->gltex,  0, sizeof(GLuint) * 8);
		}

		~OBJECT_SURFACE() {}

		float	Color[4];
		float	RColor[4];
		uint32	Flag;
		sint8	NbTex;
		GLuint	gltex[8];
		String	frag_shader_src;
		String	vert_shader_src;
		Shader	s_shader;
	};

	struct tagObject				// Structure pour l'en-tête du fichier
	{
		tagObject()
		{
			// Set the whole structure to 0
			memset(this, 0, sizeof(tagObject));
		}
		~tagObject() {}

		int		VersionSignature;
		int		NumberOfVertexes;
		int		NumberOfPrimitives;
		int		OffsetToselectionPrimitive;
		int		XFromParent;
		int		YFromParent;
		int		ZFromParent;
		int		OffsetToObjectName;
		int		Always_0;
		int		OffsetToVertexArray;
		int		OffsetToPrimitiveArray;
		int		OffsetToSiblingObject;
		int		OffsetToChildObject;
	};

	struct tagPrimitive
	{
		tagPrimitive()
		{
			memset(this, 0, sizeof(tagPrimitive));
		}
		~tagPrimitive() {}

		int		ColorIndex;
		int		NumberOfVertexIndexes;
		int		Always_0;
		int		OffsetToVertexIndexArray;
		int		OffsetToTextureName;
		int		Unknown_1;
		int		Unknown_2;
		int		IsColored;
	};

	struct tagVertex		// Structure pour lire les coordonnées des points
	{
		tagVertex()
			:x(0), y(0), z(0)
		{}
		~tagVertex() {}
		int	x;
		int	y;
		int	z;
	};

#define ROTATION				0x01
#define ROTATION_PERIODIC		0x02
#define ROTATION_COSINE			0x04		// Default calculation is linear
#define TRANSLATION				0x10
#define TRANSLATION_PERIODIC	0x20
#define TRANSLATION_COSINE		0x40

    class ANIMATION				// Class used to set default animation to a model, this animation will play if no ANIMATION_DATA is provided (ie for map features)
    {
    public:
        byte	type;
        Vector3D	angle_0;
        Vector3D	angle_1;
        float	angle_w;
        Vector3D	translate_0;
        Vector3D	translate_1;
        float	translate_w;

        ANIMATION()
			:type(0), angle_w(0.), translate_w(0.)
        {}

        void animate( float &t, Vector3D &R, Vector3D& T);
    };

    class OBJECT					// Classe pour la gestion des (sous-)objets des modèles 3do
    {
    public:
        short       nb_vtx;				// Nombre de points
        short       nb_prim;			// Nombre de primitives
        String      name;				// Nom de l'objet / Object name
        OBJECT      *next;				// Objet suivant / Next object
        OBJECT      *child;				// Objet fils / Child object
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
        int	        *tex;				// Tableau de numéros de texture OpenGl
        byte        *usetex;			// Tableau indiquant si une texture doit être appliquée
        sint16      selprim;			// Polygone de selection
        std::vector<GLuint> gltex;      // Texture pour le dessin de l'objet
        String::Vector  tex_cache_name; // Used for on-the-fly loading
        uint8       dtex;				// Indique si une texture objet doit être détruite avec l'objet
        float       *tcoord;			// Tableau de coordonnées de texture
        GLushort    sel[4];				// Primitive de sélection
        sint16      script_index;		// Indice donné par le script associé à l'unité
        bool        emitter;			// This object can or has sub-objects which can emit particles
        bool        emitter_point;		// This object directly emits particles
        std::vector<GLuint> gl_dlist;   // Display lists to speed up the drawing process

        OBJECT_SURFACE  surface;		// Tell how this object must be drawn
        ANIMATION   *animation_data;

    private:

        GLushort    *shadow_index;		// Pour la géométrie du volume d'ombre
        short   *t_line;				// Repère les arêtes
        short   *line_v_idx[2];
        short   nb_line;
        byte    *line_on;
        byte    *face_reverse;
        bool    use_strips;				// Used by converted sprites to speed things up
        Vector3D    last_dir;				// To speed up things when shadow has already been cast
        uint16  last_nb_idx;				// Remember how many things we have drawn last time

        float   min_x, max_x;		// Used by hit_fast
        float   min_y, max_y;
        float   min_z, max_z;
        bool    compute_min_max;

        uint16  obj_id;				// Used to generate a random position on the object

        bool    fixed_textures;
    public:
        uint16  nb_sub_obj;
    private:

        //-------------- EXPERIMENTAL CODE -----------------

        bool        optimised;
        GLushort    *optimised_I;
        Vector3D    *optimised_P;
        Vector3D    *optimised_N;
        float       *optimised_T;
        uint16      optimised_nb_idx;
        uint16      optimised_nb_vtx;
        GLuint      vbo_id;
        GLuint      ebo_id;
        GLuint      N_offset;
        GLuint      T_offset;

        //---------- END OF EXPERIMENTAL CODE --------------

    private:
        bool coupe(int x1,int y1,int dx1,int dy1,int x2,int y2,int dx2,int dy2);

    public:

        void check_textures();
        void load_texture_id(int id);

        uint16 set_obj_id( uint16 id );

        void optimise_mesh();			// EXPERIMENTAL, function to merge all objects in one vertex array

        int load_obj(byte *data,int offset,int dec=0,const String &filename = String());

        void save_3dm(FILE *dst, bool compressed);

        byte *load_3dm(byte *data,const String &filename = String());

        void create_from_2d(SDL_Surface *bmp,float w,float h,float max_h);

        void compute_coord(ANIMATION_DATA *data_s=NULL,Vector3D *pos=NULL,bool c_part=false,int p_tex=0,Vector3D *target=NULL,Vector3D *upos=NULL,Matrix *M=NULL,float size=0.0f,Vector3D *center=NULL,bool reverse=false,OBJECT *src=NULL,ANIMATION_DATA *src_data=NULL);

        bool draw(float t,ANIMATION_DATA *data_s=NULL,bool sel_primitive=false,bool alset=false,bool notex=false,int side=0,bool chg_col=true,bool exploding_parts=false);
        bool draw_dl(ANIMATION_DATA *data_s=NULL,bool alset=false,int side=0,bool chg_col=true);
        void draw_optimised(const bool set = true);

        bool draw_shadow(Vector3D Dir,float t,ANIMATION_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

        bool draw_shadow_basic(Vector3D Dir,float t,ANIMATION_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

        int hit(Vector3D Pos,Vector3D Dir,ANIMATION_DATA *data_s,Vector3D *I,Matrix M);

        bool hit_fast(Vector3D Pos,Vector3D Dir,ANIMATION_DATA *data_s,Vector3D *I);

        int random_pos( ANIMATION_DATA *data_s, int id, Vector3D *vec );

        bool compute_emitter();

        bool compute_emitter_point(int &obj_idx);

        void init();

        OBJECT() {init();}

        void destroy();

        ~OBJECT() {destroy();}

        void Identify(ScriptData *script);			// Identifie les pièces utilisées par le script

        void compute_center(Vector3D *center,Vector3D dec, int *coef);		// Calcule les coordonnées du centre de l'objet, objets liés compris

        float compute_size_sq(Vector3D center);		// Carré de la taille(on fera une racine après)

        float print_struct(const float Y, const float X, TA3D::Font *fnt);

        float compute_top( float top, Vector3D dec );

        float compute_bottom( float bottom, Vector3D dec );

        bool has_animation_data();

    };

    class MODEL						// Classe pour la gestion des modèles 3D
    {
    public:
        MODEL() {init();}
        ~MODEL() {destroy();}

        void init();
        void destroy();


        /*!
        ** \brief
        */
        void load_asc(const String& filename, float size); // Charge un fichier au format *.ASC

        /*!
        ** \brief
        */
        void save_3dm(const String& filename, bool compressed); // Save the model to the 3DM format

        /*!
        ** \brief
        */
        void compute_topbottom();

        /*!
        ** \brief
        */
        void load_3dm(byte* data,const String &filename = String());	// Load a model in 3DM format

        /*!
        ** \brief
        */
        int load_3do(byte* data, const String &filename = String());

        /*!
        ** \brief
        */
        void create_from_2d(SDL_Surface* bmp, float w, float h, float max_h);

        /*!
        ** \brief
        */
        void draw(float t, ANIMATION_DATA* data_s = NULL, bool sel = false, bool notex = false,
                  bool c_part = false, int p_tex = 0, Vector3D *target = NULL, Vector3D* upos = NULL,
                  Matrix* M = NULL, float Size = 0.0f, Vector3D* Center = NULL, bool reverse = false,
                  int side = 0, bool chg_col = true, OBJECT* src = NULL, ANIMATION_DATA* src_data = NULL);

        /*!
        ** \brief
        */
        void draw_optimised(const bool set = true) {obj.draw_optimised(set);}

        /*!
        ** \brief
        */
        void compute_coord(ANIMATION_DATA* data_s = NULL, Matrix* M = NULL);

        /*!
        ** \brief
        */
        void draw_shadow(const Vector3D& Dir, float t,ANIMATION_DATA* data_s = NULL);

        /*!
        ** \brief
        */
        void draw_shadow_basic(const Vector3D& Dir, float t, ANIMATION_DATA *data_s = NULL);

        /*!
        ** \brief
        */
        int hit(Vector3D &Pos, Vector3D &Dir, ANIMATION_DATA* data_s, Vector3D* I, Matrix& M)
        { return obj.hit(Pos,Dir,data_s,I,M); }

        /*!
        ** \brief
        */
        bool hit_fast(Vector3D& Pos, Vector3D& Dir, ANIMATION_DATA* data_s, Vector3D* I, Matrix& M)
        { return obj.hit_fast(Pos,Dir*M,data_s,I); }

        /*!
        ** \brief
        */
        void Identify(ScriptData *script)
        { obj.Identify(script); }

        /*!
        ** \brief
        */
        void print_struct(const float Y, const float X, TA3D::Font *fnt)
        { obj.print_struct(Y, X, fnt); }

        /*!
        ** \brief
        */
        void check_textures()
        { obj.check_textures(); }

    public:
        OBJECT		obj;			// Objet principal du modèle 3D
        Vector3D	center;				// Centre de l'objet pour des calculs d'élimination d'objets
        float	size;				// Square of the size of the sphere which contains the model
        float	size2;				// Same as above but it is its square root
        float	top;				// Max y coordinate found in the model
        float	bottom;				// Min y coordinate found in the model
        uint32	id;					// ID of the model in the model array

        GLuint	dlist;				// Display list to speed up drawings operations when no position data is given (trees, ...)
        bool	animated;
        bool	from_2d;
        uint16	nb_obj;

    }; // class MODEL



    class MODEL_MANAGER	// Classe pour la gestion des modèles 3D du jeu
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default Constructor
        MODEL_MANAGER() :nb_models(0), model(NULL) {    isLoading = false; }
        //! Destructor
        ~MODEL_MANAGER();
        //@}

        void init();
        void destroy();

        MODEL *get_model(const String& name);

        /*!
        ** \brief
        */
        int load_all(void (*progress)(float percent,const String &msg)=NULL);

        /*!
        ** \brief
        */
        void compute_ids();

        /*!
        ** \brief
        */
        void optimise_all();

        /*!
        ** \brief
        */
        void create_from_2d(SDL_Surface *bmp, float w, float h, float max_h, const String& filename);

        /*!
        ** \brief we need this small function in order to process textures on-the-fly in game and at loading time in 3DMEditor
        */
        bool loading_all();

    public:
        int  max_models;    // Size of model array
        int	 nb_models;	 // Nombre de modèles
        MODEL* model; // Tableau de modèles
        String::Vector	name; // Tableau contenant les noms des modèles

    private:
        //! hashtable used to speed up operations on MODEL objects
        cHashTable<int> model_hashtable;
        bool isLoading;

    }; // class MODEL_MANAGER


    extern MODEL_MANAGER	model_manager;


    class Instance
    {
    public:
        Vector3D	pos;
        uint32	    col;
        float	    angle;

        Instance(const Vector3D &p, const uint32 &c, const float &ang)
            :pos(p), col(c), angle(ang)
        {}
    };

    class RenderQueue
    {
    public:
        std::vector<Instance>	queue;
        uint32				    model_id;

        RenderQueue(const uint32 m_id) :queue(), model_id(m_id) {}
        ~RenderQueue() {}

        void draw_queue();
    };

#define DrawingTable_SIZE		0x100
#define DrawingTable_MASK		0xFF

    class DrawingTable							// Kind of hash table used to speed up rendering of Instances of a mesh
    {
    private:
        std::vector< std::vector< RenderQueue* > >		hash_table;

    public:
        DrawingTable() : hash_table() {hash_table.resize(DrawingTable_SIZE);}
        ~DrawingTable();

        void queue_Instance(uint32 &model_id, Instance instance);
        void draw_all();

    };


    class QUAD
    {
    public:
        Vector3D	pos;
        float	    size_x, size_z;
        uint32	    col;

        QUAD(const Vector3D &P, const float S_x, const float S_z, const uint32 c)
            :pos(P), size_x(S_x), size_z(S_z), col(c)
        {}

    };

    class QUAD_QUEUE
    {
    public:
        std::vector< QUAD > queue;
        GLuint              texture_id;

        QUAD_QUEUE( GLuint t_id ) : queue(), texture_id(t_id) {}

        ~QUAD_QUEUE() {}

        void draw_queue( Vector3D *P, uint32 *C, GLfloat	*T );
    };


    class QUAD_TABLE							// Kind of hash table used to speed up rendering of separated quads
    {
    private:
        std::vector< std::vector< QUAD_QUEUE* > >		hash_table;

    public:
        QUAD_TABLE() : hash_table() {hash_table.resize(DrawingTable_SIZE);}
        ~QUAD_TABLE();

        void queue_quad( GLuint &texture_id, QUAD quad );
        void draw_all();
    };



} // namespace TA3D

#endif
