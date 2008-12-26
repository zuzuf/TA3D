#include "weapons.ingame.h"
#include "../../EngineClass.h"
#include "../../misc/camera.h"



namespace TA3D
{

    INGAME_WEAPONS weapons;


    void INGAME_WEAPONS::set_data(MAP* map)
    {
        p_map = map;
    }

    void INGAME_WEAPONS::init(bool real)
    {
        pMutex.lock();

        thread_running = false;
        thread_ask_to_stop = false;

        index_list_size = 0;
        idx_list = NULL;
        free_index_size = 0;
        free_idx = NULL;

        nb_weapon=0;
        max_weapon=0;
        weapon=NULL;
        nuclogo.init();
        if(real)
        {
            byte *data=HPIManager->PullFromHPI("anims\\fx.gaf");
            if(data)
            {
                nuclogo.load_gaf(data,get_gaf_entry_index(data,"nuclogo"));
                nuclogo.convert();
                nuclogo.clean();
                delete[] data;
            }
        }
        pMutex.unlock();
    }


    void INGAME_WEAPONS::destroy()
    {
        DestroyThread();

        pMutex.lock();
        if(idx_list)
            delete[] idx_list;
        if(free_idx)
            delete[] free_idx;
        index_list_size = 0;		idx_list = NULL;
        free_index_size = 0;		free_idx = NULL;
        if(max_weapon>0 && weapon)
            free(weapon);
        nuclogo.destroy();
        pMutex.unlock();
        init(false);
    }


    INGAME_WEAPONS::INGAME_WEAPONS()
    {
        InitThread();
        init(false);
    }

    INGAME_WEAPONS::~INGAME_WEAPONS()
    {
        destroy();
    }


    int INGAME_WEAPONS::add_weapon(int weapon_id,int shooter)
    {
        if(weapon_id < 0)
            return -1;

        MutexLocker locker(pMutex);

        if(nb_weapon<max_weapon)// S'il y a encore de la place
        {
            uint32 i = free_idx[--free_index_size];
            idx_list[index_list_size++] = i;
            nb_weapon++;
            weapon[i].init();
            weapon[i].weapon_id=weapon_id;
            weapon[i].shooter_idx=shooter;
            weapon[i].idx=i;
            weapon[i].f_time=weapon_manager.weapon[weapon_id].flighttime;
            return i;
        }
        max_weapon += 100; // Augmente la taille du tableau
        WEAPON *new_weapon=(WEAPON*) malloc(sizeof(WEAPON)*max_weapon);

        uint32	*n_idx = new uint32[max_weapon];
        uint32	*n_new_idx = new uint32[max_weapon];
        if(index_list_size>0)
            memcpy(n_idx,idx_list,index_list_size<<2);
        if(free_index_size>0)
            memcpy(n_new_idx,free_idx,free_index_size<<2);
        if(idx_list)	delete[]	idx_list;
        if(free_idx)	delete[]	free_idx;
        idx_list = n_idx;
        free_idx = n_new_idx;
        for(uint32 i = max_weapon-100; i<max_weapon;i++)
            free_idx[free_index_size++] = i;

        for(uint32 i=0;i<max_weapon;i++)
            new_weapon[i].weapon_id=-1;
        if(weapon && nb_weapon>0)
            for(uint32 i=0;i<nb_weapon;i++)
                new_weapon[i]=weapon[i];
        if(weapon)
            free(weapon);
        weapon=new_weapon;
        uint32 index = free_idx[--free_index_size];
        idx_list[index_list_size++] = index;
        nb_weapon++;
        weapon[index].init();
        weapon[index].weapon_id=weapon_id;
        weapon[index].shooter_idx=shooter;
        weapon[index].idx=index;
        weapon[index].f_time=weapon_manager.weapon[weapon_id].flighttime;
        return index;
    }

    void INGAME_WEAPONS::move(float dt,MAP *map)
    {
        pMutex.lock();
        if (nb_weapon <= 0 || max_weapon <= 0)
        {
            pMutex.unlock();
            rest(1);
            return;
        }

        for (uint32 e=0;e<index_list_size; ++e)
        {
            // TODO Check if it is really necessary by now
            pMutex.unlock();// Pause to give the renderer the time to work and to go at the given engine speed (in ticks per sec.)
            pMutex.lock();

            uint32 i = idx_list[e];
            weapon[i].move(dt,map);
            if (weapon[i].weapon_id<0) // Remove it from the "alive" list
            {
                --nb_weapon;
                free_idx[free_index_size++] = i;
                idx_list[e--] = idx_list[--index_list_size];
            }
        }
        pMutex.unlock();
    }



    void INGAME_WEAPONS::draw(Camera* cam, MAP* map, bool underwater)
    {
        pMutex.lock();
        if(nb_weapon<=0 || max_weapon<=0)
        {
            pMutex.unlock();
            return;
        }

        gfx->lock();
        if (cam)
            cam->setView();

        for(uint32 e=0;e<index_list_size;e++)
        {
            uint32 i = idx_list[e];
            if((weapon[i].Pos.y<map->sealvl && underwater) || (weapon[i].Pos.y>=map->sealvl && !underwater))
                weapon[i].draw(cam,map);
        }

        gfx->unlock();
        pMutex.unlock();
    }



    void INGAME_WEAPONS::draw_mini(float map_w,float map_h,int mini_w,int mini_h)				// Repère les unités sur la mini-carte
    {
        MutexLocker locker(pMutex);

        if(nb_weapon<=0 || max_weapon<=0)
            return;

        float rw = 128.0f * mini_w / 252;
        float rh = 128.0f * mini_h / 252;

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_POINTS);
        for (uint32 e = 0; e < index_list_size; ++e)
        {
            uint32 i = idx_list[e];
            if(weapon_manager.weapon[weapon[i].weapon_id].cruise || weapon_manager.weapon[weapon[i].weapon_id].interceptor)
            {
                glEnd();
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                int idx=weapon[i].owner;
                PutTex(nuclogo.glbmp[idx],weapon[i].Pos.x/map_w*rw+64.0f-nuclogo.ofs_x[idx],weapon[i].Pos.z/map_h*rh+64.0f-nuclogo.ofs_y[idx],weapon[i].Pos.x/map_w*rw+63.0f-nuclogo.ofs_x[idx]+nuclogo.w[idx],weapon[i].Pos.z/map_h*rh+63.0f-nuclogo.ofs_y[idx]+nuclogo.h[idx]);
                glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_POINTS);
            }
            else
                glVertex2f(weapon[i].Pos.x/map_w*rw+64.0f,weapon[i].Pos.z/map_h*rh+64.0f);
        }
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

    
    int INGAME_WEAPONS::Run()
    {
        thread_running = true;
        float dt = 1.0f / TICKS_PER_SEC;
        int weapon_timer = msec_timer;
        int counter = 0;

        weapon_engine_thread_sync = 0;

        while (!thread_ask_to_stop)
        {
            ++counter;
            move(dt, p_map);					// Animate weapons
            features.move_forest(dt);			// Animate the forest

            ThreadSynchroniser->lock();
            ThreadSynchroniser->unlock();

            weapon_engine_thread_sync = 1;

            while (weapon_engine_thread_sync && !thread_ask_to_stop)
                rest(1);			// Wait until other thread sync with this one
        }
        thread_running = false;
        thread_ask_to_stop = false;
        LOG_INFO("Weapon engine: " << (float)(counter * 1000) / (msec_timer - weapon_timer) << " ticks/sec");
        return 0;
    }


    void INGAME_WEAPONS::SignalExitThread()
    {
        if (thread_running)
            thread_ask_to_stop = true;
    }



}
