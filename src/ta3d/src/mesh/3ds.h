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

#ifndef __TA3D_MESH_3DS_H__
#define __TA3D_MESH_3DS_H__

#include "3dm.h"

//>------ Primary chunk

#define MAIN3DS       0x4D4D

//>------ Main Chunks

#define EDIT3DS       0x3D3D  // this is the start of the editor config
#define KEYF3DS       0xB000  // this is the start of the keyframer config

//>------ sub defines of EDIT3DS

#define EDIT_MATERIAL 0xAFFF
#define EDIT_CONFIG1  0x0100
#define EDIT_CONFIG2  0x3D3E
#define EDIT_VIEW_P1  0x7012
#define EDIT_VIEW_P2  0x7011
#define EDIT_VIEW_P3  0x7020
#define EDIT_VIEW1    0x7001
#define EDIT_BACKGR   0x1200
#define EDIT_AMBIENT  0x2100
#define EDIT_OBJECT   0x4000

#define EDIT_UNKNW01  0x1100
#define EDIT_UNKNW02  0x1201
#define EDIT_UNKNW03  0x1300
#define EDIT_UNKNW04  0x1400
#define EDIT_UNKNW05  0x1420
#define EDIT_UNKNW06  0x1450
#define EDIT_UNKNW07  0x1500
#define EDIT_UNKNW08  0x2200
#define EDIT_UNKNW09  0x2201
#define EDIT_UNKNW10  0x2210
#define EDIT_UNKNW11  0x2300
#define EDIT_UNKNW12  0x2302
#define EDIT_UNKNW13  0x3000
#define EDIT_UNKNW14  0xAFFF

//>------ sub defines of EDIT_MATERIAL
#define MAT_NAME	 	 0xA000
#define MAT_AMBIENT	 	 0xA010
#define	MAT_DIFFUSE	 	 0xA020
#define	MAT_SPECULAR 	 0xA030
#define	MAT_SHININESS	 0xA040
#define	MAT_SHIN2PCT 	 0xA041
#define	MAT_SHIN3PCT 	 0xA042
#define	MAT_TRANSPARENCY 0xA050
#define	MAT_TWO_SIDE	 0xA081
#define	MAT_TEXMAP		 0xA200
#define	MAT_MAPNAME		 0xA300

//>------ sub defines of EDIT_OBJECT
#define OBJ_TRIMESH   0x4100
#define OBJ_LIGHT     0x4600
#define OBJ_CAMERA    0x4700

#define OBJ_UNKNWN01  0x4010
#define OBJ_UNKNWN02  0x4012 //>>---- Could be shadow

//>------ sub defines of OBJ_CAMERA
#define CAM_UNKNWN01  0x4710
#define CAM_UNKNWN02  0x4720

//>------ sub defines of OBJ_LIGHT
#define LIT_OFF       0x4620
#define LIT_SPOT      0x4610
#define LIT_UNKNWN01  0x465A

//>------ sub defines of OBJ_TRIMESH
#define TRI_VERTEXL   0x4110
#define TRI_FACEL2    0x4111
#define TRI_FACEL1    0x4120
#define TRI_MATERIAL  0x4130
#define TRI_MAPPING   0x4140
#define TRI_SMOOTH    0x4150
#define TRI_LOCAL     0x4160
#define TRI_VISIBLE   0x4165

//>>------ sub defs of KEYF3DS

#define KEYF_UNKNWN01 0xB009
#define KEYF_UNKNWN02 0xB00A
#define KEYF_FRAMES   0xB008
#define KEYF_OBJDES   0xB002

//>>------  these define the different color chunk types
#define COL_RGB  0x0010
#define COL_TRU  0x0011
#define COL_RGB2 0x0012
#define COL_TRU2 0x0013

//>>------  these define the different percentage chunk types
#define	PER_INT	 	0x0030
#define PER_FLOAT	0x0031

//>>------ defines for viewport chunks

#define TOP           0x0001
#define BOTTOM        0x0002
#define LEFT          0x0003
#define RIGHT         0x0004
#define FRONT         0x0005
#define BACK          0x0006
#define USER          0x0007
#define CAMERA        0x0008 // 0xFFFF is the actual code read from file
#define LIGHT         0x0009
#define DISABLED      0x0010
#define BOGUS         0x0011

#define LOG_PREFIX_3DS               "[3ds] "

namespace TA3D
{

	class Mesh3DS : public Mesh3DM		// Classe pour la gestion des (sous-)objets des modèles 3do
	{
	public:
		Mesh3DS();
		virtual ~Mesh3DS() {}

		virtual void load_texture_id(int id);
	public:
		static Model *load(const QString &filename);
		static const char *getExt();
	};
}

#endif // __TA3D_MESH_3DS_H__
