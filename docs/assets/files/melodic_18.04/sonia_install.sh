#!/usr/bin/env bash

RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
CYAN="$(tput setaf 6)"
BOLD="$(tput bold)"
RESET="$(tput sgr0)"

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
    echo "DEV: TO BE IMPLEMENTED"
}

function install_pc_environment() {
    echo "PC: TO BE IMPLEMENTED"
}

function install_jetson_agx_environment() {
    echo "AGX: TO BE IMPLEMENTED"
}

function install_jetson_tx2_environment() {
    echo "TX2: BE IMPLEMENTED"
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
echo "2) Production PC"
echo "3) Jetson AGX"
echo "4) Jetson TX2"
echo

read TARGET_ENV


case $TARGET_ENV in
    "1")
        install_dev_environment $INSTALL_PART
        ;;
    "2")
        install_pc_environment $INSTALL_PART
        ;;
    "3")
        install_jetson_agx_environment $INSTALL_PART
        ;;
    "4")
        install_jetson_tx2_environment $INSTALL_PART
        ;;

esac

########################################################################################################
