#ifndef __TA3D_INGAME_WEAPONS_MANAGER_H__
# define __TA3D_INGAME_WEAPONS_MANAGER_H__

# include "weapons.def.h"


namespace TA3D
{


    /*! \class WEAPON_MANAGER
    **
    ** \brief Manager for all king of weapons
    */
    class WEAPON_MANAGER
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        WEAPON_MANAGER();
        //! Destructor
        ~WEAPON_MANAGER();
        //@}

        void init();
        void destroy();

        /*!
        ** \brief Add a weapon in the list
        ** \param name
        ** \return
        */
        int add_weapon(const char* name);

        /*!
        ** \brief Load a TDF file
        */
        void load_tdf(char *data, const int size = 99999999);

        /*!
        ** \brief
        ** \param name
        */
        int get_weapon_index(const char *name)
        {
            return (!name || nb_weapons <= 0 || '\0' == *name) ? -1 : (weapon_hashtable.Find(Lowercase(name)) - 1);
        }


    public:
        //! Count of registered weapons
        int			nb_weapons;
        WEAPON_DEF	*weapon;
        //! Animation for firing
        ANIM		cannonshell;
        //! hashtable used to speed up operations on WEAPON_DEF objects
        cHashTable< int >	weapon_hashtable;

    }; // class WEAPON_MANAGER



    extern WEAPON_MANAGER weapon_manager;

}

#endif // __TA3D_INGAME_WEAPONS_MANAGER_H__
