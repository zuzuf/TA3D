#ifndef __TA3D_INGAME_MENUS_BASE_H__
# define __TA3D_INGAME_MENUS_BASE_H__

# include "../../stdafx.h"
# include <string>


namespace TA3D
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
        //{
        //! Default constructor
        Abstract();
        //! Destructor
        virtual ~Abstract() {}
        //}


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
        virtual bool doExecute() = 0;

        /*!
        ** \brief Release all internal variables
        **
        ** This method is called even if doInitialize() has previously returned false
        **
        ** \see doInitialize()
        */
        virtual void doFinalize() = 0;

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
        std::string pTypeName;

    }; // class Abstract




} // namespace Menus
} // namespace TA3D


#endif // __TA3D_INGAME_MENUS_BASE_H__
