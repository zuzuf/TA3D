


#
# LUA
#
macro(DEVPACK_IMPORT_LUA)
	if(YUNI_EXTERNAL_SCRIPT_LUA)
		if(APPLE)
			DEVPACK_IMPORT("lua" "5.1.4" "2" "macos" "ub" "${DEVPACK_COMPILER}" "all")
		endif(APPLE)
		if(MSVC)
			DEVPACK_SMART_IMPORT("lua" "5.1.4" "1" "all")
		endif(MSVC)
		if(MINGW)
			DEVPACK_SMART_IMPORT("lua" "5.1.4" "1" "all")
		endif(MINGW)

		if(IsLinux)
			DEVPACK_SMART_IMPORT("lua" "5.1.4" "1" "all")
		endif(IsLinux)
		list(APPEND YUNI_STATIC_SCRIPT "${YUNI_EXT_LUA_LIB}")
		list(APPEND YUNI_INCLUDE   "${YUNI_EXT_LUA_INCLUDE}")
	endif()
endmacro()



#
# FFMpeg
#
macro(DEVPACK_IMPORT_FFMPEG)
	if(WIN32 OR WIN64)
		DEVPACK_SMART_IMPORT("ffmpeg" "0.6" "1" "all")
	else()
		DEVPACK_IMPORT("ffmpeg" "22725" "3" "all" "all" "${DEVPACK_COMPILER}" "all")
	endif()
	list(APPEND YUNI_STATIC_AUDIO "${YUNI_EXT_FFMPEG_LIB}")
	list(APPEND YUNI_INCLUDE   "${YUNI_EXT_FFMPEG_INCLUDE}")
endmacro()



#
# OpenAL
#
macro(DEVPACK_IMPORT_OPENAL)
	set(OPENAL_INCLUDE_DIR)
	set(OPENAL_LIBRARY)
	if(WIN32 OR WIN64)
		DEVPACK_IMPORT("openal" "1.10.622" "1" "windows" "i386" "all" "all")
		set(OPENAL_INCLUDE_DIR "${YUNI_EXT_OPENAL_INCLUDE}")
		set(OPENAL_LIBRARY "${YUNI_EXT_OPENAL_LIB}")
	else()
		if(NOT APPLE)
			find_package(OpenAL)
		endif()
	endif()
	list(APPEND YUNI_STATIC_AUDIO "${OPENAL_LIBRARY}")
	list(APPEND YUNI_INCLUDE   "${OPENAL_INCLUDE_DIR}")
endmacro()



#
# PThreads
#
macro(DEVPACK_IMPORT_PTHREADS)
	include(FindThreads)
	if(CMAKE_USE_WIN32_THREADS_INIT)
		YMESSAGE("Threading Support: PThreads for Windows (via DevPacks)")
		if(WIN32)
			DEVPACK_SMART_IMPORT("pthreads" "2.8.0" "3" "all")
			list(APPEND YUNI_STATIC_CORE "${YUNI_EXT_PTHREADS_LIB}")
			list(APPEND YUNI_INCLUDE   "${YUNI_EXT_PTHREADS_INCLUDE}")
			LIBYUNI_CONFIG_INCLUDE_PATH("both" "core" "${YUNI_EXT_PTHREADS_INCLUDE}")
			LIBYUNI_CONFIG_LIB_RAW_COMMAND("both" "core" "${YUNI_EXT_PTHREADS_LIB}")
		endif()
	else()
		if(NOT CMAKE_USE_PTHREADS_INIT)
			YFATAL("PThreads is required.")
		endif()
		link_libraries(${CMAKE_THREAD_LIBS_INIT})
		LIBYUNI_CONFIG_LIB_RAW_COMMAND("both" "core" "${CMAKE_THREAD_LIBS_INIT}")
		YMESSAGE("Threading Support: PThreads (${CMAKE_THREAD_LIBS_INIT})")
	endif()
endmacro()



#
# Cairo - Pango
#
macro(DEVPACK_IMPORT_CAIROPANGO)
	if(WIN32 OR WIN64)
		DEVPACK_IMPORT("cairopango" "1.8.10+1.28.0+2.24.0" "1" "windows" "i386" "all" "all")
		string(REPLACE " " ";" includes "${YUNI_EXT_CAIROPANGO_INCLUDE}")
		foreach (it ${includes})
			include_directories("${it}")
		endforeach()
		set(CAIRO_FOUND TRUE)
		set(PANGO_FOUND TRUE)
	else()
		# Cairo
		execute_process(COMMAND pkg-config cairo --cflags RESULT_VARIABLE config_error OUTPUT_VARIABLE cairo_cflags OUTPUT_STRIP_TRAILING_WHITESPACE)
		if (NOT config_error)
			set(CAIRO_FOUND TRUE)
			string(REPLACE " " ";" cairo_cflags "${cairo_cflags}")
			foreach(it ${cairo_cflags})
				if(it)
					string(REPLACE "-I" "" stripped "${it}")
					include_directories(${stripped})
				endif()
			endforeach()
#			list(APPEND YUNI_INCLUDE  "${cairo_cflags}")
			execute_process(COMMAND pkg-config cairo --libs RESULT_VARIABLE config_error OUTPUT_VARIABLE cairo_libs OUTPUT_STRIP_TRAILING_WHITESPACE)
			set(YUNI_EXT_CAIROPANGO_LIB ${cairo_libs})

			# Pango
			execute_process(COMMAND pkg-config pango --cflags-only-I RESULT_VARIABLE config_error OUTPUT_VARIABLE pango_cflags OUTPUT_STRIP_TRAILING_WHITESPACE)
			if (NOT config_error)
				set(PANGO_FOUND TRUE)
				string(REPLACE " " ";" pango_cflags "${pango_cflags}")
				foreach (it ${pango_cflags})
					if(it)
						string(REPLACE "-I" "" stripped ${it})
						include_directories(${stripped})
					endif()
				endforeach()
				execute_process(COMMAND pkg-config pango --libs RESULT_VARIABLE config_error OUTPUT_VARIABLE pango_libs OUTPUT_STRIP_TRAILING_WHITESPACE)
				list(APPEND YUNI_EXT_CAIROPANGO_LIB ${pango_libs})
			endif ()
		endif ()
#		DEVPACK_IMPORT("cairopango" "1.8.10+1.28.0+2.24.0" "1" "all" "all" "${DEVPACK_COMPILER}" "all")
		list (APPEND YUNI_STATIC_GFX3D ${YUNI_EXT_CAIROPANGO_LIB})
	endif()
endmacro()


