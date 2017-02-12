#ifndef __YUNI_CORE_SYSTEM_MAIN_H__
# define __YUNI_CORE_SYSTEM_MAIN_H__

# include "../../yuni.h"


# ifdef YUNI_OS_WINDOWS
#   include "windows.hdr.h"

#	define YUNI_MAIN() \
	int main(int argc, char** argv)

#   define YUNI_MAIN_CONSOLE()  \
    /* Forward declaration */ \
    int main(int argc, char** argv); \
    \
    int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, char* cmdLine, int) \
    { \
        int argc; \
        char** argv; \
        \
        char* arg; \
        int index; \
        int result; \
        \
        /* Count the arguments */ \
        argc = 1; \
        arg  = cmdLine; \
        while (arg[0] != 0) \
        { \
            while (arg[0] != 0 && arg[0] == ' ') \
                ++arg; \
            \
            if (arg[0] != 0) \
            { \
                ++argc; \
                while (arg[0] != 0 && arg[0] != ' ') \
                    ++arg; \
            } \
        } \
        \
        /* Tokenize the arguments */ \
        argv = (char**) malloc(argc * sizeof(char*)); \
        arg = cmdLine; \
        index = 1; \
        while (arg[0] != 0) \
        { \
            while (arg[0] != 0 && arg[0] == ' ') \
                ++arg; \
            \
            if (arg[0] != 0) \
            { \
                argv[index] = arg; \
                ++index; \
                while (arg[0] != 0 && arg[0] != ' ') \
                    ++arg; \
                \
                if (arg[0] != 0) \
                { \
                    arg[0] = 0; \
                    ++arg; \
                } \
            } \
        } \
        \
        /* Put the program name into argv[0] */ \
        char filename[_MAX_PATH]; \
        \
        GetModuleFileName(NULL, filename, _MAX_PATH); \
        argv[0] = filename; \
        \
        /* Call the user specified main function */ \
        result = main(argc, argv); \
        \
        free(argv); \
        return result; \
    } \
    \
    int main(int argc, char* argv[])

#else

#   define YUNI_MAIN_CONSOLE() \
        int main(int argc, char* argv[])

#   define YUNI_MAIN() \
        int main(int argc, char* argv[])

#endif


#endif /* __YUNI_CORE_SYSTEM_MAIN_H__ */
