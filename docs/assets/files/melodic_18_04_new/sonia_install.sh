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
        sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
        
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
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/dev/bash_sonia -O ~/.bash_sonia
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/dev/GigE-V-Framework_x86_2.00.0.0108.gz -O ~/GigE-V-Framework_x86_2.00.0.0108.gz
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/common/bash_aliases -O ~/.bash_aliases

        echo "if [ -f ~/.bash_sonia ]; then" >> ~/.bashrc
        echo "  . ~/.bash_sonia" >> ~/.bashrc
        echo "fi" >> ~/.bashrc

        ## Install DALSA GiGe API Framework
        sudo tar zxvf GigE-V-Framework_x86_2.00.0.0108.gz
        cd DALSA
        sudo ./corinstall
            
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
        sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

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
            libhdf5-serial-dev \
            hdf5-tools \
            zlib1g-dev \
            zip \
            libjpeg8-dev \
            libhdf5-dev
        
        ## get every file in dev and common folder
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/agx/bash_sonia -O ~/.bash_sonia
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/agx/GigE-V-Framework_JetsonTX1_2.10.2.0158.tar.gz -O ~/GigE-V-Framework_JetsonTX1_2.10.2.0158.tar.gz
        wget http://sonia-auv.readthedocs.org/assets/files/melodic_18_04_new/common/bash_aliases -O ~/.bash_aliases
        
        ## install python package for Tensorflow
        sudo pip install -U numpy grpcio absl-py py-cpuinfo psutil portpicker grpcio six mock requests gast h5py astor termcolor
        pip install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v411 tensorflow-gpu

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

        echo "this is the agx setup, so don't forget to add a dhcp server, change password, run the network fix, change the hostname, disable the GUI, assign usb with udev, set an static IP adress and allow people to enter by public rsa key"
    fi






}

########################################################################################################


# Main script
########################################################################################################


print_sonia_logo

echo "Select target environment installation by entring the number of the element followed by [ENTER]:"
echo
echo "1) Development PC (default)"
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
