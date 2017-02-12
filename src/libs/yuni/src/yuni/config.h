#ifndef __YUNI_CONFIG_H__
# define __YUNI_CONFIG_H__

/* Generate from config.h.cmake */


/*!
** \mainpage The Yuni Framework
**
** \section what What is Yuni ?
**
** The Yuni project is a high-level cross-platform framework. This framework
** intends to provide the most complete set as possible of coherent API related
** to 3D programming, especially game programming. It intends to be a reliable and
** simple bridge between different worlds to create all sorts of applications,
** letting the user focus on the real work.
**
** The Yuni project is connected with all those domains :
** - Simulation
** - Game development
** - Artificial Intelligence, mainly on 3D objects
** - Real-time and multi-user Collaboration
** - 3D graphics
** - Physics
** - Threading / Parallel computing
** - 2D/3D User Interface in 3D context
** - 2D/3D Input devices
** - Networking
** - Sound, playback of 2D/3D sounds
** - Scripting languages
** - And Any other domain related to multimedia or user interaction.
*/





/*! \name Informations about the Yuni Library */
/*@{*/
/*! The yuni website */
# define YUNI_URL_WEBSITE              "http://www.libyuni.org"

/*! The Devpacks repository */
# define YUNI_URL_DEVPACK_REPOSITORY   "http://devpacks.libyuni.org/"
/*! The Devpacks sources */
# define YUNI_URL_DEVPACK_SOURCE       "http://devpacks.libyuni.org/downloads"

/*! The hi part of the version of the yuni library */
# define YUNI_VERSION_HI               0
/*! The lo part of the version of the yuni library */
# define YUNI_VERSION_LO               1
/*! The revision number */
# define YUNI_VERSION_REV              1

/*! The complete version of Yuni */
# define YUNI_VERSION_STRING           "0.1.1-release"
# define YUNI_VERSION_LITE_STRING      "0.1.1"

/*! The flags used to compile Yuni */
# define YUNI_COMPILED_WITH_CXX_FLAGS  "-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Woverloaded-virtual -Wall -Wextra -Wunused-parameter -Wconversion -Wmissing-noreturn -Wcast-align  -Wfloat-equal -Wundef -std=c++0x -O3 -fomit-frame-pointer -fstrict-aliasing -momit-leaf-frame-pointer -fno-tree-pre -falign-loops -mfpmath=sse -msse -msse2 -Wuninitialized "
/*! The target used to compile Yuni (debug/release) */
# define YUNI_COMPILED_WITH_TARGET     "release"

/*! List of all modules */
# define YUNI_MODULE_LIST              "algorithms;vm;vfs;vfs-local;audio;devices;display;keyboard;mouse;script;lua;ui;net;netserver;netclient;ldo;markdown;docs"
/*! List of all available modules */
# define YUNI_COMPILED_WITH_MODULES    "release"

/*! Is OpenGL available ? */
# define YUNI_HAS_SUPPORT_FOR_OPENGL   0
/*! Is DirectX available ? */
# define YUNI_HAS_SUPPORT_FOR_DIRECTX  0
/*@}*/



#endif /* __YUNI_CONFIG_H__ */
