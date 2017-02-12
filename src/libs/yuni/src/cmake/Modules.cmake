
#
# --- Modules - Default settings ---
#

# Core
set(YUNI_MODULE_CORE                      true) # Must be True

# VFS
#set(YUNI_MODULE_VFS                       false)
#	set(YUNI_MODULE_VFS_FILE              true)

# VM
set(YUNI_MODULE_VM                        false)

# Devices
set(YUNI_MODULE_DEVICES                   false)
	set(YUNI_MODULE_DEVICE_DISPLAY        true)
	set(YUNI_MODULE_DEVICE_KEYBOARD       true)
	set(YUNI_MODULE_DEVICE_MOUSE          true)

# Audio
set(YUNI_MODULE_AUDIO                     false)

# Scripts
set(YUNI_MODULE_SCRIPT                    false)
	set(YUNI_EXTERNAL_SCRIPT_LUA          true)

# Network
set(YUNI_MODULE_NET                       false)
set(YUNI_MODULE_NET_MESSAGES              false)

# LDO
set(YUNI_MODULE_LDO                       false)

# UI (User Interface)
set(YUNI_MODULE_UI                        false)

# Algorithms
set(YUNI_MODULE_ALGORITHMS                false)

# Markdown
set(YUNI_MODULE_EXTRA_MARKDOWN            false)

# Doc
set(YUNI_MODULE_DOCS                      false)



# Tests
set(YUNI_TESTS   false)
# Samples
set(YUNI_SAMPLES false)



# The list of all available modules
# There is no need for `core`, which are implicit
set(YUNI_MODULE_LIST
	algorithms
	vm
	vfs
		vfs-local
	audio
	devices
		display
		keyboard
		mouse
	script
		lua
	ui
	net
		netserver
		netclient
	ldo
	# extra
		markdown
	docs
	)




#
# --- Command lines options ---
#
if(MODULES)
	set(KeywordError false)
	string(REPLACE "," ";" MODULES "${MODULES}")
	string(REPLACE " " ";" MODULES "${MODULES}")
	string(REPLACE "+" "" MODULES "${MODULES}")

	foreach(it ${MODULES})
		set(KeywordIsKnown false)

		# core
		if("${it}" STREQUAL "core")
			set(YUNI_MODULE_CORE true)
			set(KeywordIsKnown true)
		endif()
		# -core
		if("${it}" STREQUAL "-core")
			set(KeywordIsKnown true)
			YMESSAGE("[!!] Module: Impossible to disable the core module")
			set(KeywordError true)
		endif()

		# all
		if("${it}" STREQUAL "all")
			set(YUNI_MODULE_CORE true)
			#set(YUNI_MODULE_VFS true)
			set(YUNI_MODULE_DEVICES true)
			set(YUNI_MODULE_VM true)
			set(YUNI_MODULE_AUDIO true)
			set(YUNI_MODULE_NET true)
			set(YUNI_MODULE_NET_MESSAGES true)
			set(YUNI_MODULE_SCRIPT true)
			set(YUNI_MODULE_UI true)
			set(YUNI_MODULE_DATABASE true)
			set(YUNI_MODULE_ALGORITHMS true)
			set(YUNI_MODULE_EXTRA_MARKDOWN true)
			set(YUNI_MODULE_LDO true)
			set(YUNI_SAMPLES true)
			set(YUNI_TESTS true)
			set(KeywordIsKnown true)
		endif()

		# vfs
		#if("${it}" STREQUAL "vfs")
		#	set(YUNI_MODULE_VFS true)
		#	set(KeywordIsKnown true)
		#endif()
		# -vfs
		#if("${it}" STREQUAL "-vfs")
		#	set(YUNI_MODULE_VFS false)
		#	set(KeywordIsKnown true)
		#endif()

		# vfs-local
		#if("${it}" STREQUAL "vfs-local")
		#	set(YUNI_MODULE_VFS true)
		#	set(YUNI_MODULE_VFS_LOCAL true)
		#	set(KeywordIsKnown true)
		#endif()
		# -vfs
		#if("${it}" STREQUAL "-vfs-local")
		#	set(YUNI_MODULE_VFS_LOCAL false)
		#	set(KeywordIsKnown true)
		#endif()

		# vm
		if("${it}" STREQUAL "vm")
			set(YUNI_MODULE_VM true)
			set(KeywordIsKnown true)
		endif()
		# -vm
		if("${it}" STREQUAL "-vm")
			set(YUNI_MODULE_VM false)
			set(KeywordIsKnown true)
		endif()

		# ldo
		if("${it}" STREQUAL "ldo")
			set(YUNI_MODULE_LDO true)
			set(KeywordIsKnown true)
		endif()
		# -ldo
		if("${it}" STREQUAL "-ldo")
			set(YUNI_MODULE_LDO false)
			set(KeywordIsKnown true)
		endif()

		# algorithms
		if("${it}" STREQUAL "algorithms")
			set(YUNI_MODULE_ALGORITHMS true)
			set(KeywordIsKnown true)
		endif()
		# -algorithms
		if("${it}" STREQUAL "-algorithms")
			set(YUNI_MODULE_ALGORITHMS false)
			set(KeywordIsKnown true)
		endif()


		# display
		if("${it}" STREQUAL "display")
			set(YUNI_MODULE_DEVICE_DISPLAY true)
			set(KeywordIsKnown true)
		endif()
		# -display
		if("${it}" STREQUAL "-display")
			set(YUNI_MODULE_DEVICE_DISPLAY false)
			set(KeywordIsKnown true)
		endif()

		# keyboard
		if("${it}" STREQUAL "keyboard")
			set(YUNI_MODULE_DEVICE_KEYBOARD true)
			set(KeywordIsKnown true)
		endif()
		# -keyboard
		if("${it}" STREQUAL "-keyboard")
			set(YUNI_MODULE_DEVICE_KEYBOARD false)
			set(KeywordIsKnown true)
		endif()

		# mouse
		if("${it}" STREQUAL "mouse")
			set(YUNI_MODULE_DEVICE_MOUSE true)
			set(KeywordIsKnown true)
		endif()
		# -mouse
		if("${it}" STREQUAL "-mouse")
			set(YUNI_MODULE_DEVICE_MOUSE false)
			set(KeywordIsKnown true)
		endif()

		# devices
		if("${it}" STREQUAL "devices")
			set(YUNI_MODULE_DEVICES true)
			set(KeywordIsKnown true)
		endif()
		# -devices
		if("${it}" STREQUAL "-devices")
			set(YUNI_MODULE_DEVICES false)
			set(KeywordIsKnown true)
		endif()


		# net
		if("${it}" STREQUAL "net")
			set(YUNI_MODULE_NET true)
			set(KeywordIsKnown true)
		endif()
		# -net
		if("${it}" STREQUAL "-net")
			set(YUNI_MODULE_NET false)
			set(KeywordIsKnown true)
		endif()

		# net-messages
		if("${it}" STREQUAL "net-messages")
			set(YUNI_MODULE_NET_MESSAGES true)
			set(KeywordIsKnown true)
		endif()
		# -net-messages
		if("${it}" STREQUAL "-net-messages")
			set(YUNI_MODULE_NET_MESSAGES false)
			set(KeywordIsKnown true)
		endif()

		# audio
		if("${it}" STREQUAL "audio")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_AUDIO true)
		endif()
		# -audio
		if("${it}" STREQUAL "-audio")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_AUDIO false)
		endif()


		# script
		if("${it}" STREQUAL "script")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_SCRIPT true)
		endif()
		# -lua
		if("${it}" STREQUAL "-script")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_SCRIPT false)
		endif()

		# lua
		if("${it}" STREQUAL "lua")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_SCRIPT true)
			set(YUNI_EXTERNAL_SCRIPT_LUA true)
		endif()
		# -lua
		if("${it}" STREQUAL "-lua")
			set(KeywordIsKnown true)
			set(YUNI_EXTERNAL_SCRIPT_LUA false)
		endif()


		# Tests
		if("${it}" STREQUAL "tests")
			set(KeywordIsKnown true)
			set(YUNI_TESTS true)
		endif()
		# -tests
		if("${it}" STREQUAL "-tests")
			set(KeywordIsKnown true)
			set(YUNI_TESTS false)
		endif()

		# ui (User Interface)
		if("${it}" STREQUAL "ui")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_UI true)
		endif()
		# -ui
		if("${it}" STREQUAL "-ui")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_UI false)
		endif()

		# markdown
		if("${it}" STREQUAL "markdown")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_EXTRA_MARKDOWN true)
		endif()
		# -markdown
		if("${it}" STREQUAL "-markdown")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_EXTRA_MARKDOWN false)
		endif()

		# docs
		if("${it}" STREQUAL "docs" OR "${it}" STREQUAL "doc")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_DOCS true)
		endif()
		# -docs
		if("${it}" STREQUAL "-markdown")
			set(KeywordIsKnown true)
			set(YUNI_MODULE_EXTRA_MARKDOWN false)
		endif()

		if(NOT KeywordIsKnown)
			YMESSAGE("[!!] Unknown module from command line: `${it}` (ignored)")
			set(KeywordError true)
		endif()

	endforeach()

	if(KeywordError)
		YMESSAGE("")
		YMESSAGE("Errors on modules. Here is the list of all available modules :")
		YMESSAGE("(+ : Enable the module,  - disable the module)")
		YMESSAGE(" Main and virtual modules")
		YMESSAGE("    +core          : The core module (needed)")
		YMESSAGE("    -/+tests       : Atomic Tests for the yuni framework")
		YMESSAGE("    +all           : Enable all main modules (ui,script,tests,...)")
		#YMESSAGE(" The VFS module")
		#YMESSAGE("    -/+vfs         : The Virtual filesystem")
		#YMESSAGE("    -/+vfs-local   : Support for the local filesystems")
		YMESSAGE(" The device modules")
		YMESSAGE("    -/+devices     : All devices (display,keyboard,mouse...)")
		YMESSAGE("    -/+display     : The Display device")
		YMESSAGE("    -/+keyboard    : The Keyboard device")
		YMESSAGE("    -/+mouse       : The Mouse device")
		YMESSAGE(" The audio modules")
		YMESSAGE("    -/+audio       : The Audio module (default: disabled)")
		YMESSAGE(" The scripting modules")
		YMESSAGE("    -/+script      : The script module (default: disabled)")
		YMESSAGE("    -/+lua         : The Lua extension (default: enabled)")
		YMESSAGE(" The ui modules")
		YMESSAGE("    -/+ui          : The ui module (default: disabled)")
		YMESSAGE(" The virtual machine module")
		YMESSAGE("    -/+vm          : The Virtual machine")
		YMESSAGE(" The extra modules")
		YMESSAGE("    -/+markdown    : Markdown (default: disabled)")
		YMESSAGE(" The algorithms")
		YMESSAGE("    -/+algorithms  : Standard algorithms")
		YMESSAGE("")
		message(FATAL_ERROR "Errors on module names")
	endif()
endif()


#
# Dependancies
#
if(YUNI_MODULE_UI)
	set(YUNI_MODULE_DEVICES true)
	set(YUNI_MODULE_DEVICE_DISPLAY true)
endif()
if(YUNI_MODULE_DOCS)
	set(YUNI_MODULE_EXTRA_MARKDOWN true)
endif()
if(YUNI_MODULE_LDO)
	set(YUNI_MODULE_NET_MESSAGES true)
endif()
if(YUNI_MODULE_NET_MESSAGES)
	set(YUNI_MODULE_NET true)
endif()




if(YUNI_MODULE_SCRIPT)
	if(NOT YUNI_EXTERNAL_SCRIPT_LUA)
		YMESSAGE("[!!] Warning: No external extension for the `script` module. The module has been disabled.")
		set(YUNI_MODULE_SCRIPT false)
	endif()
endif()

if(YUNI_MODULE_DATABASE)
	if(NOT YUNI_MODULE_DB_PSQL)
		YMESSAGE("[!!] Warning: No external extension for the `database` module. The module has been disabled.")
		set(YUNI_MODULE_DATABASE false)
	endif()
endif()




#
# List of all available modules
#

set(YUNI_MODULE_AVAILABLE)

if(YUNI_MODULE_ALGORITHMS)
	list(APPEND YUNI_MODULE_AVAILABLE algorithms)
endif()

if(YUNI_MODULE_DEVICES)
	list(APPEND YUNI_MODULE_AVAILABLE devices)
	if(YUNI_MODULE_DEVICE_DISPLAY)
		list(APPEND YUNI_MODULE_AVAILABLE display)
	endif()
	if(YUNI_MODULE_DEVICE_MOUSE)
		list(APPEND YUNI_MODULE_AVAILABLE mouse)
	endif()
	if(YUNI_MODULE_DEVICE_KEYBOARD)
		list(APPEND YUNI_MODULE_AVAILABLE keyboard)
	endif()
endif()

if(YUNI_MODULE_VFS)
	list(APPEND YUNI_MODULE_AVAILABLE vfs)
	if(YUNI_MODULE_VFS_FILE)
		list(APPEND YUNI_MODULE_AVAILABLE vfs-local)
	endif()
endif()

if(YUNI_MODULE_VM)
	list(APPEND YUNI_MODULE_AVAILABLE vm)
endif()



if(YUNI_MODULE_NET)
	list(APPEND YUNI_MODULE_AVAILABLE net)
	if(YUNI_MODULE_NET_MESSAGES)
		list(APPEND YUNI_MODULE_AVAILABLE net-messages)
	endif()
endif()

if(YUNI_MODULE_LDO)
	list(APPEND YUNI_MODULE_AVAILABLE ldo)
endif()


if(YUNI_MODULE_AUDIO)
	list(APPEND YUNI_MODULE_AVAILABLE audio)
endif()

if(YUNI_MODULE_SCRIPT)
	list(APPEND YUNI_MODULE_AVAILABLE script)
	if(YUNI_EXTERNAL_SCRIPT_LUA)
		list(APPEND YUNI_MODULE_AVAILABLE lua)
	endif()
endif()

if(YUNI_MODULE_UI)
	list(APPEND YUNI_MODULE_AVAILABLE ui)
endif()

if(YUNI_MODULE_EXTRA_MARKDOWN)
	LIST(APPEND YUNI_MODULE_AVAILABLE markdown)
endif()

if(YUNI_MODULE_DOCS)
	LIST(APPEND YUNI_MODULE_AVAILABLE docs)
endif()


