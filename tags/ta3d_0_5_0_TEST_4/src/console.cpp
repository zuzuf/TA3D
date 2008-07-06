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


TA3D::TA3D_DEBUG::cConsole *TA3D::VARS::Console;

cConsole::cConsole()			// Initialise la console
{
	m_Recording=false;
	m_log = NULL;
	m_InputText[0] = 0;
	m_Visible = 0.0f;
	m_Show = false;

	m_MaxDisplayItems = 15;
	m_CurrentLine = 0;
	m_CurrentTimer = msec_timer;
	m_std_output = false;
	m_log_close = false;

	DumpStartupInfo();
}

cConsole::cConsole( const String &file )
{
	m_Recording=false;
	m_log = NULL;
	m_InputText[0] = 0;
	m_Visible = 0.0f;
	m_Show = false;

	m_MaxDisplayItems = 15;
	m_CurrentLine = 0;
	m_CurrentTimer = msec_timer;
	m_std_output = false;
	m_log_close = false;

	StartRecording( file.c_str() );

	DumpStartupInfo();
}

cConsole::cConsole( const FILE *file )
{
	m_Recording=false;
	m_log = NULL;
	m_InputText[0] = 0;
	m_Visible = 0.0f;
	m_Show = false;

	m_MaxDisplayItems = 15;
	m_CurrentLine = 0;
	m_CurrentTimer = msec_timer;
	m_std_output = false;
	m_log_close = false;

	StartRecording( file );

	DumpStartupInfo();
}

cConsole::~cConsole()
{
	StopRecording();
	m_LastEntries.clear();
}

void cConsole::DumpStartupInfo()
{
	stdout_on();

	AddEntry( "Allegro: %s (%s)", ALLEGRO_VERSION_STR, ALLEGRO_DATE_STR );

#ifdef AGL_VERSION						// Version d'allegroGL
	AddEntry( "AllegroGL version: %s", AGL_VERSION_STR );
#else
	AddEntry( "AllegroGL version: unknown." );
#endif

	switch(os_type)
	{
#ifdef OSTYPE_WIN3
		case OSTYPE_WIN3:		AddEntry("OS : Windows 3.1");	break;
#endif
#ifdef OSTYPE_WIN95
		case OSTYPE_WIN95:		AddEntry("OS : Windows 95");	break;
#endif
#ifdef OSTYPE_WIN98
		case OSTYPE_WIN98:		AddEntry("OS : Windows 98");	break;
#endif
#ifdef OSTYPE_WINME
		case OSTYPE_WINME:		AddEntry("OS : Windows ME");	break;
#endif
#ifdef OSTYPE_WINNT
		case OSTYPE_WINNT:		AddEntry("OS : Windows NT");	break;
#endif
#ifdef OSTYPE_WIN2000
		case OSTYPE_WIN2000:	AddEntry("OS : Windows 2000");	break;
#endif
#ifdef OSTYPE_WINXP
		case OSTYPE_WINXP:		AddEntry("OS : Windows XP");	break;
#endif
#ifdef OSTYPE_WIN2003
		case OSTYPE_WIN2003:	AddEntry("OS : Windows 2003");	break;
#endif
#ifdef OSTYPE_WINVISTA
		case OSTYPE_WINVISTA:	AddEntry("OS : Windows Vista");	break;
#endif
#ifdef OSTYPE_LINUX
		case OSTYPE_LINUX:		AddEntry("OS : Linux :-)");		break;
#endif
#ifdef OSTYPE_FREEBSD
		case OSTYPE_FREEBSD:	AddEntry("OS : FreeBSD");		break;
#endif
#ifdef OSTYPE_NETBSD
		case OSTYPE_NETBSD:		AddEntry("OS : NetBSD");		break;
#endif
#ifdef OSTYPE_UNIX
		case OSTYPE_UNIX:		AddEntry("OS : UNIX");			break;
#endif
	};

	#define CPU_MODEL_ATHLON64_N	15

	String str;
	switch(cpu_family)
	{
	case CPU_FAMILY_I386:
		str = "CPU: i386 ";
		break;
	case CPU_FAMILY_I486:
		str = "CPU: i486 ";
		break;
	case CPU_FAMILY_I586:
		str = "CPU: i586 ";
		break;
	case CPU_FAMILY_I686:
		str = "CPU: i686 ";
		break;
	case CPU_FAMILY_ITANIUM:
		str = "CPU: Itanium ";
		break;
	case CPU_FAMILY_POWERPC:
		str = "CPU: PPC ";
		break;
	case CPU_FAMILY_EXTENDED:
		switch(cpu_model)
		{
		case CPU_MODEL_PENTIUMIV:
			str = "CPU: Pentium4 ";
			break;
		case CPU_MODEL_XEON:
			str = "CPU: Xeon ";
			break;
		case CPU_MODEL_ATHLON64_N:
		case CPU_MODEL_ATHLON64:
			str = "CPU: Athlon64 ";
			break;
		case CPU_MODEL_OPTERON:
			str = "CPU: Opteron ";
			break;
		default:
			str = "CPU: ??? ";
		};
		break;
	case CPU_FAMILY_UNKNOWN:
		str = "CPU: ??? ";
		break;
	};
	str += cpu_vendor;
	str += " ";
	if(cpu_capabilities&CPU_AMD64)		str += "amd64 ";
	if(cpu_capabilities&CPU_IA64)		str += "i64 ";
	if(cpu_capabilities&CPU_ID)			str += "-cpuid";
	if(cpu_capabilities&CPU_FPU)		str += "-x87 FPU";
	if(cpu_capabilities&CPU_MMX)		str += "-MMX";
	if(cpu_capabilities&CPU_MMXPLUS)	str += "-MMX+";
	if(cpu_capabilities&CPU_SSE)		str += "-SSE";
	if(cpu_capabilities&CPU_SSE2)		str += "-SSE2";
	if(cpu_capabilities&CPU_SSE3)		str += "-SSE3";
	if(cpu_capabilities&CPU_3DNOW)		str += "-3DNow!";
	if(cpu_capabilities&CPU_ENH3DNOW)	str += "-Enhanced 3DNow!";
	if(cpu_capabilities&CPU_CMOV)		str += "-cmov";
	
	AddEntry( str );
	stdout_off();
}

void cConsole::StopRecording( void )
{
	if( !m_Recording )
		return;

    pMutex.lock();

	if( m_log != NULL && m_log_close)
		fclose( m_log );

	m_log = NULL;
	m_Recording = false;
	m_log_close = false;
    pMutex.unlock();
}

void cConsole::StartRecording( const char *file )
{
	StopRecording();

    pMutex.lock();
	m_RecordFilename = file;

	m_log = TA3D_OpenFile( m_RecordFilename, "wt" );

	if( m_log )
		m_Recording=true;
	else {
		pMutex.unlock();
        throw( "Console:StartRecording Failed to open file to record." );
		}
	m_log_close = true;
    pMutex.unlock();
}

void cConsole::StartRecording( const FILE *file )
{
	StopRecording();

    pMutex.lock();
	m_RecordFilename = "logger";

	m_log = (FILE*)file;

	if( m_log )
		m_Recording=true;
	else
    {
        pMutex.unlock();
        throw( "Console:StartRecording Failed to open file to record." );
	}
	m_log_close = false;
    pMutex.unlock();
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

	if( m_Recording ) 
	{
		fputs( NewEntry.c_str(), m_log );
		fputs( "\n", m_log );
		fflush( m_log ); 
	}
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

	if( m_Recording ) 
	{
		fputs( NewEntry.c_str(), m_log );
		fputs( "\n", m_log );
		fflush( m_log ); 
	}
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

	if( m_Recording ) 
	{
		fputs( NewEntry.c_str(), m_log );
		fputs( "\n", m_log );
		fflush( m_log );
	}

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

	if( m_Recording ) 
	{
		fputs( NewEntry.c_str(), m_log );
		fputs( "\n", m_log );
		fflush( m_log );
	}

	if( m_std_output )
    {
        LOG_INFO(NewEntry);
		//printf( "%s\n", NewEntry.c_str() );
    }
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

    std::list<String>::iterator i_entry;
	int i = 0;
	for( i_entry=m_LastEntries.begin();i_entry!=m_LastEntries.end();i_entry++ ) 
	{
		gfx->print(fnt,0.0f,maxh-fsize*(m_LastEntries.size()+1-i)-5.0f,0.0f,0xAFAFAFAF,
			format(">%s", (char *)((*i_entry).c_str())) );
		i++;
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

