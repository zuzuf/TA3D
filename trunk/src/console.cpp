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
|                               console.cpp                             |
|      contient les classes nécessaires à la gestion d'une console dans |
| programme utilisant Allegro avec ou sans AllegroGL. La console        |
| dispose de sa propre procédure d'entrée et d'affichage mais celle-ci  |
| nécessite d'être appellée manuellement pour éviter les problèmes      |
| découlant d'un appel automatique par un timer.                        |
\----------------------------------------------------------------------*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "console.h"
#include "logs/logs.h"
#include <list>
#include "misc/osinfo.h"


TA3D::TA3D_DEBUG::cConsole *TA3D::VARS::Console;

cConsole::cConsole()			// Initialise la console
{
	m_Recording=false;
	m_InputText[0] = 0;
	m_Visible = 0.0f;
	m_Show = false;

	m_MaxDisplayItems = 15;
	m_CurrentLine = 0;
	m_CurrentTimer = msec_timer;
	m_std_output = false;
	m_log_close = false;
}


cConsole::~cConsole()
{
	m_LastEntries.clear();
}


void cConsole::AddEntryWarning( String NewEntry )
{
    LOG_WARNING(NewEntry);
    MutexLocker locker(pMutex);

	if( msec_timer - m_CurrentTimer >= 10 )
    {
		m_CurrentTimer = msec_timer;
		m_CurrentLine = 0;
	}

	if( m_Recording && m_CurrentLine >= CONSOLE_MAX_LINE ) 
		return;
	m_CurrentLine++;

	for(unsigned int i = 0 ; i < NewEntry.size() ; ++i)
    {
        if (NewEntry[i] == '\t')
            NewEntry[i] = ' ';
    }

	m_LastEntries.push_back("Warning: " + NewEntry);

	if( m_LastEntries.size() >= m_MaxDisplayItems )
		m_LastEntries.pop_front();
}


void cConsole::AddEntry( String NewEntry )
{
    LOG_INFO(NewEntry);
    MutexLocker locker(pMutex);

	if( msec_timer - m_CurrentTimer >= 10 )
    {
		m_CurrentTimer = msec_timer;
		m_CurrentLine = 0;
	}

	if( m_Recording && m_CurrentLine >= CONSOLE_MAX_LINE ) 
		return;
	m_CurrentLine++;

	for(unsigned int i = 0 ; i < NewEntry.size() ; ++i)
    {
        if (NewEntry[i] == '\t')
            NewEntry[i] = ' ';
    }

	m_LastEntries.push_back( NewEntry );

	if( m_LastEntries.size() >= m_MaxDisplayItems )
		m_LastEntries.pop_front();
}

void cConsole::AddEntryWarning(const char *txt, ...)		// Ajoute une nouvelle entrée
{
    MutexLocker locker(pMutex);
	if( msec_timer - m_CurrentTimer >= 10 )
    {
		m_CurrentTimer = msec_timer;
		m_CurrentLine = 0;
	}

	if( m_Recording && m_CurrentLine >= CONSOLE_MAX_LINE )
		return;
	++m_CurrentLine;

	int   result = -1, length = 256;
	char *buffer = 0;

	while( result == -1 )
	{
		va_list args;
		va_start(args, txt);

		if( buffer )
			delete [] buffer;

		buffer = new char [length + 1];

		memset(buffer, 0, length + 1);

#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
			result = _vsnprintf_s( buffer, length, _TRUNCATE, txt, args );
#else
			result = vsnprintf(buffer, length, txt, args);
#endif
			length *= 2;

			va_end(args);
		}

		String NewEntry( buffer );
		delete [] buffer;

	for (unsigned int i = 0 ; i < NewEntry.size() ; ++i)
    {
        if (NewEntry[i] == '\t')
            NewEntry[i] = ' ';
    }

	m_LastEntries.push_back("Warning: " + NewEntry);

	if( m_LastEntries.size() >= m_MaxDisplayItems )
		m_LastEntries.pop_front();

    LOG_WARNING(NewEntry);
}


void cConsole::AddEntry(const char *txt, ...)		// Ajoute une nouvelle entrée
{
    MutexLocker locker(pMutex);
	if( msec_timer - m_CurrentTimer >= 10 )
    {
		m_CurrentTimer = msec_timer;
		m_CurrentLine = 0;
	}

	if( m_Recording && m_CurrentLine >= CONSOLE_MAX_LINE )
		return;
	++m_CurrentLine;

	int   result = -1, length = 256;
	char *buffer = 0;

	while( result == -1 )
	{
		va_list args;
		va_start(args, txt);

		if( buffer )
			delete [] buffer;

		buffer = new char [length + 1];

		memset(buffer, 0, length + 1);

#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
			result = _vsnprintf_s( buffer, length, _TRUNCATE, txt, args );
#else
			result = vsnprintf(buffer, length, txt, args);
#endif
			length *= 2;

			va_end(args);
		}

		String NewEntry( buffer );
		delete [] buffer;

	for (unsigned int i = 0 ; i < NewEntry.size() ; ++i)
    {
        if (NewEntry[i] == '\t')
            NewEntry[i] = ' ';
    }

	m_LastEntries.push_back( NewEntry );

	if( m_LastEntries.size() >= m_MaxDisplayItems )
		m_LastEntries.pop_front();

	if( m_std_output )
        LOG_INFO(NewEntry);
    else
        LOG_DEBUG(NewEntry);
}

void cConsole::ToggleShow()
{
    pMutex.lock();
	m_Show ^= true;
    pMutex.unlock();
}

char *cConsole::draw( TA3D::Interfaces::GfxFont fnt, float dt, float fsize, bool force_show )
{
    MutexLocker locker(pMutex);

	float	m_vis = m_Visible;
	bool	m_sho = m_Show;

	if( force_show ) {
		m_Show = true;
		m_Visible = 1.0f;
		}

	set_uformat(U_UTF8);

	fsize++;

	if( m_Show )	m_Visible+=dt;
	else			m_Visible-=dt;

	if( m_Visible<0.0f ) m_Visible=0.0f;
	if( m_Visible>1.0f ) m_Visible=1.0f;

	if(!m_Show && m_Visible == 0.0f )
		return NULL;

	char keyb=0;

	if(keypressed())
		keyb=readkey();

	float maxh = fsize * m_LastEntries.size()* m_Visible + 5.0f;

	char *newline=NULL;

	glEnable(GL_BLEND);		// Dessine le cadre de la console
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.75f,0.75f,0.608f,0.5f);

	gfx->rectfill(0,0,SCREEN_W,maxh);

	glColor4f(0.75f,0.75f,0.608f,0.75f);
	gfx->line(SCREEN_W,maxh,0,maxh);

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	int i = 0;
	for (String::List::const_iterator i_entry = m_LastEntries.begin(); i_entry != m_LastEntries.end(); ++i_entry) 
	{
		gfx->print(fnt,0.0f,maxh-fsize*(m_LastEntries.size()+1-i)-5.0f,0.0f,0xAFAFAFAF,
			format(">%s", (char *)((*i_entry).c_str())));
		++i;
	}

	gfx->print(fnt,0.0f,maxh-fsize-5.0f,0.0f,0xFFFFFFFF,format(">%s_",m_InputText));

	if(keyb==13)
	{
		String x = m_InputText;
		AddEntry( x );

		m_InputText[0] = 0;

		newline = strdup((char *)x.c_str());
	}

	if(keyb==8 && strlen(m_InputText)>0 )
		m_InputText[ strlen(m_InputText)-1]=0;

	if( (keyb>='0' && keyb<='9') || 
		(keyb>='a' && keyb<='z') || 
		(keyb>='A' && keyb<='Z') || 
		keyb==32 || keyb=='_' || keyb=='+' || keyb=='-' || keyb=='.') 
	{

			if( strlen(m_InputText)<199 ) 
			{
				m_InputText[strlen(m_InputText)+1]=0;
				m_InputText[strlen(m_InputText)]=keyb;
			}
	}

	glDisable(GL_BLEND);
	set_uformat(U_ASCII);

	if( force_show ) {
		m_Show = m_sho;
		m_Visible = m_vis;
		}

	return newline;
}

