#!/bin/sh

if [ "$1" != "ta3d" ] && [ "$1" != "" ];
then
	exit 0
fi

# create the required directories
mkdir ~/.ta3d
mkdir ~/.ta3d/resources

# here we set up our file installer
export INSTALL_CMD="./ta3d --quiet --install"

if [ -f "/usr/games/ta3d" ]; then
    echo "TA3D installation found!"
    export INSTALL_CMD="ta3d --quiet --install"
fi

function copy_file {
    echo -n "."
    for i in `find /mnt/* 2> /dev/null | grep $1\$`; do
        echo -e -n "\n"
        echo "copying $i"
        cp $i ~/.ta3d/resources/
    done
    for i in `find /media/* 2> /dev/null | grep $1\$`; do
        echo -e -n "\n"
        echo copying $i
        cp $i ~/.ta3d/resources/
    done
}

function copy_file_from_hpi {
    echo -n "."
    for i in `find /mnt/* 2> /dev/null | grep $1\$`; do
        echo -e -n "\n"
        echo "copying hpi://$i:$2"
        ${INSTALL_CMD} $i $2
    done
    for i in `find /media/* 2> /dev/null | grep $1\$`; do
        echo -e -n "\n"
        echo "copying hpi://$i:$2"
        ${INSTALL_CMD} $i $2
    done
}

function header {
    # make things clear
    clear

    # some intro text :)
    echo "*-----------------------------------------------------------*"
    echo "|              Total Annihilation files installer           |"
    echo "|                                                           |"
    echo "|  Installateur de ressources Total Annihilation pour TA3D  |"
    echo "*-----------------------------------------------------------*"
    echo -n -e "\n"
    }

# OTA CD 1
header
echo "please mount/insert your TA cdrom now (TA CD 1)"
echo "press any key to continue"
read -n 1 -s
echo -e "\nsearching files"
copy_file totala1.hpi
copy_file TOTALA1.HPI
copy_file totala2.hpi
copy_file TOTALA2.HPI
copy_file totala4.hpi
copy_file TOTALA4.HPI
copy_file_from_hpi totala3.hpi install\\totala1.hpi
copy_file_from_hpi TOTALA3.HPI install\\totala1.hpi

# OTA CD 2
header
echo "please mount/insert your TA cdrom now (TA CD 2)"
echo "press any key to continue"
read -n 1 -s
echo -e "\nsearching files"
copy_file totala1.hpi
copy_file TOTALA1.HPI
copy_file totala2.hpi
copy_file TOTALA2.HPI
copy_file totala4.hpi
copy_file TOTALA4.HPI
copy_file_from_hpi totala3.hpi install\\totala1.hpi
copy_file_from_hpi TOTALA3.HPI install\\totala1.hpi

# OTA Battle Tactics CD
header
echo "please mount/insert your TA:Battle Tactics cdrom now (TA:BT CD) if you have it"
echo "press any key to continue"
read -n 1 -s
echo -e "\nsearching files"
copy_file btdata.ccx
copy_file btmaps.ccx
copy_file tactics1.hpi
copy_file tactics2.hpi
copy_file tactics3.hpi
copy_file tactics4.hpi
copy_file tactics5.hpi
copy_file tactics6.hpi
copy_file tactics7.hpi
copy_file tactics8.hpi
copy_file BTDATA.CCX
copy_file BTMAPS.CCX
copy_file TACTICS1.HPI
copy_file TACTICS2.HPI
copy_file TACTICS3.HPI
copy_file TACTICS4.HPI
copy_file TACTICS5.HPI
copy_file TACTICS6.HPI
copy_file TACTICS7.HPI
copy_file TACTICS8.HPI

# OTA Core Contingency CD
header
echo "please mount/insert your TA:Core Contingency cdrom now (TA:CC CD) if you have it"
echo "press any key to continue"
read -n 1 -s
echo -e "\nsearching files"
copy_file ccdata.ccx
copy_file ccmaps.ccx
copy_file ccmiss.ccx
copy_file CCDATA.CCX
copy_file CCMAPS.CCX
copy_file CCMISS.CCX

echo -n "."
echo "done."
