/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

/*----------------------------------------------------------------------\
|                               console.h                               |
|      contient les classes nécessaires à la gestion d'une console dans |
| programme utilisant Allegro avec ou sans AllegroGL. La console        |
| dispose de sa propre procédure d'entrée et d'affichage mais celle-ci  |
| nécessite d'être appellée manuellement pour éviter les problèmes      |
| découlant d'un appel automatique par un timer.                        |
\----------------------------------------------------------------------*/

#ifndef _TA3D_XX_CLASSE_CONSOLE_H__
# define _TA3D_XX_CLASSE_CONSOLE_H__

# include "misc/string.h"
# include "threads/mutex.h"



namespace TA3D
{


    /*! \class Console
    **
    ** \brief The user console
    */
    class Console 
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Console();
        //! Destructor
        ~Console();
        //@}


        /*!
        ** \brief Toggle the state of the visibility of the console
        */
        void toggleShow();

        /*!
        ** \brief Get if the console is activated
        */
        bool activated();

        /*!
        ** \brief Add an entry in the console
        */
        void addEntry(const String& newEntry);

        /*!
        ** \brief Draw the console
        **
        ** \param fnt Font
        ** \param dt Delta
        ** \param fsize Size of the font
        ** \param forceShow Display the console even it should be displayed
        */
        char *draw(TA3D::Interfaces::GfxFont& fnt, const float dt, float fsize = 8, const bool forceShow = false);


    private:
        //! Mutex
        Mutex pMutex;
        //!
        String::List pLastEntries;
        //!
        uint16 pMaxItemsToDisplay;
        //!
        real32 pVisible;
        //! 
        bool pShow;
        //! 
        char pInputText[200];
        //! 
        uint32 pCurrentTimer;

    }; //class Console


    extern Console console;

} // namespace TA3D

#endif // _TA3D_XX_CLASSE_CONSOLE_H__
