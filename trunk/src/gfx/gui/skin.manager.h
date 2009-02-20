#ifndef __TA3D_GFX_GUI_SKIN_MANAGER_H__
# define __TA3D_GFX_GUI_SKIN_MANAGER_H__

#include "skin.h"
#include "../../misc/hash_table.h"

namespace TA3D
{


    /*! \class SKIN_MANAGER
    **
    ** \brief
    */
    class SKIN_MANAGER
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        SKIN_MANAGER();
        //! Destructor
        ~SKIN_MANAGER();
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
        ** \brief
        **
        ** \param filename
        */
        SKIN *load(const String& filename, const float scale = 1.0f);

    private:
        std::vector<SKIN*>          skins;
        UTILS::cHashTable<SKIN*>    hash_skin;
    }; // class SKIN_MANAGER

    extern SKIN_MANAGER skin_manager;

}

#endif // __TA3D_GFX_GUI_SKIN_MANAGER_H__
