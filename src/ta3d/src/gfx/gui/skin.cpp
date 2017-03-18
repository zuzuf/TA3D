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
#include <stdafx.h>
#include "skin.h"
#include <TA3D_NameSpace.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <misc/timer.h>
#include "base.h"



namespace TA3D
{
namespace Gui
{



	Skin::Skin()
        : bkg_w(0), bkg_h(0), text_y_offset(0.f),
		pCacheFontHeight(gui_font-> height())
	{
		init();
	}

	Skin::~Skin()
	{
		destroy();
	}


	void Skin::init()
	{
		pPrefix.clear();
		pName.clear();
		text_y_offset = 0.f;

		for (int i = 0; i < 2 ; ++i)
			button_img[i].init();
		text_background.init();
		menu_background.init();
		wnd_border.init();
		wnd_title_bar.init();
		selection_gfx.init();
		for (int i = 0; i < 2; ++i)
			progress_bar[i].init();

        wnd_background = nullptr;
		checkbox[1].init();
		checkbox[0].init();
		option[1].init();
		option[0].init();
		scroll[2].init();
		scroll[1].init();
		scroll[0].init();
		bkg_h = bkg_w = 0;
	}

	void Skin::destroy()
	{
		for (int i = 0; i < 2; ++i)
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
		pPrefix.clear();
		pName.clear();
        wnd_background = nullptr;
	}


	void Skin::loadTDFFromFile(const QString& filename, const float scale)
	{
		destroy();		// In case there is a skin loaded so we don't waste memory

        if (filename.isEmpty())
		{
			LOG_WARNING("TDF: Attempting to load an empty file");
			return;
		}
		TDFParser skinFile(filename);

		// Grab the skin's name, so we can now if a skin is already in use
		QString tmp(Paths::ExtractFileNameWithoutExtension(filename));

		pName = skinFile.pullAsString("skin.name", tmp); // The TDF may override the skin name

		pPrefix = skinFile.pullAsString("skin.prefix", QString()); // The pPrefix to use for
		text_y_offset = float(skinFile.pullAsInt("skin.text.y offset", 0)) * scale;

		wnd_border.load(skinFile, "skin.window.border.", scale);
		wnd_title_bar.load(skinFile, "skin.window.title.", scale);

		button_img[0].load(skinFile, "skin.button.up.", scale);
		button_img[1].load(skinFile, "skin.button.down.", scale);
		button_color.load(skinFile, "skin.button.", scale);
		button_color_disabled = button_color;
		button_color_disabled.font_color = ((button_color.font_color & 0x00FEFEFE) >> 1) | (button_color.font_color & 0xFF000000);

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

		QString tex_file_name (skinFile.pullAsString("skin.window.image"));
		if (VFS::Instance()->fileExists(tex_file_name))
			wnd_background = gfx->load_texture( tex_file_name, FILTER_LINEAR, &bkg_w, &bkg_h, false);
	}


	void Skin::button(const float x, const float y, const float x2, const float y2, const QString &Title, const bool State, const bool enabled) const
	{
		gfx->set_alpha_blending();
		if (enabled)
			gfx->set_color(0xFFFFFFFF);
		else
			gfx->set_color(0xFF7F7F7F);

		button_img[(State ? 1 : 0)].draw( x, y, x2, y2);
		gfx->unset_alpha_blending();

        if (!Title.isEmpty())
		{
			const TEXT_COLOR &color = enabled ? button_color : button_color_disabled;
			if (State)
				color.print(gui_font, (x + x2) * 0.5f - (gui_font->length(Title) * 0.5f) + 1.0f, (y + y2 - pCacheFontHeight) * 0.5f + 1.0f, Title);
			else
				color.print(gui_font, (x + x2) * 0.5f - (gui_font->length(Title) * 0.5f), (y + y2 - pCacheFontHeight) * 0.5f, Title);
		}
	}


	/*---------------------------------------------------------------------------\
	|        Draw a list box displaying the content of Entry                     |
	\---------------------------------------------------------------------------*/

    void Skin::ListBox(float x1, float y1, float x2, float y2, const QStringList &Entry, int Index, int Scroll, uint32 flags) const
	{
        gfx->set_color(0xFFFFFFFF);

		if( !(flags & FLAG_NO_BORDER) )
		{
			gfx->set_alpha_blending();
			text_background.draw( x1, y1, x2, y2);
			gfx->unset_alpha_blending();
		}

		uint32 line = 0;
		QString rest;
		QString pCacheDrawTextStr;
		for (unsigned int i = 0; i < Entry.size(); ++i, ++line)
		{
			int e = i + Scroll;
			if (e >= (int)Entry.size() || pCacheFontHeight * float(line + 1) > y2 - y1 - text_background.y1 + text_background.y2) break;		// If we are out break the loop
            pCacheDrawTextStr = Entry[e];
			if (pCacheDrawTextStr.size() > 3 && pCacheDrawTextStr[0] == '<'  && pCacheDrawTextStr[1] == 'H' && pCacheDrawTextStr[2] == '>')       // Highlight this line
			{
				pCacheDrawTextStr = Substr(pCacheDrawTextStr, 3, pCacheDrawTextStr.size() - 3);
				glDisable(GL_TEXTURE_2D);
				gfx->rectfill(x1 + text_background.x1, y1 + text_background.y1 + pCacheFontHeight * float(line),
					x2 + text_background.x2 - scroll[ 0 ].sw, y1 + text_background.y1 + pCacheFontHeight * float(line+1), makeacol( 0x7F, 0x7F, 0xFF, 0xFF ));
			}
			rest.clear();
			do
			{
                if (!rest.isEmpty())
				{
					++line;
					pCacheDrawTextStr = rest;
					rest.clear();
				}
				if (pCacheFontHeight * float(line + 1) > y2 - y1 - text_background.y1 + text_background.y2)
					break;		// If we are out break the loop
				if (e == Index)
				{
					gfx->set_alpha_blending();
					selection_gfx.draw( x1 + text_background.x1, y1 + text_background.y1 + pCacheFontHeight * float(line), x2 + text_background.x2 - scroll[ 0 ].sw, y1 + text_background.y1 + pCacheFontHeight * float(line + 1));
					gfx->unset_alpha_blending();
				}
				if (gui_font->length(pCacheDrawTextStr) >= x2 - x1 - text_background.x1 + text_background.x2 - scroll[0].sw - 10.0f)
				{
					// Dychotomic search of the longest string that fits
					rest = pCacheDrawTextStr;
					int top = rest.size();
					int bottom = 0;
					while (top != bottom)
					{
						const int mid = (top + bottom) >> 1;
						pCacheDrawTextStr = Substr(rest, 0, mid);
						if (gui_font->length(pCacheDrawTextStr) >= x2 - x1 - text_background.x1 + text_background.x2 - scroll[0].sw - 10)
							top = mid;
						else
						{
							if (bottom == mid)
								break;
							bottom = mid;
						}
					}
					pCacheDrawTextStr = Substr(rest, 0, bottom);
					rest = Substr(rest, bottom);
				}
				else
					rest.clear();
                gui_font->print(10.0f + x1 + text_background.x1, y1 + text_background.y1 + pCacheFontHeight * float(line),
                                White, pCacheDrawTextStr);
            } while(!rest.isEmpty());
		}

		int TotalScroll = (int)Entry.size() - (int)((y2 - y1 - text_background.y1 + text_background.y2) / pCacheFontHeight);
		if (TotalScroll < 0)
			TotalScroll = 0;

		ScrollBar(	x2 + text_background.x2 - scroll[ 0 ].sw, y1 + text_background.y1,
					x2 + text_background.x2, y2 + text_background.y2,
					TotalScroll ? ((float)Scroll) / (float)TotalScroll : 0.0f, true);
	}

    void Skin::AppendLineToListBox(QStringList &Entry, float x1, float x2, QString line) const
	{
		QString rest;
		do
		{
            if (!rest.isEmpty())
			{
				line = rest;
				rest.clear();
			}
			if (gui_font->length(line) >= x2 - x1 - text_background.x1 + text_background.x2 - scroll[0].sw - 10.0f)
			{
				// Dychotomic search of the longest string that fits
				rest = line;
				int top = (int)rest.size();
				int bottom = 0;
				while (top != bottom)
				{
					int mid = (top + bottom) >> 1;
					line = Substr(rest, 0, mid);
					if (gui_font->length(line) >= x2 - x1 - text_background.x1 + text_background.x2 - scroll[0].sw - 10.0f)
						top = mid;
					else
					{
						if (bottom == mid)
							break;
						bottom = mid;
					}
				}
				line = Substr(rest, 0, bottom);
				rest = Substr(rest, bottom);
			}
			else
				rest.clear();
			Entry.push_back(line);
        } while(!rest.isEmpty());
	}

	/*---------------------------------------------------------------------------\
	  |        Draw a popup menu displaying the text msg using the skin object     |
	  \---------------------------------------------------------------------------*/

	void Skin::PopupMenu( float x1, float y1, const QString &msg) const
	{
		float x2 = x1;
		std::vector< QString > Entry;
		unsigned int last = 0;
		for (unsigned int i = 0; i < msg.length(); ++i)
		{
			if (msg[i] == '\n')
			{
				Entry.push_back(Substr(msg, last, i - last));
				x2 = Math::Max(x2, x1 + gui_font->length(Entry.back()));
				last = i + 1;
			}
		}
		if (last + 1 < msg.length())
		{
			Entry.push_back( Substr(msg, last, msg.length() - last));
			x2 = Math::Max(x2, x1 + gui_font->length(Entry.back()));
		}

		x2 += menu_background.x1 - menu_background.x2;
		float y2 = y1 + menu_background.y1 - menu_background.y2 + pCacheFontHeight * (float)Entry.size();
		const float screen_w = static_cast<float>(SCREEN_W);
		const float screen_h = static_cast<float>(SCREEN_H);
		if (x2 >= screen_w)
		{
			x1 += screen_w - x2 - 1;
			x2 = screen_w - 1;
		}
		if (y2 >= screen_h)
		{
			y1 += screen_h - y2 - 1;
			y2 = screen_h - 1;
		}

		ObjectShadow( x1, y1,
					  x2, y2,
					  2, 2,
					  0.5f, 4.0f);

		gfx->set_alpha_blending();
		gfx->set_color( 0xFFFFFFFF);

		menu_background.draw( x1, y1, x2, y2);
		gfx->unset_alpha_blending();

		for (unsigned int e = 0; e < Entry.size(); ++e)
			text_color.print(gui_font, x1 + menu_background.x1, y1 + menu_background.y1 + pCacheFontHeight * float(e), Entry[e]);
		Entry.clear();
	}

	/*---------------------------------------------------------------------------\
	  |        Draw a floatting menu with the parameters from Entry[]              |
	  \---------------------------------------------------------------------------*/

    void Skin::FloatMenu(float x, float y, const QStringList &Entry, int Index, int StartEntry) const
	{
		if (StartEntry < (int)Entry.size())
		{
			float width = 168.0f;
			for (unsigned int i = 0; i < Entry.size() - StartEntry; ++i)
				width = Math::Max(width, gui_font->length(Entry[i]));

			width += menu_background.x1 - menu_background.x2;

			ObjectShadow( x, y,
						  x + width, y + menu_background.y1 - menu_background.y2 + pCacheFontHeight * float(Entry.size() - StartEntry),
						  2, 2,
						  0.5f, 4.0f);

			gfx->set_color( 0xFFFFFFFF);
			gfx->set_alpha_blending();
			menu_background.draw( x, y, x + width, y + menu_background.y1 - menu_background.y2 + pCacheFontHeight * float(Entry.size() - StartEntry));

			for (unsigned int i = 0; i < Entry.size() - StartEntry; ++i)
			{
				unsigned int e = i + StartEntry;
				if (e == (uint32)Index)
					selection_gfx.draw( x + menu_background.x1, y + menu_background.y1 + pCacheFontHeight * float(i), x + width + menu_background.x2, y + menu_background.y1 + pCacheFontHeight * float(i + 1));
                gui_font->print(x + menu_background.x1, y + menu_background.y1 + pCacheFontHeight * float(i), White, Entry[e]);
			}
			gfx->unset_alpha_blending();
		}
	}

	/*---------------------------------------------------------------------------\
	  |        Draw an option button with text Title                               |
	  \---------------------------------------------------------------------------*/

	void Skin::OptionButton(float x,float y,const QString &Title,bool State) const
	{
		gfx->set_color( 0xFFFFFFFF);
		gfx->set_alpha_blending();

		option[ State ? 1 : 0 ].draw( x, y, x + option[ State ? 1 : 0 ].sw, y + option[ State ? 1 : 0 ].sh);
		gfx->unset_alpha_blending();

		text_color.print(gui_font, x + option[ State ? 1 : 0 ].sw + 4.0f, y + ( option[ State ? 1 : 0 ].sh - pCacheFontHeight ) * 0.5f, Title);
	}

	/*---------------------------------------------------------------------------\
	  |        Draw an option case with text Title                                 |
	  \---------------------------------------------------------------------------*/

	void Skin::OptionCase(float x,float y,const QString &Title,bool State) const
	{
		gfx->set_color( 0xFFFFFFFF);
		gfx->set_alpha_blending();

		checkbox[ State ? 1 : 0 ].draw( x, y, x + checkbox[ State ? 1 : 0 ].sw, y + checkbox[ State ? 1 : 0 ].sh);
		gfx->unset_alpha_blending();

		text_color.print(gui_font, x + checkbox[ State ? 1 : 0 ].sw + 4.0f, y + ( checkbox[ State ? 1 : 0 ].sh - pCacheFontHeight ) * 0.5f, Title);
	}

	/*---------------------------------------------------------------------------\
	  |        Draw a TEXTEDITOR widget (a large text input widget)                |
	  \---------------------------------------------------------------------------*/

    void Skin::TextEditor(float x1, float y1, float x2, float y2, const QStringList &Entry, int row, int col, bool State) const
	{
        bool blink = State && (msectimer() % 1000) >= 500;

		gfx->set_color( 0xFFFFFFFF);
		gfx->set_alpha_blending();

		text_background.draw( x1, y1, x2, y2);
		gfx->unset_alpha_blending();

		float maxlength = x2 - x1 + text_background.x2 - text_background.x1 - gui_font->length( "_");
		float maxheight = y2 - y1 + text_background.y2 - text_background.y1 - text_y_offset;
		int H = Math::Max( row - (int)(0.5f * maxheight / pCacheFontHeight), 0);
		int y = 0;
        int row_size = Entry[row].size();
		while (pCacheFontHeight * float(y + 1) <= maxheight && y + H < (int)Entry.size())
		{
			QString strtoprint;
			QString buf = Entry[y+H];
            for(int x = 0; !buf.isEmpty() ; )
			{
				uint32 k = 0;
				while(k < buf.size() && buf[k] == ' ')			// Removes useless spaces (it's unlikely we find any other type of blank character here)
					++k;
				if (k)
				{
					buf = Substr(buf, k);
					x += k;
				}

                int len = buf.size();
				int smax = len + 1;
				int smin = 0;
				int s = (smax + smin) >> 1;
				do
				{
					s = (smax + smin) >> 1;
                    strtoprint = Substr(buf, 0, s);
					if (s == smin)
						break;

					if (gui_font->length(strtoprint) > maxlength)
						smax = s;
					else
						smin = s;
					if (smax == smin)
                        strtoprint = Substr(buf, 0, smin);
				} while(smax != smin);

                if (len > s && Substr(strtoprint,s-1,1) != " " && Substr(buf,s,1) != " ")		// We're splitting a word :s
				{
					int olds = s;
                    while (s > 0 && Substr(strtoprint,s-1,1) != " " && Substr(buf,s,1) != " ")
						--s;
					if (s == 0)
						s = olds;
					else
                        strtoprint = Substr(buf, 0, s);
				}

                buf = Substr(buf, s);

                gui_font->print(x1 + text_background.x1,
                                y1 + text_background.y1 + text_y_offset + pCacheFontHeight * float(y),
                                White,
                                strtoprint);
                if (row == y + H && x <= col && col < x + s && blink)
				{
                    gui_font->print(x1 + text_background.x1 + gui_font->length(Substr(strtoprint, 0, col - x)),
                                    y1 + text_background.y1 + text_y_offset + pCacheFontHeight * float(y),
                                    White, "_");
                }
				x += s;
                if (!buf.isEmpty())
				{
					y++;
					H--;
				}
				if (pCacheFontHeight * float(y + 1) >= maxheight)
					break;
			}
			if (y + H == row && col == row_size && blink)
			{
                gui_font->print(x1 + text_background.x1 + gui_font->length(strtoprint),
                                y1 + text_background.y1 + text_y_offset + pCacheFontHeight * float(y),
                                White, "_");
            }
			++y;
		}
	}



	/*---------------------------------------------------------------------------\
	  |        Draw a text input bar, a way for user to enter text                 |
	  \---------------------------------------------------------------------------*/

	void Skin::TextBar(float x1,float y1,float x2,float y2,const QString &Caption,bool State) const
	{
        bool blink = State && (msectimer() % 1000) >= 500;

		gfx->set_color(0xFFFFFFFF);
		gfx->set_alpha_blending();

		text_background.draw( x1, y1, x2, y2);
		gfx->unset_alpha_blending();

		const float maxlength = x2 - x1 + text_background.x2 - text_background.x1 - gui_font->length( "_");
		int dec = 0;
		QString strtoprint = Substr(Caption, dec, Caption.length() - dec);
		while (gui_font->length( Substr(Caption, dec, Caption.length() - dec ) ) >= maxlength && dec < (int)Caption.length())
		{
			++dec;
			strtoprint = Substr(Caption, dec, Caption.length() - dec);
		}

        gui_font->print(x1 + text_background.x1,
                        y1 + text_background.y1 + text_y_offset,
                        White, strtoprint);
		if (blink)
            gui_font->print(x1 + text_background.x1 + gui_font->length(strtoprint),
                            y1 + text_background.y1 + text_y_offset,
                            White, "_");
	}



	/*---------------------------------------------------------------------------\
	  |                              Draw a scroll bar                             |
	  \---------------------------------------------------------------------------*/
	void Skin::ScrollBar( float x1, float y1, float x2, float y2, float Value, bool vertical) const
	{
		gfx->set_color( 0xFFFFFFFF);
		gfx->set_alpha_blending();

		if( Value < 0.0f )	Value = 0.0f;
		else if( Value > 1.0f )	Value = 1.0f;
		scroll[ vertical ? 0 : 1 ].draw( x1, y1, x2, y2);

		if (vertical)
		{
			float y = y1 + scroll[ 0 ].y1;
			float dx = x2 - x1 - scroll[ 0 ].x1 + scroll[ 0 ].x2;
			y += (y2 - y1 - scroll[ 0 ].y1 + scroll[ 0 ].y2 - dx) * Value;
			scroll[ 2 ].draw( x1 + scroll[ 0 ].x1, y, x2 + scroll[ 0 ].x2, y + dx);
		}
		else
		{
			float x = x1 + scroll[ 1 ].x1;
			float dy = y2 - y1 - scroll[ 1 ].y1 + scroll[ 1 ].y2;
			x += (x2 - x1 - scroll[ 1 ].x1 + scroll[ 1 ].x2 - dy) * Value;
			scroll[ 2 ].draw( x, y1 + scroll[ 1 ].y1, x + dy, y2 + scroll[ 1 ].y2);
		}
		gfx->unset_alpha_blending();
	}

	/*---------------------------------------------------------------------------\
	  |                     Draw a progress bar                                    |
	  \---------------------------------------------------------------------------*/

	void Skin::ProgressBar(float x1,float y1,float x2,float y2,int Value) const
	{
		gfx->set_color( 0xFFFFFFFF);
		gfx->set_alpha_blending();
		progress_bar[0].draw( x1, y1, x2, y2);
		progress_bar[1].draw( x1 + progress_bar[0].x1, y1 + progress_bar[0].y1, x1 + progress_bar[0].x1 + (progress_bar[0].x2 + x2 - x1 - progress_bar[0].x1) * float(Value) * 0.01f, y2 + progress_bar[0].y2);			// Draw the bar
		gfx->unset_alpha_blending();

        QString Buf = QString::number(Value) + "%";

        gui_font->print((x1 + x2) * 0.5f - gui_font->length(Buf) * 0.5f,
                        (y1 + y2) * 0.5f - pCacheFontHeight * 0.5f,
                        White, Buf);
	}

	/*---------------------------------------------------------------------------\
	  |        Draw a the given text within the given space                        |
	  \---------------------------------------------------------------------------*/

	int Skin::draw_text_adjust(float x1, float y1, float x2, float y2, const QString& msg, int pos, bool missionMode) const
	{
		int last = 0;
		QString pCacheDrawTextStr;
		QString pCacheDrawTextCurrent;
		QString pCacheDrawTextWord;
        QStringList pCacheDrawTextVector;

		for (unsigned int i = 0 ; i < msg.length(); ++i)
		{
			pCacheDrawTextStr.clear();
            if (((byte)msg[i].toLatin1()) < 0x80)
                pCacheDrawTextStr += msg[i];
			else
			{
				if (i + 1 < msg.length())
				{
                    pCacheDrawTextStr += msg[i];
                    pCacheDrawTextStr += msg[i+1];
                    i++;
				}
			}

			if (pCacheDrawTextStr == "\r")
				continue;
			else
			{
                QString tmp = pCacheDrawTextCurrent + ' ' + pCacheDrawTextWord + pCacheDrawTextStr;
				if (pCacheDrawTextStr == "\n" || gui_font->length(tmp) >= x2 - x1)
				{
					bool line_too_long = true;
					if (gui_font->length(tmp) < x2 - x1)
					{
                        pCacheDrawTextCurrent += ' ' + pCacheDrawTextWord;
						pCacheDrawTextWord.clear();
						line_too_long = false;
					}
					else if (pCacheDrawTextStr != "\n")
                        pCacheDrawTextWord += pCacheDrawTextStr;
					pCacheDrawTextVector.push_back(pCacheDrawTextCurrent);
					last = i + 1;
					pCacheDrawTextCurrent.clear();
					if (pCacheDrawTextStr == "\n" && line_too_long)
					{
						pCacheDrawTextVector.push_back(pCacheDrawTextWord);
						pCacheDrawTextWord.clear();
					}
				}
				else
				{
					if (pCacheDrawTextStr == " ")
					{
                        if (!pCacheDrawTextCurrent.isEmpty())
                            pCacheDrawTextCurrent += ' ';
                        pCacheDrawTextCurrent += pCacheDrawTextWord;
						pCacheDrawTextWord.clear();
					}
					else
                        pCacheDrawTextWord += pCacheDrawTextStr;
				}
			}
		}

        if (!pCacheDrawTextCurrent.isEmpty())
            pCacheDrawTextCurrent += ' ' + pCacheDrawTextWord;
		else
            pCacheDrawTextCurrent += pCacheDrawTextWord;

        if (last + 1 < (int)msg.length() && !pCacheDrawTextCurrent.isEmpty())
			pCacheDrawTextVector.push_back(pCacheDrawTextCurrent);

		gfx->set_color( 0xFFFFFFFF);

		if (missionMode)
		{
			uint32	current_color = 0xFFFFFFFF;
			for (unsigned int e = pos ; e < pCacheDrawTextVector.size() ; ++e)
			{
				const QString& item = pCacheDrawTextVector[e];
				if (y1 + pCacheFontHeight * float(e + 1 - pos) <= y2)
				{
					float x_offset = 0.0f;
					QString buf;
					for (unsigned int i = 0 ; i < item.size(); ++i)
					{
						pCacheDrawTextStr.clear();
                        if (((byte)item[i].toLatin1()) < 0x80)
                            pCacheDrawTextStr += item[i];
						else
						{
							if (i + 1 < item.size())
							{
                                pCacheDrawTextStr += item[i];
                                pCacheDrawTextStr += item[i + 1];
								i++;
							}
						}
						if (pCacheDrawTextStr == "&")
						{
							text_color.print( gui_font, x1 + x_offset, y1 + pCacheFontHeight * float(e - pos), current_color, buf);
							x_offset += gui_font->length( buf);
							buf.clear();

							current_color = 0xFFFFFFFF;									// Default: white
							if (i + 1 < item.size() && item[i+1] == 'R')
							{
								current_color = 0xFF0000FF;								// Red
								i++;
							}
							else
							{
								if (i + 1 < item.size() && item[i+1] == 'Y')
								{
									current_color = 0xFF00FFFF;								// Yellow
									i++;
								}
							}
						}
						else
                            buf += pCacheDrawTextStr;
					}
					text_color.print( gui_font, x1 + x_offset, y1 + pCacheFontHeight * float(e - pos), current_color, buf);
				}
			}
		}
		else
		{
			for (unsigned int e = pos; e < pCacheDrawTextVector.size(); ++e)
			{
				if (y1 + pCacheFontHeight * float(e + 1 - pos) <= y2)
					text_color.print(gui_font, x1, y1 + pCacheFontHeight * float(e - pos), pCacheDrawTextVector[e]);
			}
		}

		return (int)pCacheDrawTextVector.size();
	}



	void Skin::ObjectShadow(float x1, float y1, float x2, float y2, float dx, float dy, float alpha, float fuzzy) const
	{
		// Normalize shadow offsets
		dx *= float(SCREEN_W) / 800.0f;
		dy *= float(SCREEN_H) / 800.0f;
		fuzzy *= float(SCREEN_H) / 800.0f;

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

        gfx->rectfill(x1, y1, x2, y2, makeacol(0,0,0,alpha * 255));

		glBegin(GL_QUADS);
		// Left
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy, y1);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x1, y1);
		glVertex2f( x1, y2);

		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy, y2);

		// Right
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x2 + fuzzy, y1);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x2, y1);
		glVertex2f( x2, y2);

		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x2 + fuzzy, y2);

		// Top
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1, y1 - fuzzy);
		glVertex2f( x2, y1 - fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x2, y1);
		glVertex2f( x1, y1);

		// Bottom
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1, y2 + fuzzy);
		glVertex2f( x2, y2 + fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x2, y2);
		glVertex2f( x1, y2);

		// Top Left
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy2, y1 - fuzzy2);
		glVertex2f( x1, y1 - fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x1, y1);

		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy, y1);

		// Top Right
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x2 + fuzzy, y1);
		glVertex2f( x2 + fuzzy2, y1 - fuzzy2);
		glVertex2f( x2, y1 - fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x2, y1);

		// Bottom Right
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x2 + fuzzy, y2);
		glVertex2f( x2 + fuzzy2, y2 + fuzzy2);
		glVertex2f( x2, y2 + fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x2, y2);

		// Bottom Left
		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy2, y2 + fuzzy2);
		glVertex2f( x1, y2 + fuzzy);

		gfx->set_color(0.0f, 0.0f, 0.0f, alpha);
		glVertex2f( x1, y2);

		gfx->set_color(0.0f, 0.0f, 0.0f, 0.0f);
		glVertex2f( x1 - fuzzy, y2);
		glEnd();

		gfx->unset_alpha_blending();
		glEnable(GL_TEXTURE_2D);
	}




} // namespace Gui
} // namespace TA3D

