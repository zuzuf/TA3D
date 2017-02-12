

YMESSAGE(":: [Module] Core")

# Paths for yuni-config, embedded version.
LIBYUNI_CONFIG_INCLUDE_PATH("intree" "core" "${CMAKE_CURRENT_SOURCE_DIR}/..")
LIBYUNI_CONFIG_LIB_PATH("intree" "core" "${LIBRARY_OUTPUT_PATH}")

# Paths for yuni-config, installed version.
LIBYUNI_CONFIG_INCLUDE_PATH("target" "core" "${CMAKE_INSTALL_PREFIX}/include/${YUNI_VERSIONED_INST_PATH}/")
LIBYUNI_CONFIG_LIB_PATH("target" "core" "${CMAKE_INSTALL_PREFIX}/lib/${YUNI_VERSIONED_INST_PATH}/")

LIBYUNI_CONFIG_LIB("both" "core"       "yuni-static-core")


if(APPLE)
	LIBYUNI_CONFIG_CFLAG("both" "core"	"-fvisibility=hidden")
endif(APPLE)

if (NOT WIN32 AND NOT WIN64)
	LIBYUNI_CONFIG_DEFINITION("both" "core" "_FILE_OFFSET_BITS=64")
endif()

#
# PThreads
#
DEVPACK_IMPORT_PTHREADS()

include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)



# stdio.h
check_include_file_cxx(stdio.h YUNI_HAS_STDIO_H)
# String.h
check_include_file_cxx(string.h YUNI_HAS_STRING_H)
# time.h
check_include_file_cxx(time.h YUNI_HAS_TIME_H)
# assert.h
check_include_file_cxx(assert.h YUNI_HAS_ASSERT_H)
# cassert
check_include_file_cxx(cassert YUNI_HAS_CASSERT)
# errno.h
check_include_file_cxx(errno.h YUNI_HAS_ERRNO_H)
# cstddef
check_include_file_cxx(cstddef YUNI_HAS_CSTDDEF)

# climits
check_include_file_cxx(climits YUNI_HAS_CLIMITS)
# vector
check_include_file_cxx(vector YUNI_HAS_VECTOR)
# list
check_include_file_cxx(list YUNI_HAS_LIST)
# map
check_include_file_cxx(map YUNI_HAS_MAP)
# algorithm
check_include_file_cxx(algorithm YUNI_HAS_ALGORITHM)
# iostream
check_include_file_cxx(iostream YUNI_HAS_IOSTREAM)
# cassert
check_include_file_cxx(cassert YUNI_HAS_CASSERT)
# dirent.h
check_include_file_cxx(dirent.h YUNI_HAS_DIRENT_H)
# stdlib.h
check_include_file_cxx(stdlib.h YUNI_HAS_STDLIB_H)
# unistd.h
check_include_file_cxx(unistd.h YUNI_HAS_UNISTD_H)
# fcntl.h
check_include_file_cxx(fcntl.h YUNI_HAS_FCNTL_H)
# cstdlib
check_include_file_cxx(cstdlib YUNI_HAS_CSTDLIB)
# stdarg
check_include_file_cxx(stdarg.h YUNI_HAS_STDARG_H)
if(NOT APPLE AND NOT WIN32 AND NOT WIN64)
	# sys/sendfile.h
	check_include_file_cxx("sys/sendfile.h" YUNI_HAS_SYS_SENDFILE_H)
endif()
# Macro va_copy
check_cxx_source_compiles("#include <stdarg.h>
	int main() {va_list a, b; va_copy(a, b);}" YUNI_HAS_VA_COPY)




if(NOT MSVC)
	# sys/types.h
	check_include_file_cxx("sys/types.h" YUNI_HAS_SYS_TYPES_H)
endif(NOT MSVC)
if(WIN32 OR WIN64)
	# cstddef
	check_include_file_cxx(winsock2.h YUNI_HAS_WINSOCK_2)
	# ShellApi.h
	check_include_file_cxx(shellapi.h YUNI_HAS_SHELLAPI_H)
endif(WIN32 OR WIN64)




# C++0x
check_cxx_compiler_flag("-std=c++0x" YUNI_HAS_GCC_CPP0X_SUPPORT)
if(YUNI_HAS_GCC_CPP0X_SUPPORT)
	LIBYUNI_CONFIG_CFLAG("both" "core"	"-std=c++0x")
endif()


# int64
check_cxx_source_compiles("#include <stdint.h>
	int main() {int64_t a; return 0;}" YUNI_HAS_INT64_T)
# int128
check_cxx_source_compiles("#include <stdint.h>
	int main() {__int128 a; unsigned __int128 b; return 0;}" YUNI_HAS_INT128_T)

# C++0x nullptr
if(YUNI_HAS_GCC_CPP0X_SUPPORT)
	set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
endif()
check_cxx_source_compiles("
    #include <cstddef>
	typedef std::nullptr_t NullPtr;
	class A {}; int main() {A* a = nullptr;return 0;}" YUNI_HAS_NULLPTR)

# GCC
if (NOT MSVC)
	check_cxx_source_compiles("
		int main() {   int i = 3;   int j = __sync_add_and_fetch(&i, 1);   return 0; } " YUNI_HAS_SYNC_ADD_AND_FETCH)
endif()


# long
if(MSVC)
	check_cxx_source_compiles(
		"#include <iostream>
		#include <stdio.h>
		void foo(unsigned int a) {std::cout << a;}
		void foo(int a) {std::cout << a;}
		void foo(unsigned long a) {std::cout << a;}
		void foo(long a) {std::cout << a;}
		void foo(__int64 a) {std::cout << a;}
		int main() {return 0;}"
		YUNI_HAS_LONG)
else(MSVC)
	if(YUNI_HAS_SYS_TYPES_H)
		check_cxx_source_compiles(
			"#include <iostream>
			#include <sys/types.h>
			#include <stdio.h>
			void foo(unsigned int a) {std::cout << a;}
			void foo(int a) {std::cout << a;}
			void foo(unsigned long a) {std::cout << a;}
			void foo(long a) {std::cout << a;}
			void foo(int64_t a) {std::cout << a;}
			int main() {return 0;}"
			YUNI_HAS_LONG)
	else(YUNI_HAS_SYS_TYPES_H)
		check_cxx_source_compiles(
			"#include <iostream>
			#include <stdio.h>
			void foo(unsigned int a) {std::cout << a;}
			void foo(int a) {std::cout << a;}
			void foo(unsigned long a) {std::cout << a;}
			void foo(long a) {std::cout << a;}
			void foo(int64_t a) {std::cout << a;}
			int main() {return 0;}"
			YUNI_HAS_LONG)
	endif(YUNI_HAS_SYS_TYPES_H)
endif(MSVC)



#
# Generating informations about the platform
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform.cmake ${CMAKE_CURRENT_SOURCE_DIR}/platform.h)



set(SRC_CORE_ATOMIC
		core/atomic/int.h
		core/atomic/int.hxx
		core/atomic/traits.h
		core/atomic/customstring.hxx
)
source_group(core\\atomic FILES ${SRC_CORE_ATOMIC})


set(SRC_CORE_PREPROCESSOR
		core/preprocessor/std.h
		core/preprocessor/capabilities.h
		core/preprocessor/unixes.h
		core/preprocessor/windows.h
		core/preprocessor/enum.h
)
source_group(core\\preprocessor FILES ${SRC_CORE_PREPROCESSOR})

set(SRC_CORE_ITERATOR
		core/iterator.h
		core/iterator/iterator.h
		core/iterator/iterator.hxx
)
source_group(core\\iterator FILES ${SRC_CORE_ITERATOR})


set(SRC_CORE_STATIC
		core/static/if.h
		core/static/remove.h
		core/static/assert.h
		core/static/types.h
		core/static/inherit.h
		core/static/dynamiccast.h
		core/static/moveconstructor.h
)
source_group(core\\static FILES ${SRC_CORE_STATIC})

set(SRC_CORE_TRAITS
		core/traits/cstring.h
		core/traits/extension/into-cstring.h
		core/traits/extension/length.h
		core/traits/length.h
)
source_group(core\\traits FILES ${SRC_CORE_TRAITS})


set(SRC_CORE_EXCEPTIONS
		core/exceptions.h
		core/exceptions/badcast.h
)
source_group(core\\exceptions FILES ${SRC_CORE_EXCEPTIONS})


set(SRC_CORE_VALIDATOR
		core/validator/text/default.h
		core/validator/text/default.hxx
		core/validator/text/default.private.h
		core/validator/text/validator.h
		core/validator/validator.h
)
source_group(core\\validator FILES ${SRC_CORE_VALIDATOR})

set(SRC_CORE_SLIST
		core/slist/iterator.h
		core/slist/slist.h
		core/slist/slist.hxx
		core/slist.h
)
source_group(core\\linkedlist FILES ${SRC_CORE_SLIST})



set(SRC_CORE_STRING
		core/customstring/customstring.h
		core/customstring/customstring.hxx
		core/customstring/operators.hxx
		core/customstring/iterator.h
		core/customstring/iterator.hxx
		core/customstring/utf8char.h
		core/customstring/utf8char.hxx
		core/customstring/traits/append.h
		core/customstring/traits/assign.h
		core/customstring/traits/fill.h
		core/customstring/traits/integer.h
		core/customstring/traits/vnsprintf.h
		core/customstring/traits/traits.h
		core/customstring/traits/traits.cpp
		core/customstring/traits/traits.hxx
		core/customstring/traits/into.h

		# Wide String
		core/customstring/wstring.h
		core/customstring/wstring.hxx

		# deprecated
		core/string/string.h
		core/string/string.hxx
		core/string/converters.from.hxx
		core/string/converters.to.hxx
		core/string/forward.h
		core/string/traits.hxx
		core/string/iterators.hxx
		core/string/traits/countchar.hxx
		core/string/traits/find.hxx
		core/string/traits/haschar.hxx
		core/string/traits/length.hxx
		core/string/traits/remove.hxx
		core/string/traits/vsprintf.hxx
		core/string/traits/findlastof.hxx
		core/string/traits/findfirstof.hxx
		core/string.h
)
source_group(core\\string FILES ${SRC_CORE_STRING})


set(SRC_CORE_STRING_DEPRECATED
		# deprecated
		core/string/string.h
		core/string/string.hxx
		core/string/converters.from.hxx
		core/string/converters.to.hxx
		core/string/forward.h
		core/string/traits.hxx
		core/string/iterators.hxx
		core/string/traits/countchar.hxx
		core/string/traits/find.hxx
		core/string/traits/haschar.hxx
		core/string/traits/length.hxx
		core/string/traits/remove.hxx
		core/string/traits/vsprintf.hxx
		core/string/traits/findlastof.hxx
		core/string/traits/findfirstof.hxx
		core/string.h
)
source_group(core\\string\\deprecated FILES ${SRC_CORE_STRING_DEPRECATED})

set(SRC_CORE_STL
		core/stl/algorithm.h
)
source_group(core\\Stl FILES ${SRC_CORE_STL})


set(SRC_CORE_LOGS
		core/logs/logs.h
		core/logs/logs.hxx
		core/logs/verbosity.h
		core/logs/null.h
		core/logs/buffer.h
		# Handlers
		core/logs/handler/stdcout.h
		core/logs/handler/file.h
		core/logs/handler/file.hxx
		core/logs/handler/callback.h
		# Decorators
		core/logs/decorators/verbositylevel.h
		core/logs/decorators/time.h
		core/logs/decorators/time.cpp
		core/logs/decorators/applicationname.h
		core/logs/decorators/message.h
		core/logs.h
)
source_group(core\\logs FILES ${SRC_CORE_LOGS})



set(SRC_CORE_SMARTPTR
		core/smartptr/policies/checking.h
		core/smartptr/policies/ownership.h
		core/smartptr/policies/storage.h
		core/smartptr/policies/constness.h
		core/smartptr/smartptr.h
		core/smartptr/smartptr.hxx
)
source_group(core\\smartptr FILES ${SRC_CORE_SMARTPTR})


set(SRC_CORE_ANY
		core/any.h
		core/any/any.h
		core/any/any.private.h
		core/any/any.hxx
		core/any/any.cpp
)
source_group(core\\any FILES ${SRC_CORE_ANY})

set(SRC_CORE_VARIANT
		core/variant.h
		core/variant/variant.h
		core/variant/variant.hxx
		core/variant/variant.cpp
		core/variant/variant.private.h
		core/variant/variant.private.hxx
)
source_group(core\\variant FILES ${SRC_CORE_VARIANT})



set(SRC_CORE_DYNAMICLIBRARY
		core/dynamiclibrary/file.h
		core/dynamiclibrary/file.hxx
		core/dynamiclibrary/file.cpp
		core/dynamiclibrary/symbol.h
		core/dynamiclibrary/symbol.hxx
		core/dynamiclibrary.h
)
source_group(core\\dynamiclibrary FILES ${SRC_CORE_DYNAMICLIBRARY})


set(SRC_CORE_BIND
		core/bind/bind.h
		core/bind/bind.hxx
		core/bind/traits.h
		core/bind.h
)
source_group(core\\bind FILES ${SRC_CORE_BIND})

set(SRC_CORE_CHARSET
		core/charset/charset.h
		core/charset/charset.hxx
		core/charset/charset.cpp
)
source_group(core\\charset FILES ${SRC_CORE_CHARSET})


set(SRC_CORE_EVENT
		core/event/event.h
		core/event/event.hxx
		core/event/interfaces.h
		core/event/interfaces.hxx
		core/event/traits.h
		core/event/observer/observer.h
		core/event/observer/observer.hxx
		core/event.h

		# Event loop
		core/event/loop.h
		core/event/loop.hxx
		core/event/loop.fwd.h
		core/event/flow
		core/event/flow/continuous.h
		core/event/flow/timer.h
		core/event/statistics
		core/event/statistics/none.h
)
source_group(core\\event FILES ${SRC_CORE_EVENT})


set(SRC_CORE_MATH
		# Standard functions
		core/math/math.h
		core/math.h
		core/math/math.hxx
		core/math/msvc.hxx
		core/math/base.h
		# Log(N)
		core/math/log.h
		core/math/log.hxx
		# Trigonometric functions
		core/math/trigonometric.h
		core/math/trigonometric.hxx
		# Exponential functions
		core/math/exponential.h
		core/math/exponential.hxx
		core/math/complex.h
		core/math/complex.hxx
		# Random number generators
		core/math/random/distribution.h
		core/math/random/distribution.hxx
		core/math/random/default.h
		core/math/random/default.hxx
		core/math/random/range.h
		core/math/random/range.hxx
		core/math/random/constant.h

		core/rect2D.h
		core/math/geometry/rect2D.h
		core/math/geometry/rect2D.hxx
		core/vector3D.h
		core/math/geometry/vector3D.h
		core/math/geometry/vector3D.hxx
		core/point2D.h
		core/math/geometry/point2D.h
		core/point3D.h
		core/math/geometry/point3D.h
		core/boundingbox.h
		core/math/geometry/boundingbox.h
		core/math/geometry/boundingbox.hxx
		core/math/geometry/edge.h
		core/math/geometry/triangle.h
		core/math/geometry/vertex.h
)
source_group(core\\math FILES ${SRC_CORE_MATH})

set(SRC_CORE_BIT
		core/bit.h
		core/bit/bit.h core/bit/bit.hxx
		core/bit/array.h
		core/bit/array.hxx
		core/bit/array.cpp
)
source_group("core\\bit" FILES ${SRC_CORE_BIT})


# Singleton
set(SRC_CORE_SINGLETON
		core/singleton.h
		core/singleton/singleton.h
		core/singleton/singleton.hxx
		core/singleton/policies/creation.h
		core/singleton/policies/lifetime.h
)
source_group(core\\singleton FILES ${SRC_CORE_SINGLETON})



# Hash - Checksums
set(SRC_CORE_HASH_CHECKSUM
		core/hash/checksum/checksum.h
		core/hash/checksum/checksum.hxx
		core/hash/checksum/md5.hxx
		core/hash/checksum/md5.h
		core/hash/checksum/md5.cpp
)
source_group(core\\hash\\checksum FILES ${SRC_CORE_HASH_CHECKSUM})


# System
set(SRC_CORE_SYSTEM
		core/system/stdint.h
		core/system/nullptr.h
		core/system/capabilities.h
		core/system/suspend.h
		core/system/suspend.cpp
		core/suspend.h
		core/fwd.h
		core/system/cpu.h
		core/system/cpu.cpp
		core/system/memory.h
		core/system/memory.hxx
		core/system/memory.cpp
		core/system/environment.h
		core/system/environment.hxx
		core/system/username.h
		core/system/username.hxx
		core/system/endian.h
)
if(WIN32 OR WIN64)
	set(SRC_CORE_SYSTEM ${SRC_CORE_SYSTEM} core/system/username.cpp)
endif()
source_group(core\\system FILES ${SRC_CORE_SYSTEM})


set(SRC_CORE_TREE_N
		core/tree/treeN.h
		core/tree/n/treeN.h
		core/tree/n/treeN.hxx
		core/tree/n/treeN.iterator.def.h
		core/tree/n/treeN.iterator.undef.h
		core/tree/n/treeN.iterator.h
)
source_group(core\\tree FILES ${SRC_CORE_TREE_N})


set(SRC_CORE_UNIT
		core/unit/define.h
		core/unit/undef.h
		core/unit/unit.h
		core/unit/unit.hxx
		core/unit.h
		core/unit/angle.h
		core/unit/length.h
		core/unit/length/length.h
		core/unit/length/metric.h
		core/unit/length/extra.h
		core/unit/luminance.h
)
source_group(core\\unit FILES ${SRC_CORE_UNIT})

set(SRC_CORE_UTILS
		core/utils/utils.h
		core/utils/hexdump.h core/utils/hexdump.hxx core/utils/hexdump.cpp
)
source_group(core\\utils FILES ${SRC_CORE_UTILS})

set(SRC_CORE_URI
		core/uri/uri.private.h  core/uri/uri.private.cpp
		core/uri/uri.h core/uri/uri.hxx core/uri/uri.cpp
		core/uri.h
)
source_group(core\\uri FILES ${SRC_CORE_URI})

set(SRC_CORE_PROCESS
		core/process.h
		core/process/fwd.h
		core/process/process.h
		core/process/process.hxx
		core/process/process.cpp
)
source_group(core\\process FILES ${SRC_CORE_PROCESS})


set(SRC_CORE_IO_FILENAME
		core/io/io.h
		core/io/io.cpp
		core/io/exists.cpp
		core/io/io.hxx
)
source_group(core\\io FILES ${SRC_CORE_IO_FILENAME})



set(SRC_CORE_IO_DIRECTORY
		core/io/directory.h
		core/io/directory/directory.hxx
		core/io/directory/commons.h
		core/io/directory/remove.cpp
		core/io/directory/current.cpp
		core/io/directory/make.cpp
		core/io/directory/copy.cpp
		core/io/directory/info.h
		core/io/directory/iterator.h
)
source_group(core\\io\\directory FILES ${SRC_CORE_IO_DIRECTORY})

set(SRC_CORE_IO_DIRECTORY_ITERATOR
		core/io/directory/iterator/iterator.cpp
		core/io/directory/iterator/iterator.h
		core/io/directory/iterator/iterator.hxx
		core/io/directory/iterator/detachedthread.h
)
source_group(core\\io\\directory\\iterator FILES ${SRC_CORE_IO_DIRECTORY_ITERATOR})


set(SRC_CORE_IO_DIRECTORY_INFO
		core/io/directory/info/iterator.h
		core/io/directory/info/info.h
		core/io/directory/info/info.hxx
		core/io/directory/info/info.cpp
		core/io/directory/info/platform.cpp
		core/io/directory/info/platform.h
)
source_group(core\\io\\directory\\Info FILES ${SRC_CORE_IO_DIRECTORY_INFO})



set(SRC_CORE_IO_FILE
		core/io/file/openmode.h
		core/io/file/openmode.cpp
		core/io/file/stream.h
		core/io/file/stream.hxx
		core/io/file/stream.cpp
		core/io/file.h
		core/io/file/file.hxx
		core/io/file/file.cpp
)
source_group(core\\io\\file FILES ${SRC_CORE_IO_FILE})


set(SRC_CORE_GETOPT
		core/getopt.h
		core/getopt/option.h
		core/getopt/option.cpp
		core/getopt/parser.h
		core/getopt/parser.hxx
		core/getopt/parser.cpp
)
source_group(core\\getopt FILES ${SRC_CORE_GETOPT})


set(SRC_CORE_VERSION
		core/version/version.h
		core/version/version.cpp
		core/version/version.hxx
)
source_group(core\\version FILES ${SRC_CORE_VERSION})




# System - Microsoft Windows
set(SRC_CORE_SYSTEM_WINDOWS core/system/windows.hdr.h  core/system/gettimeofday.h)
if(WIN32 AND MSVC)
	set(SRC_CORE_SYSTEM_WINDOWS ${SRC_CORE_SYSTEM_WINDOWS} core/system/gettimeofday.cpp)
endif(WIN32 AND MSVC)
source_group(core\\system\\windows FILES ${SRC_CORE_SYSTEM_WINDOWS})




# Threads
set(SRC_THREADS
		thread/policy.h
		thread/pthread.h
		thread/mutex.h
		thread/mutex.hxx
		thread/thread.h
		thread/thread.hxx
		thread/thread.cpp
		thread/timer.h
		thread/timer.hxx
		thread/timer.cpp
		thread/condition.h
		thread/condition.cpp
		thread/condition.hxx
		thread/array.h
		thread/array.hxx)
source_group(threads FILES ${SRC_THREADS})

# Jobs
set(SRC_JOBS
		job/enum.h
		job/forward.h
		job/job.h
		job/job.hxx
		job/job.cpp
		job/queue.h
		job/queue/queue.h
		job/queue/queue.hxx
		job/queue/thread.h
		job/queue/thread.hxx
		job/queue/waitingroom.h
		job/queue/waitingroom.hxx
		job/queue/waitingroom.cpp

# Scheduler
		job/scheduler/highestpriorityfirst.h
		job/scheduler/highestpriorityfirst.hxx
		)
source_group(jobs FILES ${SRC_JOBS})


# Application
set(SRC_APPLICATION
		application/application.h
		application/application.hxx application/application.cpp
		application/console.h application/console.cpp)
source_group(core\\application FILES ${SRC_APPLICATION})


set(SRC_CORE_COLOR
		# Color
		core/color/rgba.h
		core/color/rgba.hxx
		core/color/rgb.h
		core/color/rgb.hxx
		core/color/fwd.h
	)
source_group(core\\color FILES ${SRC_CORE_COLOR})


set(SRC_CORE_PREDICATES_RESULTS
		core/predicate/result/sum.h
		core/predicate/result/and.h
		core/predicate/result/or.h
		)
source_group(core\\predicate\\Result FILES ${SRC_CORE_PREDICATES_RESULTS})


set(SRC_CORE_SYSTEM_CONSOLE
		core/system/console.h
		core/system/console/console.h
		core/system/console/console.hxx
		)
source_group(core\\system\\Console FILES ${SRC_CORE_SYSTEM_CONSOLE})



set(SRC_CORE_MISC
		core/noncopyable.h
)
source_group(core\\any FILES ${SRC_CORE_ANY})



add_library(yuni-static-core STATIC
		yuni.h
		${SRC_CORE_MISC}
		${SRC_CORE_PREDICATES_RESULTS}
		${SRC_CORE_STATIC}
		${SRC_CORE_ITERATOR}
		${SRC_CORE_TRAITS}
		${SRC_CORE_PREPROCESSOR}
		${SRC_CORE_BIT}
		${SRC_CORE_EXCEPTIONS}
		${SRC_CORE_SYSTEM}
		${SRC_CORE_SYSTEM_WINDOWS}
		${SRC_CORE_SYSTEM_CONSOLE}
		${SRC_CORE_STRING}
		${SRC_CORE_STRING_DEPRECATED}
		${SRC_CORE_STL} ${SRC_CORE_VALIDATOR}
		${SRC_CORE_IO_FILENAME}
		${SRC_CORE_IO_DIRECTORY}
		${SRC_CORE_IO_DIRECTORY_INFO}
		${SRC_CORE_IO_DIRECTORY_ITERATOR}
		${SRC_CORE_IO_FILE}
		${SRC_CORE_PROCESS}
		${SRC_CORE_LOGS}
		${SRC_CORE_ATOMIC}
		${SRC_CORE_SMARTPTR}
		${SRC_CORE_ANY}
		${SRC_CORE_COLOR}
		${SRC_CORE_SLIST}
		${SRC_CORE_VARIANT}
		${SRC_CORE_DYNAMICLIBRARY}
		${SRC_CORE_BIND}
		#${SRC_CORE_CHARSET}
		${SRC_CORE_EVENT}
		${SRC_CORE_MATH}
		${SRC_CORE_SINGLETON}
		${SRC_CORE_HASH_CHECKSUM}
		${SRC_CORE_TREE_N}
		${SRC_CORE_VERSION}
		${SRC_CORE_UNIT}
		${SRC_CORE_UTILS}
		${SRC_CORE_URI}
		${SRC_CORE_GETOPT}
		${SRC_PREDICATES_RESULTS}
		${SRC_THREADS} ${SRC_JOBS}
		${SRC_APPLICATION}
	)

# Setting output path
set_target_properties(yuni-static-core PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/lib")

# Installation
install(TARGETS yuni-static-core ARCHIVE DESTINATION lib/${YUNI_VERSIONED_INST_PATH})

# Install Core-related headers
install(
	DIRECTORY application core job thread
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}/yuni
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hxx"
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeFiles" EXCLUDE
	PATTERN "cmake" EXCLUDE
)
# Install other core-related base files
install(FILES
	Doxygen.txt
	doxygen.h
	platform.h
	yuni.h
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}/yuni
)

install(FILES yuni.version DESTINATION include/${YUNI_VERSIONED_INST_PATH}/ RENAME "yuni.version")
install(FILES "${YUNI_LIBYUNI_CONFIG_TARGET_INIFILE}" DESTINATION include/${YUNI_VERSIONED_INST_PATH}/ RENAME "yuni.config.${YUNI_LIBYUNI_CONFIG_COMPILER}")

