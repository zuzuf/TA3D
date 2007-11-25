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

/*---------------------------------------------------------------------------\
|                              vector.h (pour allegro)                       |
|   Définition des classes et des fonctions relatives aux vecteurs(2D et 3D) |
|                                                                            |
\---------------------------------------------------------------------------*/

#ifndef CLASS_VECTOR

#define CLASS_VECTOR

/*---------------------------------------------------------------------------\
|           Définition des classes                                           |
\---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
|              Pour les points dans un plan(2D)
\-------------------------------------*/
struct POINT2D
{
   float x,y;
};

/*------------------------------------------------------------------------
|              Pour les points dans l'espace(3D)
\-------------------------------------*/
struct POINT3D
{
   float x,y,z;
};

/*------------------------------------------------------------------------
|              Pour le traitement de vecteurs dans un plan(2D)
\-------------------------------------*/

class VECTOR2D
{
public:         // les coordonnées sont public
   float x,y;      // Coordonnées
      // Calcul des coordonnées à partir de 2 points
   inline void V(POINT2D a,POINT2D b)
   {
      x=b.x-a.x;
      y=b.y-a.y;
   }
      // Fonction qui renvoie le carré scalaire du vecteur
   inline float Sq()
   {
      return (x*x+y*y);         // carré scalaire
   }
      // Fonction qui renvoie la norme du vecteur
   inline float Norm()
   {
      return sqrt(x*x+y*y);     // racine du carré scalaire
   }
      // Rend le vecteur unitaire si possible(de norme 1)
   inline void Unit()
   {
      if(x!=0.0f || y!=0.0f) {        // Si le vecteur n'est pas nul
         float n=1.0f/sqrt(x*x+y*y);    // Inverse de la norme du vecteur
         x*=n;
         y*=n;
         }
   }

inline VECTOR2D operator+=(VECTOR2D& b)     // 2D
{
   x+=b.x; y+=b.y;
   return (*this);
}

};

/*------------------------------------------------------------------------
|              Pour le traitement de vecteurs dans l'espace(3D)
\-------------------------------------*/

class VECTOR3D
{
public:         // les coordonnées sont public
float x,y,z;    // Coordonnées spatiales du vecteur

	VECTOR3D()
	{
		x=y=z=0.0f;
	}

	VECTOR3D( const float &X, const float &Y, const float &Z )
	{
		x=X;	y=Y;	z=Z;
	}

      // Calcul des coordonnées à partir de 2 points
   inline const void V(POINT3D a,POINT3D b)
   {
      x=b.x-a.x;
      y=b.y-a.y;
      z=b.z-a.z;
   }
      // Fonction qui renvoie le carr‚ scalaire du vecteur
   inline const float Sq()
   {
      return (x*x+y*y+z*z);     // carré scalaire
//      return dot_product_f(x,y,z,x,y,z);//(x*x+y*y+z*z);     // carré scalaire
   }
      // Fonction qui renvoie la norme du vecteur
   inline const float Norm()
   {
//   		return hypot( x, hypot( y, z ) );
      return sqrt(x*x+y*y+z*z); // racine du carré scalaire
   }
      // Rend le vecteur unitaire si possible(de norme 1)
   inline const void Unit()
   {
      if(x!=0.0f || y!=0.0f || z!=0.0f) {    // Si le vecteur n'est pas nul
        float n=Norm();				// Inverse de la norme du vecteur
        if(n!=0.0f) {
			n=1.0f/n;
			x*=n;			y*=n;
			z*=n;
			}
		}
   }

inline const VECTOR3D operator+=(const VECTOR3D& b)     // 3D
{
   x+=b.x; y+=b.y; z+=b.z;
   return (*this);
}

};

// Pour une utilisation courante
typedef VECTOR3D VECTOR;        // On utilise les vecteurs 3D
typedef POINT3D POINTF;         // On utilise les points 3D

/*---------------------------------------------------------------------------\
|           Définition des opérateurs relatifs aux vecteurs                  |
\---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
|              Produit scalaire
\-------------------------------------*/
inline const float operator%(const VECTOR2D &a, const VECTOR2D &b)         // 2D
{
   return (a.x*b.x+a.y*b.y);
}

inline const float operator%(const VECTOR3D &a, const VECTOR3D &b)         // 3D
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
//   return dot_product_f(a.x,a.y,a.z,b.x,b.y,b.z);
}

/*------------------------------------------------------------------------
|              Addition
\-------------------------------------*/
inline VECTOR2D operator+(VECTOR2D a, const VECTOR2D &b)      // 2D
{
   a.x+=b.x, a.y+=b.y;  // Addition
   return a;
}

inline VECTOR3D operator+(VECTOR3D a, const VECTOR3D &b)      // 3D
{
   a.x+=b.x, a.y+=b.y, a.z+=b.z;  // Addition
   return a;
}

/*------------------------------------------------------------------------
|              Soustraction
\-------------------------------------*/
inline VECTOR2D operator-(VECTOR2D a, const VECTOR2D &b)      // 2D
{
   a.x-=b.x, a.y-=b.y;  // Soustraction
   return a;
}

inline VECTOR3D operator-(VECTOR3D a, const VECTOR3D &b)      // 3D
{
   a.x-=b.x, a.y-=b.y, a.z-=b.z;  // Soustraction
   return a;
}

/*------------------------------------------------------------------------
|              Opposé
\-------------------------------------*/
inline VECTOR2D operator-(VECTOR2D a)                  // 2D
{
   a.x=-a.x, a.y=-a.y;
   return a;
}

inline VECTOR3D operator-(VECTOR3D a)                  // 3D
{
   a.x=-a.x, a.y=-a.y, a.z=-a.z;
   return a;
}

/*------------------------------------------------------------------------
|              Scalaire
\-------------------------------------*/
inline VECTOR2D operator*(const float &s, VECTOR2D a)         // 2D
{
   a.x*=s, a.y*=s;
   return a;
}

inline VECTOR3D operator*(const float &s, VECTOR3D a)         // 3D
{
   a.x*=s, a.y*=s, a.z*=s;
   return a;
}

/*------------------------------------------------------------------------
|              Produit vectoriel(3D seulement)
\-------------------------------------*/
inline VECTOR3D operator*(const VECTOR3D &a,const VECTOR3D &b)
{
   VECTOR3D c;
   cross_product_f(a.x,a.y,a.z,b.x,b.y,b.z,&c.x,&c.y,&c.z);
   return c;
}

/*------------------------------------------------------------------------
|              Création d'un vecteur à partir de points
\-------------------------------------*/
inline VECTOR2D operator>>(const POINT2D &a,const POINT2D &b)        // 2D
{
   VECTOR2D ab;
   ab.x=b.x-a.x;
   ab.y=b.y-a.y;
   return ab;
}

inline VECTOR3D operator>>(const POINT3D &a,const POINT3D &b)        // 3D
{
   VECTOR3D ab;
   ab.x=b.x-a.x;
   ab.y=b.y-a.y;
   ab.z=b.z-a.z;
   return ab;
}

/*------------------------------------------------------------------------
|              Verification d'égalité
\-------------------------------------*/
inline bool operator==(const VECTOR2D &a, const VECTOR2D &b)         // 2D
{
   if(a.x==b.x&&a.y==b.y) return true;
   else return false;
}
inline bool operator==(const POINT2D &a, const POINT2D &b)           // 2D
{
   if(a.x==b.x&&a.y==b.y) return true;
   else return false;
}

inline bool operator==(const VECTOR3D &a, const VECTOR3D &b)         // 3D
{
   if(a.x==b.x&&a.y==b.y&&a.z==b.z) return true;
   else return false;
}
inline bool operator==(const POINT3D &a, const POINT3D &b)           // 3D
{
   //if(a.x==b.x&&a.y==b.y&&a.z==b.z) return true;
   if(fabs(a.x-b.x)<0.0001f&&fabs(a.y-b.y)<0.0001f&&fabs(a.z-b.z)<0.0001f) return true;
   else return false;
}

/*------------------------------------------------------------------------
|              Translation d'un point par un vecteur
\-------------------------------------*/
inline POINT2D operator+(POINT2D a, const VECTOR2D &b)        // 2D
{
   a.x+=b.x, a.y+=b.y;
   return a;
}
inline POINT2D operator-(POINT2D a, VECTOR2D &b)        // 2D
{
   a.x-=b.x, a.y-=b.y;
   return a;
}

inline POINT3D operator+(POINT3D a, const VECTOR3D &b)        // 3D
{
   a.x+=b.x, a.y+=b.y, a.z+=b.z;
   return a;
}
inline POINT3D operator-(POINT3D a, const VECTOR3D &b)        // 3D
{
   a.x-=b.x, a.y-=b.y, a.z-=b.z;
   return a;
}

/*------------------------------------------------------------------------
|              Retourne l'angle en radians entre deux vecteurs
\-------------------------------------*/

inline double VAngle(VECTOR &A,VECTOR &B)
{
float a = sqrt( A.Sq() * B.Sq() );
return a == 0.0f ? 0.0f : acos( ( A % B ) / a );
}

#endif
