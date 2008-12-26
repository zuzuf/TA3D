#ifndef __TA3D_INGAME_MENUS_SOLO_H__
# define __TA3D_INGAME_MENUS_SOLO_H__

# include "base.h"


namespace TA3D
{
namespace Menus
{


    /*! \class Solo
    **
    ** \brief The Solo menu, to launch a solo campaign, a skirmish, or load a savegame 
    */
    class Solo : public Abstract
    {
    public:
        /*!
        ** \brief Execute an instance of MainMenu
        */
        static bool Execute();

    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Solo();
        //! Destructor
        virtual ~Solo();
        //@}

    protected:
        virtual bool doInitialize();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();

    private:
        bool doDisplayAllSavegames();
        /*!
        ** \brief Go to the loading room 
        ** \return Always equals to false
        */
        bool doGoMenuLoadSingleGame();
    
        /*!
        ** \brief Go to Campaign menu
        ** \return Always equals to false
        */
        bool doGoMenuCompaign();

        /*!
        ** \brief Go to the Skirmish menu
        ** \return Always equals to false
        */
        bool doGoMenuSkirmish();

    }; // class Solo


} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_SOLO_H__
