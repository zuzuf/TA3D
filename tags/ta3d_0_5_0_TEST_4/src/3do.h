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

# include "misc/hash_table.h"
# include "ta3dbase.h"
# include "gaf.h"
# include <vector>
# include <list>

		//	Classe pour la gestion des textures du jeu
class TEXTURE_MANAGER
{
public:
	int		nbtex;			// Nombre de textures
	ANIM	*tex;			// Textures

	void init()
	{
		nbtex=0;
		tex=NULL;
	}

	void destroy()
	{
		if(tex) {
			for(int i=0;i<nbtex;i++)
				tex[i].destroy();
			free(tex);
			}
		init();
	}

	TEXTURE_MANAGER()
	{
		init();
	}

	~TEXTURE_MANAGER()
	{
		destroy();
	}

	int get_texture_index(char *texture_name)
	{
		if(nbtex==0) return -1;
		for(int i=0;i<nbtex;i++)
			if(strcasecmp(texture_name,tex[i].name)==0)
				return i;
		return -1;
	}

	GLuint get_gl_texture(char *texture_name,int frame=0)
	{
		int index=get_texture_index(texture_name);
		if(index==-1)
			return 0;
		return tex[index].glbmp[frame];
	}

	BITMAP *get_bmp_texture(char *texture_name,int frame=0)
	{
		int index=get_texture_index(texture_name);
		if(index==-1)
			return NULL;
		return tex[index].bmp[frame];
	}

	int load_gaf(byte *data);

	int all_texture()
	{
								// Crée des textures correspondant aux couleurs de la palette de TA
		nbtex=256;
		tex=(ANIM*)	malloc(sizeof(ANIM)*nbtex);
		for(int i=0;i<256;i++) {
			tex[i].init();
			tex[i].nb_bmp=1;
			tex[i].bmp=(BITMAP**) malloc(sizeof(BITMAP*));
			tex[i].glbmp=(GLuint*) malloc(sizeof(GLuint));
			tex[i].ofs_x=(short*) malloc(sizeof(short));
			tex[i].ofs_y=(short*) malloc(sizeof(short));
			tex[i].w=(short*) malloc(sizeof(short));
			tex[i].h=(short*) malloc(sizeof(short));
			char tmp[10];
			uszprintf(tmp,10,"_%d",i);
			tex[i].name=strdup(tmp);

			tex[i].ofs_x[0]=0;
			tex[i].ofs_y[0]=0;
			tex[i].w[0]=16;
			tex[i].h[0]=16;
			tex[i].bmp[0]=create_bitmap_ex(32,16,16);
			clear_to_color(tex[i].bmp[0],makeacol(pal[i].r<<2,pal[i].g<<2,pal[i].b<<2, 0xFF ));
			}
        std::list<String> file_list;
		HPIManager->getFilelist("textures\\*.gaf", file_list);
		for(std::list<String>::iterator cur_file=file_list.begin();cur_file!=file_list.end();cur_file++)
        {
            byte *data=HPIManager->PullFromHPI(cur_file->c_str());
            load_gaf(data);
            delete[] data;
        }
        return 0;
    }
};

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

    void reset_move()
    {
        move_speed = 0.0f;
    }

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

    void init()
    {
        is_moving=false;
        nb_piece=0;
        axe[0]=axe[1]=axe[2]=NULL;
        flag=NULL;
        explosion_flag=NULL;
        pos=NULL;
        dir=NULL;
        matrix=NULL;
        explode=false;
        explode_time=0.0f;
    }

    inline SCRIPT_DATA()
    {
        init();
    }

    void destroy()
    {
        for(int i=0;i<3;i++)
            if(axe[i])
                delete[] axe[i];
        if(matrix)
            delete[] matrix;
        if(dir)
            delete[] dir;
        if(pos)
            delete[] pos;
        if(flag)
            delete[] flag;
        if(explosion_flag)
            delete[] explosion_flag;
        init();
    }

    inline ~SCRIPT_DATA()
    {
        destroy();
    }

    void load(int nb)
    {
        destroy();		// Au cas où
        nb_piece=nb;
        flag=new short[nb_piece];
        explosion_flag=new short[nb_piece];
        pos=new VECTOR[nb_piece];
        dir=new VECTOR[nb_piece];
        matrix=new MATRIX_4x4[nb_piece];
        int i;
        for(i=0;i<nb_piece;i++) {
            flag[i] = 0;
            explosion_flag[i] = 0;
            pos[i].x = pos[i].y = pos[i].z = 0.0f;
            dir[i] = pos[i];
            matrix[i] = Scale( 1.0f );
        }
        for(i=0;i<3;i++) {
            axe[i]=new AXE[nb_piece];
            for(int e=0;e<nb_piece;e++) {
                axe[i][e].move_speed=0.0f;
                axe[i][e].move_distance=0.0f;
                axe[i][e].pos=0.0f;
                axe[i][e].rot_angle=0.0f;
                axe[i][e].rot_speed=0.0f;
                axe[i][e].rot_accel=0.0f;
                axe[i][e].angle=0.0f;
                axe[i][e].rot_limit=true;
                axe[i][e].rot_speed_limit=false;
                axe[i][e].rot_target_speed=0.0f;
                axe[i][e].is_moving=false;
            }
        }
    }

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
    bool coupe(int x1,int y1,int dx1,int dy1,int x2,int y2,int dx2,int dy2)
    {
        int u1=x1, v1=y1, u2=x2+dx2, v2=y2+dy2;
        if(u1>x2) u1=x2;
        if(v1>y2) v1=y2;
        if(x1+dx1>u2) u2=x1+dx1;
        if(y1+dy1>v2) v2=y1+dy1;
        if(u2-u1+1<dx1+dx2 && v2-v1+1<dy1+dy2) return true;
        return false;
    }

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
    void draw_optimised( bool set = true );

    bool draw_shadow(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

    bool draw_shadow_basic(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL,bool alset=false,bool exploding_parts=false);

    bool hit(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I,MATRIX_4x4 M);

    bool hit_fast(VECTOR Pos,VECTOR Dir,SCRIPT_DATA *data_s,VECTOR *I);

    int random_pos( SCRIPT_DATA *data_s, int id, VECTOR *vec );

    bool compute_emitter()
    {
        emitter=((nb_t_index==0 || nb_vtx==0) && child==NULL && next==NULL);
        if(child)
            emitter|=child->compute_emitter();
        if(next)
            emitter|=next->compute_emitter();
        return emitter;
    }

    bool compute_emitter_point( int &obj_idx )
    {
        emitter_point |= ( script_index == obj_idx );
        emitter |= emitter_point;
        if(child)
            emitter |= child->compute_emitter_point( obj_idx );
        if(next)
            emitter |= next->compute_emitter_point( obj_idx );
        return emitter;
    }

    void init()
    {
        optimised = false;
        optimised_I = NULL;
        optimised_P = NULL;
        optimised_N = NULL;
        optimised_T = NULL;
        optimised_nb_idx = 0;
        optimised_nb_vtx = 0;
        vbo_id = 0;
        ebo_id = 0;

        animation_data = NULL;

        compute_min_max = true;

        last_nb_idx = 0;
        last_dir.x = last_dir.y = last_dir.z = 0.0f;

        use_strips = false;

        line_on=NULL;
        emitter_point=false;
        emitter=false;
        t_line=NULL;
        line_v_idx[0]=NULL;
        line_v_idx[1]=NULL;
        nb_line=0;
        face_reverse=NULL;
        shadow_index=NULL;
        tcoord=NULL;
        dtex=0;
        pos_from_parent.x=pos_from_parent.y=pos_from_parent.z=0.0f;
        nb_vtx=0;
        nb_prim=0;
        name=NULL;
        next=child=NULL;
        points=NULL;
        p_index=NULL;
        l_index=NULL;
        t_index=NULL;
        nb_p_index=0;
        nb_l_index=0;
        nb_t_index=0;
        F_N=NULL;
        N=NULL;
        tex=NULL;
        nb_index=NULL;
        usetex=NULL;
        selprim=-1;
        script_index=-1;
        for(int i=0;i<10;i++)
            gltex[ i ] = gl_dlist[ i ] = 0;
        surface.Flag=0;
        for(int i=0;i<4;i++)
            surface.Color[i]=surface.RColor[i]=1.0f;
        surface.NbTex=0;
        for(int i=0;i<8;i++)
            surface.gltex[i]=0;
        surface.s_shader.succes=false;
        surface.frag_shader_src=NULL;
        surface.vert_shader_src=NULL;
        surface.frag_shader_size=0;
        surface.vert_shader_size=0;
    }

    OBJECT()
    {
        init();
    }

    void destroy()
    {
        if( animation_data )	delete animation_data;
        surface.s_shader.destroy();
        if(surface.frag_shader_src)	free(surface.frag_shader_src);
        if(surface.vert_shader_src)	free(surface.vert_shader_src);
        if(surface.NbTex>0)
            for(int i=0;i<surface.NbTex;i++)
                if(surface.gltex[i])	glDeleteTextures(1,&(surface.gltex[i]));
        if(line_on)			delete[] line_on;
        if(t_line)			free(t_line);
        if(line_v_idx[0])	free(line_v_idx[0]);
        if(line_v_idx[1])	free(line_v_idx[1]);
        if(shadow_index)	free(shadow_index);
        if(tcoord)			free(tcoord);
        if(dtex)
            for(int i=0;i<dtex;i++)
                glDeleteTextures(1,&(gltex[i]));
        for(int i=0;i<10;i++)
            if( gl_dlist[ i ] )
                glDeleteLists(gl_dlist[i],1);
        if(usetex)			free(usetex);
        if(nb_index)		free(nb_index);
        if(tex)				// Ne détruit pas les textures qui le seront par la suite(celles-ci ne sont chargées qu'une fois
            free(tex);		// mais peuvent être utilisées par plusieurs objets
        if(face_reverse)	delete[] face_reverse;
        if(F_N)				delete[] F_N;
        if(N)				free(N);
        if(points)			free(points);
        if(p_index)			free(p_index);
        if(l_index)			free(l_index);
        if(t_index)			free(t_index);
        if(name)			free(name);
        if(optimised_I)		free(optimised_I);
        if(optimised_T)		free(optimised_T);
        if(optimised_P)		free(optimised_P);
        if(optimised_N)		free(optimised_N);
        if(vbo_id)			glDeleteBuffersARB( 1, &vbo_id );
        if(ebo_id)			glDeleteBuffersARB( 1, &ebo_id );
        if(next) {
            next->destroy();
            free(next);
        }
        if(child) {
            child->destroy();
            free(child);
        }
        init();
    }

    ~OBJECT()
    {
        destroy();
    }

    void Identify(int nb_piece,char **piece_name)			// Identifie les pièces utilisées par le script
    {
        script_index=-1;				// Pièce non utilisée
        for(int i=0;i<nb_piece;i++)
            if(strcasecmp(name,piece_name[i])==0) {		// Pièce identifiée
                script_index=i;
                break;
            }
        if(next)
            next->Identify(nb_piece,piece_name);
        if(child)
            child->Identify(nb_piece,piece_name);
    }

    void compute_center(VECTOR *center,VECTOR dec, int *coef)		// Calcule les coordonnées du centre de l'objet, objets liés compris
    {
        for(int i=0;i<nb_vtx;i++) {
            (*coef)++;
            center->x+=points[i].x+dec.x+pos_from_parent.x;
            center->y+=points[i].y+dec.y+pos_from_parent.y;
            center->z+=points[i].z+dec.z+pos_from_parent.z;
        }
        if(next)
            next->compute_center(center,dec,coef);
        if(child)
            child->compute_center(center,dec+pos_from_parent,coef);
    }

    float compute_size_sq(VECTOR center)		// Carré de la taille(on fera une racine après)
    {
        float size=0.0f;
        for(int i=0;i<nb_vtx;i++) {
            float dist=(points[i]-center).sq();
            if(size<dist)
                size=dist;
        }
        if(next) {
            float size_next=next->compute_size_sq(center);
            if(size<size_next)
                size=size_next;
        }
        if(child) {
            float size_child=child->compute_size_sq(center);
            if(size<size_child)
                size=size_child;
        }
        return size;
    }

    float print_struct(float Y,float X,TA3D::Interfaces::GfxFont fnt);

    float compute_top( float top, VECTOR dec )
    {
        for(int i=0;i<nb_vtx;i++)
            top = max( top, points[i].y+dec.y+pos_from_parent.y );
        if(next)
            top = next->compute_top( top, dec );
        if(child)
            top = child->compute_top( top, dec+pos_from_parent );
        return top;
    }

    float compute_bottom( float bottom, VECTOR dec )
    {
        for(int i=0;i<nb_vtx;i++)
            bottom = min( bottom, points[i].y+dec.y+pos_from_parent.y );
        if(next)
            bottom = next->compute_bottom( bottom, dec );
        if(child)
            bottom = child->compute_bottom( bottom, dec+pos_from_parent );
        return bottom;
    }

    bool has_animation_data()
    {
        if( animation_data )	return true;
        if( next )	return next->has_animation_data();
        if( child )	return child->has_animation_data();
        return false;
    }
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

    void init()
    {
        nb_obj = 0;
        obj.init();
        center.x=center.y=center.z=0.0f;
        size=0.0f;
        size2=0.0f;
        dlist = 0;
        animated = false;
        top = bottom = 0.0f;
        from_2d = false;
    }

    MODEL()
    {
        init();
    }

    void destroy()
    {
        obj.destroy();
        if( dlist )	glDeleteLists( dlist, 1 );
        init();
    }

    ~MODEL()
    {
        destroy();
    }

    void load_asc(char *filename,float size);		// Charge un fichier au format *.ASC

    void save_3dm(char *filename,bool compressed)					// Save the model to the 3DM format
    {
        FILE *dst = TA3D_OpenFile(filename,"wb");
        if(dst) {
            obj.save_3dm(dst, compressed);
            fclose(dst);
        }
        else
            Console->AddEntry("Error: save_3dm -> cannot open %s for writing", filename == NULL ? "NULL" : filename);
    }

    void compute_topbottom()
    {
        VECTOR O;
        O.x = O.y = O.z = 0.0f;
        top = obj.compute_top( -99999.0f, O );
        bottom = obj.compute_bottom( 99999.0f, O );
    }

    void load_3dm(byte *data)					// Load a model in 3DM format
    {
        destroy();
        obj.load_3dm(data);
        nb_obj = obj.set_obj_id( 0 );

        animated = obj.has_animation_data();

        VECTOR O;
        O.x=O.y=O.z=0.0f;
        int coef=0;
        center.x=center.y=center.z=0.0f;
        obj.compute_center(&center,O,&coef);
        center=(1.0f/coef)*center;
        size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
        size2=sqrt(0.5f*size);
        obj.compute_emitter();
        compute_topbottom();
    }

    int load_3do(byte *data,const char *filename=NULL)
    {
        int err=obj.load_obj(data,0,0,filename);		// Charge les objets composant le modèle
        if(err==0) {
            nb_obj = obj.set_obj_id( 0 );

            VECTOR O;
            O.x=O.y=O.z=0.0f;
            int coef=0;
            center.x=center.y=center.z=0.0f;
            obj.compute_center(&center,O,&coef);
            center=(1.0f/coef)*center;
            size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
            size2=sqrt(0.5f*size);
            obj.compute_emitter();
            compute_topbottom();
        }
        return err;
    }

    void create_from_2d(BITMAP *bmp,float w,float h,float max_h)
    {
        obj.create_from_2d(bmp,w,h,max_h);

        nb_obj = obj.set_obj_id( 0 );

        from_2d = true;
        VECTOR O;
        O.x=O.y=O.z=0.0f;
        int coef=0;
        center.x=center.y=center.z=0.0f;
        obj.compute_center(&center,O,&coef);
        center=(1.0f/coef)*center;
        obj.compute_emitter();
        size=2.0f*obj.compute_size_sq(center);			// On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
        size2=sqrt(0.5f*size);
        compute_topbottom();
    }

    void draw(float t,SCRIPT_DATA *data_s=NULL,bool sel=false,bool notex=false,bool c_part=false,int p_tex=0,VECTOR *target=NULL,VECTOR *upos=NULL,MATRIX_4x4 *M=NULL,float Size=0.0f,VECTOR *Center=NULL,bool reverse=false,int side=0,bool chg_col=true,OBJECT *src=NULL,SCRIPT_DATA *src_data=NULL)
    {
        if(notex)
            glDisable(GL_TEXTURE_2D);
        VECTOR pos;
        if(chg_col) {
            if(notex) {
                byte var = abs(255 - (((int)(t * 256) & 0xFF)<<1));
                glColor3ub(0,var,0);
            }
            else
                glColor3ub(255,255,255);
        }

        if( data_s == NULL && animated )
            obj.draw(t,NULL,sel,false,notex,side,chg_col);
        else if( data_s == NULL && dlist == 0 && !sel && !notex && !chg_col ) {
            dlist = glGenLists (1);
            glNewList (dlist, GL_COMPILE);
            obj.draw_dl(data_s,false,side,chg_col);
            glEndList();
            glCallList( dlist );
        }
        else if( data_s == NULL && !sel && !notex && !chg_col )
            glCallList( dlist );
        else {
            obj.draw(t,data_s,sel,false,notex,side,chg_col);
            if( data_s && data_s->explode )
                obj.draw(t,data_s,sel,false,notex,side,chg_col,true);
        }
        if(c_part)
            obj.compute_coord(data_s,&pos,c_part,p_tex,target,upos,M,Size,Center,reverse,src,src_data);
    }

    void draw_optimised( bool set = true )
    {
        obj.draw_optimised( set );
    }

    void compute_coord(SCRIPT_DATA *data_s=NULL,MATRIX_4x4 *M=NULL)
    {
        VECTOR pos;
        pos.x=pos.y=pos.z=0.0f;
        obj.compute_coord(data_s,&pos,false,0,NULL,NULL,M);
    }

    void draw_shadow(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL)
    {
        glDisable(GL_TEXTURE_2D);
        obj.draw_shadow(Dir,t,data_s,false);
        if( data_s && data_s->explode )
            obj.draw_shadow(Dir,t,data_s,false,true);
    }

    void draw_shadow_basic(VECTOR Dir,float t,SCRIPT_DATA *data_s=NULL)
    {
        glDisable(GL_TEXTURE_2D);
        obj.draw_shadow_basic(Dir,t,data_s,false);
        if( data_s && data_s->explode )
            obj.draw_shadow_basic(Dir,t,data_s,false,true);
    }

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
};

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

    void destroy()
    {
        if(model) {
            for(int i=0;i<nb_models;i++)
                model[i].destroy();
            free(model);
        }
        if(name) {
            for(int i=0;i<nb_models;i++)
                free(name[i]);
            free(name);
        }

        model_hashtable.EmptyHashTable();
        model_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

        init();
    }

    ~MODEL_MANAGER()
    {
        destroy();
        model_hashtable.EmptyHashTable();
    }

    MODEL *get_model(const char *nom)
    {
        if(nom==NULL)
            return NULL;

        int e = model_hashtable.Find( "objects3d\\" + Lowercase( nom ) + ".3do" ) - 1;
        if( e >= 0 )
            return &(model[e]);

        e = model_hashtable.Find( "objects3d\\" + Lowercase( nom ) + ".3dm" ) - 1;
        if( e >= 0 )
            return &(model[e]);

        e = model_hashtable.Find( Lowercase( nom ) ) - 1;
        if( e >= 0 )
            return &(model[e]);

        e = model_hashtable.Find( Lowercase( nom ) + ".3do" ) - 1;
        if( e >= 0 )
            return &(model[e]);

        e = model_hashtable.Find( Lowercase( nom ) + ".3dm" ) - 1;
        if( e >= 0 )
            return &(model[e]);
        return NULL;
    }

    int load_all(void (*progress)(float percent,const String &msg)=NULL);

    void compute_ids()
    {
        for( int i = 0 ; i < nb_models ; i++ )
            model[ i ].id = i;
    }

    void optimise_all()
    {
        for( int i = 0 ; i < nb_models ; i++ )
            model[ i ].obj.optimise_mesh();
    }

    void create_from_2d(BITMAP *bmp,float w,float h,float max_h,char *filename)
    {
        MODEL *n_model=(MODEL*) malloc(sizeof(MODEL)*(nb_models+1));
        char **n_name=(char**) malloc(sizeof(char*)*(nb_models+1));
        if(model) {
            memcpy(n_model,model,sizeof(MODEL)*nb_models);
            free(model);
            memcpy(n_name,name,sizeof(char*)*nb_models);
            free(name);
        }
        model=n_model;
        name=n_name;
        model[nb_models].init();
        name[nb_models]=strdup(filename);

        model_hashtable.Insert( Lowercase( filename ), nb_models + 1 );

        model[nb_models].create_from_2d(bmp,w,h,max_h);
        nb_models++;
    }
};

extern MODEL_MANAGER	model_manager;

class INSTANCE
{
public:
    VECTOR	pos;
    uint32	col;
    float	angle;

    INSTANCE( const VECTOR &p, const uint32 &c, const float &ang ) {	pos=p;	col=c;	angle=ang;	}
};

class RENDER_QUEUE
{
public:
    std::list<INSTANCE>	queue;
    uint32				model_id;

    RENDER_QUEUE( uint32 m_id ) : queue()	{ model_id = m_id; }

    ~RENDER_QUEUE()	{	queue.clear();	}

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

    QUAD( const VECTOR &P, const float &S_x, const float &S_z, const uint32 &c )	{	pos = P;	size_x = S_x;	size_z = S_z;	col = c;	}
};

class QUAD_QUEUE
{
public:
    std::list< QUAD >	queue;
    GLuint			texture_id;

    QUAD_QUEUE( GLuint t_id ) : queue()	{ texture_id = t_id; }

    ~QUAD_QUEUE()	{	queue.clear();	}

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

#endif
