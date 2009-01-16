#ifndef __TA3D_INGAME_WEAPONS_ING_H__
# define __TA3D_INGAME_WEAPONS_ING_H__

# include "../../stdafx.h"
# include "../../threads/thread.h"
# include "../../threads/cThread.h"
# include "weapons.h"
# include "weapons.single.h"
# include "../../misc/camera.h"


namespace TA3D
{


    /*! \class INGAME_WEAPONS
    **
    ** \brief
    */
    class INGAME_WEAPONS : public ObjectSync, public cThread
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        INGAME_WEAPONS();
        //! Destructor
        ~INGAME_WEAPONS();
        //@}


        /*!
        ** \brief
        ** \param map
        */
        void set_data(MAP* map);

        /*!
        ** \brief
        ** \param real
        */
        void init(bool real = true);

        /*!
        ** \brief
        */
        void destroy();


        /*!
        ** \brief
        ** \param weapon_id
        ** \param shooter
        */
        int add_weapon(int weapon_id,int shooter);

        /*!
        ** \brief
        */
        void move(const float dt,MAP *map);

        /*!
        ** \brief
        */
        void draw(Camera *cam = NULL, MAP *map = NULL, bool underwater = false);

        /*!
        ** \brief
        */
        void draw_mini(float map_w, float map_h, int mini_w, int mini_h); // Repère les unités sur la mini-carte


    public:
        //! Weapons count
        uint32 nb_weapon;			// Nombre d'armes
        //!
        std::vector< WEAPON > weapon;			// Tableau regroupant les armes
        //!
        Gaf::Animation nuclogo;			// Logos des armes atomiques sur la minicarte / Logo of nuclear weapons on minimap

        //!
        std::vector< uint32 > idx_list;
        //!
        std::vector< uint32 > free_idx;

    protected:
        //!
        bool thread_running;
        //!
        bool thread_ask_to_stop;
        //!
        MAP* p_map;
        //!
        int	Run();
        //!
        void SignalExitThread();

    }; // class INGAME_WEAPONS



    extern INGAME_WEAPONS weapons;


}

#endif // __TA3D_INGAME_WEAPONS_ING_H__
