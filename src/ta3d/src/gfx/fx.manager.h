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
#ifndef __TA3D_GFX_FXMANAGER_H__
# define __TA3D_GFX_FXMANAGER_H__

# include <stdafx.h>
# include <misc/string.h>
# include <threads/thread.h>
# include <gaf.h>
# include <ta3dbase.h>
# include "fx.base.h"
# include "fx.particle.h"
# include "fx.electric.h"
# include <misc/camera.h>
# include <mesh/mesh.h>
# include "texture.h"


namespace TA3D
{


    /*! \class FXManager
    **
    ** \brief
    **
    ** \warning This class mustn't be executed in its own thread in order to remain
    ** thread safe. It must run in main thread (the one that can call OpenGL functions)
    */
    class FXManager : public ObjectSync
    {
    public:
        //! \name Constructor & destructor
        //@{
        //! Default constructor
        FXManager();
        //! Destructor
        ~FXManager();
        //@}

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief Load needed textures if not already loaded
        */
        void loadData();

        /*!
        ** \brief
        **
        ** \param dt Delta time
        */
        void move(const float dt);

        /*!
        ** \brief
        **
        ** \param cam
        ** \param w_lvl
        ** \param UW
        */
		void draw(Camera& cam, float w_lvl = 0.0f, bool UW = false);

		/*!
		** \brief
		**
		*/
		void drawWaterDistortions();

        /*!
        ** \brief
        **
        ** \param filename
        ** \param entryName
        ** \param pos
        ** \param size
        ** \return
        */
        int add(const QString& filename, const QString& entryName, const Vector3D& Pos, const float size);

        /*!
        ** \brief
        ** \param pos
        ** \param size
        */
        int addFlash(const Vector3D& pos, const float size);

        /*!
        ** \brief Add a wave
        ** \param pos
        ** \param size
        ** \return
        */
        int addWave(const Vector3D& Pos,float size);

        /*!
        ** \brief Add a ripple
        ** \param pos
        ** \param size
        ** \return
        */
        int addRipple(const Vector3D& Pos,float size);

        /*!
        ** \brief Add a particle
        ** \param p
        ** \param s
        ** \param l
        */
        void addParticle(const Vector3D& p, const Vector3D& s, const float l);

        /*!
        ** \brief Add an explosion effect
        ** \param p
        ** \param n
        ** \param power
        */
        void addExplosion(const Vector3D& p, const int n, const float power);

        /*!
        ** \brief Add an explosion effect (with initial speed)
        ** \param p
        ** \param s
        ** \param n
        ** \param power
        */
        void addExplosion(const Vector3D& p, const Vector3D& s, const int n, const float power);

        /*!
        ** \brief Add an electrical effect
        ** \param p
        */
        void addElectric(const Vector3D& p);

    public:
        //!
        QIODevice* fx_data;
        //!
        GfxTexture::Ptr flash_tex;
        //!
        GfxTexture::Ptr wave_tex[3];
        //!
        GfxTexture::Ptr ripple_tex;

    private:
        /*!
        ** \brief Find the index of a filename in the cache
        ** \param filename The filename to look for
        ** \return The index of the item, -1 if not found
        ** \warning This method is not thread-safe
        */
        int findInCache(const QString& filename) const;

        /*!
        ** \brief
        ** \param filename
        ** \param anm
        ** \return
        ** \warning This method is not thread-safe
        */
        int putInCache(const QString& filename, Gaf::Animation* anm);

        /*!
        ** \brief Delete all particles
        ** \see pParticles
        */
        void doClearAllParticles();

        /*!
        ** \brief Move all particles
        **
        ** Each particles in `pParticles` will be moved via
        ** a call to FXParticle::move() and will be removed if
        ** it returns `true`
        **
        ** \see FXParticle::move()
        */
        void doMoveAllParticles(const float& dt);

        /*!
        ** \brief Move all FX
        ** \param dt Delta time
        */
        void doMoveAllFX(const float& dt);

    private:
		typedef std::vector<FXParticle>  ListOfParticles;
		typedef std::vector<FXElectric>  ListOfElectrics;

    private:
        int max_fx;
        int nb_fx;
        std::vector<FX> fx;

		// Cache
		QStringList cacheName;
		HashMap<int>::Sparse hashName;
		std::vector<Gaf::Animation*> cacheAnm;
		std::vector<int> use;
		std::deque<int> freeSlot;
        bool pCacheIsDirty;

        //! List of particles bouncing around
        ListOfParticles pParticles;
        ListOfElectrics pElectrics;

    public:
        //!
		static Model* currentParticleModel;

    }; // class FXManager



    extern FXManager fx_manager;


} // namespace TA3D

#endif // __TA3D_GFX_FXMANAGER_H__
