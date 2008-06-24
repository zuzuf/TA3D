
#include "fx.manager.h"


namespace TA3D
{

    FX_MANAGER	fx_manager;
    MODEL* FX_MANAGER::currentParticleModel = NULL;



    int FX_MANAGER::add(const String&filename,char *entry_name,VECTOR Pos,float size)
    {
        MutexLocker locker(pMutex);

        if(game_cam != NULL && ((VECTOR)(Pos-game_cam->Pos)).sq() >= game_cam->zfar2)
            return -1;


        if(nb_fx+1>max_fx)
        {
            max_fx+=100;
            FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
            memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
            for(int i=max_fx-100;i<max_fx;i++)
                n_fx[i].init();
            free(fx);
            fx=n_fx;
        }
        ++nb_fx;
        int idx = -1;

        for(int i = 0;i < max_fx; ++i)
        {
            if(!fx[i].playing)
            {
                idx=i;
                break;
            }
        }
        
        String tmp("anims\\");
        tmp += filename;
        tmp += ".gaf";

        String fullname(tmp);
        fullname += "-";
        fullname += entry_name;

        int anm_idx = isInCache((char*)fullname.c_str());
        if(anm_idx == -1)
        {
            byte *data;
            if(strcasecmp(filename.c_str(),"fx"))
                data = HPIManager->PullFromHPI(tmp);
            else
                data = fx_manager.fx_data;
            if(data)
            {
                ANIM *anm=new ANIM;
                anm->init();
                anm->load_gaf(data,get_gaf_entry_index(data,entry_name));
                // Next line has been removed in order to remain thread safe, conversion is done in main thread
                //			anm->convert(false,true);
                pCacheIsDirty = true;				// Set cache as dirty so we will do conversion at draw time

                anm_idx = putInCache((char*)fullname.c_str(),anm);

                if(data!=fx_manager.fx_data)
                    free(data);
            }
        }
        else
            use[anm_idx]++;
        fx[idx].load(anm_idx,Pos,size);

        return idx;
    }


    void FX_MANAGER::load_data()
    {
        pMutex.lock();
        currentParticleModel = model_manager.get_model("fxpart");
        if (flash_tex == 0)
            flash_tex = gfx->load_texture( "gfx/flash.tga" );
        if (ripple_tex == 0)
            ripple_tex = gfx->load_texture( "gfx/ripple.tga" );
        if (wave_tex[0] == 0)
            wave_tex[0] = gfx->load_texture( "gfx/wave0.tga" );
        if (wave_tex[1] == 0)
            wave_tex[1] = gfx->load_texture( "gfx/wave1.tga" );
        if (wave_tex[2] == 0)
            wave_tex[2] = gfx->load_texture( "gfx/wave2.tga" );
        pMutex.unlock();
    }

    int FX_MANAGER::addFlash(const VECTOR& pos, float size)
    {
        MutexLocker locker(pMutex);

        if(game_cam!=NULL && ((VECTOR)(pos-game_cam->Pos)).sq()>=game_cam->zfar2)
            return -1;

        if(nb_fx + 1 > max_fx)
        {
            max_fx+=100;
            FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
            memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
            for(int i=max_fx-100;i<max_fx;i++)
                n_fx[i].init();
            free(fx);
            fx=n_fx;
        }
        ++nb_fx;
        int idx=-1;
        for (int i=0;i<max_fx; ++i)
        {
            if(!fx[i].playing)
            {
                idx=i;
                break;
            }
        }
        fx[idx].load(-1, pos, size);

        return idx;
    }

    int FX_MANAGER::addWave(const VECTOR& pos,float size)
    {
        MutexLocker locker(pMutex);

        if (game_cam != NULL && ((VECTOR)(pos-game_cam->Pos)).sq()>=game_cam->zfar2)
            return -1;

        if(nb_fx+1>max_fx)
        {
            max_fx+=100;
            FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
            memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
            for(int i=max_fx-100;i<max_fx;i++)
                n_fx[i].init();
            free(fx);
            fx=n_fx;
        }
        ++nb_fx;
        int idx=-1;
        for(int i=0;i<max_fx;i++)
            if(!fx[i].playing)
            {
                idx=i;
                break;
            }
        fx[idx].load(-2-(rand_from_table()%3),pos,size*4.0f);

        return idx;
    }

    int FX_MANAGER::addRipple(const VECTOR& pos,float size)
    {
        MutexLocker locker(pMutex);

        if (game_cam != NULL && ((VECTOR)(pos - game_cam->Pos)).sq() >= game_cam->zfar2)
            return -1;

        if(nb_fx+1>max_fx)
        {
            max_fx+=100;
            FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
            memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
            for (int i = max_fx - 100;i < max_fx; ++i)
                n_fx[i].init();
            free(fx);
            fx=n_fx;
        }
        nb_fx++;
        int idx=-1;
        for (int i=0;i<max_fx; ++i)
        {
            if(!fx[i].playing)
            {
                idx=i;
                break;
            }
        }
        fx[idx].load(-5, pos, size);

        return idx;
    }

    void FX_MANAGER::init()
    {
        particles.clear();

        currentParticleModel = NULL;

        pCacheIsDirty = false;

        fx_data=NULL;

        max_fx=0;
        nb_fx=0;
        fx=NULL;

        max_cache_size=0;
        cache_size=0;
        cache_name=NULL;
        cache_anm=NULL;
        use=NULL;

        flash_tex = 0;
        wave_tex[0] = 0;
        wave_tex[1] = 0;
        wave_tex[2] = 0;
        ripple_tex = 0;
    }

    void FX_MANAGER::destroy()
    {
        particles.clear();

        gfx->destroy_texture( flash_tex );
        gfx->destroy_texture( ripple_tex );
        gfx->destroy_texture( wave_tex[0] );
        gfx->destroy_texture( wave_tex[1] );
        gfx->destroy_texture( wave_tex[2] );

        if(fx_data)	free(fx_data);
        if(fx)
        {
            for(int i=0;i<max_fx; ++i)
                fx[i].destroy();
            free(fx);
        }
        if (cache_size>0)
        {
            if (cache_name)
            {
                for(int i=0;i<max_cache_size;i++)
                    if(cache_name[i])
                        free(cache_name[i]);
                free(cache_name);
            }
            if (cache_anm)
            {
                for(int i=0;i<max_cache_size;i++)
                {
                    if(cache_anm[i])
                    {
                        cache_anm[i]->destroy();
                        delete cache_anm[i];
                    }
                }
                free(cache_anm);
            }
        }
        init();
    }

    void FX_MANAGER::draw(CAMERA *cam, MAP *map, float w_lvl, bool UW)
    {
        pMutex.lock();

        if( pCacheIsDirty )// We have work to do
        {
            for( uint32 i = 0 ; i < max_cache_size ; i++ )
            {
                if( cache_anm[i] )
                    cache_anm[i]->convert(false,true);
            }
            pCacheIsDirty = false;
        }

        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        cam->SetView();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f,-1600.0f);
        if(UW)
        {
            for(int i=0;i<max_fx;i++)
                if( fx[i].playing && fx[i].Pos.y<w_lvl )
                    fx[i].draw(cam,map,cache_anm);
        }
        else
        {
            for(int i = 0; i < max_fx; ++i)
            {
                if( fx[i].playing && fx[i].Pos.y >= w_lvl )
                    fx[i].draw(cam,map,cache_anm);
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f,0.0f);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        if(!UW && lp_CONFIG->explosion_particles)
        {
            foreach( particles, i )
                i->draw();
        }

        pMutex.unlock();
    }

    void FX_MANAGER::addParticle(const VECTOR &p, const VECTOR &s, const float l)
    {
        if (lp_CONFIG->explosion_particles)
        {
            pMutex.lock();
            particles.push_back(FX_PARTICLE(p, s, l));
            pMutex.unlock();
        }
    }

    void FX_MANAGER::addExplosion(const VECTOR &p, const int n, const float power)
    {
        if (!lp_CONFIG->explosion_particles)
            return;
        pMutex.lock();
        for (int i = 0 ; i < n ; ++i) 
        {
            float a = (rand_from_table() % 36000) * 0.01f * DEG2RAD;
            float b = (rand_from_table() % 18000) * 0.01f * DEG2RAD;
            float s = power * ((rand_from_table() % 9001) * 0.0001f + 0.1f);
            VECTOR vs(s * cos(a) * cos(b),
                      s * sin(b),
                      s * sin(a) * cos(b));
            float l = min( 5.0f * vs.y / (the_map->ota_data.gravity + 0.1f), 10.0f);

            particles.push_back(FX_PARTICLE(p, vs, l));
        }
        pMutex.unlock();
    }


    int FX_MANAGER::putInCache(char *filename,ANIM *anm)
    {
        MutexLocker locker(pMutex);

        int is_in = isInCache(filename);
        if(is_in>=0)
            return is_in;		// On ne le garde pas 2 fois
        int idx=-1;
        if(cache_size + 1 > max_cache_size)
        {
            max_cache_size += 100;
            char **n_name=(char**)malloc(sizeof(char*)*max_cache_size);
            ANIM **n_anm=(ANIM**)malloc(sizeof(ANIM*)*max_cache_size);
            int *n_use=(int*)malloc(sizeof(int)*max_cache_size);
            for (int i = max_cache_size - 100; i < max_cache_size; ++i)
            {
                n_use[i]=0;
                n_name[i]=NULL;
                n_anm[i]=NULL;
            }
            if(cache_size>0)
            {
                for(int i = 0; i < max_cache_size - 100; ++i)
                {
                    n_name[i]=cache_name[i];
                    n_anm[i]=cache_anm[i];
                    n_use[i]=use[i];
                }
                free(cache_name);
                free(cache_anm);
                free(use);
            }
            use=n_use;
            cache_name=n_name;
            cache_anm=n_anm;
            idx=cache_size;
        }
        else
        {
            idx=0;
            for(int i=0; i < max_cache_size; ++i)
            {
                if(cache_anm[i]==NULL)
                    idx=i;
            }
        }
        use[idx] = 1;
        cache_anm[idx] = anm;
        cache_name[idx] = strdup(filename);
        ++cache_size;

        return idx;
    }


    int FX_MANAGER::isInCache(const String& filename)
    {
        if (cache_size <= 0)
            return -1;
        for(int i = 0; i < max_cache_size; ++i)
        {
            if(cache_anm[i] != NULL && cache_name[i] != NULL)
            {
                if (filename == cache_name[i])
                    return i;
            }
        }
        return -1;
    }


    void FX_MANAGER::move(const float dt)
    {
        pMutex.lock();

        for(int i=0;i<max_fx;i++)
            if(fx[i].move(dt,cache_anm))
            {
                if( fx[i].anm == -1 || fx[i].anm == -2 || fx[i].anm == -3 || fx[i].anm == -4 || fx[i].anm == -5 )
                {
                    --nb_fx;
                    continue;		// Flash, ripple or Wave
                }
                use[fx[i].anm]--;
                --nb_fx;
                if(use[fx[i].anm]<=0) // Animation used nowhere
                {
                    free(cache_name[fx[i].anm]);		cache_name[fx[i].anm]=NULL;
                    cache_anm[fx[i].anm]->destroy();	delete cache_anm[fx[i].anm];	cache_anm[fx[i].anm]=NULL;
                    --cache_size;
                }
            }

        foreach_( particles, i )
            if( i->move( dt ) )
                particles.erase( i++ );
            else
                i++;

        pMutex.unlock();
    }


    FX_MANAGER::FX_MANAGER()
    {
        init();
    }

    FX_MANAGER::~FX_MANAGER()
    {
        destroy();
    }


} // namespace TA3D
