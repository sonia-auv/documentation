#!/usr/bin/env bash

RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
CYAN="$(tput setaf 6)"
BOLD="$(tput bold)"
RESET="$(tput sgr0)"

REMOTE_ROS_VERSION_FOLDER="melodic_18_04_new"
REMOTE_AGX_FOLDER=${REMOTE_ROS_VERSION_FOLDER}/agx
REMOTE_TX2_FOLDER=${REMOTE_ROS_VERSION_FOLDER}/tx2
REMOTE_PC_FOLDER=${REMOTE_ROS_VERSION_FOLDER}/pc
REMOTE_DEV_FOLDER=${REMOTE_ROS_VERSION_FOLDER}/dev

# Utility functions
########################################################################################################

function print_sonia_logo() {
echo "${CYAN}"
cat << "EOF"
####################################################################################################

MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN/oyNMMMMMMMMMMMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMmhso+++++++++++++oydNMs +/-omMMMMMMMMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNhys++++o++//////+osyhddysMN`/MMy-:hMMMMMMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMy+ossyhmNNMMMMMMMNmdhso/-.:+y.-MMMMd/-yMMMMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNyyymMMMNmmddddmmNMMMMMMMMMMMNdho:`-ohNMMNs/+dMMMMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMmyhNho++//+oooooo+/:---:+shNMMMMNmddhhyo+hMMMMNy++dMMMMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMyho+ohNMMMMMMMMMMMMMMMMMNdyo/-:odMMMMMhyyhNMMMMMMMNo-sNMMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMmo+mMMMMMMMMMMMMMMMMMMMMMMMMMMMNho:/yNMMMMNyoohMMMMMMMo.dMMMMM
MMMMMs+//////////+yMMMMh+//////////+sNM:MNooooodMMMMMhoohMMMhoooMMMMMMMN+do+yNMMMMMMh+/+sdMMmy.hMMMM
MMMM+   :::::::::::yMMh   -:::::::`  :MdNm      +MMMM/  +MMMo  `MMMMMMm. .+`+++osNMMMMMhy:dMMyh.NMMM
MMMM/  `ddddddddddMMMMy   mMMMMMMM-  -MMMm   s   .dMM/  +MMMo  `MMMMMd`  `dy+oymNNMMMMMMMs.dMhd oMMM
MMMMh`            `hMMy   mMMMMMMM-  -MMMm   Md.   oM/  +MMMo  `MMMMh`  `d-- .:/+mMMMMMMMMd.-.d `NMM
MMMMMMmmmmmmmmmm   oMMy   mMMMMMMM-  -MMMm   MMM+   --  -+++-  `+++-    /`      :-:smMMMMMMMs`-o`yMM
MMMMy........... -/hMMd   ........``/sMMMm   MMdho`    .:+//- -.:::   `yyyy`.yo   ./.`-/oyhmMNy+s+MM
MMMMMdsssssssssssmMMMMMmsssssssssssyMMMMMNsssdossshsssyMMMMMdyMMMNyyys+:.-+`-MMhyys-+hho/-.``/++/mMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMhyydMMNdhyyssssyyhdmMMMMMMMMMNmdho:`:odMMMdo/smMMMNmNMMMMM
MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMhymho+++osyyhhhhyyso+/---/ohmMMMMNddhhdyshMMMMMmo/sNMMMMMMMM

####################################################################################################
EOF
echo "${RESET}"
echo
}

function error() {
    echo
    echo "${RED}${BOLD}ERROR${RESET}${BOLD}: $1${RESET}"
    echo
    exit 1
}
function usage() {

    echo "S.O.N.I.A - AUV Environement installation script"
    echo
    echo "./sonia_install.sh"
    echo "\t-h --help"
    echo "\t--part=$INSTALL_PART"
    echo
}

########################################################################################################


# Environement installation functions
########################################################################################################


function install_dev_environment() {

    if [ -z "$SONIA_WORKSPACE_ROOT" ]; then
        sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
        sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key 421C365BD9FF1F717815A3895523BAEEB01FA116
        
        sudo apt update -y && sudo apt dist-upgrade -y
        sudo apt install -y -f make \
            gcc \
            cmake \
            git \
            vim \
            ros-melodic-desktop-full  \
            lm-sensors \
            libglade2-dev \
            libxext-dev \
            libqwt-dev \
            libdc1394-22-dev \
            libarmadillo-dev \
            libmlpack-dev \
            python-pip
            
        ## get every file in dev and common folder
        wget -r --no-parent http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/dev/
        wget -r --no-parent http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/common/
            
        source ~/.bashrc
        source /opt/ros/melodic/setup.bash
            
        ## ADD ssh key on github before pulling git
        ssh-keygen -t rsa -b 4096 -C "you@email.ext"
        eval "$(ssh-agent -s)"
        ssh-add ~/.ssh/id_rsa

        cat ~/.ssh/id_rsa.pub
        echo "Set your github account than press enter to reboot your PC"
        read test
        reboot
    else
        ## fetch the SONIA repository
        git clone git@github.com:sonia-auv/ros_sonia_ws.git $ROS_SONIA_WS
        cd $ROS_SONIA_WS
        git checkout develop
        $ROS_SONIA_WS/git_update.sh
        sudo ldconfig
        
        ## config ROS
        sudo rosdep init
        rosdep update
        
        ## compilation
        catkin_make -j8 -DCMAKE_CXX_FLAGS="-O2"
        source devel/setup.bash
    fi
    
}

function install_jetson_agx_environment() {
    if [ -z "$SONIA_WORKSPACE_ROOT" ]; then
        ## Add source to ROS melodic
        sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
        sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key 421C365BD9FF1F717815A3895523BAEEB01FA116

        #Install basic system dependcies and upgrades
        sudo apt update -y && sudo apt upgrade -y
        sudo apt install -y -f make \
            gcc \
            cmake \
            git \
            vim \
            ros-melodic-desktop-full  \
            lm-sensors \
            libglade2-dev \
            libxext-dev \
            libqwt-dev \
            libdc1394-22-dev \
            libarmadillo-dev \
            libmlpack-dev \
            python-pip \
            ibhdf5-serial-dev \
            hdf5-tools
            zlib1g-dev \
            zip \
            libjpeg8-dev \
            libhdf5-dev
        
        ## get every file in dev and common folder
        wget -r --no-parent http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/agx/
        wget -r --no-parent http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/common/
        
        ## install python package for Tensorflow
        pip install -U pip
        sudo pip install -U numpy grpcio absl-py py-cpuinfo psutil portpicker grpcio six mock requests gast h5py astor termcolor
        pip install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v411 tensorflow-gpu

        ## Install DALSA GiGe API Framework
        sudo tar zxvf GigE-V-Framework_JetsonTX1_2.10.2.0158.tar.gz
        cd DALSA
        ./corinstall

        ## install the bash script
        echo "if [ -f ~/.bash_sonia ]; then" >> ~/.bashrc
        echo "  . ~/.bash_sonia" >> ~/.bashrc
        echo "fi" >> ~/.bashrc

        source ~/.bashrc
        source /opt/ros/melodic/setup.bash

        ## ADD ssh key on github before pulling git
        ssh-keygen -t rsa -b 4096 -C "you@email.ext"
        eval "$(ssh-agent -s)"
        ssh-add ~/.ssh/id_rsa

        cat ~/.ssh/id_rsa.pub
        echo "Set your github account than press enter to reboot your PC"
        read test
        reboot
    else
        ## fetch the SONIA repository
        git clone git@github.com:sonia-auv/ros_sonia_ws.git $ROS_SONIA_WS
        cd $ROS_SONIA_WS
        git checkout develop
        $ROS_SONIA_WS/git_update.sh
        sudo ldconfig
        
        ## config ROS
        sudo rosdep init
        rosdep update
        
        ## compilation
        catkin_make -j8 -DCMAKE_CXX_FLAGS="-O2"
        source devel/setup.bash
    fi






}

########################################################################################################


# Main script
########################################################################################################

while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        --part)
            INSTALL_PART=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

if [ "${INSTALL_PART}" -lt 1 ] || [ "${INSTALL_PART}" -gt 2 ] ; then
    # TODO: Validate why this is not working (Error msg)
    error || "Installation script part attribute value must be 1 or 2"
fi


print_sonia_logo

echo "Select target environment installation by entring the number of the element followed by [ENTER]:"
echo
echo "1) Development PC"
echo "2) Jetson AGX"
echo

read TARGET_ENV


case $TARGET_ENV in
    "1")
        install_dev_environment $INSTALL_PART
        ;;
    "2")
        install_jetson_agx_environment $INSTALL_PART
        ;;

esac

########################################################################################################
