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




// Signals should be disabled under OS X, since the system already produces a crash report
// More information are available here :
// http://developer.apple.com/technotes/tn2004/tn2123.html
#ifndef TA3D_PLATFORM_DARWIN


#include "stdafx.h"
#include "TA3D_NameSpace.h"

#ifdef TA3D_PLATFORM_LINUX
#   define TA3D_BACKTRACE_SUPPORT
#endif

#ifdef TA3D_BACKTRACE_SUPPORT
#   include <execinfo.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>




/*!
 * \brief Obtain a backtrace and print it to stdout.
 *
 * Only if TA3D_BACKTRACE_SUPPORT is defined, a backtrace is obtained
 * then writen into a log file. It will be displayed in stdout in any case.
 * After this call, the program will exit with a exit status code equals
 * to `-1`.
 *
 * \param signum Which signal was received
 */
void backtrace_handler (int signum)
{
    # ifdef TA3D_BACKTRACE_SUPPORT

    // Retrieving a backtrace
	void *array[400];
	size_t size = backtrace (array, 400);
	char** strings = backtrace_symbols(array, size);

    // Try to log it
	std::ofstream m_File;
	m_File.open((TA3D_OUTPUT_DIR + "backtrace.txt").c_str(), std::ios::out | std::ios::trunc);
	if(m_File.is_open())
    {
		m_File << "received signal " << strsignal( signum ) << "\n";
		m_File << "Obtained " << size << " stack frames.\n";
		for (int i = 0; i < size; ++i)
			m_File << strings[i] << "\n";
		m_File.flush();
		m_File.close();

		printf("received signal %s\n", strsignal( signum ));
		printf ("Obtained %zd stack frames.\n", size);
		for (int i = 0; i < size; ++i)
			printf ("%s\n", strings[i]);

		String szErrReport = "An error has occured.\nDebugging information have been logged to:\n"
            + TA3D_OUTPUT_DIR
            + "backtrace.txt\nPlease report to our forums (http://ta3d.darkstars.co.uk/)\nand keep this file, it'll help us debugging.\n";

        # ifdef TA3D_PLATFORM_WINDOWS
		    ::MessageBoxA( NULL, szErrReport.c_str(), "TA3D Application Error", MB_OK  | MB_TOPMOST | MB_ICONERROR );
        # else
		    allegro_init();
		    allegro_message( szErrReport.c_str() );
		    allegro_exit();
        # endif // TA3D_PLATFORM_WINDOWS
	}
	else
    {
        // The file is not opened
        // The backtrace will be directly to stdout instead.
		printf("received signal %s\n", strsignal(signum));
		printf("couldn't open file for writing!!\n");
		printf ("Obtained %zd stack frames.\n", size);
		for (int i = 0; i < size; ++i)
			printf ("%s\n", strings[i]);
	}
	free(strings);

    #else // ifdef TA3D_BACKTRACE_SUPPORT

        // The backtrace support is disabled: warns the user
	    String szErrReport = "An error has occured.\nDebugging information could not be logged because this isn't\nnot supported for your system.\nPlease report to \
our forums (http://ta3d.darkstars.co.uk/) so we can fix it.";
        # ifdef TA3D_PLATFORM_WINDOWS
	        ::MessageBoxA( NULL, szErrReport.c_str(), "TA3D Application Error", MB_OK  | MB_TOPMOST | MB_ICONERROR );
        # else
	        allegro_init();
	        allegro_message( szErrReport.c_str() );
	        allegro_exit();
        # endif // ifdef TA3D_PLATFORM_LINUX
    
    #endif // ifdef TA3D_BACKTRACE_SUPPORT
	exit(-1);
}

#endif // ifdef TA3D_PLATFORM_DARWIN





int init_signals (void)
{
    // Signals should be disabled under OS X, since the system already produces a crash report
    // More information are available here :
    // http://developer.apple.com/technotes/tn2004/tn2123.html
    #ifndef TA3D_PLATFORM_DARWIN

    # ifdef TA3D_PLATFORM_WINDOWS
	int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGABRT };
	int nb_signals = 4;
    # else
	int signum[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGIOT, SIGTRAP, SIGSYS };
	int nb_signals = 8;
    # endif // ifdef TA3D_PLATFORM_WINDOWS
	for( int i = 0 ; i < nb_signals ; i++ )
		if (signal (signum[i], backtrace_handler) == SIG_IGN)
			signal (signum[i], SIG_IGN);

    #endif // ifdef TA3D_PLATFORM_DARWIN
}


