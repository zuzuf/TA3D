#ifndef __TA3D_GFX_FXMANAGER_H__
# define __TA3D_GFX_FXMANAGER_H__

# include "../stdafx.h"
# include "../threads/thread.h"
# include "../gaf.h"
# include "../ta3dbase.h"
# include "../EngineClass.h"
# include "fx.base.h"
# include "fx.particle.h"




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
        //{
        //! Default constructor
        FXManager();
        //! Destructor
        ~FXManager();
        //}

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
        */
        void move(const float dt);

        /*!
        ** \brief
        **
        ** \param cam
        ** \param map
        ** \param w_lvl
        ** \param UW
        */
        void draw(CAMERA& cam, MAP *map, float w_lvl = 0.0f, bool UW = false);

        /*!
        ** \brief
        **
        ** \param filename
        ** \param entryName
        ** \param pos
        ** \param size
        ** \return
        */
        int add(const String& filename, const String& entryName, const VECTOR& Pos, const float size);

        /*!
        ** \brief
        ** \param pos
        ** \param size
        */
        int addFlash(const VECTOR& pos, const float size);

        /*!
        ** \brief Add a wave
        ** \param pos
        ** \param size
        ** \return
        */
        int addWave(const VECTOR& Pos,float size);

        /*!
        ** \brief Add a ripple
        ** \param pos
        ** \param size
        ** \return
        */
        int addRipple(const VECTOR& Pos,float size);

        /*!
        ** \brief Add a particle
        ** \param p
        ** \param s
        ** \param l
        */
        void addParticle(const VECTOR &p, const VECTOR &s, const float l);

        /*!
        ** \brief Add an explosion effect
        ** \param p
        ** \param n
        ** \param power
        */
        void addExplosion(const VECTOR &p, const int n, const float power);


    public:
        byte		*fx_data;
        GLuint		flash_tex;
        GLuint		wave_tex[3];
        GLuint		ripple_tex;

    private:
        /*!
        ** \brief Find the index of a filename in the cache
        ** \param filename The filename to look for
        ** \return The index of the item, -1 if not found
        ** \warning This method is not thread-safe
        */
        int findInCache(const String& filename) const;

        /*!
        ** \brief
        ** \param filename
        ** \param anm
        ** \return
        ** \warning This method is not thread-safe
        */
        int putInCache(const String& filename, ANIM *anm);

    private:
        int			max_fx;
        int			nb_fx;
        FX			*fx;

        int			cache_size;			// Cache
        int			max_cache_size;
        char		**cache_name;
        ANIM		**cache_anm;
        int			*use;
        bool pCacheIsDirty;

        List<FX_PARTICLE> pParticles;	// List of particles bouncing around

    public:
        //! 
        static MODEL* currentParticleModel;

    }; // class FXManager



    extern FXManager fx_manager;


} // namespace TA3D

#endif // __TA3D_GFX_FXMANAGER_H__
