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
# include "misc/hash_table.h"
# include "ta3dbase.h"
# include "gaf.h"
# include <vector>
# include <list>


namespace TA3D
{


    //	Classe pour la gestion des textures du jeu
    class TEXTURE_MANAGER
    {
    public:
        int		nbtex;			// Nombre de textures
        ANIM	*tex;			// Textures

        TEXTURE_MANAGER() :nbtex(0), tex(NULL) {}
        ~TEXTURE_MANAGER() {destroy();}

        void init();

        void destroy();


        int get_texture_index(const String& texture_name);

        GLuint get_gl_texture(const String& texture_name, const int frame = 0);

        BITMAP *get_bmp_texture(const String& texture_name, const int frame = 0);

        int load_gaf(byte *data);

        int all_texture();

    }; // class TEXTURE_MANAGER



    extern TEXTURE_MANAGER	texture_manager;

    /*--------------Classes pour l'animation---------------------------------------------*/

#define FLAG_HIDE			0x01
#define FLAG_WAIT_FOR_TURN	0x02
#define FLAG_NEED_COMPUTE	0x04
#define FLAG_EXPLODE		0x08
#define FLAG_ANIMATE		0x10

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

    class SCRIPT_DATA
    {
    public:
        int			nb_piece;
        AXE			*axe[3];			// 3 axes (dans l'ordre x,y,z)
        short		*flag;
        short		*explosion_flag;
        float		explode_time;
        bool		explode;
        VECTOR		*pos;
        VECTOR		*dir;			// Orientation des objets (quand il n'y a qu'une ligne)
        MATRIX_4x4	*matrix;		// Store local matrixes
        bool		is_moving;


        SCRIPT_DATA() {init();}

        void init();

        void destroy();

        ~SCRIPT_DATA() {destroy();}

        void load(const int nb);

        const void move(const float dt,const float g=9.81f);
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
        float	Color[4];
        float	RColor[4];
        uint32	Flag;
        sint8	NbTex;
        GLuint	gltex[8];
        char	*frag_shader_src;
        char	*vert_shader_src;
        uint32	frag_shader_size;
        uint32	vert_shader_size;
        SHADER	s_shader;
    };

    struct tagObject				// Structure pour l'en-tête du fichier
    {
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

    class ANIMATION				// Class used to set default animation to a model, this animation will play if no SCRIPT_DATA is provided (ie for map features)
    {
    public:
        byte	type;
        VECTOR	angle_0;
        VECTOR	angle_1;
        float	angle_w;
        VECTOR	translate_0;
        VECTOR	translate_1;
        float	translate_w;

        ANIMATION()
        {
            type = 0;
        }

        void animate( float &t, VECTOR &R, VECTOR &T )
        {
            if( type & ROTATION ) {
                if( type & ROTATION_PERIODIC ) {
                    float coef;
                    if( type & ROTATION_COSINE )
                        coef = 0.5f + 0.5f * cos( t * angle_w );
                    else {
                        coef = t * angle_w;
                        int i = (int) coef;
                        coef = coef - i;
                        coef = (i&1) ? (1.0f - coef) : coef;
                    }
                    R = coef * angle_0 + (1.0f - coef) * angle_1;
                }
                else
                    R = t * angle_0;
            }
            if( type & TRANSLATION ) {
                if( type & TRANSLATION_PERIODIC ) {
                    float coef;
                    if( type & TRANSLATION_COSINE )
                        coef = 0.5f + 0.5f * cos( t * translate_w );
                    else {
                        coef = t * translate_w;
                        int i = (int) coef;
                        coef = coef - i;
                        coef = (i&1) ? (1.0f - coef) : coef;
                    }
                    T = coef * translate_0 + (1.0f - coef) * translate_1;
                }
                else
                    T = t * translate_0;
            }
        }
    };

    class OBJECT					// Classe pour la gestion des (sous-)objets des modèles 3do
    {
    public:
        short		nb_vtx;				// Nombre de points
        short		nb_prim;			// Nombre de primitives
        char		*name;				// Nom de l'objet
        OBJECT		*next;				// Objet suivant
        OBJECT		*child;				// Objet fils
        VECTOR		*points;			// Points composant l'objet
        short		nb_p_index;			// Nombre d'indices de points
        short		nb_l_index;			// Nombre d'indices de lignes
        short		nb_t_index;			// Nombre d'indices de triangles
        VECTOR		pos_from_parent;	// Position par rapport à l'objet parent
        GLushort	*p_index;			// Tableau d'indices pour les points isolés
        GLushort	*l_index;			// Tableau d'indices pour les lignes
        GLushort	*t_index;			// Tableau d'indices pour les triangles
        short		*nb_index;			// Nombre d'indices par primitive
        VECTOR		*N;					// Tableau de normales pour les sommet
        VECTOR		*F_N;				// face normals
        int			*tex;				// Tableau de numéros de texture OpenGl
        byte		*usetex;			// Tableau indiquant si une texture doit être appliquée
        sint16		selprim;			// Polygone de selection
        GLuint		gltex[10];			// Texture pour le dessin de l'objet
        uint8		dtex;				// Indique si une texture objet doit être détruite avec l'objet
        float		*tcoord;			// Tableau de coordonnées de texture
        GLushort	sel[4];				// Primitive de sélection
        sint16		script_index;		// Indice donné par le script associé à l'unité
        bool		emitter;			// This object can or has sub-objects which can emit particles
        bool		emitter_point;		// This object directly emits particles
        GLuint		gl_dlist[10];		// Display lists to speed up the drawing process

        OBJECT_SURFACE	surface;		// Tell how this object must be drawn
        ANIMATION	*animation_data;

    private:

        GLushort	*shadow_index;		// Pour la géométrie du volume d'ombre
        short	*t_line;				// Repère les arêtes
        short	*line_v_idx[2];
        short	nb_line;
        byte	*line_on;
        byte	*face_reverse;
        bool	use_strips;				// Used by converted sprites to speed things up
        VECTOR	last_dir;				// To speed up things when shadow has already been cast
        uint16	last_nb_idx;				// Remember how many things we have drawn last time

        float	min_x, max_x;		// Used by hit_fast
        float	min_y, max_y;
        float	min_z, max_z;
        bool	compute_min_max;

        uint16	obj_id;				// Used to generate a random position on the object
    public:
        uint16	nb_sub_obj;
    private:

        //-------------- EXPERIMENTAL CODE -----------------

        bool		optimised;
        GLushort	*optimised_I;
        VECTOR		*optimised_P;
        VECTOR		*optimised_N;
        float		*optimised_T;
        uint16		optimised_nb_idx;
        uint16		optimised_nb_vtx;
        GLuint		vbo_id;
        GLuint		ebo_id;
        GLuint		N_offset;
        GLuint		T_offset;

        //---------- END OF EXPERIMENTAL CODE --------------

    private:
        bool coupe(int x1,int y1,int dx1,int dy1,int x2,int y2,int dx2,int dy2);

    public:

        uint16 set_obj_id( uint16 id );

        void optimise_mesh();			// EXPERIMENTAL, function to merge all objects in one vertex array

        int load_obj(byte *data,int offset,int dec=0,const char *filename=NULL);

        void save_3dm(FILE *dst, bool compressed);

        byte *load_3dm(byte *data);

        void create_from_2d(BITMAP *bmp,float w,float h,float max_h);

        void compute_coord(SCRIPT_DATA *data_s=NULL,VECTOR *pos=NULL,bool c_part=false,int p_tex=0,VECTOR *target=NULL,VECTOR *upos=NULL,MATRIX_4x4 *M=NULL,float size=0.0f,VECTOR *center=NULL,bool reverse=false,OBJECT *src=NULL,SCRIPT_DATA *src_data=NULL);

        bool draw(float t,SCRIPT_DATA *data_s=NULL,bool sel_primitive=false,bool alset=false,bool notex=false,int side=0,bool chg_col=true,bool exploding_parts=false);
        bool draw_dl(SCRIPT_DATA *data_s=NULL,bool alset=false,int side=0,bool chg_col=true);
        void draw_optimised(const bool set = true);

        bool draw_shadow(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

        bool draw_shadow_basic(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

        bool hit(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I,MATRIX_4x4 M);

        bool hit_fast(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I);

        int random_pos( SCRIPT_DATA *data_s, int id, VECTOR *vec );

        bool compute_emitter();

        bool compute_emitter_point(int &obj_idx);

        void init();

        OBJECT() {init();}

        void destroy();

        ~OBJECT() {destroy();}

        void Identify(int nb_piece,char **piece_name);			// Identifie les pièces utilisées par le script

        void compute_center(VECTOR *center,VECTOR dec, int *coef);		// Calcule les coordonnées du centre de l'objet, objets liés compris

        float compute_size_sq(VECTOR center);		// Carré de la taille(on fera une racine après)

        float print_struct(float Y,float X,TA3D::Interfaces::GfxFont fnt);

        float compute_top( float top, VECTOR dec );

        float compute_bottom( float bottom, VECTOR dec );

        bool has_animation_data();

    };

    class MODEL						// Classe pour la gestion des modèles 3D
    {
    public:
        OBJECT		obj;			// Objet principal du modèle 3D

    public:

        VECTOR	center;				// Centre de l'objet pour des calculs d'élimination d'objets
        float	size;				// Square of the size of the sphere which contains the model
        float	size2;				// Same as above but it is its square root
        float	top;				// Max y coordinate found in the model
        float	bottom;				// Min y coordinate found in the model
        uint32	id;					// ID of the model in the model array

        GLuint	dlist;				// Display list to speed up drawings operations when no position data is given (trees, ...)
        bool	animated;
        bool	from_2d;
        uint16	nb_obj;

    public:
        MODEL() {init();}
        ~MODEL() {destroy();}

        void init();


        void destroy();


        void load_asc(char *filename,float size); // Charge un fichier au format *.ASC

        void save_3dm(char *filename,bool compressed); // Save the model to the 3DM format

        void compute_topbottom();

        void load_3dm(byte* data);	// Load a model in 3DM format

        int load_3do(byte* data, const char *filename=NULL);

        void create_from_2d(BITMAP* bmp, float w, float h, float max_h);

        void draw(float t, SCRIPT_DATA* data_s = NULL, bool sel = false, bool notex = false,
                  bool c_part = false, int p_tex = 0, VECTOR *target = NULL, VECTOR* upos = NULL,
                  MATRIX_4x4* M = NULL, float Size = 0.0f, VECTOR* Center = NULL, bool reverse = false,
                  int side = 0, bool chg_col = true, OBJECT* src = NULL, SCRIPT_DATA* src_data = NULL);


        void draw_optimised(bool set = true) {obj.draw_optimised(set);}

        void compute_coord(SCRIPT_DATA *data_s=NULL,MATRIX_4x4 *M=NULL);

        void draw_shadow(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL);

        void draw_shadow_basic(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL);

        bool hit(VECTOR &Pos,VECTOR &Dir,SCRIPT_DATA *data_s,VECTOR *I,MATRIX_4x4 &M)
        {
            return obj.hit(Pos,Dir,data_s,I,M);
        }

        bool hit_fast(VECTOR &Pos,VECTOR &Dir,SCRIPT_DATA *data_s,VECTOR *I,MATRIX_4x4 &M)
        {
            return obj.hit_fast(Pos,Dir*M,data_s,I);
        }

        void Identify(int nb_piece,char **piece_name)
        {
            obj.Identify(nb_piece,piece_name);
        }

        void print_struct(float Y,float X,TA3D::Interfaces::GfxFont fnt)
        {
            obj.print_struct(Y,X,fnt);
        }

    }; // class MODEL



    class MODEL_MANAGER							// Classe pour la gestion des modèles 3D du jeu
    {
    public:
        int			nb_models;		// Nombre de modèles
        MODEL		*model;			// Tableau de modèles
        char		**name;			// Tableau contenant les noms des modèles

    private:
        cHashTable< int >	model_hashtable;		// hashtable used to speed up operations on MODEL objects

    public:

        void init()
        {
            nb_models=0;
            model=NULL;
            name=NULL;
        }

        MODEL_MANAGER() : model_hashtable()
        {
            init();
        }

        void destroy();

        ~MODEL_MANAGER();

        MODEL *get_model(const String& name);

        int load_all(void (*progress)(float percent,const String &msg)=NULL);

        void compute_ids();

        void optimise_all();

        void create_from_2d(BITMAP *bmp,float w,float h,float max_h,const String& filename);
            
    };

    extern MODEL_MANAGER	model_manager;

    class INSTANCE
    {
    public:
        VECTOR	pos;
        uint32	col;
        float	angle;

        INSTANCE(const VECTOR &p, const uint32 &c, const float &ang)
            :pos(p), col(c), angle(ang)
        {}
    };

    class RENDER_QUEUE
    {
    public:
        std::list<INSTANCE>	queue;
        uint32				model_id;

        RENDER_QUEUE(const uint32 m_id) :queue(), model_id(m_id) {}

        ~RENDER_QUEUE()	{}

        void draw_queue();
    };

#define DRAWING_TABLE_SIZE		0x100
#define DRAWING_TABLE_MASK		0xFF

    class DRAWING_TABLE							// Kind of hash table used to speed up rendering of instances of a mesh
    {
    private:
        std::vector< std::list< RENDER_QUEUE* > >		hash_table;

    public:

        DRAWING_TABLE() : hash_table()	{	hash_table.resize( DRAWING_TABLE_SIZE );	}

        ~DRAWING_TABLE();

        void queue_instance( uint32 &model_id, INSTANCE instance );
        void draw_all();
    };

    class QUAD
    {
    public:
        VECTOR	pos;
        float	size_x, size_z;
        uint32	col;

        QUAD(const VECTOR &P, const float S_x, const float S_z, const uint32 c)
            :pos(P), size_x(S_x), size_z(S_z), col(c)
        {}

    };

    class QUAD_QUEUE
    {
    public:
        std::list< QUAD >	queue;
        GLuint			texture_id;

        QUAD_QUEUE( GLuint t_id ) : queue(), texture_id(t_id) {}

        ~QUAD_QUEUE() {}

        void draw_queue( VECTOR *P, uint32 *C, GLfloat	*T );
    };

    class QUAD_TABLE							// Kind of hash table used to speed up rendering of separated quads
    {
    private:
        std::vector< std::list< QUAD_QUEUE* > >		hash_table;

    public:

        QUAD_TABLE() : hash_table()	{	hash_table.resize( DRAWING_TABLE_SIZE );	}

        ~QUAD_TABLE();

        void queue_quad( GLuint &texture_id, QUAD quad );
        void draw_all();
    };



} // namespace TA3D

#endif
