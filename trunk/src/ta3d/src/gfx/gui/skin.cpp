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
#include "../../stdafx.h"
#include "skin.h"
#include "../../TA3D_NameSpace.h"
#include "../../misc/paths.h"
#include "../../misc/math.h"
#include "base.h"



namespace TA3D
{


    SKIN::~SKIN()
    {
        destroy();
    }


    void SKIN::init()
    {
        prefix.clear();
        text_y_offset = 0;

        for (sint8 i = 0; i < 2 ; ++i)
            button_img[i].init();
        text_background.init();
        menu_background.init();
        wnd_border.init();
        wnd_title_bar.init();
        selection_gfx.init();
        for (sint8 i = 0; i < 2; ++i)
            progress_bar[i].init();

        wnd_background = 0;
        checkbox[1].init();
        checkbox[0].init();
        option[1].init();
        option[0].init();
        scroll[2].init();
        scroll[1].init();
        scroll[0].init();
        bkg_h = bkg_w = 0;
    }

    void SKIN::destroy()
    {
        for (sint8 i = 0; i < 2; ++i)
        {
            progress_bar[i].destroy();
            button_img[i].destroy();
            checkbox[i].destroy();
            option[i].destroy();
            scroll[i].destroy();
        }
        scroll[2].destroy();
        text_background.destroy();
        menu_background.destroy();
        wnd_border.destroy();
        wnd_title_bar.destroy();
        selection_gfx.destroy();
        prefix.clear();
        Name.clear();
        gfx->destroy_texture(wnd_background);
    }


    void SKIN::load_tdf(const String& filename, const float scale)
    {
        destroy();		// In case there is a skin loaded so we don't waste memory

        LOG_DEBUG("loading " << filename);
        TDFParser skinFile(filename);

        // Grab the skin's name, so we can now if a skin is already in use
        String::size_type e = filename.find('.');
        if (e != String::npos)
            Name = filename.substr( 0, e );
        else
            Name = filename;

        e = Name.find_last_of("/\\");

        if (e != String::npos)
            Name = Name.substr(e + 1, Name.size() - e - 1);

        Name = skinFile.pullAsString("skin.name", Name); // The TDF may override the skin name

        prefix = skinFile.pullAsString("skin.prefix", ""); // The prefix to use for
        text_y_offset = skinFile.pullAsInt("skin.text.y offset", 0) * scale;

        wnd_border.load(skinFile, "skin.window.border.", scale);
        wnd_title_bar.load(skinFile, "skin.window.title.", scale);

        button_img[0].load(skinFile, "skin.button.up.", scale);
        button_img[1].load(skinFile, "skin.button.down.", scale);
        button_color.load(skinFile, "skin.button.", scale);

        text_background.load(skinFile, "skin.text bar.", scale);
        text_color.load(skinFile, "skin.text.", scale);

        menu_background.load(skinFile, "skin.menu.", scale);

        progress_bar[0].load(skinFile, "skin.progress bar.background.", scale);
        progress_bar[1].load(skinFile, "skin.progress bar.bar.", scale);

        selection_gfx.load(skinFile, "skin.selection.", scale);

        option[0].load(skinFile, "skin.option.off.", scale);
        option[1].load(skinFile, "skin.option.on.", scale);

        checkbox[0].load(skinFile, "skin.checkbox.off.", scale);
        checkbox[1].load(skinFile, "skin.checkbox.on.", scale);

        scroll[0].load(skinFile, "skin.scroll.v.", scale);
        scroll[1].load(skinFile, "skin.scroll.h.", scale);
        scroll[2].load(skinFile, "skin.scroll.s.", scale);

        String tex_file_name (skinFile.pullAsString("skin.window.image"));
        if(HPIManager->Exists(tex_file_name))
            wnd_background = gfx->load_texture( tex_file_name, FILTER_LINEAR, &bkg_w, &bkg_h, false );
    }

// Rendering functions

    /*---------------------------------------------------------------------------\
    |        Draw a button between (x,y) and (x2,y2) with Title and State        |
    \---------------------------------------------------------------------------*/

    void SKIN::button (float x,float y,float x2,float y2,const String &Title,bool State)
    {
        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );

        button_img[ (int)State ].draw( x, y, x2, y2 );
        gfx->unset_alpha_blending();

        if (!Title.empty())
        {
            if (State)
                button_color.print(gui_font, (int)((x + x2) * 0.5f - (gui_font->length(Title) * 0.5f) + 1), (int)((y + y2 - (int)(gui_font->height())) * 0.5f + 1), Title);
            else
                button_color.print(gui_font, (int)((x + x2) * 0.5f - (gui_font->length(Title) * 0.5f)) ,(int)(y + y2 - (int)(gui_font->height())) * 0.5f, Title);
        }
    }

    /*---------------------------------------------------------------------------\
    |        Draw a list box displaying the content of Entry                     |
    \---------------------------------------------------------------------------*/

    void SKIN::ListBox(float x1,float y1, float x2, float y2,const String::Vector &Entry,int Index, int Scroll, uint32 flags )
    {
        gfx->set_color( 0xFFFFFFFF );

        if( !(flags & FLAG_NO_BORDER) )
        {
            gfx->set_alpha_blending();
            text_background.draw( x1, y1, x2, y2 );
            gfx->unset_alpha_blending();
        }

        int i;
        for( i = 0 ; i < Entry.size() ; i++ )
        {
            int e = i+Scroll;
            if (e >= Entry.size() || gui_font->height() * (i+1) > y2 - y1 - text_background.y1 + text_background.y2) break;		// If we are out break the loop
            if (e == Index)
                selection_gfx.draw( x1 + text_background.x1, y1 + text_background.y1 + gui_font->height() * i, x2 + text_background.x2, y1 + text_background.y1 + gui_font->height() * (i+1) );
            String str = Entry[ e ];
            if (str.substr(0,3) == "<H>")       // Highlight this line
            {
                str = str.substr(3, str.size() - 3);
                glEnable( GL_BLEND );
                glDisable( GL_TEXTURE_2D );
                glBlendFunc( GL_ONE, GL_ONE );
                gfx->rectfill( x1 + text_background.x1, y1 + text_background.y1 + gui_font->height() * i, x2 + text_background.x2, y1 + text_background.y1 + gui_font->height() * (i+1), makeacol( 0x7F, 0x7F, 0xFF, 0xFF ) );
                glDisable( GL_BLEND );
            }
            while( gui_font->length( str ) >= x2 - x1 - text_background.x1 + text_background.x2 - scroll[0].sw && str.size() > 0 )
                str.resize( str.size() - 1 );
            gfx->print(gui_font, x1 + text_background.x1, y1 + text_background.y1 + gui_font->height() * i, 0.0f, White, str);
        }

        int TotalScroll = Entry.size() - (int)( (y2 - y1 - text_background.y1 + text_background.y2) / gui_font->height() );
        if (TotalScroll < 0)	TotalScroll = 0;

        ScrollBar(	x2 + text_background.x2 - scroll[ 0 ].sw, y1 + text_background.y1,
                    x2 + text_background.x2, y2 + text_background.y2,
                    TotalScroll ? ((float)Scroll) / TotalScroll : 0.0f, true);
    }

    /*---------------------------------------------------------------------------\
    |        Draw a popup menu displaying the text msg using the skin object     |
    \---------------------------------------------------------------------------*/

    void SKIN::PopupMenu( float x1, float y1, const String &msg)
    {
        float x2 = x1;
        std::vector< String > Entry;
        int last = 0;
        for( int i = 0 ; i < msg.length() ; i++ )
            if (msg[i] == '\n')
            {
                Entry.push_back( msg.substr( last, i - last ) );
                x2 = Math::Max(x2, x1 + gui_font->length(Entry.back()));
                last = i+1;
            }
        if (last + 1 < msg.length())
        {
            Entry.push_back( msg.substr( last, msg.length() - last));
            x2 = Math::Max(x2, x1 + gui_font->length(Entry.back()));
        }

        x2 += menu_background.x1 - menu_background.x2;
        float y2 = y1 + menu_background.y1 - menu_background.y2 + gui_font->height() * Entry.size();
        if (x2 >= SCREEN_W)
        {
            x1 += SCREEN_W - x2 - 1;
            x2 = SCREEN_W - 1;
        }
        if (y2 >= SCREEN_H)
        {
            y1 += SCREEN_H - y2 - 1;
            y2 = SCREEN_H - 1;
        }

        ObjectShadow( x1, y1,
                      x2, y2,
                      2, 2,
                      0.5f, 4.0f);

        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );

        menu_background.draw( x1, y1, x2, y2 );
        gfx->unset_alpha_blending();

        for( int e = 0 ; e < Entry.size() ; e++ )
            text_color.print(gui_font,x1 + menu_background.x1, y1 + menu_background.y1 + gui_font->height() * e,Entry[e]);
        Entry.clear();
    }

    /*---------------------------------------------------------------------------\
    |        Draw a floatting menu with the parameters from Entry[]              |
    \---------------------------------------------------------------------------*/

    void SKIN::FloatMenu(float x, float y, const String::Vector &Entry, int Index, int StartEntry)
    {
        int i;
        float width = 168.0f;
        for (i = 0; i < Entry.size() - StartEntry; ++i)
            width = Math::Max(width, gui_font->length(Entry[i]));

        width += menu_background.x1 - menu_background.x2;

        ObjectShadow( x, y,
                      x + width, y + menu_background.y1 - menu_background.y2 + gui_font->height() * (Entry.size() - StartEntry),
                      2, 2,
                      0.5f, 4.0f);

        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();
        menu_background.draw( x, y, x + width, y + menu_background.y1 - menu_background.y2 + gui_font->height() * (Entry.size() - StartEntry) );

        for (i = 0; i < Entry.size() - StartEntry; ++i)
        {
            int e = i + StartEntry;
            if( e == Index )
                selection_gfx.draw( x + menu_background.x1, y + menu_background.y1 + gui_font->height() * i, x + width + menu_background.x2, y + menu_background.y1 + gui_font->height() * (i + 1) );
            gfx->print(gui_font, x + menu_background.x1, y + menu_background.y1 + gui_font->height() * i, 0.0f, White, Entry[e]);
        }
        gfx->unset_alpha_blending();
    }

    /*---------------------------------------------------------------------------\
    |        Draw an option button with text Title                               |
    \---------------------------------------------------------------------------*/

    void SKIN::OptionButton(float x,float y,const String &Title,bool State)
    {
        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();

        option[ State ? 1 : 0 ].draw( x, y, x + option[ State ? 1 : 0 ].sw, y + option[ State ? 1 : 0 ].sh );
        gfx->unset_alpha_blending();

        text_color.print(gui_font, x + option[ State ? 1 : 0 ].sw + 4.0f, y + ( option[ State ? 1 : 0 ].sh - gui_font->height() ) * 0.5f, Title);
    }

    /*---------------------------------------------------------------------------\
    |        Draw an option case with text Title                                 |
    \---------------------------------------------------------------------------*/

    void SKIN::OptionCase(float x,float y,const String &Title,bool State)
    {
        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();

        checkbox[ State ? 1 : 0 ].draw( x, y, x + checkbox[ State ? 1 : 0 ].sw, y + checkbox[ State ? 1 : 0 ].sh );
        gfx->unset_alpha_blending();

        text_color.print(gui_font, x + checkbox[ State ? 1 : 0 ].sw + 4.0f, y + ( checkbox[ State ? 1 : 0 ].sh - gui_font->height() ) * 0.5f, Title);
    }

    /*---------------------------------------------------------------------------\
    |        Draw a TEXTEDITOR widget (a large text input widget)                |
    \---------------------------------------------------------------------------*/

    void SKIN::TextEditor(float x1, float y1, float x2, float y2, const String::Vector &Entry, int row, int col, bool State)
    {
        bool blink = State && (msec_timer % 1000) >= 500;

        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();

        text_background.draw( x1, y1, x2, y2 );
        gfx->unset_alpha_blending();

        float maxlength = x2 - x1 + text_background.x2 - text_background.x1 - gui_font->length( "_" );
        float maxheight = y2 - y1 + text_background.y2 - text_background.y1 - text_y_offset;
        int H = Math::Max( row - (int)(0.5f * maxheight / gui_font->height()), 0 );
        int y = 0;
        int row_size = Entry[row].sizeUTF8();
        while (gui_font->height() * (y+1) <= maxheight && y + H < Entry.size())
        {
            float xdec = -1.0f;
            String strtoprint;
            String buf = Entry[y+H];
            int len = buf.sizeUTF8();
            for (int x = 0; !buf.empty() ; x++)
            {
                String ch = buf.substrUTF8(0,1);
                buf = buf.substrUTF8(1, --len);
                if (gui_font->length( strtoprint + ch) > maxlength)
                {
                    gfx->print( gui_font,x1+text_background.x1,
                                y1+text_background.y1+text_y_offset+gui_font->height() * y,
                                0.0f,White,strtoprint);
                    if (xdec >= 0.0f)
                        gfx->print( gui_font,x1+text_background.x1+xdec,
                                    y1+text_background.y1+text_y_offset+gui_font->height() * y,
                                    0.0f,White,"_");
                    xdec = -1.0f;
                    y++;
                    H--;
                    strtoprint.clear();
                    if (gui_font->height() * (y+1) >= maxheight)    break;
                }
                if (row == y+H && x == col && blink)
                    xdec = gui_font->length( strtoprint );
                strtoprint << ch;
            }
            gfx->print( gui_font,x1+text_background.x1,
                        y1+text_background.y1+text_y_offset+gui_font->height() * y,
                        0.0f,White,strtoprint);
            if (y+H == row && col == row_size && blink)
                xdec = gui_font->length( strtoprint );
            if (xdec >= 0.0f)
                gfx->print( gui_font,x1+text_background.x1+xdec,
                            y1+text_background.y1+text_y_offset+gui_font->height() * y,
                            0.0f,White,"_");
            y++;
        }
    }

    /*---------------------------------------------------------------------------\
    |        Draw a text input bar, a way for user to enter text                 |
    \---------------------------------------------------------------------------*/

    void SKIN::TextBar(float x1,float y1,float x2,float y2,const String &Caption,bool State)
    {
        bool blink = State && (msec_timer % 1000) >= 500;

        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();

        text_background.draw( x1, y1, x2, y2 );
        gfx->unset_alpha_blending();

        float maxlength = x2 - x1 + text_background.x2 - text_background.x1 - gui_font->length( "_" );
        int dec = 0;
        String strtoprint = Caption.substr( dec, Caption.length() - dec );
        while (gui_font->length( Caption.substr( dec, Caption.length() - dec ) ) >= maxlength && dec < Caption.length())
        {
            dec++;
            strtoprint = Caption.substr( dec, Caption.length() - dec );
        }

        gfx->print(gui_font,x1+text_background.x1,y1+text_background.y1+text_y_offset,0.0f,White,strtoprint);
        if (blink) gfx->print(gui_font,x1+text_background.x1+gui_font->length( strtoprint ),y1+text_background.y1+text_y_offset,0.0f,White,"_");
    }

    /*---------------------------------------------------------------------------\
    |                              Draw a scroll bar                             |
    \---------------------------------------------------------------------------*/
    void SKIN::ScrollBar( float x1, float y1, float x2, float y2, float Value, bool vertical)
    {
        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();

        if( Value < 0.0f )	Value = 0.0f;
        else if( Value > 1.0f )	Value = 1.0f;
        scroll[ vertical ? 0 : 1 ].draw( x1, y1, x2, y2 );

        if (vertical)
        {
            float y = y1 + scroll[ 0 ].y1;
            float dx = x2 - x1 - scroll[ 0 ].x1 + scroll[ 0 ].x2;
            y += (y2 - y1 - scroll[ 0 ].y1 + scroll[ 0 ].y2 - dx) * Value;
            scroll[ 2 ].draw( x1 + scroll[ 0 ].x1, y, x2 + scroll[ 0 ].x2, y + dx );
        }
        else
        {
            float x = x1 + scroll[ 1 ].x1;
            float dy = y2 - y1 - scroll[ 1 ].y1 + scroll[ 1 ].y2;
            x += (x2 - x1 - scroll[ 1 ].x1 + scroll[ 1 ].x2 - dy) * Value;
            scroll[ 2 ].draw( x, y1 + scroll[ 1 ].y1, x + dy, y2 + scroll[ 1 ].y2 );
        }
        gfx->unset_alpha_blending();
    }

    /*---------------------------------------------------------------------------\
    |                     Draw a progress bar                                    |
    \---------------------------------------------------------------------------*/

    void SKIN::ProgressBar(float x1,float y1,float x2,float y2,int Value)
    {
        gfx->set_color( 0xFFFFFFFF );
        gfx->set_alpha_blending();
        progress_bar[0].draw( x1, y1, x2, y2 );
        progress_bar[1].draw( x1 + progress_bar[0].x1, y1 + progress_bar[0].y1, x1 + progress_bar[0].x1 + (progress_bar[0].x2 + x2 - x1 - progress_bar[0].x1) * Value * 0.01f, y2 + progress_bar[0].y2 );			// Draw the bar
        gfx->unset_alpha_blending();

        String Buf = format("%d", Value) + "%%";

        gfx->print(gui_font,(x1+x2)*0.5f-gui_font->length( Buf ) * 0.5f,(y1+y2)*0.5f-gui_font->height()*0.5f,0.0f,White,Buf);
    }

    /*---------------------------------------------------------------------------\
    |        Draw a the given text within the given space                        |
    \---------------------------------------------------------------------------*/

    int SKIN::draw_text_adjust(float x1, float y1, float x2, float y2, String msg, int pos, bool mission_mode)
    {
        String current = "";
        String current_word = "";
        std::vector< String > Entry;
        int last = 0;
        for( int i = 0 ; i < msg.length() ; i++ )
        {
            String str;
            if (((byte)msg[i]) < 0x80)
                str << msg[i];
            else if (i + 1 < msg.length())
            {
                str << msg[i] << msg[i+1];
                i++;
            }

            if (str == "\r")	continue;
            else if (str == "\n" || gui_font->length( current + ' ' + current_word + str ) >= x2 - x1)
            {
                bool line_too_long = true;
                if (gui_font->length( current + ' ' + current_word + str ) < x2 - x1)
                {
                    current << ' ' << current_word;
                    current_word.clear();
                    line_too_long = false;
                }
                else if (str != "\n")
                    current_word << str;
                Entry.push_back( current );
                last = i + 1;
                current.clear();
                if (str == "\n" && line_too_long)
                {
                    Entry.push_back( current_word );
                    current_word.clear();
                }
            }
            else
            {
                if (str == " ")
                {
                    if (!current.empty())
                        current << ' ';
                    current << current_word;
                    current_word.clear();
                }
                else
                    current_word << str;
            }
        }

        if (!current.empty())
            current << ' ' << current_word;
        else
            current << current_word;

        if (last + 1 < msg.length() && !current.empty())
            Entry.push_back( current );

        gfx->set_color( 0xFFFFFFFF );

        if (mission_mode)
        {
            uint32	current_color = 0xFFFFFFFF;
            for( int e = pos ; e < Entry.size() ; e++ )
                if (y1 + gui_font->height() * (e + 1 - pos) <= y2)
                {
                    float x_offset = 0.0f;
                    String buf;
                    for( int i = 0 ; i < Entry[e].size() ; i++ )
                    {
                        String str;
                        if (((byte)Entry[e][i]) < 0x80)
                            str << Entry[e][i];
                        else if (i + 1 < Entry[e].size())
                        {
                            str << Entry[e][i] << Entry[e][i + 1];
                            i++;
                        }
                        if (str == "&")
                        {
                            gfx->print( gui_font, x1 + x_offset, y1 + gui_font->height() * (e - pos), 0.0f, current_color, buf );
                            x_offset += gui_font->length( buf );
                            buf.clear();

                            current_color = 0xFFFFFFFF;									// Default: white
                            if (i + 1 < Entry[e].size() && Entry[e][i+1] == 'R')
                            {
                                current_color = 0xFF0000FF;								// Red
                                i++;
                            }
                            else if (i + 1 < Entry[e].size() && Entry[e][i+1] == 'Y')
                            {
                                current_color = 0xFF00FFFF;								// Yellow
                                i++;
                            }
                        }
                        else
                            buf << str;
                    }
                    text_color.print( gui_font, x1 + x_offset, y1 + gui_font->height() * (e - pos), current_color, buf );
                }
        }
        else
        {
            for( int e = pos ; e < Entry.size() ; e++ )
                if( y1 + gui_font->height() * (e + 1 - pos) <= y2 )
                    text_color.print( gui_font, x1, y1 + gui_font->height() * (e - pos), Entry[e] );
        }

        return Entry.size();
    }

    void SKIN::ObjectShadow(float x1, float y1, float x2, float y2, float dx, float dy, float alpha, float fuzzy)
    {
        x1 += dx;
        y1 += dy;
        x2 += dx;
        y2 += dy;

        x1 += fuzzy * 0.5f;
        y1 += fuzzy * 0.5f;
        x2 -= fuzzy * 0.5f;
        y2 -= fuzzy * 0.5f;

        glDisable(GL_TEXTURE_2D);
        gfx->set_alpha_blending();

        float fuzzy2 = fuzzy / sqrtf(2.0f);

        gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
        gfx->rectfill( x1, y1, x2, y2 );

        glBegin(GL_QUADS);
            // Left
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy, y1 );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x1, y1 );
            glVertex2f( x1, y2 );

            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy, y2 );

            // Right
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x2 + fuzzy, y1 );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x2, y1 );
            glVertex2f( x2, y2 );

            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x2 + fuzzy, y2 );

            // Top
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1, y1 - fuzzy );
            glVertex2f( x2, y1 - fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x2, y1 );
            glVertex2f( x1, y1 );

            // Bottom
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1, y2 + fuzzy );
            glVertex2f( x2, y2 + fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x2, y2 );
            glVertex2f( x1, y2 );

            // Top Left
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy2, y1 - fuzzy2 );
            glVertex2f( x1, y1 - fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x1, y1 );

            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy, y1 );

            // Top Right
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x2 + fuzzy, y1 );
            glVertex2f( x2 + fuzzy2, y1 - fuzzy2 );
            glVertex2f( x2, y1 - fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x2, y1 );

            // Bottom Right
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x2 + fuzzy, y2 );
            glVertex2f( x2 + fuzzy2, y2 + fuzzy2 );
            glVertex2f( x2, y2 + fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x2, y2 );

            // Bottom Left
            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy2, y2 + fuzzy2 );
            glVertex2f( x1, y2 + fuzzy );

            gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
            glVertex2f( x1, y2 );

            gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
            glVertex2f( x1 - fuzzy, y2 );
        glEnd();

        gfx->unset_alpha_blending();
        glEnable(GL_TEXTURE_2D);
    }
} // namespace TA3D
