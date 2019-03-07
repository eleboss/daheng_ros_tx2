#!/bin/bash
#-------------------------------------------------------------
#**
#file      install.sh
#brief     Galaxy embedded SDK install script 
#version   1.0.1807.8231
#date      2018.07.23
#/
#-------------------------------------------------------------

# Display Galaxy welcom message
echo "
Welcome to use Galaxy cameras

-----------------------------------------------------------------
-----------------------------------------------------------------
   ######        ###       ####        ###    ###  ###  ###   ###
 ###   ###      ####       ##         ####     ##  ##    ##   ## 
###            #  ##      ##         #  ##      # ##      ## ##  
##   #####    #   ##     ##         #   ##      ###        ###   
##     ###   #######    ##         #######     ## ##       ##    
###    ##   ##    ##   ##         ##    ##    ##   ##      ##    
  ######   ####  #### ########## ####  #### ###    ###    ####   
-----------------------------------------------------------------
-----------------------------------------------------------------

This script will install Galaxy embedded SDK to your system
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

CURRENT_PATH=`pwd`

USR_LIB=/usr/lib
LIB_PATH=$CURRENT_PATH/lib
LIB_GXIAPI=libgxiapi.so
LIB_GXU3VTL=GxU3VTL.cti
CONFIG_PATH=$CURRENT_PATH/config

# Exit if user lib did not exist
if [ ! -d "$USR_LIB" ] ; 
then
    echo "***Libary file add failed*** : Libary directory '$USR_LIB' not found in your system."
    echo "Script Exit..."
    exit 1;
fi
# Exit if lib not found in package
if [ ! -e "$LIB_PATH/$LIB_GXIAPI" ] || [ ! -e "$LIB_PATH/$LIB_GXU3VTL" ]; 
then
    echo "***Libary file add failed*** : Libary missing!"
    echo "Script Exit..."
    exit 1;
fi

# Override old library or not
if [ -e "$USR_LIB/$LIB_GXIAPI" ] || [ -e "$USR_LIB/$LIB_GXU3VTL" ]; 
then      
    if ChooseYorN "$LIB_GXIAPI or $LIB_GXU3VTL already exist, are you sure to override?"; 
    then
    $SUDO rm -f $USR_LIB/$LIB_GXIAPI
    $SUDO rm -f $USR_LIB/$LIB_GXU3VTL
    $SUDO cp -af $LIB_PATH/$LIB_GXIAPI $USR_LIB
    $SUDO cp -af $LIB_PATH/$LIB_GXU3VTL $USR_LIB
    echo "Libary already override."
    echo ""
    else
    echo "Libary did not override."
    echo ""
    fi
else
    $SUDO cp -af $LIB_PATH/$LIB_GXIAPI $USR_LIB
    $SUDO cp -af $LIB_PATH/$LIB_GXU3VTL $USR_LIB
fi

# Copy udev file to get device access permission of normal user, take effect after replug.
UDEV_RULES_PATH=/etc/udev/rules.d
UDEV_RULES_FILE=99-galaxy-u3v.rules
if [ -e "$CONFIG_PATH/$UDEV_RULES_FILE" ] ;
then
    if [ -d "$UDEV_RULES_PATH" ] ; 
    then
       $SUDO cp -af $CONFIG_PATH/$UDEV_RULES_FILE $UDEV_RULES_PATH
	   $SUDO service udev reload
	   $SUDO service udev restart
    else
        echo "***Udev file add failed*** : Udev rules directory '$UDEV_RULES_PATH' not found in your system.
You may need to add your device access permission manually,
or you can use 'sudo' to get permission to access device." 
        echo "" 
    fi
else
    echo "***Udev file add failed*** : Udev rules file '$UDEV_RULES_FILE' missing!.
You may need to add your device access permission manually,
or you can use 'sudo' to get permission to access device." 
    echo "" 
fi

# Copy limits file to get increase thread priority permission, not particularly necessary, take effect after reboot.
LIMITS_CONF_PATH=/etc/security/limits.d
LIMITS_CONF_FILE=galaxy-limits.conf
if [ -e "$CONFIG_PATH/$LIMITS_CONF_FILE" ] ;
then
    if [ -d "$LIMITS_CONF_PATH" ] ; 
    then
        $SUDO cp -af $CONFIG_PATH/$LIMITS_CONF_FILE $LIMITS_CONF_PATH
    else
        echo "***Limits file add failed*** : Limits conf directory '$LIMITS_CONF_PATH' not found in your system."
        echo ""
    fi
else
    echo "***Limits file add failed*** : Limits conf file '$LIMITS_CONF_FILE' missing!." 
    echo ""
fi

echo "-----------------------------------------------------------------"
echo "All configurations will take effect after the system is rebooted"
echo "If you don't want to reboot the system for a while"
echo "you will need to unplug and replug the camera."
echo "-----------------------------------------------------------------"
