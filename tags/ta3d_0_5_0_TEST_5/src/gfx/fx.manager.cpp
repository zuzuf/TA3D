
#include "fx.manager.h"
#include "../misc/math.h"


namespace TA3D
{

    FXManager fx_manager;
    MODEL* FXManager::currentParticleModel = NULL;



    int FXManager::add(const String& filename, const String& entryName, const Vector3D& pos, const float size)
    {
        if(Camera::inGame != NULL && ((Vector3D)(pos - Camera::inGame->pos)).sq() >= Camera::inGame->zfar2)
            return -1;

        MutexLocker locker(pMutex);

        if(nb_fx+1>max_fx)
        {
            max_fx+=100;
            FX* n_fx = (FX*) malloc(sizeof(FX)*max_fx);
            memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
            for (int i = max_fx - 100; i < max_fx; ++i)
                n_fx[i].init();
            free(fx);
            fx = n_fx;
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
        fullname += entryName;

        int anm_idx = findInCache((char*)fullname.c_str());
        if(anm_idx == -1)
        {
            byte *data;
            if(strcasecmp(filename.c_str(),"fx"))
                data = HPIManager->PullFromHPI(tmp);
            else
                data = fx_data;
            if(data)
            {
                ANIM *anm=new ANIM;
                anm->init();
                anm->load_gaf(data,get_gaf_entry_index(data, entryName.c_str()));
                // Next line has been removed in order to remain thread safe, conversion is done in main thread
                //			anm->convert(false,true);
                pCacheIsDirty = true;				// Set cache as dirty so we will do conversion at draw time

                anm_idx = putInCache(fullname, anm);

                if(data != fx_data)
                    delete[] data;
            }
        }
        else
            use[anm_idx]++;
        fx[idx].load(anm_idx, pos, size);

        return idx;
    }


    void FXManager::loadData()
    {
        pMutex.lock();
        currentParticleModel = model_manager.get_model("fxpart");
        // Reload the texture for flashes
        if (flash_tex == 0)
            flash_tex = gfx->load_texture("gfx/flash.tga");
        // Reload the texture for ripples
        if (ripple_tex == 0)
            ripple_tex = gfx->load_texture("gfx/ripple.tga");
        // Reload textures for waves
        if (wave_tex[0] == 0)
            wave_tex[0] = gfx->load_texture("gfx/wave0.tga");
        if (wave_tex[1] == 0)
            wave_tex[1] = gfx->load_texture("gfx/wave1.tga");
        if (wave_tex[2] == 0)
            wave_tex[2] = gfx->load_texture("gfx/wave2.tga");
        pMutex.unlock();
    }


    int FXManager::addFlash(const Vector3D& pos, const float size)
    {
        if(Camera::inGame != NULL && ((Vector3D)(pos - Camera::inGame->pos)).sq() >= Camera::inGame->zfar2)
            return -1;

        MutexLocker locker(pMutex);
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



    int FXManager::addWave(const Vector3D& pos,float size)
    {
        if (Camera::inGame != NULL && ((Vector3D)(pos-Camera::inGame->pos)).sq() >= Camera::inGame->zfar2)
            return -1;
        
        MutexLocker locker(pMutex);

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
        for (int i = 0; i < max_fx; ++i)
        {
            if(!fx[i].playing)
            {
                idx=i;
                break;
            }
        }
        fx[idx].load(-2 - (Math::RandFromTable() % 3), pos, size * 4.0f);

        return idx;
    }

    int FXManager::addRipple(const Vector3D& pos,float size)
    {
        if (Camera::inGame != NULL && ((Vector3D)(pos - Camera::inGame->pos)).sq() >= Camera::inGame->zfar2)
            return -1;
        
        MutexLocker locker(pMutex);

        if(nb_fx + 1 > max_fx)
        {
            max_fx += 100;
            FX* n_fx = (FX*)malloc(sizeof(FX) * max_fx);
            memcpy(n_fx, fx, sizeof(FX) * (max_fx - 100));
            for (int i = max_fx - 100;i < max_fx; ++i)
                n_fx[i].init();
            free(fx);
            fx=n_fx;
        }
        ++nb_fx;
        int idx = -1;
        for (int i = 0; i < max_fx; ++i)
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




    void FXManager::doClearAllParticles()
    {
        if (!pParticles.empty())
        {
            for (ListOfParticles::iterator i = pParticles.begin(); i != pParticles.end(); ++i)
                delete (*i);
            pParticles.clear();
        }
    }

    void FXManager::init()
    {
        doClearAllParticles();
        currentParticleModel = NULL;
        pCacheIsDirty = false;
        fx_data = NULL;
        max_fx = 0;
        nb_fx = 0;
        fx = NULL;

        max_cache_size = 0;
        cache_size = 0;
        cache_name = NULL;
        cache_anm = NULL;
        use=NULL;

        flash_tex = 0;
        wave_tex[0] = 0;
        wave_tex[1] = 0;
        wave_tex[2] = 0;
        ripple_tex = 0;
    }



    void FXManager::destroy()
    {
        doClearAllParticles();

        gfx->destroy_texture(flash_tex);
        gfx->destroy_texture(ripple_tex);
        gfx->destroy_texture(wave_tex[0]);
        gfx->destroy_texture(wave_tex[1]);
        gfx->destroy_texture(wave_tex[2]);

        if(fx_data)
            delete[] fx_data;
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
                for (int i = 0; i < max_cache_size; ++i)
                {
                    if(cache_name[i])
                        free(cache_name[i]);
                }
                free(cache_name);
            }
            if (cache_anm)
            {
                for (int i = 0;i < max_cache_size; ++i)
                {
                    if (cache_anm[i])
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

    void FXManager::draw(Camera& cam, MAP *map, float w_lvl, bool UW)
    {
        pMutex.lock();

        if (pCacheIsDirty)// We have work to do
        {
            for (int i = 0 ; i < max_cache_size ; ++i)
            {
                if (cache_anm[i])
                    cache_anm[i]->convert(false,true);
            }
            pCacheIsDirty = false;
        }

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        cam.setView();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f,-1600.0f);
        if(UW)
        {
            for(int i=0;i<max_fx;i++)
            {
                if( fx[i].playing && fx[i].Pos.y<w_lvl )
                    fx[i].draw(cam, map, cache_anm);
            }
        }
        else
        {
            for(int i = 0; i < max_fx; ++i)
            {
                if( fx[i].playing && fx[i].Pos.y >= w_lvl )
                    fx[i].draw(cam, map, cache_anm);
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f,0.0f);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        if(!UW && lp_CONFIG->explosion_particles)
        {
            for (ListOfParticles::iterator i = pParticles.begin(); i != pParticles.end(); ++i)
                (*i)->draw();
        }

        pMutex.unlock();
    }

    void FXManager::addParticle(const Vector3D& p, const Vector3D& s, const float l)
    {
        if (lp_CONFIG->explosion_particles)
        {
            pMutex.lock();
            pParticles.push_back(new FXParticle(p, s, l));
            pMutex.unlock();
        }
    }

    void FXManager::addExplosion(const Vector3D& p, const int n, const float power)
    {
        if (!lp_CONFIG->explosion_particles)
            return;
        pMutex.lock();
        for (int i = 0 ; i < n ; ++i) 
        {
            float a = (Math::RandFromTable() % 36000) * 0.01f * DEG2RAD;
            float b = (Math::RandFromTable() % 18000) * 0.01f * DEG2RAD;
            float s = power * ((Math::RandFromTable() % 9001) * 0.0001f + 0.1f);
            Vector3D vs(s * cos(a) * cos(b), 
                        s * sin(b),
                        s * sin(a) * cos(b));
            float l = Math::Min(5.0f * vs.y / (the_map->ota_data.gravity + 0.1f), 10.0f);

            pParticles.push_back(new FXParticle(p, vs, l));
        }
        pMutex.unlock();
    }


    int FXManager::putInCache(const String& filename, ANIM *anm)
    {
        // Already available in the cache ?
        const int is_in = findInCache(filename);
        if(is_in >= 0)
            return is_in;

        int idx = -1;
        if(cache_size + 1 > max_cache_size)
        {
            max_cache_size += 100;
            char **n_name = (char**)malloc(sizeof(char*)*max_cache_size);
            ANIM **n_anm = (ANIM**)malloc(sizeof(ANIM*)*max_cache_size);
            int *n_use = (int*)malloc(sizeof(int)*max_cache_size);
            for (int i = max_cache_size - 100; i < max_cache_size; ++i)
            {
                n_use[i] = 0;
                n_name[i] = NULL;
                n_anm[i] = NULL;
            }
            if (cache_size > 0)
            {
                for(int i = 0; i < max_cache_size - 100; ++i)
                {
                    n_name[i] = cache_name[i];
                    n_anm[i] = cache_anm[i];
                    n_use[i] = use[i];
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
            idx = 0;
            for(int i = 0; i < max_cache_size; ++i)
            {
                if(cache_anm[i]==NULL)
                    idx=i;
            }
        }
        use[idx] = 1;
        cache_anm[idx] = anm;
        cache_name[idx] = strdup(filename.c_str());
        ++cache_size;

        return idx;
    }


    int FXManager::findInCache(const String& filename) const
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


    void FXManager::doMoveAllParticles(const float& dt)
    {
        for (ListOfParticles::iterator i = pParticles.begin(); i != pParticles.end(); )
        {
            if ((*i)->move(dt))
            {
                delete (*i);
                pParticles.erase(i++);
            }
            else
                ++i;
        }
    }


    void FXManager::doMoveAllFX(const float& dt)
    {
        for (int i = 0; i < max_fx; ++i)
        {
            if(fx[i].move(dt, cache_anm))
            {
                if (fx[i].anm == -1 || fx[i].anm == -2 || fx[i].anm == -3 || fx[i].anm == -4 || fx[i].anm == -5)
                {
                    --nb_fx;
                    continue;		// Flash, ripple or Wave
                }
                use[fx[i].anm]--;
                --nb_fx;
                if (use[fx[i].anm] <= 0) // Animation used nowhere
                {
                    free(cache_name[fx[i].anm]);
                    cache_name[fx[i].anm] = NULL;
                    cache_anm[fx[i].anm]->destroy();
                    delete cache_anm[fx[i].anm];
                    cache_anm[fx[i].anm] = NULL;
                    --cache_size;
                }
            }
        }

    }


    void FXManager::move(const float dt)
    {
        pMutex.lock();
        doMoveAllFX(dt);
        doMoveAllParticles(dt);
        pMutex.unlock();
    }


    FXManager::FXManager()
    {
        init();
    }

    FXManager::~FXManager()
    {
        destroy();
    }


} // namespace TA3D
