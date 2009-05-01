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

/*---------------------------------------------------------------------------------\
  |                                        gui.cpp                                   |
  |         Contient les fonctions nécessaires à la gestion de l'interface de ta3D   |
  |  comme les boutons, les fenêtres,...                                             |
  |                                                                                  |
  \---------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "misc/matrix.h"
#include "ta3dbase.h"
#include "misc/paths.h"
#include "languages/i18n.h"
#include "gfx/gui/skin.h"
#include "gfx/gui/skin.object.h"
#include "gfx/gui/area.h"
#include "misc/math.h"




namespace TA3D
{

String curDir = "";

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre de selection de fichier                  |
  \---------------------------------------------------------------------------*/

const String Dialogf(const String &Title, String Filter)
{
    AREA *current_area = AREA::current();
    String result = "";

    if (current_area)
    {
        if (current_area->get_wnd( "open" ) == NULL)            // The window isn't loaded => load it now !
            current_area->load_window( "gui/open_dialog.tdf" );
        current_area->set_title("open",Title);

        if (curDir.empty())                         // If empty grab current directory
            curDir = TA3D::Paths::CurrentDirectory() + TA3D::Paths::Separator;

        String::List files, dirs;

        TA3D::Paths::GlobFiles( files, curDir + Filter );
        TA3D::Paths::GlobDirs( dirs, curDir + "*" );
        TA3D::Paths::ExtractFileName( files );
        TA3D::Paths::ExtractFileName( dirs );

        files.sort();
        dirs.sort();

        current_area->set_entry("open.file_list", files);
        current_area->set_entry("open.folder_list", dirs);
        current_area->msg("open.show");

        current_area->set_state("open.b_ok", false);        // We don't want to leave right now
        current_area->set_state("open.b_cancel", false);

        bool done = false;
        int amx, amy, amz, amb;
        int cur_folder_idx = -1;

        do
        {
            bool key_is_pressed = false;
            do
            {
                amx = mouse_x;
                amy = mouse_y;
                amz = mouse_z;
                amb = mouse_b;

                key_is_pressed = keypressed();
                current_area->check();
                rest( 8 );
            } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !current_area->scrolling );

            if (key[KEY_ESC] || current_area->get_state("open.b_cancel"))   done = true;

            if (key[KEY_ENTER] || current_area->get_state("open.b_ok"))
            {
                done = true;
                result = curDir + TA3D::Paths::Separator + current_area->get_caption("open.t_filename");
            }

            if (current_area->get_state("open.file_list"))
            {
                GUIOBJ *obj = current_area->get_object("open.file_list");
                if (obj && obj->Pos >= 0 && obj->Pos < obj->Text.size())
                {
                    if (current_area->get_caption("open.t_filename") == obj->Text[ obj->Pos ])      // Double-click
                    {
                        done = true;
                        result = curDir + TA3D::Paths::Separator + current_area->get_caption("open.t_filename");
                    }
                    else
                        current_area->set_caption("open.t_filename", obj->Text[ obj->Pos ]);
                }
            }

            if (current_area->get_state("open.folder_list"))
            {
                GUIOBJ *obj = current_area->get_object("open.folder_list");
                if (obj && obj->Pos >= 0 && obj->Pos < obj->Text.size())
                {
                    if (obj->Pos == cur_folder_idx)     // Change current dir (double-click)
                    {
                        if (obj->Text[obj->Pos] == "..")
                            curDir = TA3D::Paths::ExtractFilePath(curDir);
                        else
                            curDir << obj->Text[ obj->Pos ] << TA3D::Paths::Separator;
                        TA3D::Paths::GlobFiles( files, curDir + Filter );
                        TA3D::Paths::GlobDirs( dirs, curDir + "*" );
                        TA3D::Paths::ExtractFileName( files );
                        TA3D::Paths::ExtractFileName( dirs );

                        files.sort();
                        dirs.sort();

                        current_area->set_entry("open.file_list", files);
                        current_area->set_entry("open.folder_list", dirs);
                        cur_folder_idx = -1;
                    }
                    else
                        cur_folder_idx = obj->Pos;
                }
            }

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            current_area->draw();
            draw_cursor();

            gfx->flip();

        }while(!done);

        current_area->msg("open.hide");
    }

    reset_keyboard();

    return result;
}

/*---------------------------------------------------------------------------\
|               Affiche une fenêtre de demande de type oui/non               |
\---------------------------------------------------------------------------*/

bool WndAsk(const String &Title,const String &Msg,int ASW_TYPE)
{
    AREA *current_area = AREA::current();

    if (current_area)
    {
        if (current_area->get_wnd( "yesno" ) == NULL)            // The window isn't loaded => load it now !
            current_area->load_window( "gui/yesno_dialog.tdf" );
        current_area->set_title("yesno",Title);

        current_area->msg("yesno.show");
        current_area->set_caption("yesno.msg", Msg);

        // Boutons OK/Oui et Annuler/Non
        if (ASW_TYPE==ASW_OKCANCEL)
        {
            current_area->set_caption("yesno.b_ok",I18N::Translate("OK"));
            current_area->set_caption("yesno.b_cancel",I18N::Translate("Cancel"));
        }
        else
        {
            current_area->set_caption("yesno.b_ok",I18N::Translate("Yes"));
            current_area->set_caption("yesno.b_cancel",I18N::Translate("No"));
        }

        bool done = false;
        int amx, amy, amz, amb;
        bool answer = false;

        do
        {
            bool key_is_pressed = false;
            do
            {
                amx = mouse_x;
                amy = mouse_y;
                amz = mouse_z;
                amb = mouse_b;

                key_is_pressed = keypressed();
                current_area->check();
                rest( 8 );
            } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !current_area->scrolling );

            if (key[KEY_ESC] || current_area->get_state("yesno.b_cancel"))
            {
                done = true;
                answer = false;
            }

            if (key[KEY_ENTER] || current_area->get_state("yesno.b_ok"))
            {
                done = true;
                answer = true;
            }

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            current_area->draw();
            draw_cursor();

            gfx->flip();

        }while(!done);
        current_area->msg("yesno.hide");

        reset_keyboard();

        return answer;
    }

    return false;
}

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre Popup d'affichage d'infos                |
  \---------------------------------------------------------------------------*/

void Popup(const String &Title,const String &Msg)
{
    AREA *current_area = AREA::current();

    if (current_area)
    {
        if (current_area->get_wnd( "popup" ) == NULL)            // The window isn't loaded => load it now !
            current_area->load_window( "gui/popup_dialog.tdf" );
        current_area->set_title("popup",Title);

        current_area->msg("popup.show");
        current_area->set_caption("popup.msg", Msg);

        bool done = false;
        int amx, amy, amz, amb;

        do
        {
            bool key_is_pressed = false;
            do
            {
                amx = mouse_x;
                amy = mouse_y;
                amz = mouse_z;
                amb = mouse_b;

                key_is_pressed = keypressed();
                current_area->check();
                rest( 8 );
            } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !current_area->scrolling );

            if (key[KEY_ESC])   done = true;

            if (key[KEY_ENTER] || current_area->get_state("popup.b_ok"))
                done = true;

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            current_area->draw();
            draw_cursor();

            gfx->flip();

        }while(!done);
        current_area->msg("popup.hide");

        reset_keyboard();
    }
}

/*---------------------------------------------------------------------------\
|               Affiche une fenêtre de demande d'entrée utilisateur          |
\---------------------------------------------------------------------------*/

const String GetVal(const String &Title)
{
    AREA *current_area = AREA::current();
    String result = "";

    if (current_area)
    {
        if (current_area->get_wnd( "ask" ) == NULL)            // The window isn't loaded => load it now !
            current_area->load_window( "gui/ask_dialog.tdf" );
        current_area->set_title("ask",Title);

        current_area->msg("ask.show");
        current_area->msg("ask.t_result.focus");

        bool done = false;
        int amx, amy, amz, amb;

        do
        {
            bool key_is_pressed = false;
            do
            {
                amx = mouse_x;
                amy = mouse_y;
                amz = mouse_z;
                amb = mouse_b;

                key_is_pressed = keypressed();
                current_area->check();
                rest( 8 );
            } while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed && !current_area->scrolling );

            if (key[KEY_ESC] || current_area->get_state("ask.b_cancel"))   done = true;

            if (key[KEY_ENTER] || current_area->get_state("ask.b_ok"))
            {
                done = true;
                result = current_area->get_caption("ask.t_result");
            }

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            current_area->draw();
            draw_cursor();

            gfx->flip();

        }while(!done);
        current_area->msg("ask.hide");
    }

    reset_keyboard();

    return result;
}

} // namespace TA3D
