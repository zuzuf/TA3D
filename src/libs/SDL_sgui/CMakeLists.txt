cmake_minimum_required (VERSION 2.6)
project (SDL_sgui)

if(NOT SDL_FOUND)
	include(FindSDL)
	if(NOT SDL_FOUND)
		message(FATAL_ERROR "SDL not found, SDL is required")
	endif(NOT SDL_FOUND)
endif(NOT SDL_FOUND)
message(STATUS "[sdl] include: ${SDL_INCLUDE_DIR}")
message(STATUS "[sdl] libs   : ${SDL_LIBRARY}")
LINK_LIBRARIES(${SDL_LIBRARY})
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

SET(SOURCES
	include/SDL/SDL_sgui.h
	include/SDL/sgui/types.h
	src/arrowdata.h
	src/checkdata.h
	src/fontdata.h

	src/types.cpp
	src/button.cpp			include/SDL/sgui/button.h
	src/checkbox.cpp		include/SDL/sgui/checkbox.h
	src/floatting.cpp		include/SDL/sgui/floatting.h
	src/font.cpp			include/SDL/sgui/font.h
	src/frame.cpp			include/SDL/sgui/frame.h
	src/group.cpp			include/SDL/sgui/group.h
	src/hboxlayout.cpp		include/SDL/sgui/hboxlayout.h
	src/label.cpp			include/SDL/sgui/label.h
	src/layout.cpp			include/SDL/sgui/layout.h
	src/lineinput.cpp		include/SDL/sgui/lineinput.h
	src/listbox.cpp			include/SDL/sgui/listbox.h
	src/menubar.cpp			include/SDL/sgui/menubar.h
	src/menu.cpp			include/SDL/sgui/menu.h
	src/menuentry.cpp		include/SDL/sgui/menuentry.h
	src/picture.cpp			include/SDL/sgui/picture.h
	src/progressbar.cpp		include/SDL/sgui/progressbar.h
	src/radiobutton.cpp		include/SDL/sgui/radiobutton.h
	src/receiver.cpp		include/SDL/sgui/receiver.h
	src/renderapi.cpp		include/SDL/sgui/renderapi.h
	src/scrollarea.cpp		include/SDL/sgui/scrollarea.h
	src/scrollbar.cpp		include/SDL/sgui/scrollbar.h
	src/spacer.cpp			include/SDL/sgui/spacer.h
	src/spinbox.cpp			include/SDL/sgui/spinbox.h
	src/tabwidget.cpp		include/SDL/sgui/tabwidget.h
	src/textedit.cpp		include/SDL/sgui/textedit.h
	src/unicode.cpp			include/SDL/sgui/unicode.h
	src/unmanagedlayout.cpp	include/SDL/sgui/unmanagedlayout.h
	src/utils.cpp			include/SDL/sgui/utils.h
	src/vboxlayout.cpp		include/SDL/sgui/vboxlayout.h
	src/widget.cpp			include/SDL/sgui/widget.h
	src/window.cpp			include/SDL/sgui/window.h)

SET(DEMO_SOURCES
	samples/demo.cpp)

SET(CMAKE_CXX_FLAGS_RELEASE "-Os -ffast-math")

include_directories(include)
add_library(SDL_sgui ${SOURCES})

IF(BUILD_DEMO)
	include(FindSDL_image)
	if(NOT SDLIMAGE_FOUND)
		message(FATAL_ERROR "SDL_image not found, SDL_image is required to build the demo program")
	endif(NOT SDLIMAGE_FOUND)
	LINK_LIBRARIES(${SDLIMAGE_LIBRARY} SDL_sgui)
	INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
	add_executable(demo ${DEMO_SOURCES})
ENDIF(BUILD_DEMO)
