===============================================================================
GenICam SDK v3_0

This package contains the GenICam SDK for use on Linux systems. 
It is, essentially, the Linux versions of the standard packages provided at 
www.genicam.org, for i86, x86_64, and ARM architectures. They are combined 
here with an install script (install.sh) and an uninstall script (uninstall.sh). 

Each package contains the same license information which is repeated in the
License_ReadMe.txt file in this directory.

By default, the installation script with install the SDK to
/opt/genicam_v3_0 and will set up the GenICam environment variable as :

GENICAM_ROOT=/opt/genicam_v3_0
GENICAM_ROOT_V3_0=/opt/genicam_v3_0
GENICAM_LOG_CONFIG=/opt/genicam_v3_0/log/config-unix
GENICAM_CACHE=/var/opt/genicam/xml/cache

(via scripts added to /etc/profile.d)

It will update the ld.so.cache database (via scripts in /etc/ld.so.config.d) 
in order to find GenICam libraries for linking and loading.

The GenICam package is shared by multiple APIs / applications so this 
package and installer attempts to treat it in a stand-alone manner to keep 
it as generic as possible.

: Note : 
If installing using "./install.sh", you will need to log out and log back in 
in order to pick up the environment variables for subsequent use.
-- However --
If installing using "source install.sh" or ". install.sh", the environment 
variables will be available within the shell after completion of the 
installation.

===============================================================================

