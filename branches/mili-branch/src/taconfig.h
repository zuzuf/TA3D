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
|                                      taconfig.h                                    |
|  ce fichier contient les structures, classes et fonctions nécessaires à la gestion |
| de la configuration de TA3D: sauvegarde, chargement, modification des options du   |
| jeu.                                                                               |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef CLASS_CONFIG
#define CLASS_CONFIG

struct CONFIG_OPTION		// Structure utilisée pour faire le lien avec les options du programme
{
	void	*data;
	bool	boolean;
	bool	floatting;
	char	*name;
	short	d;
	float	f;
};

class TA3D_CONFIG
{
public:
	int				nb_options;
	CONFIG_OPTION	*options;			// Données de configuration

	void init()
	{
		nb_options=0;
		options=NULL;
	}

	void destroy()
	{
		if(nb_options>0 && options!=NULL) {
			for(int i=0;i<nb_options;i++)
				if(options[i].name)	free(options[i].name);
			delete[] options;
			}
		init();
	}

	TA3D_CONFIG()
	{
		init();
	}

	~TA3D_CONFIG()
	{
		destroy();
	}

	void add_option(char *name,bool b,bool f,void *p);

	float get_option(char *name);

	float get_option(int idx);

	void set_option(char *name,float v);
};

#endif
