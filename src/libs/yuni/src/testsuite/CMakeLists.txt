Project(Yuni-Testsuite)
CMake_minimum_required(VERSION 2.8)
include("../cmake/message.cmake")

YMESSAGE("")
YMESSAGE("Testsuite for the Yuni Framework")
YMESSAGE("")

IF(POLICY CMP0011)
	CMake_Policy(SET CMP0011 NEW)
ENDIF(POLICY CMP0011)

YMESSAGE("")
YMESSAGE("Looking for testcases...")
YMESSAGE("Path: ${PROJECT_SOURCE_DIR}")

Set(YuniTestCase_LastExe               "")
Set(YuniTestCase_LastID                "")
Set(YuniTestCase_Code                  "")
Set(YuniTestCase_ExeExtension          "")
Set(YuniTestCase_ModulesHaveBeenFound  "")
Set(YuniTestCase_CXXFlags              "")
Set(YuniTestCase_LinkFlags             "")

# TODO For Visual Studio ? MinGW ?
IF(MSVC)
	Set(YuniTestCaseMake "cl.exe")
ELSE(MSVC)
	IF(MINGW OR MSYS OR CYGWIN)
		Set(YuniTestCaseMake "mingw32-make")
	ELSE(MINGW OR MSYS OR CYGWIN)
		Set(YuniTestCaseMake "make")
	ENDIF(MINGW OR MSYS OR CYGWIN)
ENDIF(MSVC)
IF(WIN32)
	Set(YuniTestCase_ExeExtension ".exe")
ENDIF(WIN32)


add_custom_target(tests)
add_custom_target(test_html DEPENDS tests)


Set(YuniTestCaseOutput "${PROJECT_SOURCE_DIR}/results/htdocs/${YUNI_VERSION_REV}")
Set(YuniTestCaseRubyInclude "${PROJECT_SOURCE_DIR}/../tools/testsuite")


MACRO(YUNITESTCASE_BEGIN exe settings)
	String(REPLACE "_" "." ID "${exe}")
	Set(YuniTestCase_LastID "${ID}")
	Set(YuniTestCase_LastExe "${exe}")
	Set(YuniTestCase_RubySettings "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/testcase_${ID}.rb")
	Set(YuniTestCase_ModulesHaveBeenFound "")
	Set(YuniTestCase_Code "
	${settings}
	options.identifier = '${ID}'
	options.report = '${CMAKE_CURRENT_SOURCE_DIR}/testcase_${ID}.xml'
	options.path = '${CMAKE_CURRENT_SOURCE_DIR}'\n
	options.output = '${YuniTestCaseOutput}'
	"
	)
ENDMACRO(YUNITESTCASE_BEGIN)


MACRO(YUNITESTCASE_MODULES)
	Find_Package(Yuni COMPONENTS ${ARGV})
	IF(Yuni_FOUND)
		Set(YuniTestCase_ModulesHaveBeenFound "1")
		Set(YuniTestCase_CXXFlags "${Yuni_CXXFLAGS}")
		Set(YuniTestCase_LinkFlags "${Yuni_LIBS}")
	ENDIF(Yuni_FOUND)
ENDMACRO(YUNITESTCASE_MODULES)


MACRO(YUNITESTCASE_SOURCE)
	IF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
		Set(CMAKE_CXX_FLAGS "${YuniTestCase_CXXFlags}")
		ADD_EXECUTABLE(test_${YuniTestCase_LastExe} ${ARGV})
		Set(YuniTestCase_Code "${YuniTestCase_Code} options.program = '${CMAKE_CURRENT_SOURCE_DIR}/test_${YuniTestCase_LastExe}${YuniTestCase_ExeExtension}'\n")
		Set(YuniTestCase_Code "${YuniTestCase_Code} options.make = '${YuniTestCaseMake} test_${YuniTestCase_LastExe}'\n")
		target_link_libraries(test_${YuniTestCase_LastExe} yuni-static-tests "${YuniTestCase_LinkFlags}")
	ENDIF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
ENDMACRO(YUNITESTCASE_SOURCE)



MACRO(YUNITESTCASE_LINK)
	IF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
		target_link_libraries(test_${YuniTestCase_LastExe} ${ARGV})
	ENDIF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
ENDMACRO(YUNITESTCASE_LINK)


MACRO(YUNITESTCASE_END)
	IF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
		Set(YuniTestCase_RubySettings "${PROJECT_SOURCE_DIR}/CMakeFiles/testcase_${YuniTestCase_LastID}.rb")
		FILE(WRITE "${YuniTestCase_RubySettings}" "${YuniTestCase_Code}")

		ADD_CUSTOM_COMMAND(OUTPUT "testcase_${YuniTestCase_LastID}.xml"
			COMMAND ruby ARGS
				"-I${YuniTestCaseRubyInclude}"
				-W0 "${PROJECT_SOURCE_DIR}/../tools/testsuite/testcase.rb"
				"${PROJECT_SOURCE_DIR}/CMakeFiles/testcase_${YuniTestCase_LastID}.rb"
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			)
		ADD_CUSTOM_TARGET(testcase_${YuniTestCase_LastExe} SOURCES "testcase_${YuniTestCase_LastID}.xml")
		add_dependencies(tests testcase_${YuniTestCase_LastExe})
		set_property(GLOBAL APPEND PROPERTY YuniTestCaseListCount 1)
		set_property(GLOBAL APPEND PROPERTY YuniTestCaseList "${CMAKE_CURRENT_SOURCE_DIR}/testcase_${YuniTestCase_LastExe}${YuniTestCase_ExeExtension}.xml")
	ENDIF("${YuniTestCase_ModulesHaveBeenFound}" STREQUAL "1")
ENDMACRO(YUNITESTCASE_END)



Include("${PROJECT_SOURCE_DIR}/../cmake/CommonSettings.cmake")
Include("${PROJECT_SOURCE_DIR}/../cmake/Modules.cmake")

# To find 'FindYuni.cmake'
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../packages/cmake")



set_property(GLOBAL PROPERTY YuniTestCaseList)
set_property(GLOBAL PROPERTY YuniTestCaseListCount)
include_directories("${PROJECT_SOURCE_DIR}/..")





Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	#
	# Adding Testcases
	#
	Add_SubDirectory(automated)
	Add_SubDirectory(interactive)
ENDIF(Yuni_FOUND)





get_property(YnList GLOBAL PROPERTY YuniTestCaseListCount)
List(LENGTH YnList YnListCount)

YMESSAGE("Testcase count: ${YnListCount}")
YMESSAGE("")
get_property(YnList GLOBAL PROPERTY YuniTestCaseList)

