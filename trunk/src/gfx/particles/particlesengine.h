#ifndef __TA3D_GFX_PARTICLES_ENGINE_H__
# define __TA3D_GFX_PARTICLES_ENGINE_H__

# include "../../stdafx.h"
# include "../../threads/cThread.h"
# include "../../threads/thread.h"
# include "../../misc/vector.h"
# include "../../ta3dbase.h"
# include "../../misc/camera.h"
# include "particlessystem.h"
# include "particle.h"
# include <list>
# include <vector>



namespace TA3D
{


    /*! \class PARTICLE_ENGINE
    **
    ** \brief Engine for particles
    */
    class PARTICLE_ENGINE : public ObjectSync, public cThread
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
        int addtex(const String& file,const String& filealpha = "");


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
        void emit_part(Vector3D pos,Vector3D Dir,int tex,int nb,float speed,float life=10.0f,float psize=10.0f,bool white=false,float trans_factor=1.0f);

        /*!
        ** \brief
        */
        ParticlesSystem *emit_part_fast( ParticlesSystem *system, Vector3D pos, Vector3D Dir,
                                         int tex, int nb, float speed, float life=10.0f,
                                         float psize=10.0f, bool white=false, float trans_factor=1.0f );

        /*!
        ** \brief
        */
        void emit_lava(Vector3D pos,Vector3D Dir,int tex,int nb,float speed,float life=10.0f);

        /*!
        ** \brief
        */
        void make_smoke(Vector3D pos,int tex,int nb,float speed,float mass=-1.0f, float ddsize=0.0f,float alpha=1.0f);

        /*!
        ** \brief
        */
        void make_dark_smoke(Vector3D pos,int tex,int nb,float speed,float mass=-1.0f, float ddsize=0.0f,float alpha=1.0f);

        /*!
        ** \brief
        */
        void make_fire(Vector3D pos,int tex,int nb,float speed);

        /*!
        ** \brief
        */
        void make_shockwave(Vector3D pos,int tex,int nb,float speed);

        /*!
        ** \brief
        */
        void make_nuke(Vector3D pos,int tex,int nb,float speed);

        /*!
        ** \brief
        **
        ** \param dt
        ** \param wind_dir
        ** \param g Constant of Gravity
        */
        void move(float dt,Vector3D wind_dir,float g = 9.81f);


        /*!
        ** \brief
        **
        ** \param cam
        ** \param map_w
        ** \param map_h
        ** \param bloc_w
        ** \param bloc_h
        ** \param bmap
        */
        void draw(Camera *cam,int map_w,int map_h,int bloc_w,int bloc_h, byte **bmap);


    public:
        uint32		nb_part;		// Nombre de particules
        uint32		size;			// Quantité maximale de particules stockables dans le tableau
        std::vector<PARTICLE>	part;	// Liste des particules / Particle list
        GLuint		parttex;		// Textures des particules
        BITMAP		*partbmp;		// Textures des particules
        bool		dsmoke;
        uint32		ntex;
        std::vector< GLuint >	gltex;

        Vector3D	*point;			// Vertex array
        GLfloat		*texcoord;		// Texture coordinates array
        GLubyte		*color;			// Color array

    protected:
        bool  thread_running;
        bool  thread_ask_to_stop;
        Vector3D* p_wind_dir;
        float* p_g;
        std::vector<ParticlesSystem*> particle_systems;
        int  Run();
        void  SignalExitThread();

    }; // class PARTICLE_ENGINE



} // namespace TA3D

#endif // __TA3D_GFX_PARTICLES_ENGINE_H__
