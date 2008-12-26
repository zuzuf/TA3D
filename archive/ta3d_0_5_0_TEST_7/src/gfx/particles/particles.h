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
|                                      particles.h                                   |
|  Ce fichier contient les structures, classes et fonctions nécessaires aux effets   |
| graphiques utilisants des particules comme la poussière, les effets de feu,        |
| d'explosion, de fumée ... Ce fichier est conçu pour fonctionner avec la librairie  |
| Allegro et l'addon AllegroGL pour l'utilisation d'OpenGl avec Allegro.             |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_PARTICLES
#define __CLASSE_PARTICLES



// class ParticlesSystem
# include "particlessystem.h"
// 
# include "particlesengine.h"
// Single particle
# include "particle.h"



namespace TA3D
{

    /*!
    ** \brief
    */
    extern PARTICLE_ENGINE particle_engine;

} // namespace TA3D


#endif
