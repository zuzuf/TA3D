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
|                                     taconfig.cpp                                   |
|  ce fichier contient les structures, classes et fonctions nécessaires à la gestion |
| de la configuration de TA3D: sauvegarde, chargement, modification des options du   |
| jeu.                                                                               |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "taconfig.h"

void TA3D_CONFIG::add_option(char *name,bool b,bool f,void *p)
{
	CONFIG_OPTION	*n_options = new CONFIG_OPTION[nb_options+1];
	if(options!=NULL && nb_options>0)
		for(int i=0;i<nb_options;i++)
			n_options[i]=options[i];
	if(options)	delete[] options;
	options=n_options;
	options[nb_options].data=p;
	char *abc = strdup( name );
	strlwr( abc );
	options[nb_options].name=abc;
	options[nb_options].boolean=b;
	options[nb_options].floatting=f;
	if(p==NULL) {
		options[nb_options].d=0;
		options[nb_options].f=0.0f;
		}
	nb_options++;
}

float TA3D_CONFIG::get_option(char *name)
{
	if(options==NULL || nb_options<=0)	return 0.0f;
	for(int i=0;i<nb_options;i++)
		if(strcasecmp(name,options[i].name)==0) {
			if(options[i].data) {
				if(options[i].boolean)
					return *((bool*)options[i].data);
				else if(options[i].floatting)
					return *((float*)options[i].data);
				else
					return *((short*)options[i].data);
				}
			else {
				if(options[i].floatting)
					return options[i].f;
				else
					return options[i].d;
				}
			}
	return 0.0f;
}

float TA3D_CONFIG::get_option(int idx)
{
	if(options==NULL || nb_options<=0 || idx<0 || idx>=nb_options)	return 0.0f;
	if(options[idx].data) {
		if(options[idx].boolean)
			return *((bool*)options[idx].data);
		else if(options[idx].floatting)
			return *((float*)options[idx].data);
		else
			return *((short*)options[idx].data);
		}
	else {
		if(options[idx].floatting)
			return options[idx].f;
		else
			return options[idx].d;
		}
}

void TA3D_CONFIG::set_option(char *name,float v)
{
	if(options==NULL || nb_options<=0)	return;
	for(int i=0;i<nb_options;i++)
		if(strcasecmp(name,options[i].name)==0) {
			if(options[i].data) {
				if(options[i].boolean)
					*((bool*)options[i].data)=( (v>0.0f) ? true : false);
				else if(options[i].floatting)
					*((float*)options[i].data)=v;
				else
					*((short*)options[i].data)=(short)v;
				}
			else {
				if(options[i].floatting)
					options[i].f=v;
				else
					options[i].d=(short)v;
				}
			return;
			}
}
