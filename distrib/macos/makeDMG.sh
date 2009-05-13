#!/bin/sh

root=`dirname ${0}`

# Title of the DMG
#caption="TA3D - A remake of Total Annihilation" # Really too long for a name...
caption="TA3D" 

# Used to get the filename
name="ta3d"  

# Version
version="0.5.3"
# Revision
revision=`svnversion  -n "${root}/../../" | cut -d':' -f2`


# Folders
targetDirectory="${root}/../../bin"
trunk="${root}/../../"
tmp="/tmp/__ta3d__osx__"

# The target DMG File
dmgFile="${targetDirectory}/${name}-${version}-r${revision}.dmg"




# ---------------------------------

echo "TA3D - A remake of Total Annihilation - DMG Maker"
echo "Version: ${version}"

echo "Cleaning..."
if [ -f "${dmgFile}" ]; then
    rm -f "${dmgFile}"
fi
# Clean up
if [ -e "${tmp}" ]; then
    rm -rf "${tmp}"
fi
mkdir -p "${tmp}"

# Preparation
echo "Preparation..."
cp -f "${trunk}/AUTHORS" "${tmp}/Authors.txt"
cp -f "${trunk}/COPYING" "${tmp}/Copying.txt"
if [ -d "${trunk}/ta3d.app" ]; then
    cp -rf "${trunk}/ta3d.app" "${tmp}/"
fi

# Create the DMG file
captionOfTheDMG="${caption} - ${version}"
echo "Building the Disk image..."
echo "Caption: ${captionOfTheDMG}"
echo "Target: ${dmgFile}"
hdiutil create -anyowners -format UDZO -imagekey zlib-level=9 -volname "${captionOfTheDMG}" -uid 99 -gid 99 -srcfolder "${tmp}/"  "${dmgFile}"

# Last clean up
rm -rf "${tmp}"
echo "Done."


