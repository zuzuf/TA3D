#ifndef __EDITOR_MENUS_BASE_H__
# define __EDITOR_MENUS_BASE_H__

# include "../stdafx.h"
# include <string>
# include <memory>
# include "../gui.h"
# include "../gfx/gui/area.h"


# define TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING  8

namespace Editor
{

/*!
** \brief All kinds of menu that a player might encounter 
*/
namespace Menus
{


    /*! \class Abstract
    **
    ** \brief Abstract class to manage a single menu
    */
    class Abstract
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Abstract();
        //! Destructor
        virtual ~Abstract() {}
        //@}


        /*!
        ** \brief Execute the menu
        **
        ** \return True if the operation succeeded, false otherwise
        **
        ** \see doInitialize()
        ** \see doExecute()
        ** \see doFinalize()
        */
        bool execute();

    protected:
        /*!
        ** \brief Reset all internal variables
        ** \return True if the operation succeeded, False otherwise
        ** \see doExecute
        ** \see doFinalize()
        */
        virtual bool doInitialize() = 0;

        /*!
        ** \brief Execute the local implementation for the menu
        ** \return True if the operation succeeded
        ** \warning This method will not be called if doInitialize() has previously returned false
        ** \see doInitialize()
        */
        virtual bool doExecute();

        /*!
        ** \brief Release all internal variables
        **
        ** This method is called even if doInitialize() has previously returned false
        **
        ** \see doInitialize()
        */
        virtual void doFinalize() = 0;

        /*!
        ** \brief Wait for an user event (mouse, keyboard...)
        */
        virtual void waitForEvent() = 0;

        /*!
        ** \brief Init the area	with a given TDF filename
        **
        ** \see AREA::load_tdf()
        ** \see pArea
        */
        void loadAreaFromTDF(const String& caption, const String& relFilename);

        /*!
        ** \brief Execute another menu according the user inputs
        ** \return True if the execution should be aborted, false otherwise
        */
        virtual bool maySwitchToAnotherMenu() = 0;

        /*!
        ** \brief Redraw the entire screen using the Area
        ** \see pArea
        */
        virtual void redrawTheScreen();

        /*!
        ** 
        */
        virtual bool doLoop();

    protected:
        //! Our Window handle
        std::auto_ptr<AREA> pArea;

        //! Last value of `mouse_x`
        int pMouseX;
        //! last value of `mouse_y`
        int pMouseY;
        //! Last value of `mouse_z`
        int pMouseZ;
        //! Last value of `mouse_b`
        int pMouseB;

    private:
        /*!
        ** \brief Call doInitialize() using Guards functions
        ** \see doInitialize()
        */
        bool doGuardInitialize();
        /*!
        ** \brief Call doExecute() using Guards functions
        ** \see doExecute()
        */
        bool doGuardExecute();
        /*!
        ** \brief Call doFinalize() using Guards functions
        ** \see doFinalize()
        */
        void doGuardFinalize();

    private:
        //! Cached value for the current class name
        String pTypeName;

    }; // class Abstract




} // namespace Menus
} // namespace Editor


#endif // __EDITOR_MENUS_BASE_H__
