#ifndef __TA3D_GFX_FX_MANAGER_H__
# define __TA3D_GFX_FX_MANAGER_H__

# include "../stdafx.h"
# include "../threads/thread.h"
# include "../gaf.h"
# include "../ta3dbase.h"
# include "../EngineClass.h"
# include "fx.base.h"
# include "fx.particle.h"


namespace TA3D
{

    class FX_MANAGER : public ObjectSync	// This class mustn't be executed in its own thread in order to remain thread safe,
    {												// it must run in main thread (the one that can call OpenGL functions)!!
    public:
        FX_MANAGER();
        ~FX_MANAGER();


        void init();
        void destroy();
        void load_data();
        void move(const float dt);
        void draw(CAMERA *cam, MAP *map, float w_lvl=0.0f, bool UW=false);
        int add(const String& filename,char *entry_name,VECTOR Pos,float size);
        int addFlash(const VECTOR& Pos,float size);

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
        void addExplosion(const  VECTOR &p, const int n, const float power);


    public:
        byte		*fx_data;
        GLuint		flash_tex;
        GLuint		wave_tex[3];
        GLuint		ripple_tex;

    private:
        /*!
        ** \brief
        ** \param filename
        ** \return The index of the item, -1 if not found
        */
        int isInCache(const String& filename);

        /*!
        ** \brief
        ** \param filename
        ** \param anm
        ** \return
        */
        int putInCache(char *filename,ANIM *anm);

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

        List<FX_PARTICLE> particles;			// List of particles bouncing around

    public:
        //! 
        static MODEL* currentParticleModel;

    }; // class FX_MANAGER



    extern FX_MANAGER fx_manager;


} // namespace TA3D

#endif // __TA3D_GFX_FX_MANAGER_H__
