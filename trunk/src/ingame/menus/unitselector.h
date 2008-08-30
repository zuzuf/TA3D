#ifndef __TA3D_INGAME_MENUS_UNIT_SELECTOR_H__
# define __TA3D_INGAME_MENUS_UNIT_SELECTOR_H__

# include "base.h"
# include <vector>


namespace TA3D
{
namespace Menus
{


    /*! \class MapSelector
    **
    ** \brief 
    */
    class UnitSelector : public Abstract
    {
    public:
        /*!
        ** \brief Execute an instance of UnitSelector
        **
        ** \param preSelectedUnits The name of the file that contains selected units
        ** \param[out] useOnly Name of the useOnly file. 
        ** It is guaranteed this var will be set to preSelectedMap if cancel button is clicked (returned value = false)
        **
        ** \return True if selection has been validated by the user, false otherwise
        */
        static bool Execute(const String& preSelectedUnits, String& useOnly);

    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        UnitSelector();
        /*!
        ** \brief Constructor
        ** \param PreSelected map
        */
        UnitSelector(const String& preSelectedUnits);
        //! Destructor
        virtual ~UnitSelector();
        //@}


        /*!
        ** \brief Get the useOnly file created if any
        **
        ** This value is affected by the last call to execute(), and is empty by default
        **
        ** \return Name of the useOnly file, empty if none has been selected
        */
        const String& selected() const { return pUseOnly; }


    protected:
        virtual bool doInitialize();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();

    private:
        void reloadUnitsForGUIControl();
        bool doGoSelectSingleUnit(const int unitIndex);
        void doUpdateUnitInfo();
        void createUseOnlyFile();

    private:
        //! Name of the UseOnly file
        String pUseOnly;
        //! Default Name of the UseOnly file
        String pDefaultUseOnly;

        //! List of units
        String::List pUnitList;

        //! Reference to the unit picture object (Gui)
        GUIOBJ* pUnitPicObj;

        //! Last selected index
        int pLastUnitIndex;

        //! The list of units (Gui)
        GUIOBJ* pUnitListObj;
    }; // class MapSelector




} // namespace Menus
} // namespace TA3D



#endif // __TA3D_INGAME_MENUS_MAP_SELECTOR_H__
