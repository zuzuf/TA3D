#ifndef __TA3D_INGAME_MENUS_STATISTICS_H__
# define __TA3D_INGAME_MENUS_STATISTICS_H__


# include "base.h"


namespace TA3D
{
namespace Menus
{


    /*! \class Statistics
    **
    ** \brief All statistics about the last bloody battle
    */
    class Statistics : public Abstract
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
        Statistics();
        //! Destructor
        virtual ~Statistics();
        //@}

    protected:
        virtual bool doInitialize();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();

    private:
        /*!
        ** \brief Update the statistics for a single player and a single column
        **
        ** \param id The object on the area to update (pArea)
        ** \param indx Index of the player
        ** \param color The color of the player
        ** \param value The text
        */
        void doUpdateObject(const String& id, const short indx, const uint32 color, const String& value);

    }; // class Statistics


} // namespace Menus
} // namespace TA3D


#endif // __TA3D_INGAME_MENUS_STATISTICS_H__
