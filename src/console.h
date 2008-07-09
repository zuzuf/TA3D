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

# include "threads/thread.h"


#define CONSOLE_MAX_LINE	100

namespace TA3D
{
namespace TA3D_DEBUG
{

    class cConsole : public ObjectSync
    {
    private:
        String::List m_LastEntries;
        bool				m_Recording;
        FILE				*m_log;
        String				m_RecordFilename;
        uint16				m_MaxDisplayItems;
        real32				m_Visible;
        bool				m_Show;
        char				m_InputText[200];
        uint16				m_CurrentLine;
        uint32				m_CurrentTimer;
        bool				m_std_output;
        bool				m_log_close;

    private:
        void DumpStartupInfo();

    public:
        cConsole();
        cConsole( const String &file );
        cConsole( const FILE *file );

        ~cConsole();

        void ToggleShow();

        bool activated() const {return m_Show || m_Visible > 0.0f;}

        void StopRecording( void );
        void StartRecording( const char *file );
        void StartRecording( const FILE *file );

        void stdout_on()	{	m_std_output = true;	}
        void stdout_off()	{	m_std_output = false;	}

        void AddEntryWarning(const char *txt, ...);
        void AddEntryWarning( String NewEntry );
        void AddEntry(const char *txt, ...);
        void AddEntry( String NewEntry );

        char *draw( TA3D::Interfaces::GfxFont fnt, float dt, float fsize=8, bool force_show=false );

    }; //class cConsole


} // namespace TA3D_DEBUG
} // namespace TA3D

#endif // _TA3D_XX_CLASSE_CONSOLE_H__
