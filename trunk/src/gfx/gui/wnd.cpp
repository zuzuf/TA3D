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
#include "wnd.h"
#include "../../languages/i18n.h"
#include "../../ta3dbase.h"
#include "../../misc/math.h"
#include "../../sounds/manager.h"
#include "../../console.h"
#include "../../gfx/glfunc.h"
#include "../../misc/tdf.h"

#define FIX_COLOR(col)  col = makeacol(getb(col), getg(col), getr(col), getr(col))


namespace TA3D
{


    WND::WND()
        :obj_hashtable()
    {
        get_focus = false;
        title_h = 0;
        bkg_w = bkg_h = 1;
        repeat_bkg = false;
        color = makeacol(0x7F, 0x7F, 0x7F, 0xFF);			// Default : grey
        hidden = false;
        was_hidden = false;
        delete_gltex = false;
        width = SCREEN_W >> 1;
        height = SCREEN_H >> 1;
        x = SCREEN_W >> 2;
        y = SCREEN_H >> 2;
        NbObj = 0;
        Objets = NULL;
        Lock = false;
        show_title = true;
        draw_borders = true;
        background = 0;
        background_wnd = false;
        size_factor = 1.0f;
        tab_was_pressed = false;
    }

    WND::WND(const String& filename)
        :obj_hashtable()
    {
        get_focus = false;
        bkg_w = bkg_h = 1;
        repeat_bkg = false;
        color = makeacol(0x7F, 0x7F, 0x7F, 0xFF);			// Default : grey
        hidden = false;
        delete_gltex = false;
        width = SCREEN_W >> 1;
        height = SCREEN_H >> 1;
        x = SCREEN_W>>2;
        y = SCREEN_H>>2;
        NbObj = 0;
        Objets = NULL;
        Lock = false;
        show_title = true;
        draw_borders = true;
        background = 0;
        size_factor = 1.0f;
        tab_was_pressed = false;
        load_tdf(filename);
    }



    WND::~WND()
    {
        obj_hashtable.emptyHashTable();
        destroy();
    }



    void WND::draw(String& helpMsg, const bool focus, const bool deg, SKIN* skin)
    {
        MutexLocker locker(pMutex);
        if (hidden) // If it's hidden don't draw it
            return;

        // Shadow
        doDrawWindowShadow(skin);
        // Background
        doDrawWindowBackground(skin);
        // Skin
        doDrawWindowSkin(skin, focus, deg);

        if (NbObj > 0 && Objets != NULL)
        {
            // Background objects
            for (int i = 0; i < NbObj; ++i)
            {
                if (!(Objets[i].Flag & FLAG_HIDDEN)) // Affiche les objets d'arrière plan
                    doDrawWindowBackgroundObject(helpMsg, i, focus, skin);
            }
            for (int i = 0; i < NbObj; ++i) // Affiche les objets de premier plan
            {
                if (!(Objets[i].Flag & FLAG_HIDDEN))
                    doDrawWindowForegroundObject(skin, i);
            }
        }
    }

    void WND::doDrawWindowShadow(SKIN* skin)
    {
        if (skin == NULL || !draw_borders || Lock)   return;

        skin->ObjectShadow( x - skin->wnd_border.x1, y - skin->wnd_border.y1,
                            x + width - skin->wnd_border.x2, y + height - skin->wnd_border.y2,
                            3, 3,
                            0.5f, 10.0f);
    }


    void WND::doDrawWindowBackground(SKIN* skin)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (background == 0)
        {
            if (skin && skin->wnd_background)
            {
                gfx->set_color(color);
                gfx->drawtexture(skin->wnd_background, x, y, x + width, y + height);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, 0);
                gfx->rectfill(x, y, x + width, y + height, color);
            }
        }
        else
        {
            gfx->set_color(color);
            if (repeat_bkg)
                gfx->drawtexture(background, x, y, x + width, y + height, 0.0f, 0.0f,
                                 ((float)width) / bkg_w, ((float)height) / bkg_h);
            else
                gfx->drawtexture(background, x, y, x + width, y + height);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glDisable(GL_BLEND);
    }


    void WND::doDrawWindowSkin(SKIN* skin, const bool focus, const bool deg)
    {
        if (skin)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Alpha blending activated
            gfx->set_color(color);
            if (draw_borders && skin->wnd_border.tex)
            {
                skin->wnd_border.draw(x - skin->wnd_border.x1,
                                      y - skin->wnd_border.y1,
                                      x + width - skin->wnd_border.x2,
                                      y + height - skin->wnd_border.y2,  false);
            }
            if (show_title && skin->wnd_title_bar.tex)
            {
                title_h = (int)(Math::Max(2 + gui_font->height(), (float)skin->wnd_title_bar.y1) - skin->wnd_title_bar.y2);
                skin->wnd_title_bar.draw(x+3, y+3, x+width-4, y + 3 + title_h);
                gfx->print(gui_font, x + 5 + skin->wnd_title_bar.x1,
                           y + 3 + (title_h - gui_font->height()) * 0.5f,
                           0, White, Title);
            }
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            if (draw_borders)
            {
                gfx->rect(x - 2, y - 2, x + width + 1, y + height + 1, Black);
                gfx->rect(x - 1, y - 1, x + width,     y + height,     DGray);
                gfx->line(x - 2, y - 2, x + width + 1, y - 2,          White);
                gfx->line(x - 2, y - 2, x - 2,         y + height + 1, White);
                gfx->line(x - 1, y - 1, x + width,     y - 1,          LGray);
                gfx->line(x - 1, y - 1, x - 1,         y + height,     LGray);
            }
            if (show_title)
            {
                title_h = (int)(2 + gui_font->height());
                if (deg)
                {
                    if (focus)
                    {
                        glBegin(GL_QUADS);
                        glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(x + 3,         y + 3);
                        glColor3f(0.5f, 0.5f, 0.75f);  glVertex2f(x + width - 4, y + 3);
                        glColor3f(0.5f, 0.5f, 0.75f);  glVertex2f(x + width - 4, y + 5 + gui_font->height());
                        glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(x + 3,         y + 5 + gui_font->height());
                        glEnd();
                    }
                    else
                    {
                        glBegin(GL_QUADS);
                        glColor3f(0.75f, 0.75f, 0.75f); glVertex2f(x + 3,          y + 3);
                        glColor3f(0.5f,  0.5f,  0.5f);  glVertex2f(x + width - 4 , y + 3);
                        glColor3f(0.5f,  0.5f,  0.5f);  glVertex2f(x + width - 4 , y + 5 + gui_font->height());
                        glColor3f(0.75f, 0.75f, 0.75f); glVertex2f(x + 3,          y + 5 + gui_font->height());
                        glEnd();
                    }
                }
                else
                {
                    if (focus)
                        gfx->rectfill(x + 3 , y + 3 , x + width - 4 , y + 5 + gui_font->height(), Blue);
                    else
                        gfx->rectfill(x + 3 , y + 3 , x + width - 4 , y + 5 + gui_font->height(), DGray);
                }
                gfx->print(gui_font, x + 4 , y + 4 , 0 , White, Title);
            }
        }
    }


    void WND::doDrawWindowBackgroundObject(String& helpMsg, const int i, const bool focus, SKIN* skin)
    {
        if (Objets[i].MouseOn && !Objets[i].help_msg.empty())
            helpMsg = Objets[i].help_msg;
        switch (Objets[i].Type)
        {
        case OBJ_TA_BUTTON:
            {
                unsigned int cur_img = (Objets[i].Flag & FLAG_DISABLED)
                                       ? Objets[i].gltex_states.size() - 1
                                   : ((Objets[i].activated && Objets[i].nb_stages == 1)
                                      ? Objets[i].gltex_states.size() - 2
                                  : Objets[i].current_state);
                if (cur_img < Objets[i].gltex_states.size() && cur_img >= 0)
                {
                    gfx->set_color(0xFFFFFFFF);
                    gfx->set_alpha_blending();
                    Objets[i].gltex_states[cur_img].draw(x + Objets[i].x1, y + Objets[i].y1);
                    gfx->unset_alpha_blending();
                }
                break;
            }
        case OBJ_LIST:
            skin->ListBox(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2,
                          Objets[i].Text, Objets[i].Pos, Objets[i].Data, Objets[i].Flag);
            break;
        case OBJ_LINE:
            gfx->disable_texturing();
            gfx->line(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Data);
            gfx->enable_texturing();
            break;
        case OBJ_BOX:
            gfx->set_alpha_blending();
            gfx->disable_texturing();
            if (Objets[i].Flag & FLAG_FILL)
                gfx->rectfill(x + Objets[i].x1, y + Objets[i].y1,
                              x + Objets[i].x2, y + Objets[i].y2, Objets[i].Data);
            else
                gfx->rect(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Data);
            gfx->enable_texturing();
            gfx->unset_alpha_blending();
            break;
        case OBJ_IMG:
            if (Objets[i].Data)     // Draws the texture associated with the image
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, (GLuint)Objets[i].Data);
                gfx->set_color(0xFFFFFFFF);
                glBegin(GL_QUADS);
                glTexCoord2f(Objets[i].u1,Objets[i].v1);  glVertex2f(x+Objets[i].x1,y+Objets[i].y1);
                glTexCoord2f(Objets[i].u2,Objets[i].v1);  glVertex2f(x+Objets[i].x2,y+Objets[i].y1);
                glTexCoord2f(Objets[i].u2,Objets[i].v2);  glVertex2f(x+Objets[i].x2,y+Objets[i].y2);
                glTexCoord2f(Objets[i].u1,Objets[i].v2);  glVertex2f(x+Objets[i].x1,y+Objets[i].y2);
                glEnd();
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else                    // No texture present, draw a black frame
            {
                gfx->rect( x+Objets[i].x1,y+Objets[i].y1, x+Objets[i].x2,y+Objets[i].y2, makeacol(0x7F, 0x7F, 0x7F, 0xFF) );
                gfx->line( x+Objets[i].x1,y+Objets[i].y1, x+Objets[i].x2,y+Objets[i].y2, makeacol(0x7F, 0x7F, 0x7F, 0xFF) );
                gfx->line( x+Objets[i].x2,y+Objets[i].y1, x+Objets[i].x1,y+Objets[i].y2, makeacol(0x7F, 0x7F, 0x7F, 0xFF) );
            }
            break;
        case OBJ_BUTTON:		// Button
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            skin->button(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Text[0], Objets[i].activated);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,DGray);
            break;
        case OBJ_OPTIONC:		// Checkbox
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            skin->OptionCase(x + Objets[i].x1, y + Objets[i].y1, Objets[i].Text[0], Objets[i].Etat);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1 + x - 2, Objets[i].y1 + y - 2, Objets[i].x2 + x + 2, Objets[i].y2 + y + 2, DGray);
            break;
        case OBJ_OPTIONB:		// Boutton d'option
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            skin->OptionButton(x + Objets[i].x1, y + Objets[i].y1, Objets[i].Text[0], Objets[i].Etat);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1 + x - 2, Objets[i].y1 + y - 2, Objets[i].x2 + x + 2, Objets[i].y2 + y + 2, DGray);
            break;
        case OBJ_PBAR:			// Progress Bar
            skin->ProgressBar(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Data);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1 + x - 2, Objets[i].y1 + y - 2, Objets[i].x2 + x + 2, Objets[i].y2 + y + 2, DGray);
            break;
        case OBJ_TEXTBAR:		// Text edit
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            skin->TextBar(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Text[0], Objets[i].Focus);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1 + x - 2, Objets[i].y1 + y - 2, Objets[i].x2 + x + 2, Objets[i].y2 + y + 2, DGray);
            break;
        case OBJ_TEXTEDITOR:	// Large text edit
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            skin->TextEditor(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2, Objets[i].Text, Objets[i].Data, Objets[i].Pos, Objets[i].Focus);
            if (Objets[i].Focus && focus)
                gfx->rectdot(Objets[i].x1 + x - 2, Objets[i].y1 + y - 2, Objets[i].x2 + x + 2, Objets[i].y2 + y + 2, DGray);
            break;
        case OBJ_TEXT:
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            if (!(Objets[i].Flag & FLAG_TEXT_ADJUST))
                skin->text_color.print(gui_font, x + Objets[i].x1, Objets[i].y1 + y, Objets[i].Data, Objets[i].Text[0]);
            else
            {
                Objets[i].Data = skin->draw_text_adjust(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2,
                                                        Objets[i].Text[0], Objets[i].Pos, Objets[i].Flag & FLAG_MISSION_MODE);
                if (Objets[i].Data > 0)
                    Objets[i].Pos %= Objets[i].Data;
            }
            break;
        case OBJ_MENU:			// Menu
            if (Objets[i].Text.empty())
                Objets[i].Text.push_back("");
            if (!Objets[i].Etat)
                skin->button(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2,
                             Objets[i].Text[0], Objets[i].activated || Objets[i].Etat);
            break;
        }

        // Make it darker when disabled
        if (Objets[i].Type != OBJ_TA_BUTTON && (Objets[i].Flag & FLAG_DISABLED))
        {
            glEnable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            gfx->rectfill(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }

        // Highlight the object
        if ((Objets[i].Flag & FLAG_HIGHLIGHT) && Objets[i].MouseOn)
        {
            glEnable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
            gfx->rectfill(x + Objets[i].x1, y + Objets[i].y1, x + Objets[i].x2, y + Objets[i].y2);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    }



    void WND::doDrawWindowForegroundObject(SKIN* skin, const int i)
    {
        switch (Objets[i].Type)
        {
        case OBJ_FMENU:			// Menu flottant
            skin->FloatMenu(x + Objets[i].x1, y + Objets[i].y1, Objets[i].Text,
                            Objets[i].Data, 0);
            break;
        case OBJ_MENU: // Menu déroulant
            if (Objets[i].Etat)
            {
                skin->button(x + Objets[i].x1, y + Objets[i].y1,
                             x + Objets[i].x2, y + Objets[i].y2,
                             Objets[i].Text[0],
                             Objets[i].activated || Objets[i].Etat);
                skin->FloatMenu(x + Objets[i].x1, y + Objets[i].y2 + 1,
                                Objets[i].Text, Objets[i].Data + 1,
                                1 + Objets[i].Pos);
            }
            break;
        }
    }


    byte WND::WinMov(const int AMx, const int AMy, const int AMb, const int Mx, const int My, const int Mb, SKIN* skin)
    {
        MutexLocker locker(pMutex);
        if (AMb == 1 && Mb == 1 && !Lock)
        {
            if (AMx >= x + 3 && AMx <= x + width - 4)
            {
                if (AMy >= y + 3 && AMy <= y + 3 + title_h)
                {
                    x += Mx - AMx;
                    y += My - AMy;
                }
            }
        }
        if (skin)
        {
            if (Mx >= x - skin->wnd_border.x1 && Mx <= x + width - skin->wnd_border.x2
                && My >= y - skin->wnd_border.y1 && My <= y + height - skin->wnd_border.y2)
                return 1;
        }
        else
        {
            if (Mx >= x && Mx <= x + width && My >= y && My <= y + height)
                return 1;
        }
        return 0;
    }



    void WND::destroy()
    {
        pMutex.lock();
        Title.clear();
        Name.clear();
        if (delete_gltex)
        {
            gfx->destroy_texture(background);
            delete_gltex = false;
        }
        background = 0;
        if (NbObj > 0 && Objets != NULL)
        {
            delete[] Objets;
            NbObj = 0;
            Objets = NULL;
        }
        pMutex.unlock();
    }


    void WND::doCheckWasOnFLoattingMenu(const int i, bool& wasOnFloattingMenu, int& indxMenu, SKIN* skin)
    {
        if (Objets[i].Type == OBJ_TA_BUTTON && Objets[i].current_state < Objets[i].gltex_states.size())
        {
            Objets[i].x2 = Objets[i].x1 + Objets[i].gltex_states[ Objets[i].current_state ].width  - 1;
            Objets[i].y2 = Objets[i].y1 + Objets[i].gltex_states[ Objets[i].current_state ].height - 1;
        }

        // Vérifie si la souris est sur l'objet
        if (mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x2
            && mouse_y >= y + Objets[i].y1 && mouse_y <= y + Objets[i].y2)
            return;

        if (Objets[i].Type == OBJ_MENU && Objets[i].Etat && Objets[i].MouseOn && !wasOnFloattingMenu)
        {
            float m_width = 168.0f;
            if (skin)
            {
                for (unsigned int e = 0 ; e < Objets[i].Text.size() - (1 + Objets[i].Pos) ; ++e)
                    m_width = Math::Max(m_width, gui_font->length(Objets[i].Text[ e ]));

                m_width += skin->menu_background.x1 - skin->menu_background.x2;
            }
            else
                m_width = 168.0f;

            if (mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
                && mouse_y > y + Objets[i].y2
                && mouse_y <= y + Objets[i].y2 + 1 + gui_font->height() * Objets[i].Text.size())
            {
                wasOnFloattingMenu = true;
                indxMenu = i;
            }
        }
    }

    int WND::check(int AMx,int AMy,int AMz,int AMb,bool timetoscroll, SKIN *skin)
    {
        MutexLocker locker(pMutex);
        if (hidden)
        {
            was_hidden = true;
            return 0;		// if it's hidden you cannot interact with it
        }
        if (was_hidden)
        {
            for (int i = 0; i < NbObj; ++i)
            {
                if (Objets[i].Type == OBJ_MENU || Objets[i].Type == OBJ_FMENU)
                    Objets[i].Etat = false;
            }
        }
        was_hidden = false;
        int IsOnGUI;
        // Vérifie si la souris est sur la fenêtre et/ou si elle la déplace
        IsOnGUI = WinMov(AMx, AMy, AMb, mouse_x, mouse_y, mouse_b, skin);
        // S'il n'y a pas d'objets, on arrête
        if (NbObj <= 0 || Objets == NULL)
            return IsOnGUI;

        // Interactions utilisateur/objets
        int index,e;
        uint16 Key;
        bool was_on_floating_menu = false;
        int  on_menu = -1;
        bool close_all = false;
        bool already_clicked = false;
        int hasFocus = -1;

        for (int i = 0; i < NbObj; ++i)
        {
            if (Objets[i].Type != OBJ_NONE)
                doCheckWasOnFLoattingMenu(i, was_on_floating_menu, on_menu, skin);
            if (Objets[i].Focus && Objets[i].Type != OBJ_TEXTEDITOR)
                hasFocus = i;
        }
        if (hasFocus >= 0 && key[KEY_TAB] && !tab_was_pressed)      // Select another widget with TAB key
        {
            for (e = 1; e < NbObj; ++e)
            {
                int i = (e + hasFocus) % NbObj;
                if (Objets[i].Flag & FLAG_CAN_GET_FOCUS)
                {
                    Objets[hasFocus].Focus = false;
                    Objets[i].Focus = true;
                    break;
                }
            }
        }
        tab_was_pressed = key[KEY_TAB];

        for (int i = 0; i < NbObj; ++i)
        {
            if (Objets[i].Type == OBJ_NONE)
                continue;

            bool MouseWasOn = Objets[i].MouseOn;
            Objets[i].MouseOn = false;
            if (Objets[i].wait_a_turn)
            {
                Objets[i].wait_a_turn = false;
                continue;
            }
            // Object is hidden so don't handle its events
            if ((Objets[i].Flag & FLAG_HIDDEN) == FLAG_HIDDEN)
                continue;

            if (on_menu == i)
                was_on_floating_menu = false;

            // Vérifie si la souris est sur l'objet
            if (mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x2
                && mouse_y >= y + Objets[i].y1 && mouse_y <= y + Objets[i].y2 && !was_on_floating_menu)
            {
                Objets[i].MouseOn = true;
            }

            if (Objets[i].Type == OBJ_MENU && Objets[i].Etat && !Objets[i].MouseOn && !was_on_floating_menu)
            {
                //int e;
                float m_width = 168.0f;
                if (skin)
                {
                    for (unsigned int e = 0; e < Objets[i].Text.size() - (1 + Objets[i].Pos); ++e)
                        m_width = Math::Max(m_width, gui_font->length(Objets[i].Text[ e ]));

                    m_width += skin->menu_background.x1 - skin->menu_background.x2;
                }
                else
                    m_width = 168.0f;

                if (mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
                    && mouse_y > y + Objets[i].y2 && mouse_y <= y + Objets[i].y2 + 1 + gui_font->height() * Objets[i].Text.size())
                    Objets[i].MouseOn = true;
            }

            if (Objets[i].MouseOn)
                IsOnGUI |= 2;

            if (mouse_b!=0 && Objets[i].MouseOn && !was_on_floating_menu) // Obtient le focus
            {
                for (e = 0; e < NbObj; ++e)
                    Objets[e].Focus = false;
                Objets[i].Focus = true;
            }

            if (mouse_b != 0 && !Objets[i].MouseOn) // Hav lost the focus
            {
                Objets[i].Focus = false;
                switch (Objets[i].Type)
                {
                case OBJ_MENU: Objets[i].Etat = false; break;
                }
            }

            if (Objets[i].MouseOn && (Objets[i].Type==OBJ_FMENU || Objets[i].Type == OBJ_MENU))
            {
                for (e = 0; e < NbObj; ++e)
                {
                    Objets[e].Focus = false;
                    if (Objets[e].Type == OBJ_BUTTON)
                        Objets[e].Etat = false;
                }
                was_on_floating_menu = Objets[i].Etat;
                Objets[i].Focus = true;
            }

            if (!(Objets[i].Flag & FLAG_CAN_GET_FOCUS))
                Objets[i].Focus = false;

            bool previous_state = Objets[i].Etat;

            switch (Objets[i].Type)
            {
            case OBJ_MENU:			// Choses à faire quoi qu'il arrive
                Objets[i].Data = -1;		// Pas de séléction
                if (!Objets[i].Etat)
                    Objets[i].Value = -1;
                {
                    float m_width = 168.0f;
                    if (skin)
                    {
                        for (unsigned int e = 0; e < Objets[i].Text.size() - (1 + Objets[i].Pos); ++e)
                            m_width = Math::Max(m_width, gui_font->length(Objets[i].Text[e]));

                        m_width += skin->menu_background.x1 - skin->menu_background.x2;
                    }
                    else
                        m_width = 168.0f;

                    if (Objets[i].MouseOn && mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
                        && mouse_y > y + Objets[i].y2 + 4 && mouse_y <= y + Objets[i].y2 + 1 + gui_font->height() * Objets[i].Text.size()
                        && Objets[i].Etat)
                    {
                        if (timetoscroll)
                        {
                            if (mouse_y<y+Objets[i].y2+12 && Objets[i].Pos>0)
                                Objets[i].Pos--;
                            if (mouse_y>SCREEN_H-8 && y+Objets[i].y2+1+gui_font->height()*(Objets[i].Text.size()-Objets[i].Pos)>SCREEN_H)
                                Objets[i].Pos++;
                        }
                        Objets[i].Data=(int)((mouse_y-y-Objets[i].y2-5)/(gui_font->height())+Objets[i].Pos);
                        if (Objets[i].Data>=Objets[i].Text.size() - 1)
                            Objets[i].Data = -1;
                    }
                }
                break;
            case OBJ_FMENU:
                Objets[i].Data = -1;		// Pas de séléction
                if (Objets[i].MouseOn && mouse_y>=y+Objets[i].y1+4 && mouse_y<=y+Objets[i].y2-4)
                {
                    Objets[i].Data = (int)((mouse_y-y-Objets[i].y1-4)/(gui_font->height()));
                    if (Objets[i].Data>=Objets[i].Text.size())
                        Objets[i].Data = -1;
                }
                break;
            case OBJ_TEXTBAR:				// Permet l'entrée de texte
                Objets[i].Etat=false;
                if (Objets[i].Focus && keypressed())
                {
                    uint32 keyCode = readkey();
                    Key = keyCode & 0xFFFF;
                    uint16 scancode = keyCode >> 16;

                    switch(scancode)
                    {
                    case KEY_ENTER:
                        Objets[i].Etat=true;
                        if (Objets[i].Func!=NULL)
                            (*Objets[i].Func)(Objets[i].Text[0].sizeUTF8());
                        break;
                    case KEY_BACKSPACE:
                        if (Objets[i].Text[0].sizeUTF8()>0)
                            Objets[i].Text[0] = Objets[i].Text[0].substrUTF8(0, Objets[i].Text[0].sizeUTF8() - 1);
                        break;
                    case KEY_TAB:
                    case KEY_ESC:
                        break;
                    default:
                        switch (Key)
                        {
                                case 9:
                                case 27:
                                case 0:
                            break;
                                default:
                            if (Objets[i].Text[0].sizeUTF8() + 1 < Objets[i].Data)
                                Objets[i].Text[0] << InttoUTF8( Key );
                        };
                    };
                }
                break;

            case OBJ_TEXTEDITOR:				// Permet l'entrée de texte / Enable text input
                if (Objets[i].Text.empty()) Objets[i].Text.push_back("");
                if (Objets[i].Data < 0) Objets[i].Data = 0;
                else if(Objets[i].Data >= Objets[i].Text.size())    Objets[i].Data = Objets[i].Text.size() - 1;

                if (Objets[i].Pos < 0)  Objets[i].Pos = 0;
                else if(Objets[i].Pos > Objets[i].Text[Objets[i].Data].sizeUTF8())
                    Objets[i].Pos = Objets[i].Text[Objets[i].Pos].sizeUTF8();
                Objets[i].Etat=false;
                if (Objets[i].Focus && keypressed())
                {
                    uint32 keyCode = readkey();
                    Key = keyCode & 0xFFFF;
                    uint16 scancode = (keyCode >> 16);
                    switch (scancode)
                    {
                    case KEY_ESC:
                        break;
                    case KEY_TAB:
                        Objets[i].Text[Objets[i].Data] << "    ";
                        Objets[i].Pos += 4;
                        break;
                    case KEY_ENTER:
                        Objets[i].Text.push_back("");
                        if (Objets[i].Data + 1 < Objets[i].Text.size())
                            for(int e = Objets[i].Text.size() - 1 ; e > Objets[i].Data + 1 ; e--)
                                Objets[i].Text[e] = Objets[i].Text[e-1];

                        if (Objets[i].Text[ Objets[i].Data ].sizeUTF8() - Objets[i].Pos > 0)
                            Objets[i].Text[ Objets[i].Data + 1 ] = Objets[i].Text[ Objets[i].Data ].substrUTF8( Objets[i].Pos, Objets[i].Text[ Objets[i].Data ].sizeUTF8() - Objets[i].Pos );
                        else
                            Objets[i].Text[ Objets[i].Data + 1 ].clear();
                        Objets[i].Text[ Objets[i].Data ] = Objets[i].Text[ Objets[i].Data ].substrUTF8( 0, Objets[i].Pos );
                        Objets[i].Pos = 0;
                        Objets[i].Data++;
                        break;
                    case KEY_DEL:                                 // Remove next character
                        if (Objets[i].Pos < Objets[i].Text[Objets[i].Data].sizeUTF8())
                        {
                            Objets[i].Text[Objets[i].Data] = Objets[i].Text[Objets[i].Data].substrUTF8(0,Objets[i].Pos)
                                                             + Objets[i].Text[Objets[i].Data].substrUTF8(Objets[i].Pos+1, Objets[i].Text[Objets[i].Data].sizeUTF8() - Objets[i].Pos-1);
                        }
                        else if (Objets[i].Data + 1 < Objets[i].Text.size())
                        {
                            Objets[i].Text[Objets[i].Data] << Objets[i].Text[Objets[i].Data+1];
                            for( int e = Objets[i].Data + 1 ; e < Objets[i].Text.size() - 1 ; e++ )
                                Objets[i].Text[e] = Objets[i].Text[e+1];
                            Objets[i].Text.resize(Objets[i].Text.size()-1);
                        }
                        break;
                    case KEY_BACKSPACE:                                 // Remove previous character
                        if (Objets[i].Pos > 0)
                        {
                            Objets[i].Text[Objets[i].Data] = Objets[i].Text[Objets[i].Data].substrUTF8(0,Objets[i].Pos-1)
                                                             + Objets[i].Text[Objets[i].Data].substrUTF8(Objets[i].Pos, Objets[i].Text[Objets[i].Data].sizeUTF8() - Objets[i].Pos);
                            Objets[i].Pos--;
                        }
                        else if (Objets[i].Data > 0)
                        {
                            Objets[i].Data--;
                            Objets[i].Pos = Objets[i].Text[Objets[i].Data].sizeUTF8();
                            Objets[i].Text[Objets[i].Data] << Objets[i].Text[Objets[i].Data+1];
                            for( int e = Objets[i].Data + 1 ; e < Objets[i].Text.size() - 1 ; e++ )
                                Objets[i].Text[e] = Objets[i].Text[e+1];
                            Objets[i].Text.resize(Objets[i].Text.size()-1);
                        }
                        break;
                    case KEY_LEFT:            // Left
                        if (Objets[i].Pos > 0)
                            Objets[i].Pos--;
                        else if (Objets[i].Data > 0)
                        {
                            Objets[i].Data--;
                            Objets[i].Pos = Objets[i].Text[Objets[i].Data].sizeUTF8();
                        }
                        break;
                    case KEY_RIGHT:            // Right
                        if (Objets[i].Pos < Objets[i].Text[Objets[i].Data].sizeUTF8())
                            Objets[i].Pos++;
                        else if (Objets[i].Data + 1 < Objets[i].Text.size())
                        {
                            Objets[i].Data++;
                            Objets[i].Pos = 0;
                        }
                        break;
                    case KEY_UP:            // Up
                        if (Objets[i].Data > 0)
                        {
                            Objets[i].Data--;
                            Objets[i].Pos = Math::Min( (uint32)Objets[i].Text[Objets[i].Data].sizeUTF8(), Objets[i].Pos );
                        }
                        break;
                    case KEY_DOWN:            // Down
                        if (Objets[i].Data + 1 < Objets[i].Text.size())
                        {
                            Objets[i].Data++;
                            Objets[i].Pos = Math::Min( (uint32)Objets[i].Text[Objets[i].Data].sizeUTF8(), Objets[i].Pos );
                        }
                        break;
                    default:
                        switch (Key)
                        {
                                case 0:
                                case 27:
                            break;
                                default:
                            Objets[i].Text[Objets[i].Data] = Objets[i].Text[ Objets[i].Data ].substrUTF8( 0, Objets[i].Pos )
                                                             + InttoUTF8( Key )
                                                             + Objets[i].Text[ Objets[i].Data ].substrUTF8( Objets[i].Pos, Objets[i].Text[ Objets[i].Data ].sizeUTF8() - Objets[i].Pos );
                            Objets[i].Pos++;
                        };
                    };
                }
                break;

            case OBJ_LIST:
                if ((Objets[i].MouseOn || Objets[i].Focus) && skin)
                {
                    bool onDeco = (mouse_x - x <= Objets[i].x1 + skin->text_background.x1
                                   || mouse_x - x >= Objets[i].x2 + skin->text_background.x2
                                   || mouse_y - y <= Objets[i].y1 + skin->text_background.y1
                                   || mouse_y - y >= Objets[i].y2 + skin->text_background.y2);			// We're on ListBox decoration!
                    int widgetSize = (int)((Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2) / gui_font->height());
                    int TotalScroll = Objets[i].Text.size() - widgetSize;
                    if (TotalScroll < 0)
                        TotalScroll = 0;

                    if (mouse_b == 1 && !onDeco
                        && mouse_x - x >= Objets[i].x2 + skin->text_background.x2 - skin->scroll[0].sw
                        && mouse_x - x <= Objets[i].x2 + skin->text_background.x2
                        && mouse_y - y >= Objets[i].y1 + skin->text_background.y1
                        && mouse_y - y <= Objets[i].y2 + skin->text_background.y2) // We're on the scroll bar!
                    {

                        if (mouse_y - y > Objets[i].y1 + skin->text_background.y1 + skin->scroll[0].y1
                            && mouse_y - y < Objets[i].y2 + skin->text_background.y2 + skin->scroll[0].y2) // Set scrolling position
                        {
                            Objets[i].Data = (int)(0.5f + TotalScroll
                                                   * (mouse_y - y - Objets[i].y1 - skin->text_background.y1 - skin->scroll[0].y1
                                                      - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2) * 0.5f)
                                                   / (Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2
                                                      - skin->scroll[0].y1 + skin->scroll[0].y2
                                                      - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2) * 0.5f));
                        }
                        if (Objets[i].Data > (unsigned int)TotalScroll)
                            Objets[i].Data = TotalScroll;
                    }
                    else
                    {
                        int nscroll = (int)Objets[i].Data - mouse_z + AMz;
                        int npos = Objets[i].Pos;
                        if (Objets[i].Focus)
                        {
                            int key_code = (readkey() >> 16) & 0xFFFF;
                            if (key_code == KEY_UP)
                                npos--;
                            if (key_code == KEY_DOWN)
                                npos++;
                            if (npos != Objets[i].Pos)
                            {
                                if (npos < 0)   npos = 0;
                                if (npos >= Objets[i].Text.size())
                                    npos = Objets[i].Text.size() - 1;
                                if (nscroll > npos)
                                    nscroll = npos;
                                if (nscroll + widgetSize <= npos)
                                    nscroll = npos - widgetSize + 1;
                            }
                        }

                        if (nscroll < 0)
                            nscroll = 0;
                        else
                            if (nscroll > TotalScroll)
                                nscroll = TotalScroll;

                        Objets[i].Data = nscroll;
                        Objets[i].Pos = npos;
                    }
                }
                break;
            }
            if (Objets[i].Flag & FLAG_DISABLED)
            {
                Objets[i].activated = false;
                Objets[i].Etat = false;
            }
            else
            {
                if ((mouse_b != 1 || !Objets[i].MouseOn || mouse_b == AMb) && (Objets[i].Flag & FLAG_CAN_BE_CLICKED)
                    && !(Objets[i].Flag & FLAG_SWITCH) && !(Objets[i].Etat ^ previous_state)
                    && Objets[i].Etat && !was_on_floating_menu)
                {
                    if (Objets[i].Func!=NULL)
                        (*Objets[i].Func)(0);		// Lance la fonction associée
                    Objets[i].Etat=false;
                }
                if (!Objets[i].activated && mouse_b==1 && Objets[i].MouseOn && ((Objets[i].Flag & FLAG_CAN_BE_CLICKED) || (Objets[i].Flag & FLAG_SWITCH)))
                {
                    switch(Objets[i].Type)
                    {
                    case OBJ_BOX:
                    case OBJ_BUTTON:
                    case OBJ_MENU:
                    case OBJ_TA_BUTTON:
                    case OBJ_OPTIONB:
                    case OBJ_OPTIONC:
                        if (sound_manager)
                            sound_manager->playTDFSoundNow("SPECIALORDERS.sound");
                    }
                }
                Objets[i].activated = mouse_b==1 && Objets[i].MouseOn;

                bool clicked = false;
                if (Objets[i].shortcut_key >= 0 && Objets[i].shortcut_key <= 255 && lp_CONFIG->enable_shortcuts && !TA3D_CTRL_PRESSED && !TA3D_SHIFT_PRESSED && !console.activated()
                    && (key[ ascii_to_scancode[ Objets[i].shortcut_key ] ]
                        || (Objets[i].shortcut_key >= 65 && Objets[i].shortcut_key <= 90 && key[ ascii_to_scancode[ Objets[i].shortcut_key + 32 ] ])
                        || (Objets[i].shortcut_key >= 97 && Objets[i].shortcut_key <= 122 && key[ ascii_to_scancode[ Objets[i].shortcut_key - 32 ] ])))
                    {
                    if (!Objets[i].Etat)
                        clicked = true;
                    Objets[i].activated = Objets[i].Etat = true;
                }

                if (((mouse_b!=1 && AMb==1) || clicked) && Objets[i].MouseOn && MouseWasOn
                    && ((Objets[i].Flag & FLAG_CAN_BE_CLICKED) || (Objets[i].Flag & FLAG_SWITCH)) && !already_clicked) // Click sur l'objet
                {
                    already_clicked = true;
                    switch (Objets[i].Type)
                    {
                    case OBJ_LIST:
                        if (skin
                            && mouse_x - x >= Objets[i].x2 + skin->text_background.x2 - skin->scroll[0].sw
                            && mouse_x - x <= Objets[i].x2 + skin->text_background.x2
                            && mouse_y - y >= Objets[i].y1 + skin->text_background.y1
                            && mouse_y - y <= Objets[i].y2 + skin->text_background.y2) // We're on the scroll bar!
                        {

                            int TotalScroll = Objets[i].Text.size() - (int)((Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2) / gui_font->height());
                            if (TotalScroll < 0)
                                TotalScroll = 0;

                            if (mouse_y - y <= Objets[i].y1 + skin->text_background.y1 + skin->scroll[0].y1)// Scroll up
                            {
                                if (Objets[i].Data > 0)
                                    Objets[i].Data--;
                                if (sound_manager)
                                    sound_manager->playTDFSoundNow("SPECIALORDERS.sound");
                            }
                            else
                            {
                                if (mouse_y - y >= Objets[i].y2 + skin->text_background.y2 + skin->scroll[0].y2) // Scroll down
                                {
                                    Objets[i].Data++;
                                    if (sound_manager)
                                        sound_manager->playTDFSoundNow("SPECIALORDERS.sound");
                                }
                                else
                                {							// Set scrolling position
                                    Objets[i].Data = (int)(0.5f + TotalScroll * (mouse_y - y - Objets[i].y1 - skin->text_background.y1 - skin->scroll[0].y1
                                                                                 - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2) * 0.5f)
                                                           / (Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2
                                                              - skin->scroll[0].y1 + skin->scroll[0].y2 - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2) * 0.5f));
                                }
                            }
                            if (Objets[i].Data > (unsigned int)TotalScroll)
                                Objets[i].Data = TotalScroll;
                        }
                        else
                        {
                            if (skin && (
                                    mouse_x - x <= Objets[i].x1 + skin->text_background.x1
                                    || mouse_x - x >= Objets[i].x2 + skin->text_background.x2
                                    || mouse_y - y <= Objets[i].y1 + skin->text_background.y1
                                    || mouse_y - y >= Objets[i].y2 + skin->text_background.y2))			// We're on ListBox decoration!
                                break;
                            Objets[i].Pos = (uint32) ((mouse_y - y - Objets[i].y1 - (skin ? skin->text_background.y1:4)) / gui_font->height() + Objets[i].Data);
                            Objets[i].Etat = true;
                        }
                        break;
                    case OBJ_TA_BUTTON:
                        if (Objets[i].nb_stages > 0)
                            Objets[i].current_state = (++Objets[i].current_state) % Objets[i].nb_stages;
                        Objets[i].Etat = true;
                        break;
                    case OBJ_BOX:			// Rectangle
                    case OBJ_IMG:			// Image
                    case OBJ_BUTTON:		// Boutton
                        if (was_on_floating_menu)	break;
                        Objets[i].Etat=true;
                        break;
                    case OBJ_OPTIONC:		// Case à cocher
                        if (was_on_floating_menu)	break;
                        if (skin && skin->checkbox[0].tex && skin->checkbox[1].tex)
                        {
                            if (mouse_x<=x+Objets[i].x1+skin->checkbox[Objets[i].Etat?1:0].sw && mouse_y<=y+Objets[i].y1+skin->checkbox[Objets[i].Etat?1:0].sh)
                                Objets[i].Etat^=true;
                        }
                        else
                            if (mouse_x<=x+Objets[i].x1+12 && mouse_y<=y+Objets[i].y1+12)
                                Objets[i].Etat^=true;
                        if (Objets[i].Func!=NULL)
                            (*Objets[i].Func)(Objets[i].Etat);	// Lance la fonction associée
                        break;
                    case OBJ_OPTIONB:		// Bouton d'option
                        if (was_on_floating_menu)	break;
                        if (skin && skin->option[0].tex && skin->option[1].tex)
                        {
                            if (mouse_x<=x+Objets[i].x1+skin->option[Objets[i].Etat?1:0].sw && mouse_y <= y + Objets[i].y1+skin->option[Objets[i].Etat?1:0].sh)
                                Objets[i].Etat^=true;
                        }
                        else
                            if (mouse_x<=x+Objets[i].x1+12 && mouse_y<=y+Objets[i].y1+12)
                                Objets[i].Etat^=true;
                        if (Objets[i].Func!=NULL)
                            (*Objets[i].Func)(Objets[i].Etat);	// Lance la fonction associée
                        break;
                    case OBJ_FMENU:			// Menu Flottant
                        if (mouse_y >= y + Objets[i].y1 + (skin ? skin->menu_background.y1 : 0) + 4 && mouse_y <= y + Objets[i].y2 + (skin ? skin->menu_background.y2 : 0) - 4)
                        {
                            index = (int)((mouse_y - y - Objets[i].y1 - 4 - (skin ? skin->menu_background.y1 : 0)) / gui_font->height());
                            if (index >= (int)(Objets[i].Text.size()))
                                index = Objets[i].Text.size() - 1;
                            if (Objets[i].Func!=NULL)
                                (*Objets[i].Func)(index);		// Lance la fonction associée
                        }
                        break;
                    case OBJ_MENU:			// Menu déroulant
                        {
                            float m_width = 168.0f;
                            if (skin)
                            {
                                for (unsigned int e = 0 ; e < Objets[i].Text.size() - (1 + Objets[i].Pos) ; e++)
                                    m_width = Math::Max(m_width, gui_font->length(Objets[i].Text[ e ]));

                                m_width += skin->menu_background.x1 - skin->menu_background.x2;
                            }
                            else
                                m_width = 168.0f;
                            if (mouse_x >= x + Objets[i].x1 + (skin ? skin->menu_background.x1 : 0) && mouse_x <= x + Objets[i].x1 + m_width + (skin ? skin->menu_background.x2 : 0)
                                && mouse_y > y + Objets[i].y2 + (skin ? skin->menu_background.y1 : 0) && mouse_y <= y + Objets[i].y2 + (skin ? skin->menu_background.y2 : 0) + 1 + gui_font->height() * Objets[i].Text.size()
                                && Objets[i].Etat)
                                {
                                index = (int)((mouse_y - y - Objets[i].y2 - 5 - (skin ? skin->menu_background.y1 : 0)) / gui_font->height() + Objets[i].Pos);
                                if (index >= (int)(Objets[i].Text.size() - 1))
                                    index = Objets[i].Text.size()-2;
                                if (Objets[i].Func != NULL)
                                    (*Objets[i].Func)(index);		// Lance la fonction associée
                                Objets[i].Value = Objets[i].Data;
                                Objets[i].Etat = false;
                                close_all = true;
                            }
                            else
                                Objets[i].Etat ^= true;
                        }
                        break;
                    default:
                        Objets[i].Etat = true;
                    }
                    // Send a signal to the interface (the OnClick signal defined at initialization time)
                    for (uint16 cur = 0 ; cur < Objets[i].OnClick.size() ; cur++)
                        I_Msg(TA3D::TA3D_IM_GUI_MSG, (void *)Objets[i].OnClick[cur].c_str(), NULL, NULL);
                }
                else if (Objets[i].MouseOn)			// Send a signal to the interface (the OnHover signal defined at initialization time)
                    for (uint16 cur = 0 ; cur < Objets[i].OnHover.size() ; cur++)
                        I_Msg(TA3D::TA3D_IM_GUI_MSG, (void *)Objets[i].OnHover[cur].c_str(), NULL, NULL);
            }

            for (uint16 cur = 0; cur < Objets[i].SendDataTo.size(); ++cur) // Send Data to an Object
            {
                String::size_type e = Objets[i].SendDataTo[cur].find('.');
                if (e != String::npos)
                {
                    int target = atoi(Objets[i].SendDataTo[cur].substr(0, e).c_str());
                    if (target >= 0 && target < NbObj)
                    {
                        if (Objets[i].SendDataTo[cur].substr(e+1, Objets[i].SendDataTo[cur].length()-e) == "data")
                            Objets[target].Data = Objets[i].Data;
                        else
                            Objets[target].Pos = Objets[i].Data;
                    }
                }
            }
            for (uint16 cur = 0; cur < Objets[i].SendPosTo.size() ; ++cur)	// Send Pos to an Object
            {
                String::size_type e = Objets[i].SendPosTo[cur].find('.');
                if (e != String::npos)
                {
                    int target = atoi(Objets[i].SendPosTo[cur].substr(0, e).c_str());
                    if (target >= 0 && target < NbObj)
                    {
                        if (Objets[i].SendPosTo[cur].substr(e+1, Objets[i].SendPosTo[cur].length()-e) == "data")
                            Objets[target].Data = Objets[i].Pos;
                        else
                            Objets[target].Pos = Objets[i].Pos;
                    }
                }
            }
        }
        if (close_all)
        {
            for (int i = 0 ; i < NbObj; ++i)
                if (Objets[i].Type == OBJ_MENU)
                    Objets[i].Etat = false;
        }
        return IsOnGUI;
    }




    uint32 WND::msg(const String& message)
    {
        MutexLocker locker(pMutex);
        String::size_type i = message.find('.');

        if (i != String::npos) // When it targets a subobject
        {
            GUIOBJ* obj = doGetObject(message.substr(0, i));
            if (obj)
                return obj->msg(message.substr(i + 1, message.size() - i - 1), this);
        }
        else // When it targets the window itself
        {
            if (String::ToLower(message) == "show")
            {
                hidden = false;
                return INTERFACE_RESULT_HANDLED;
            }
            if (String::ToLower(message) == "hide")
            {
                hidden = true;
                return INTERFACE_RESULT_HANDLED;
            }
        }
        return INTERFACE_RESULT_CONTINUE;
    }



    bool WND::get_state(const String& message)
    {
        MutexLocker locker(pMutex);
        GUIOBJ* obj = doGetObject(message);
        if (obj)
            return obj->Etat;
        if (message.empty())
            return !hidden;
        return false;
    }

    sint32 WND::get_value(const String& message)
    {
        MutexLocker locker(pMutex);
        GUIOBJ* obj = doGetObject(message);
        return (obj) ? obj->Value : -1;
    }

    String WND::get_caption(const String& message)
    {
        MutexLocker locker(pMutex);
        GUIOBJ *obj = doGetObject(message);
        if (obj)
        {
            if (!obj->Text.empty())
            {
                if (obj->Type == OBJ_TEXTEDITOR)
                {
                    String result = obj->Text[0];
                    for( int i = 1 ; i < obj->Text.size() ; i++ )
                        result << '\n' << obj->Text[i];
                    return result;
                }
                else
                    return  obj->Text[0];
            }
            return "";
        }
        return (message.empty()) ? Title : "";
    }


    GUIOBJ *WND::doGetObject(String message)
    {
        sint16 e = obj_hashtable.find(message.toLower()) - 1;
        return (e >= 0) ? &(Objets[e]) : NULL;
    }

    GUIOBJ *WND::get_object(String message)
    {
        MutexLocker locker(pMutex);
        sint16 e = obj_hashtable.find(message.toLower()) - 1;
        return (e >= 0) ? &(Objets[e]) : NULL;
    }



    void WND::load_gui(const String& filename, TA3D::UTILS::cHashTable< std::vector< TA3D::Interfaces::GfxTexture >* > &gui_hashtable)
    {
        if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
            gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
        else
            gfx->set_texture_format(GL_RGB8);

        TDFParser wndFile(filename, false, false, true);

        // Grab the window's name, so we can send signals to it (to hide/show for example)
        Name = filename;
        String::size_type e = Name.find('.');		// Extracts the file name
        if (e != String::npos)
            Name = Name.substr(0, e);
        e = Name.find_last_of("/\\");
        if (e != String::npos)
            Name = Name.substr(e + 1, Name.size() - e - 1);

        hidden = !wndFile.pullAsBool("gadget0.common.active");

        Title.clear();
        x = wndFile.pullAsInt("gadget0.common.xpos");
        y = wndFile.pullAsInt("gadget0.common.ypos");
        if (x < 0)
            x += SCREEN_W;
        if (y < 0)
            y += SCREEN_H;

        width  = wndFile.pullAsInt("gadget0.common.width");
        height = wndFile.pullAsInt("gadget0.common.height");

        if (x + width >= SCREEN_W)
            x = SCREEN_W - width;
        if (y + height >= SCREEN_H)
            y = SCREEN_H - height;

        float x_factor = 1.0f;
        float y_factor = 1.0f;

        Lock = true;
        draw_borders = false;
        show_title = false;
        delete_gltex = false;

        String panel = wndFile.pullAsString("gadget0.panel"); // Look for the panel texture
        int w;
        int h;
        background = gfx->load_texture_from_cache(panel, FILTER_LINEAR, (uint32*)&w, (uint32*)&h);
        if (!background)
        {
            background = Gaf::ToTexture("anims\\" + Name + ".gaf", panel, &w, &h, true);
            if (background == 0)
                background = Gaf::ToTexture("anims\\commongui.gaf", panel, &w, &h, true);
            if (background == 0)
            {
                String::List file_list;
                HPIManager->getFilelist("anims\\*.gaf", file_list);
                for (String::List::const_iterator i = file_list.begin(); i != file_list.end() && background == 0 ; ++i)
                {
                    LOG_DEBUG("trying(1) " << *i << " (" << Name << ")");
                    background = Gaf::ToTexture(*i, panel, &w, &h, true, FILTER_LINEAR);
                }
            }
            if (background)
                gfx->save_texture_to_cache(panel, background, w, h);
        }

        delete_gltex = background;
        background_wnd = background;
        color = background ? makeacol(0xFF, 0xFF, 0xFF, 0xFF) : 0x0;
        NbObj = wndFile.pullAsInt("gadget0.totalgadgets");

        Objets = new GUIOBJ[NbObj];

        for (uint16 i = 0; i < NbObj; ++i) // Loads each object
        {
            String obj_key;
            obj_key << "gadget" << i + 1 << ".";
            int obj_type = wndFile.pullAsInt(obj_key + "common.id");

            Objets[i].Name = wndFile.pullAsString(obj_key + "common.name", format("gadget%d", i + 1));
            obj_hashtable.insert(String::ToLower(Objets[i].Name), i + 1);

            int X1 = (int)(wndFile.pullAsInt(obj_key + "common.xpos")   * x_factor); // Reads data from TDF
            int Y1 = (int)(wndFile.pullAsInt(obj_key + "common.ypos")   * y_factor);
            int W  = (int)(wndFile.pullAsInt(obj_key + "common.width")  * x_factor - 1);
            int H  = (int)(wndFile.pullAsInt(obj_key + "common.height") * y_factor - 1);

            //float size = Math::Min(x_factor, y_factor);
            uint32 obj_flags = 0;

            if (X1 < 0)
                X1 += SCREEN_W;
            if (Y1 < 0)
                Y1 += SCREEN_H;

            if (!wndFile.pullAsBool(obj_key + "common.active"))
                obj_flags |= FLAG_HIDDEN;

            String::Vector Caption;
            wndFile.pullAsString(obj_key + "text").split(Caption, ",");
            I18N::Translate(Caption);

            if (TA_ID_BUTTON == obj_type)
            {
                int t_w[100];
                int t_h[100];
                String key(Objets[i].Name);
                key.toLower();
                std::vector<TA3D::Interfaces::GfxTexture>* result = gui_hashtable.find(key);

                std::vector<GLuint> gaf_imgs;
                bool found_elsewhere = false;

                if (!result)
                {
                    Gaf::ToTexturesList(gaf_imgs, "anims\\" + Name + ".gaf", Objets[i].Name, t_w, t_h, true, FILTER_LINEAR);
                    if (!gaf_imgs.size())
                    {
                        Gaf::ToTexturesList(gaf_imgs, "anims\\commongui.gaf", Objets[i].Name, t_w, t_h, true, FILTER_LINEAR);
                        found_elsewhere = true;
                    }
                    if (!gaf_imgs.size())
                    {
                        String::List file_list;
                        HPIManager->getFilelist("anims\\*.gaf", file_list);
                        for (String::List::const_iterator e = file_list.begin() ; e != file_list.end() && gaf_imgs.size() == 0 ; ++e)
                        {
                            LOG_DEBUG("trying(0) " << *e << " (" << Name << ")");
                            Gaf::ToTexturesList(gaf_imgs, *e, Objets[i].Name, t_w, t_h, true, FILTER_LINEAR);
                        }
                        if (gaf_imgs.size() > 0)
                            found_elsewhere = true;
                    }
                }
                else
                {
                    gaf_imgs.resize(result->size());
                    for (unsigned int e = 0 ; e < result->size() ; ++e)
                    {
                        gaf_imgs[ e ] = (*result)[ e ].tex;
                        t_w[ e ] = (*result)[ e ].width;
                        t_h[ e ] = (*result)[ e ].height;
                    }
                }

                int nb_stages = wndFile.pullAsInt(obj_key + "stages");
                Objets[i].create_ta_button(X1, Y1, Caption, gaf_imgs, nb_stages > 0 ? nb_stages : gaf_imgs.size() - 2);
                if (result == NULL && found_elsewhere)
                    gui_hashtable.insert(key, &Objets[i].gltex_states);
                for (unsigned int e = 0; e < Objets[i].gltex_states.size(); ++e)
                {
                    Objets[i].gltex_states[e].width = t_w[e];
                    Objets[i].gltex_states[e].height = t_h[e];
                    if (result)
                        Objets[i].gltex_states[e].destroy_tex = false;
                    else
                        Objets[i].gltex_states[e].destroy_tex = true;
                }
                Objets[i].current_state = wndFile.pullAsInt(obj_key + "status");
                Objets[i].shortcut_key = wndFile.pullAsInt(obj_key + "quickkey", -1);
                if (wndFile.pullAsBool(obj_key + "common.grayedout"))
                    Objets[i].Flag |= FLAG_DISABLED;
                //			if (wndFile.pullAsInt(obj_key + "common.commonattribs") == 4) {
                if (wndFile.pullAsInt(obj_key + "common.attribs") == 32)
                    Objets[i].Flag |= FLAG_HIDDEN | FLAG_BUILD_PIC;
            }
            else
            {
                if (obj_type == TA_ID_TEXT_FIELD)
                    Objets[i].create_textbar(X1, Y1, X1 + W, Y1 + H, Caption.size() > 0 ? Caption[0] : "", wndFile.pullAsInt(obj_key + "maxchars"), NULL);
                else
                {
                    if (obj_type == TA_ID_LABEL)
                        Objets[i].create_text(X1, Y1, Caption.size() ? Caption[0] : "", 0xFFFFFFFF, 1.0f);
                    else
                    {
                        if (obj_type == TA_ID_BLANK_IMG || obj_type == TA_ID_IMG)
                        {
                            Objets[i].create_img(X1, Y1, X1 + W, Y1 + H, gfx->load_texture(wndFile.pullAsString(obj_key + "source"),FILTER_LINEAR));
                            Objets[i].destroy_img = Objets[i].Data != 0 ? true : false;
                        }
                        else
                        {
                            if (obj_type == TA_ID_LIST_BOX)
                                Objets[i].create_list(X1, Y1, X1+W, Y1+H, Caption);
                            else
                                Objets[i].Type = OBJ_NONE;
                        }
                    }
                }
            }

            Objets[i].OnClick.clear();
            Objets[i].OnHover.clear();
            Objets[i].SendDataTo.clear();
            Objets[i].SendPosTo.clear();

            Objets[i].Flag |= obj_flags;
        }
    }



    void WND::load_tdf(const String& filename, SKIN *skin)
    {
        TDFParser wndFile(filename);

        Name = filename; // Grab the window's name, so we can send signals to it (to hide/show for example)
        String::size_type e = Name.find('.'); // Extracts the file name
        if (e != String::npos)
            Name = Name.substr(0, e);
        e = Name.find_last_of("/\\");
        if (e != String::npos)
            Name = Name.substr(e + 1, Name.size() - e - 1);

        Name = wndFile.pullAsString("window.name", Name);
        hidden = wndFile.pullAsBool("window.hidden");

        Title = I18N::Translate(wndFile.pullAsString("window.title"));
        x = wndFile.pullAsInt("window.x");
        y = wndFile.pullAsInt("window.y");
        width = wndFile.pullAsInt("window.width");
        height = wndFile.pullAsInt("window.height");
        repeat_bkg = wndFile.pullAsBool("window.repeat background", false);
        get_focus = wndFile.pullAsBool("window.get focus", false);

        float x_factor = 1.0f;
        float y_factor = 1.0f;
        if (wndFile.pullAsBool("window.fullscreen"))
        {
            int ref_width = wndFile.pullAsInt("window.screen width", width);
            int ref_height = wndFile.pullAsInt("window.screen height", height);
            if (ref_width > 0.0f)
                x_factor = ((float)gfx->width) / ref_width;
            if (ref_height > 0.0f)
                y_factor = ((float)gfx->height) / ref_height;
            width  = (int)(width  * x_factor);
            height = (int)(height * y_factor);
            x = (int)(x * x_factor);
            y = (int)(y * y_factor);
        }

        if (x < 0)
            x += SCREEN_W;
        if (y < 0)
            y += SCREEN_H;
        if (width < 0)
            width += SCREEN_W;
        if (height < 0)
            height += SCREEN_H;
        size_factor = gfx->height / 600.0f;			// For title bar

        background_wnd = wndFile.pullAsBool("window.background window");
        Lock = wndFile.pullAsBool("window.lock");
        draw_borders = wndFile.pullAsBool("window.draw borders");
        show_title = wndFile.pullAsBool("window.show title");
        delete_gltex = false;
        if (HPIManager->Exists(wndFile.pullAsString("window.background")))
        {
            background = gfx->load_texture(wndFile.pullAsString("window.background"), FILTER_LINEAR, &bkg_w, &bkg_h, false);
            delete_gltex = true;
        }
        else
            background = 0;
        color = wndFile.pullAsInt("window.color", delete_gltex ?  0xFFFFFFFF : makeacol(0x7F, 0x7F, 0x7F, 0xFF));
        FIX_COLOR(color);
        NbObj = wndFile.pullAsInt("window.number of objects");

        Objets = new GUIOBJ[NbObj];

        for (uint16 i = 0 ; i < NbObj ; ++i) // Loads each object
        {
            String obj_key = format("window.object%d." , i);
            String obj_type = wndFile.pullAsString(obj_key + "type");
            Objets[i].Name = wndFile.pullAsString(obj_key + "name", format("object%d", i));
            obj_hashtable.insert(String::ToLower(Objets[i].Name), i + 1);
            Objets[i].help_msg = I18N::Translate(wndFile.pullAsString(obj_key + "help"));

            float X1 = wndFile.pullAsFloat(obj_key + "x1") * x_factor;				// Reads data from TDF
            float Y1 = wndFile.pullAsFloat(obj_key + "y1") * y_factor;
            float X2 = wndFile.pullAsFloat(obj_key + "x2") * x_factor;
            float Y2 = wndFile.pullAsFloat(obj_key + "y2") * y_factor;
            String caption = I18N::Translate(wndFile.pullAsString(obj_key + "caption"));
            float size_factor = wndFile.pullAsFloat(obj_key + "size", 1.0f);
            float size = size_factor * Math::Min(x_factor, y_factor);
            int val = wndFile.pullAsInt(obj_key + "value");
            uint32 obj_flags = 0;
            uint32 obj_negative_flags = 0;

            if (X1<0) X1 += width;
            if (X2<0) X2 += width;
            if (Y1<0) Y1 += height;
            if (Y2<0) Y2 += height;
            //		if (X1<0)	X1+=SCREEN_W;
            //		if (X2<0)	X2+=SCREEN_W;
            //		if (Y1<0)	Y1+=SCREEN_H;
            //		if (Y2<0)	Y2+=SCREEN_H;

            if (wndFile.pullAsBool(obj_key + "can be clicked"))
                obj_flags |= FLAG_CAN_BE_CLICKED;
            if (wndFile.pullAsBool(obj_key + "can get focus"))
                obj_flags |= FLAG_CAN_GET_FOCUS;
            if (wndFile.pullAsBool(obj_key + "highlight"))
                obj_flags |= FLAG_HIGHLIGHT;
            if (wndFile.pullAsBool(obj_key + "fill"))
                obj_flags |= FLAG_FILL;
            if (wndFile.pullAsBool(obj_key + "hidden"))
                obj_flags |= FLAG_HIDDEN;
            if (wndFile.pullAsBool(obj_key + "no border"))
                obj_flags |= FLAG_NO_BORDER;
            if (wndFile.pullAsBool(obj_key + "cant be clicked"))
                obj_negative_flags |= FLAG_CAN_BE_CLICKED;
            if (wndFile.pullAsBool(obj_key + "cant get focus"))
                obj_negative_flags |= FLAG_CAN_GET_FOCUS;

            if (wndFile.pullAsBool(obj_key + "centered"))
            {
                obj_flags |= FLAG_CENTERED;
                X1 -= gui_font->length(caption) * 0.5f;
            }

            String::Vector Entry;
            wndFile.pullAsString(obj_key + "entry").split(Entry, ",");
            I18N::Translate(Entry);

            if (obj_type == "BUTTON")
                Objets[i].create_button(X1, Y1, X2, Y2, caption, NULL, size);
            else if (obj_type == "FMENU")
                Objets[i].create_menu(X1, Y1, Entry, NULL, size);
            else if (obj_type == "OPTIONB")
                Objets[i].create_optionb(X1, Y1, caption, val, NULL, skin, size);
            else if (obj_type == "PBAR")
                Objets[i].create_pbar(X1, Y1, X2, Y2, val, size);
            else if (obj_type == "TEXTEDITOR")
                Objets[i].create_texteditor(X1, Y1, X2, Y2, caption, size);
            else if (obj_type == "TEXTBAR")
                Objets[i].create_textbar(X1, Y1, X2, Y2, caption, val, NULL, size);
            else if (obj_type == "OPTIONC")
                Objets[i].create_optionc(X1, Y1, caption, val, NULL, skin, size);
            else if (obj_type == "MENU")
                Objets[i].create_menu(X1, Y1, X2, Y2, Entry, NULL, size);
            else if (obj_type == "TABUTTON" || obj_type == "MULTISTATE")
            {
                String::Vector imageNames;
                caption.split(imageNames, ",");
                std::vector<GLuint> gl_imgs;
                std::vector<uint32> t_w;
                std::vector<uint32> t_h;

                for (String::Vector::iterator e = imageNames.begin() ; e != imageNames.end() ; e++)
                {
                    uint32 tw, th;
                    GLuint texHandle = gfx->load_texture(*e, FILTER_LINEAR, &tw, &th);
                    if (texHandle)
                    {
                        gl_imgs.push_back(texHandle);
                        t_w.push_back(tw);
                        t_h.push_back(th);
                    }
                }

                Objets[i].create_ta_button(X1, Y1, Entry, gl_imgs, gl_imgs.size());
                for (unsigned int e = 0; e < Objets[i].gltex_states.size(); ++e)
                {
                    Objets[i].x2 = X1 + t_w[e] * size_factor * x_factor;
                    Objets[i].y2 = Y1 + t_h[e] * size_factor * y_factor;
                    Objets[i].gltex_states[e].width = t_w[e] * size_factor * x_factor;
                    Objets[i].gltex_states[e].height = t_h[e] * size_factor * x_factor;
                    Objets[i].gltex_states[e].destroy_tex = true;       // Make sure it'll be destroyed
                }
            }
            else if (obj_type == "TEXT")
            {
                FIX_COLOR(val);
                Objets[i].create_text(X1, Y1, caption, val, size);
                if (X2 > 0 && Y2 > Y1)
                {
                    Objets[i].x2 = X2;
                    Objets[i].y2 = Y2;
                    Objets[i].Flag |= FLAG_TEXT_ADJUST;
                }
            }
            else if (obj_type == "MISSION")
            {
                Objets[i].create_text(X1, Y1, caption, val, size);
                if (X2 > 0 && Y2 > Y1)
                {
                    Objets[i].x2 = X2;
                    Objets[i].y2 = Y2;
                    Objets[i].Flag |= FLAG_TEXT_ADJUST | FLAG_MISSION_MODE | FLAG_CAN_BE_CLICKED;
                }
            }
            else if (obj_type == "LINE")
            {
                FIX_COLOR(val);
                Objets[i].create_line(X1, Y1, X2, Y2, val);
            }
            else if (obj_type == "BOX")
            {
                FIX_COLOR(val);
                Objets[i].create_box(X1, Y1, X2, Y2, val);
            }
            else if (obj_type == "IMG")
            {
                Objets[i].create_img(X1, Y1, X2, Y2, gfx->load_texture(I18N::Translate(wndFile.pullAsString(obj_key + "source"))));
                Objets[i].destroy_img = Objets[i].Data != 0 ? true : false;
            }
            else if (obj_type == "LIST")
                Objets[i].create_list(X1, Y1, X2, Y2, Entry, size);

            wndFile.pullAsString(obj_key + "on click").split(Objets[i].OnClick, ",");
            wndFile.pullAsString(obj_key + "on hover").split(Objets[i].OnHover, ",");
            wndFile.pullAsString(obj_key + "send data to").toLower().split(Objets[i].SendDataTo, ",");
            wndFile.pullAsString(obj_key + "send pos to").toLower().split(Objets[i].SendPosTo, ",");

            Objets[i].Flag |= obj_flags;
            Objets[i].Flag &= ~obj_negative_flags;
        }
    }




} // namespace TA3D
