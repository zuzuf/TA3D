
#
# Profile
#
Set(YUNI_PROFILE_NAME "Yuni's profile for TA3D")



#
# Target
#
# Uncomment the following line to override the build target
# Accepted values: `debug`, `release`
#Set(YUNI_TARGET  "release")


#
# Modules
#
# Uncomment the following line to override the module list
# Example : `gfx3d,scripts,-net`
# Note : `cmake -DMODULES=help` to have a list of all available modules
Set(MODULES "core,net")



#
# Packages
#
# A module often requires one or more external packages (`lua` for example).
# Most of the time the system has its own package management utility, which will
# provide all needed and up-to-date packages (`lua`, `libxml`...).
# It is not always the case (Windows for example), so some pre-built packages (DevPacks)
# are available on http://devpacks.libyuni.org and can be automatically downloaded.
#
# Several modes can be given in the prefered order to find and use the appropriate package.
# If nothing suits to your needs, it is possible to use the `custom` mode and to set
# the prefix path where the package can be found.
#
# Modes :
#    system  : Try to use the standard way to find the package provided by the system
#              Standard paths, System Frameworks (OS X)
#    custom  : Try to find the package from a custom prefix path
#              The variable `YUNI_DvP_<pkg>_PREFIX` must be set
#    macport : Try to find the package from a macport installation (Mac OS X)
#              (http://www.macports.org)
#    devpack : Download and use the pre-build package from `devpacks.libyuni.org`
#              This is the recommended way when the package is not available on the system
#
# Example :
# Use `lua` compiled by hand, installed in `/local/opt` (/local/opt/include and `/local/opt/lib`) :
#   Set(YUNI_DvP_LUA_MODE custom)
#   Set(YUNI_DvP_LUA_PREFIX "/local/opt")
#

# PThread (Yuni Core)
Set(YUNI_DvP_PTHREAD_MODE     system devpack)

# Lua (+script,+lua)
Set(YUNI_DvP_LUA_MODE         macport system devpack)



#
# Auto-Compile yuni-config from CMake
#
# Enable this option to automatically compile yuni-config from CMake
#
Set(YUNI_AUTO_COMPILE_YUNI_CONFIG  true)



#
# C++ Flags
#
# Uncomment the following line to add some C++ compiler flags
#Set(YUNI_CXX_FLAGS_OVERRIDE_ADD "-Wextra")

# Uncomment the following line to override the C++ compiler flags
# This is not recommended.
#Set(YUNI_CXX_FLAGS_OVERRIDE "-g -ggdb -Wall -Wextra")


