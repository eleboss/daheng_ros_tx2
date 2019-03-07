#!/bin/bash
#-------------------------------------------------------------
#**
#file      uninstall.sh
#brief     Galaxy embedded SDK uninstall script 
#version   1.0.1807.8231
#date      2018.07.23
#/
#-------------------------------------------------------------



echo "
Thank you for using Galaxy cameras

This script will uninstall Galaxy embedded SDK to your system
press 'Enter' to continue..."
read -p "$1" arg

# Return 0 for 'n', 1 for 'y'
ChooseYorN() 
{
    local q="$1 (Y/n) : "
    while true; do
        read -p "$q" yn
        case $yn in 
            [Yy]* ) return 0; ;;
            [Nn]* ) return 1; ;;
            #* );;
        esac
    done
}

if ChooseYorN "Are you sure to uninstall Galaxy embedded SDK from your device?"
then
    echo "Uninstall confirmed."
else
    echo "Uninstall canceled, Script Exit..."
    exit 1
fi

# Make sure script is only being run with root privileges
if [ "$(id -u)" != "0" ] ;
then
    if ChooseYorN "You don't have root permission, Script will try to use sudo. Sure to continue?" ;
    then
        SUDO="sudo "
        $SUDO echo ""
    else
        echo "***This script must be run with root privileges!***"
        echo "Script Exit..."
        exit 1;
    fi
fi

# Delete SDK files

USR_LIB=/usr/lib
LIB_GXIAPI=libgxiapi.so
LIB_GXU3VTL=GxU3VTL.cti

$SUDO rm -f $USR_LIB/$LIB_GXIAPI
$SUDO rm -f $USR_LIB/$LIB_GXU3VTL

UDEV_RULES_PATH=/etc/udev/rules.d
UDEV_RULES_FILE=99-galaxy-u3v.rules

$SUDO rm -f $UDEV_RULES_PATH/$UDEV_RULES_FILE

LIMITS_CONF_PATH=/etc/security/limits.d
LIMITS_CONF_FILE=galaxy-limits.conf

$SUDO rm -f $LIMITS_CONF_PATH/$LIMITS_CONF_FILE

echo "-----------------------------------------------------------------"
echo "Galaxy embedded SDK uninstall finished."
echo "-----------------------------------------------------------------"


