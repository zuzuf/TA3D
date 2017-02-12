


find_package(Yuni COMPONENTS core markdown)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")


	message(STATUS "Sample: Extra / Markdown / A Simple markdown to HTML generator (not functional yet)")
	add_executable(markdown_00_helloworld main.cpp)
endif(Yuni_FOUND)

