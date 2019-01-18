#-*-Shell-script-*-
#
#

PLATFORM_FOUND="FALSE"
PLATFORM_REV="0"
PLATFORM_NAME="Unknown"

MODPATH=/lib/modules/`uname -r`/misc
LIBINSTALLDIR=/usr/local/lib
CORECOUSERPATH=/usr/coreco
DALSAUSERPATH=/usr/dalsa


function get_install_type
{
   if [ -x /sbin/chkconfig ]; then
      PLATFORM_INSTALL_TYPE="redhat_type"
      return 0
   fi
   if [ -x /sbin/update-rc.d ]; then
      PLATFORM_INSTALL_TYPE="debian_type"
      return 0
   fi
   if [ -x /usr/sbin/update-rc.d ]; then
      PLATFORM_INSTALL_TYPE="debian_type"
      return 0
   fi
   if [ -x /sbin/insserv ]; then
      PLATFORM_INSTALL_TYPE="suse_type"
      return 0
   fi
   PLATFORM_INSTALL_TYPE="UNKNOWN"
   return 1 
}

function check_debian
{
   if [ ! -f /etc/debian_version ]; then
      return 1
   fi

   PLATFORM_REV=`cat /etc/debian_version`
   PLATFORM_NAME="Debian"
   if [ $PLATFORM_REV == "3.0" ]; then
      PLATFORM_FOUND="TRUE"
   else
       PLATFORM_FOUND="MABY"
   fi
}

function check_redhat
{
   if [ -f /etc/redhat-release ]; then
      PLATFORM_REV=`awk "(\\$1 == \"Red\") && (\\$2 == \"Hat\") && (\\$3 == \"Linux\") && ( \\$4 == \"release\") {print \\$5}" /etc/redhat-release`
      if [ $PLATFORM_REV ]; then
          PLATFORM_NAME="RedHat"
          if [ $PLATFORM_REV == "9" ]; then
             PLATFORM_FOUND="TRUE"
          else
             PLATFORM_FOUND="MABY"
          fi
      else
          PLATFORM_REV=`awk "(\\$1 == \"CentOS\") && (\\$2 == \"release\") {print \\$3}" /etc/redhat-release`
          if [ $PLATFORM_REV ]; then
             PLATFORM_NAME="RedHat"
             PLATFORM_FOUND="TRUE"
          else
             PLATFORM_REV=`awk "(\\$1 == \"Fedora\") && (\\$2 == \"Core\") && (\\$3 == \"release\") {print \\$4}" /etc/redhat-release`
             if [ $PLATFORM_REV ]; then
                PLATFORM_NAME="Fedora"
                if [ $PLATFORM_REV -ge "3" ]; then
                   PLATFORM_FOUND="TRUE"
                else
                   PLATFORM_FOUND="MABY"
                fi
             else
                PLATFORM_REV=`awk "(\\$1 == \"Fedora\") && (\\$2 == \"release\") {print \\$3}" /etc/redhat-release`
                if [ $PLATFORM_REV ]; then
                   PLATFORM_NAME="Fedora"
                   if [ $PLATFORM_REV -ge "3" ]; then
                      PLATFORM_FOUND="TRUE"
                   else
                      PLATFORM_FOUND="MABY"
                   fi
                fi
             fi
          fi
      fi
   else
      if [ -f /etc/fedora-release ]; then
         PLATFORM_REV=`awk "(\\$1 == \"Fedora\") && (\\$2 == \"Core\") && (\\$3 == \"release\") {print \\$4}" /etc/fedora-release`

         if [ $PLATFORM_REV ]; then
             PLATFORM_NAME="Fedora"
             if [ $PLATFORM_REV -ge "3" ]; then
                PLATFORM_FOUND="TRUE"
             else
                PLATFORM_FOUND="MABY"
             fi
         else
             PLATFORM_REV=`awk "(\\$1 == \"Fedora\") && (\\$2 == \"release\") {print \\$3}" /etc/redhat-release`
             if [ $PLATFORM_REV ]; then
                PLATFORM_NAME="Fedora"
                if [ $PLATFORM_REV -ge "3" ]; then
                   PLATFORM_FOUND="TRUE"
                else
                   PLATFORM_FOUND="MABY"
                fi
             fi
         fi
      else
         return 1
      fi
   fi
}

function check_suse
{
   if [ ! -f /etc/SuSE-release ]; then
      return 1
   fi

   PLATFORM_REV=`awk "\\$1 == \"VERSION\" {print \\$3}" /etc/SuSE-release`
   if [ $PLATFORM_REV ]; then
      PLATFORM_NAME="SuSE"
      if [ $PLATFORM_REV == "9.0" ]; then
	      PLATFORM_FOUND="TRUE"
      else
	      PLATFORM_FOUND="MABY"
      fi
   fi
}

function check_mandrake
{
   if [ ! -f /etc/mandrake-release ]; then
      return 1
   fi

   PLATFORM_REV=`awk "(\\$1 == \"Mandrake\") && ( \\$2 == \"Linux\") && ( \\$3 == \"release\") {print \\$4}" /etc/mandrake-release`
   if [ $PLATFORM_REV ]; then
      PLATFORM_NAME="Mandrake"
      if [ $PLATFORM_REV == "9.2" ]; then
         PLATFORM_FOUND="TRUE"
      else
	      PLATFORM_FOUND="MABY"
      fi
   fi
}

function check_other
{
   TEMPFILENAME=$(ls -m /etc/*release | cut -d "," -f 1)
   if [ -f $TEMPFILENAME ]; then
      PLATFORM_NAME=$(cat $TEMPFILENAME)
      PLATFORM_REV=
      PLATFORM_FOUND="MABY"
   fi   
}

function get_platform_info
{
   for i in check_redhat check_suse check_mandrake check_debian check_other; do
      if [ $PLATFORM_FOUND == "FALSE" ]; then
         $i
      fi
   done
}

function display_platform_info
{
   echo "$PLATFORM_NAME-$PLATFORM_REV"
}

# add_start_script
# This function add a script in Linux boot sequence
#
# $1 - Script path
# $2 - script name (no INITD_XXXX)
 
function add_start_script()
{
   case "$PLATFORM_INSTALL_TYPE" in
      debian_type)
          SCRIPT_START_INDEX=`awk "\\$2 == \"chkconfig:\" {print \\$4}" $1/INITD.$2`
          SCRIPT_STOP_INDEX=`awk "\\$2 == \"chkconfig:\" {print \\$5}" $1/INITD.$2`
          sudo install -m755 $1/INITD.$2 /etc/init.d/$2
          sudo update-rc.d $2 start $SCRIPT_START_INDEX 2 3 4 5 . stop $SCRIPT_STOP_INDEX 0 1 6 .
      ;;
      suse_type)
          sudo install -m755 $1/INITD.$2 /etc/init.d/$2
          sudo insserv $2
      ;;
      redhat_type)
          sudo install -m755 $1/INITD.$2 /etc/init.d/$2
          sudo /sbin/chkconfig --add $2 --level 2345
      ;;

   esac
}
# del_start_script
# This function remove a script in Linux boot sequence
#
# $1 - script name
 
function del_start_script()
{
   case "$PLATFORM_INSTALL_TYPE" in
      debian_type)
         sudo update-rc.d -f $1 remove
         sudo rm /etc/init.d/$1 2>/dev/null
      ;;
      suse_type)
         sudo /sbin/insserv -r $1
         sudo rm /etc/init.d/$1 2>/dev/null
      ;;
      redhat_type)
         sudo /sbin/chkconfig --del $1
         sudo rm /etc/init.d/$1 2>/dev/null
      ;;
   esac  
}

# add_coreco_link
# This function added path (link) in the 
# DALSA config folder (/usr/dalsa)
#
# $1 - Name or the link
# $2 - file or folder to link to


function add_coreco_link()
{
   if [ ! -d /usr/local/include ] ; then
      sudo mkdir -p /usr/local/include
   fi

   if [ -L /usr/local/include/$1 ] ; then
      sudo rm /usr/local/include/$1
   fi
   sudo ln -s $2 /usr/local/include/$1

   if [ -L $DALSAUSERPATH/$1 ] ; then
      sudo rm $DALSAUSERPATH/$1
   fi
   sudo ln -s $2 $DALSAUSERPATH/$1

   sudo chmod 755 /usr/local/include/$1
   sudo chmod 755 $DALSAUSERPATH/$1

}

# del_coreco_link
# This function delete path (link) in the
# Coreco config folder (/usr/local/include/coreco)
#
# $1 - Name or the link
function del_coreco_link()
{
   if [ -d /usr/local/include/$1 ]; then
      sudo rm /usr/local/include/$1
   fi

   if [ -d $DALSAUSERPATH/$1 ]; then
      sudo rm $DALSAUSERPATH/$1
   fi
}

# build_shared_library
# This function builds a shared library (*.so) from a static library (*_a)
#
# $1 = Library name
# $2 = Library directory
# $3 = Library version number
# $4 = Library build number
# $5 = List of additional libraries for the linker
function build_shared_library()
{
	SONAME=$2/lib$1.so
	SHAREDLIBNAME=$SONAME.$3.$4
	LIBNAME=$2/lib$1_$3_$4_a
	gcc -shared -W1,soname,$SONAME.$3 -o $SHAREDLIBNAME $LIBNAME $5
}

# new_build_shared_library
# This function builds a shared library (*.so) from a static library (*_a)
# The version information (version / build #) is extracted from the name.
#
# $1 = Static Library name
# $2 = Library directory
# $3 = List of additional libraries for the linker
function new_build_shared_library()
{
	LIBNAME=$1
	SONAME=`echo $LIBNAME | awk -F_ '{print $1".so."$2}'`
	LIBVER=`echo $LIBNAME | awk -F_ '{print $3}'`
	SHAREDLIBNAME=$SONAME.$LIBVER
	gcc -shared -W1,soname,$SONAME -o $2/$SHAREDLIBNAME $2/$LIBNAME $3
}
# build_utility
# This function builds an executable from its object files
#
# $1 = Executable file name
# $2 = List of object files
# $3 = List of additional libraries for the linker
function build_utility()
{
	gcc -o $1 $2 $3
}

# install_library
# This function add a libraty in the
# share library folder
#
# $1 = Library name
# $2 = Source library path
function install_library()
{
	LIBNAME=$1
	SONAME=`echo $LIBNAME | awk -F. '{print $1"."$2}'`
	LINKNAME=`echo $LIBNAME | awk -F. '{print $1"."$2"."$3}'`
	sudo cp $2/$LIBNAME $LIBINSTALLDIR
	if [ -L $LIBINSTALLDIR/$LINKNAME ] ; then
		sudo rm $LIBINSTALLDIR/$LINKNAME
	fi
	if [ -L $LIBINSTALLDIR/$SONAME ] ; then
		sudo rm $LIBINSTALLDIR/$SONAME
	fi

	sudo /sbin/ldconfig -n $LIBINSTALLDIR

	if [ ! -L $LIBINSTALLDIR/$LINKNAME ] ; then
		sudo ln -s $LIBINSTALLDIR/$LIBNAME $LIBINSTALLDIR/$LINKNAME
	fi

	if [ ! -L $LIBINSTALLDIR/$SONAME ] ; then
		sudo ln -s $LIBINSTALLDIR/$LINKNAME $LIBINSTALLDIR/$SONAME
	fi
}

# remove_library
# This function remove a libraty in the
# share library folder
#
# $1 = Library name
function remove_library()
{
	LIBNAME=$1
	SONAME=`echo $LIBNAME | awk -F. '{print $1"."$2}'`
	LINKNAME=`echo $LIBNAME | awk -F. '{print $1"."$2"."$3}'`

	if [ -f $LIBINSTALLDIR/$LIBNAME ] ; then
	   sudo rm  $LIBINSTALLDIR/$LIBNAME
   fi
	if [ -L $LIBINSTALLDIR/$SONAME ] ; then
		sudo rm $LIBINSTALLDIR/$SONAME
	fi
	if [ -L $LIBINSTALLDIR/$LINKNAME ] ; then
		sudo rm $LIBINSTALLDIR/$LINKNAME
	fi

}


# dalsa_genie_init
# This function initialize this script
function dalsa_genie_init
{

#   if [ $UID != 0 ]; then
#       echo "You have to be root (sudo) to run this program!"
#       exit 1
#   fi

   # Validate Platform (distribution and version)
   get_platform_info
   get_install_type

   # Init library configuration
   if [ ! -d $LIBINSTALLDIR ]; then
      sudo mkdir -p $LIBINSTALLDIR
   fi

   LIBPATH=`awk "\\$1 == \"$LIBINSTALLDIR\" {print \\$1}" /etc/ld.so.conf`
   if [ -z $LIBPATH ]; then
      sudo echo "$LIBINSTALLDIR" >> /etc/ld.so.conf
   fi

    # Create user path (dalsauserpath as directory linked by corecouserpath if it already exists).
    if [ ! -d $CORECOUSERPATH ] ; then
        # CORECOUSERPATH does not exist - new installation - use DALSAUSERPATH
        if [  ! -d $DALSAUSERPATH ] ; then
            sudo mkdir -p $DALSAUSERPATH
        fi
    else
        # CORECOUSERPATH exists as an existing directory (old installation)
        # Update the name since we use DALSAUSERPATH now
        if [ ! -d $DALSAUSERPATH ] ; then
            sudo mv $CORECOUSERPATH $DALSAUSERPATH
            # Make a link for backwards compatibility.
            sudo  ln -s $DALSAUSERPATH $CORECOUSERPATH
            if [ -f $DALSAUSERPATH/coreco.config ] ; then
                # Make the old coreco.config file into the dalsa.config file
                # keeping the old name as a link.
                sudo mv $DALSAUSERPATH/coreco.config $DALSAUSERPATH/dalsa.config
                sudo ln -s $DALSAUSERPATH/dalsa.config $DALSAUSERPATH/coreco.config
            fi
        fi
    fi
}


dalsa_genie_init
