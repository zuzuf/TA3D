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

#ifndef __3DMEDITOR_ANIMATOR_H__
# define __3DMEDITOR_ANIMATOR_H__

# include "base.h"

namespace Editor
{
namespace Menus
{


    /*! \class Animator
    **
    ** \brief The Animator menu, edit unit animation data
    */
    class Animator : public Abstract
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
        Animator();
        //! Destructor
        virtual ~Animator();
        //@}

    protected:
        virtual bool doInitialize();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();

    private:
        //! \brief Render the model to out background texture (we don't need to update every frame)
        void renderModel();
        //! \brief Get/Create the texture we are going to use as render target
        void getTexture();
        //! \brief Let the user interact with the model
        bool userInteraction();
        //! \brief get a Vector3D in cursor direction
        Vector3D getCursor();

    private:
        //! \brief The texture we want to render to
        GLuint  texture;
        Camera  cam;

        SCRIPT_DATA anim_data;
        int sel_idx;        // ID of selected object
        int cursor_idx;     // ID of the object the cursor is pointing

        float r1,r2,r3,zoom;
        int amx, amy, amz;
    }; // class Animator


} // namespace Menus
} // namespace Editor

#endif // __3DMEDITOR_ANIMATOR_H__
