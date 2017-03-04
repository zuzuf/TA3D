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
#ifndef __TA3D_GFX_PARTICLES_ENGINE_H__
# define __TA3D_GFX_PARTICLES_ENGINE_H__

# include <stdafx.h>
# include <misc/string.h>
# include <threads/thread.h>
# include <misc/vector.h>
# include <ta3dbase.h>
# include <misc/camera.h>
# include "particlessystem.h"
# include "particle.h"
# include <vector>



namespace TA3D
{


    /*! \class PARTICLE_ENGINE
    **
    ** \brief Engine for particles
    */
    class PARTICLE_ENGINE : public ObjectSync, public Thread
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        PARTICLE_ENGINE();
        //! Destructor
        ~PARTICLE_ENGINE();
        //@}

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief
        ** \param load
        */
        void init(bool load = true);

        /*!
        ** \brief
        **
        ** \param file
        ** \param filealpha
        */
        int addtex(const QString& file,const QString& filealpha = "");


        /*!
        ** \brief
        **
        ** \param g
        ** \param wind_dir
        */
        void set_data(float &g, Vector3D& wind_dir) { p_g = &g; p_wind_dir = &wind_dir; }

        /*!
        ** \brief
        */
		void emit_part(const Vector3D &pos, const Vector3D &Dir,int tex,int nb,float speed,float life=10.0f,float psize=10.0f,bool white=false,float trans_factor=1.0f);

        /*!
        ** \brief
        */
		ParticlesSystem *emit_part_fast( ParticlesSystem *system, const Vector3D &pos, const Vector3D &Dir,
                                         int tex, int nb, float speed, float life=10.0f,
                                         float psize=10.0f, bool white=false, float trans_factor=1.0f );

        /*!
        ** \brief
        */
		void emit_lava(const Vector3D &pos, const Vector3D &Dir,int tex,int nb,float speed,float life=10.0f);

        /*!
        ** \brief
        */
		void make_smoke(const Vector3D &pos,int tex,int nb,float speed,float mass=-1.0f, float ddsize=0.0f,float alpha=1.0f);

        /*!
        ** \brief
        */
		void make_dark_smoke(const Vector3D &pos,int tex,int nb,float speed,float mass=-1.0f, float ddsize=0.0f,float alpha=1.0f);

        /*!
        ** \brief
        */
		void make_fire(const Vector3D &pos,int tex,int nb,float speed);

        /*!
        ** \brief
        */
		void make_shockwave(const Vector3D &pos,int tex,int nb,float speed);

        /*!
        ** \brief
        */
		void make_nuke(const Vector3D &pos,int tex,int nb,float speed);

        /*!
        ** \brief
        **
        ** \param dt
        ** \param wind_dir
        ** \param g Constant of Gravity
        */
		void move(float dt, const Vector3D &wind_dir, float g = 9.81f);


        /*!
        ** \brief
        */
		void draw(Camera *cam);

		/*!
		** \brief
		*/
		void drawUW();

    public:
        uint32		nb_part;		// Nombre de particules
        uint32		size;			// Quantité maximale de particules stockables dans le tableau
        std::vector<PARTICLE>	part;	// Liste des particules / Particle list
        GfxTexture::Ptr parttex;		// Textures des particules
        QImage partbmp;		// Textures des particules
        bool		dsmoke;
        uint32		ntex;
        std::vector<GfxTexture::Ptr>	gltex;

        Vector3D	*point;			// Vertex array
        GLfloat		*texcoord;		// Texture coordinates array
        GLubyte		*color;			// Color array

    protected:
		volatile bool  thread_running;
		volatile bool  thread_ask_to_stop;
        Vector3D* p_wind_dir;
        float* p_g;
        std::vector<ParticlesSystem*> particle_systems;
    private:
        void  proc(void*);
        void  signalExitThread();

    }; // class PARTICLE_ENGINE



} // namespace TA3D

#endif // __TA3D_GFX_PARTICLES_ENGINE_H__
