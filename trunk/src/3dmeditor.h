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


extern int cur_part;
extern bool ClickOnExit;

namespace TA3D
{
	namespace VARS
	{
		extern MODEL *TheModel;
		extern OBJECT **obj_table;
		extern int h_table[1000];
		extern OBJECT_SURFACE obj_surf;
	}
}

RGB *LoadPal(const char *filename);
void init();

/*---------------------------------------------------------------------------------------------------\
|                                       Déclarations de fonctions                                    |
\---------------------------------------------------------------------------------------------------*/

void SurfPaint(int index);			// Editeur de texture de surface sur l'objet
void TexturePosEdit(int index);		// Editeur de plaquage de texture
void CylinderTexturing(int part);	// Plaquage cylindrique de la texture
void CubeTexturing(int part);		// Plaquage cubique de la texture
void SurfEdit();					// Editeur de surfaces
int  nb_obj();						// Compte le nombre d'objets dans le modèle
void convert_to_3dm();				// Convertit le modèle chargé au format 3dm
void glslEditor();                  // Fragment and vertex programs editor

/*---------------------------------------------------------------------------------------------------\
|               Fonctions associées aux menus déroulant de la barre de menus de l'interface          |
|   principale. Gère les actions liées aux options proposées par les menus.                          |
\---------------------------------------------------------------------------------------------------*/
void mnu_file(int mnu_index);
void mnu_surf(int mnu_index);
void mnu_selec(int mnu_index);
void button_rename(int mnu_index);
void button_child(int mnu_index);
void button_remove(int mnu_index);
void button_scale(int mnu_index);
void button_mirror_x(int mnu_index);
void button_mirror_y(int mnu_index);
void button_mirror_z(int mnu_index);
void button_change_xy(int mnu_index);
void button_change_yz(int mnu_index);
void button_change_zx(int mnu_index);

GLuint copy_tex(GLuint gltex);
void obj_maj_normal(int idx);
void obj_geo_optimize(int idx,bool notex=false);
void obj_geo_split(int idx);
BITMAP *read_tex(GLuint gltex);
BITMAP *read_tex_luminance(GLuint gltex);

int intersect(Vector3D O,Vector3D Dir,OBJECT *obj,Vector3D *PA,Vector3D *PB);	// Calcule l'intersection d'un rayon avec une partie de la meshe

inline void init_surf_buf()
{
	obj_surf.NbTex=0;
	obj_surf.Flag=SURFACE_ADVANCED;
	for(int i=0;i<4;i++)
		obj_surf.Color[i]=obj_surf.RColor[i]=1.0f;
}
