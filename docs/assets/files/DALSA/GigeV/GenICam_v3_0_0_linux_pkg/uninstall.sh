#!/bin/bash
#!/bin/ksh
#-----------------------------------------------------------------------
# uninstall.sh
#					Copyright(C) 20016 Teledyne DALSA Inc
#						All rights reserved.
#
# Description:
#       Manual uninstall script for GenICam libraries under Linux.
#-----------------------------------------------------------------------
#
#=======================================================================
#
# This is to play nice with out corinstall script which expects a 
# response to different command line options which we do not give here.
SKIPIT="No"
if  [ $# -gt 0 ] ; then
    if [ ! $1 = "Uninstall" ] ; then 
        SKIPIT="Yes"
    fi
fi
# Definitions for version of GenICam library in this package.
# (Update for each new released version)
#
GENICAM_PATH_VERSION="v3_0"
GENICAM_BUILD_VERSION="0"
GENICAM_VERSION_ENV_STRING="GENICAM_ROOT_V3_0"
GENICAM_INSTALL_BASE_PATH="/opt/genicam_$GENICAM_PATH_VERSION"
GENICAM_LOG_CONFIG_PATH="$GENICAM_INSTALL_BASE_PATH/log/config-unix"
GENICAM_CACHE_PATH="/var/opt/genicam/xml/cache"

#==================================================================================
# Check CPU architecture to install to.
ARCH=`uname -m | sed -e s/i.86/i386/ -e s/x86_64/x86_64/ -e s/arm*.*/armhf/ `

if [[  $ARCH == "x86_64" ]] ; then
    ARCHNAME=x86_64
    ARCH_GENICAM_BIN="Linux64_x64"
    ARCH_GENICAM_SDK_TGZ_FILE="GenICam_SDK_gcc421_Linux64_x64_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
    ARCH_GENICAM_TGZ_FILE="GenICam_Runtime_gcc421_Linux64_x64_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
elif [[ $ARCH == "i386" ]] ; then
    ARCHNAME=i386
    ARCH_GENICAM_BIN="Linux32_i86"
    ARCH_GENICAM_SDK_TGZ_FILE="GenICam_SDK_gcc421_Linux32_i86_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
    ARCH_GENICAM_TGZ_FILE="GenICam_Runtime_gcc421_Linux32_i86_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
elif [[ $ARCH == "armsf" ]] ; then
    ARCHNAME=arm
    ARCH_GENICAM_BIN="Linux32_ARMsf"
    ARCH_GENICAM_SDK_TGZ_FILE="GenICam_SDK_gcc46_Linux32_ARMsf_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
    ARCH_GENICAM_TGZ_FILE="GenICam_Runtime_gcc46_Linux32_ARMsf_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
elif [[ $ARCH == "armhf" ]] ; then
    ARCHNAME=arm
    ARCH_GENICAM_BIN="Linux32_ARMhf"
    ARCH_GENICAM_SDK_TGZ_FILE="GenICam_SDK_gcc46_Linux32_ARMhf_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
    ARCH_GENICAM_TGZ_FILE="GenICam_Runtime_gcc46_Linux32_ARMhf_""$GENICAM_PATH_VERSION""_""$GENICAM_BUILD_VERSION"".tgz"
else
    echo "Architecture $ARCH is not supported by this package!!"
    SKIPIT="Yes"
fi

#==================================================================================
# Check for existing GenICam library installation of the same version as this
# (in case it is already installed)
GENICAM_ENV=`env | grep $GENICAM_VERSION_ENV_STRING`
if [[ $SKIPIT != "Yes" ]] ; then
    if [[  $GENICAM_ENV != "" ]] ; then
        # No environment variable in this shell.
        # In case this is uninstalled before a login updated the environment variable
        # also check the location we target to see if we can uninstall it.
        if [[  -d $GENICAM_INSTALL_BASE_PATH ]] ; then
            SKIPIT="No"
        else
            echo "********************************************************************"
            echo " No GenICam library installation was found : environment variable "
            echo " $GENICAM_VERSION_ENV_STRING is not defined"
            echo " (Nothing to uninstall) : Exiting " 
            echo "********************************************************************"
            SKIPIT="Yes"
        fi
    fi
fi

if [[ $SKIPIT != "Yes" ]] ; then
	#==============================================================================
	# Here we will need the sudo password set up to remove the installation dir.
	#
	INSTALLER_NAME="GenICam Library Uninstaller"
	if [[ $DISPLAY ]] ; then
		 if [ -x /usr/bin/gksudo ] ; then
			  if ! gksudo -m "$INSTALLER_NAME needs administrative privileges" echo " " ; then
					SKIPIT="Yes"
			  fi
		 elif [ -x /usr/bin/kdesudo ] ; then
			  if ! kdesudo --comment "$INSTALLER_NAME needs administrative privileges" echo " " ; then
					SKIPIT="Yes"
			  fi
		 else
			  echo "**************************************************************"
			  echo "****** $INSTALLER_NAME needs administrative privileges ******" 
			  sudo echo " "
			  RETVAL=$?
			  if [[ "$RETVAL" -ne 0 ]] ; then
					SKIPIT="Yes"
			  fi
		 fi
	else
		 echo "**************************************************************"
		 echo "****** $INSTALLER_NAME needs administrative privileges ******" 
		 sudo echo " "
		 RETVAL=$?
		 if [[ "$RETVAL" -ne 0 ]] ; then
			  SKIPIT="Yes"
		 fi
	fi

	if [[ $SKIPIT != "Yes" ]] ; then
		#==============================================================================
		# Here we proceed with the removal.
		#
		echo "Uninstalling GenICam $GENICAM_PATH_VERSION from $GENICAM_INSTALL_BASE_PATH "
		#
		# Remove the environment variable files.
		# Usually this is /etc/profile.d where they get sourced by /etc/profile
		# for interactive shells.
		if [ -f /etc/profile.d/td_genapi_"$GENICAM_PATH_VERSION".sh ] ; then
			 sudo rm /etc/profile.d/td_genapi_"$GENICAM_PATH_VERSION".sh
		fi
		if [ -f /etc/profile.d/td_genapi_"$GENICAM_PATH_VERSION".csh ] ; then
			 sudo rm /etc/profile.d/td_genapi_"$GENICAM_PATH_VERSION".csh
		fi
		. /etc/profile


		GENICAM_LIBRARY_PATH="$GENICAM_INSTALL_BASE_PATH/bin/$ARCH_GENICAM_BIN"
		# Remove the ldconf config file from the /etc/ld.so.conf.d directory
		if [ -f /etc/ld.so.conf.d/td_genicam_$GENICAM_PATH_VERSION.conf ] ; then
			 sudo rm -f /etc/ld.so.conf.d/td_genicam_$GENICAM_PATH_VERSION.conf
			 if [ -x /sbin/ldconfig ] ; then
				  sudo /sbin/ldconfig
			 fi
		fi

		# Clean out the XMl cache.
		if [ -d $GENICAM_CACHE_PATH ] ; then
			 # Sanity check !!!!
			 if [ $GENICAM_CACHE_PATH != "/" ] ; then
				  SAVEPATH=$(pwd)
				  cd $GENICAM_CACHE_PATH
				  #echo " calling sudo rm -f *  in $(pwd) "
				  sudo rm -f *
				  cd $SAVEPATH
			 else
				  echo "*********************************************************"
				  echo " Configured XML cache path looks WRONG /UNSAFE "
				  echo " Refusing to delete files "
				  echo " $GENICAM_CACHE_PATH  "
				  echo " Please do it manually"
				  echo "*********************************************************"
			 fi
		fi  

		# 
		# Remove the files from the install
		if [ -d $GENICAM_INSTALL_BASE_PATH ] ; then
			 # Sanity check !!!!
			 if [ $GENICAM_INSTALL_BASE_PATH != "/" ] ; then
				  SAVEPATH=$(pwd)
				  cd $GENICAM_INSTALL_BASE_PATH
				  #echo " calling sudo rm -rf *  in $(pwd) "
				  sudo rm -rf *
				  cd $SAVEPATH
			 else
				  echo "*********************************************************"
				  echo " Configured installation path looks WRONG /UNSAFE "
				  echo " Refusing to recursively delete  "
				  echo " $GENICAM_INSTALL_BASE_PATH  "
				  echo " Please do it manually - Exitting  "
				  echo "*********************************************************"

			 fi
		fi 

		echo "Done"
	fi
fi
