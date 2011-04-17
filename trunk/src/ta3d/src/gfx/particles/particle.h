#ifndef __TA3D_PARTICLES_SINGLE_PARTICLE_H__
# define __TA3D_PARTICLES_SINGLE_PARTICLE_H__

# include <stdafx.h>
# include <misc/vector.h>



namespace TA3D
{

	struct PARTICLE					// Structure définissant une particule
	{
		Vector3D	Pos;			// Position
		Vector3D	V;				// Vitesse
		float		size;			// Taille
		float		dsize;			// Variation de la taille au cours du temps
		float		ddsize;			// Variation de dsize au cours du temps
		float		col[4];			// Couleur
		float		dcol[4];		// Variation de couleur
		GLuint		gltex;			// Texture associée
		float		mass;			// Masse apparente de la particule si celle-ci est soumise à la force de gravitation(signe seulement)
		float		life;			// Temps d'existence restant à la particule
		float		smoking;		// Produit de la fumée?
		float		angle;			// Angle
		float		v_rot;			// Vitesse de rotation
		float		slow_factor;
		short		px;				// Coordonnées du bloc de référence pour la visibilité
		short		py;
		bool		use_wind;		// Affected by wind ?
		bool		light_emitter;	// For fire effects
		bool		slow_down;		// Decrease speed ?
	};


} // namespace TA3D




#endif // __TA3D_PARTICLES_SINGLE_PARTICLE_H__
