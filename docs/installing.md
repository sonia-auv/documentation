Installing
==========

This document provides all the steps for configuring your environment.

There is two kind of environment to configure.
You can either configure a production environment, which is the environment that is running on the submarine.
Or you can also configure a development environment, that is going to help you to develop our softwares.

Once you have correctly installed and configured you environment, you are ready to use our softwares.
In order to simplify the usage of S.O.N.I.A. Software, we are using a single git repository that regroup all the modules that we use as git submodule.

This repository is located at `git@github.com:sonia-auv/ros_sonia_ws.git` and you will have to be part of the SONIA organisation to access it. If it is not the case yet, please ask an administrator to add you.
This repository is composed of several branches:

- `ros_sonia_ws:core` - Barbones environment for running the submarine
- `ros_sonia_ws:desktop` - Full environment for development.

You will find the details on how to install and configure you environment on all the specific sections:

- [Configure Production Environment](#S-Production)
- [Configure Development Environment](#S-Development)
- [Install our softwares](#S-Software)

Configure Production Environment
--------------------------------


### Use `root` as default user

Set the password of the `root` user:

    sudo su –
	passwd
	userdel sonia
	exit
	exit

Then login with `root` and delete the user `sonia`:

	userdel sonia
	rm -rf /home/sonia

### Installing the dependencies

Add the partner repositories in the source list:

	sed -i '/http:\/\/archive.canonical.com\/ubuntu trusty partner/s/^# //g' /etc/apt/sources.list

You can now install the upgrade and required packages:

	aptitude update
	aptitude upgrade -y
	aptitude dist-upgrade -y
	aptitude install -y \
	    make\
	    cmake\
	    openssh-server\
	    git\
	    xautomation\
	    tig\
	    lm-sensors\
	    tree

### Install Devices Modules

Type this command in order to list the modules needed by your system:

	sensors-detect

Add the recommended modules into your `/etc/modules`.

You can now check the temperature of the CPU by using `sensors` command.

### Installing the Network

Configure the network interface to use the IP `192.168.0.11/24`
Open the file `/etc/network/interfaces`, it should look like this:

	# This file describes the network interfaces available on your system
	# and how to activate them. For more information, see interfaces(5).
	
	# The loopback network interface
	auto lo
	iface lo inet loopback
	
	# The primary network interface
	auto em1
	iface em1 inet static
	  address 192.168.0.11
	  gateway 192.168.0.1
	  netmask 255.255.255.0
	  network 192.168.0.0
	  broadcast 192.168.0.255
	  dns-nameservers 8.8.8.8

You can now restart the `networking` service and restart network interfaces:

	/etc/init.d/networking restart
	ifdown em1
	ifup em1

Now configure the SSH

	cp /etc/ssh/sshd_config /etc/ssh/sshd_config.original
	chmod a-w /etc/ssh/sshd_config.original
	sed -i -- 's/PermitRootLogin without-password/PermitRootLogin yes/g'\
	    /etc/ssh/sshd_config
	service ssh restart

### Installing Git

	ssh-keygen -t rsa -b 4096 -C "sonia@ens.etsmtl.ca"
	eval "$(ssh-agent -s)"
	ssh-add ~/.ssh/id_rsa
	git config --global user.name "S.O.N.I.A. Guest"
	git config --global user.email sonia@ens.etsmtl.ca

You can now add the content of ~/.ssh/id_rsa.pub to your Github/Gitlab: `cat ~/.ssh/id_rsa.pub`

### Installing the drivers

	cd ~
	wget http://www.kvaser.com/software/7330130980754/V5_12_0/linuxcan.tar.gz
	tar zxvf linuxcan.tar.gz
	sudo mv linuxcan/ /opt/
	cd linuxcan
	make
	sudo make install
	cd ~
	rm -r linuxcan.tar.gz linuxcan

### Setup directories

Create a directory called `Workspace` in your home directory and link `/sonia` to it:

	cd ~
	mkdir Workspace
	ln -sf ~/Workspace /sonia

### Configure CLI and aliases

Now install bash_it in order to have a better command line interface:

	git clone --depth=1 https://github.com/Bash-it/bash-it.git ~/.bash_it
	~/.bash_it/install.sh
	rm ~/.bashrc.bak
	sed -i -e 's/bobby/nwinkler/g' ~/.bashrc

Now edit your `~/.bashrc` and add the following configuration:

	if ! shopt -oq posix; then
	  if [ -f /usr/share/bash-completion/bash_completion ]; then
	    . /usr/share/bash-completion/bash_completion
	  elif [ -f /etc/bash_completion ]; then
	    . /etc/bash_completion
	  fi
	fi
	
	# Load common aliases
	if [ -f ~/.bash_aliases ]; then
	    . ~/.bash_aliases
	fi
	
	# Load SONIA Configuration
	if [ -f ~/.bash_sonia ]; then
	    . ~/.bash_sonia
	fi

You can now add the file `~/.bash_aliases` with the following configuration:

	# Aliases Configuration
	
	# some more ls aliases
	alias ll='ls -alF'
	alias la='ls -A'
	alias l='ls -CF'
	
	# My aliases
	alias ..='cd ..'
	alias port='netstat -natp'
	alias resource="source ~/.bashrc"

And then add the file `~/.bash_sonia` with this configuration:

	# SONIA Configuration
	
	## ROS specific configurations
	source /opt/ros/indigo/setup.bash
	if [ -f $ROS_SONIA_WS/devel/setup.bash ]; then
	    source 
	fi
	
	## Environment variables
	export SONIA_WORKSPACE_ROOT=~/Workspace/
	export ROS_SONIA_WS=$SONIA_WORKSPACE_ROOT/ros_sonia_ws/
	export AUV6_SONIA_WS=$SONIA_WORKSPACE_ROOT/sonia/
	export SONIA_SCRIPTS=$AUV6_SONIA_WS/sonia_scripts/
	export ROS_HOSTNAME=localhost
	export ROS_MASTER_URI=http://localhost:11311/
	export LC_ALL="en_US.UTF-8"
	export EDITOR="vim"
	
	## Working with screens
	alias sa="cd /sonia/auv6; screen -m -d -S auv6 ./auvServer.sh; cd -"
	alias aa="screen -x auv6"
	
	alias sc="cd /sonia/can-server; screen -m -d -S canserver ./cserver; cd -"
	alias ac="screen -x canserver"
	
	alias sv="cd /sonia/sonia-scripts; screen -m -d -S start_ros sh vision-init/start_ros_vision.sh;cd -"
	alias av="screen -x vision"
	alias kv="screen -S start_ros -X quit;screen -S vision -X quit;screen -S communicator -X quit;"
	
	alias ar="screen -x roscore"
	alias avc="screen -x communicator"
	
	alias smcompress="screen -m -d -S mcompress java -jar \
	    /sonia/movie-compressor/target/movieCompressor-jar-with-dependencies.jar $1"
	alias amcompress="screen -x mcompress"
	
	## Useful aliases
	alias sonia-source='source ${ROS_SONIA_WS}/devel/setup.bash'
	alias sonia-cd='cd ${ROS_SONIA_WS}'
	alias sonia-dockbox='ssh root@192.168.0.10'
	alias poof="killall -s SIGTERM java;killall -s SIGTERM vserver;poweroff"
	alias repoof="killall -s SIGTERM java;killall -s SIGTERM vserver;reboot"
	alias wdmesg="watch \"dmesg | tail -n 30\""
	alias catkin_make='catkin_make -j8 -DCMAKE_BUILD_TYPE=Release'
	alias rusb="sh $SONIA_SCRIPTS/rebootUSB.sh"
	alias auvc='cd $SONIA_WORKSPACE_ROOT/auv6; mvn clean install -o;cd -'
	alias sup="cd $SONIA_SCRIPTS; ./update_auv_dependencies.sh; cd -"
	alias sbu="cd /$SONIA_SCRIPTS; ./build_auv_dependencies.sh; cd -"
	
	alias logpack='mkdir $SONIA_WORKSPACE_ROOT/log/package 2>>/dev/null;find \
	    . \( \! -iname "*\.raw*" \) -type f|xargs -n1 -i cp -f \
	    --parents {} $SONIA_WORKSPACE_ROOT/log/package/'
	
	$SONIA_SCRIPTS/checkCores.sh
	$SONIA_SCRIPTS/checkDisks.sh
	$SONIA_SCRIPTS/checkCamera.sh
	$SONIA_SCRIPTS/check-svn.sh


Configure Development Environment
---------------------------------


Install S.O.N.I.A. Software
---------------------------

### Installing AUV6

First of all, install all AUV6 dependencies:

	 aptitude install -y openjdk-7-jdk scons build-essential maven2

You can now clone the AUV6 script repository and launch the install script:

	mkdir -p $AUV6_SONIA_WS
	cd $AUV6_SONIA_WS
	git clone ssh://git@sonia.etsmtl.ca:4223/logiciel/sonia-scripts.git
	sh sonia-scripts/git_update_repo.sh

You must change your maven `setttings.xml` file for adding our repo/mirrors:

	mkdir -p ~/.m2
	wget http://sonia.etsmtl.ca:120/soniapedia/images/5/51/Settings.xml \
	    -O ~/.m2/settings.xml
	
You can now build the CAN server:

	cd $AUV6_SONIA_WS/can-server
	scons

And now build the whole AUV6 system:

	cd $AUV6_SONIA_WS/sonia-jaus-library
    mvn clean install
    cd $AUV6_SONIA_WS/octets-common
    mvn clean install
    cd $AUV6_SONIA_WS/auv-mission-library
    mvn clean install
    cd $AUV6_SONIA_WS/auv6
	mvn clean install

That's it ! You can now work with AUV6

### Installing AUV7

First of all, you must install ROS and its dependencies:

	sudo sh -c \
	    'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" >\
	    /etc/apt/sources.list.d/ros-latest.list'
	sudo apt-key adv --keyserver hkp://pool.sks-keyservers.net:80 --recv-key 0xB01FA116
	sudo apt-get update

We recommend that you install the desktop-full ditribution of ROS even if you do not use the GUI provided. This distribution provides several packages that we may use in our softwares.

	sudo apt-get install -y \
	    ros-indigo-desktop-full\
	    python-rosinstall
	sudo rosdep init
	rosdep update
	source /opt/ros/indigo/setup.bash

You can now clone the AUV7 workspace repository and build it:

	mkdir -p $ROS_SONIA_WS
	git clone git@github.com:sonia-auv/ros_sonia_ws.git $ROS_SONIA_WS
	cd $ROS_SONIA_WS
	git submodule init
	git submodule update --remote --rebase
	catkin_make
	source devel/setup.bash

That's it ! You can now work with AUV7

### Install ROS Java

When you have installed AUV6 and AUV7, you may want to use AUV6 with AUV7. We are using `rosjava` in order to have an interface between the two software suite.

You will have to install few packages that allows us to build rosjava packages as java `.jar`:

	aptitude install -y ros-indigo-rosjava
	source /opt/ros/indigo/setup.bash

We provide a git repository with our messages and scripts that configure `rosjava` without complication. You can perform these tasks in order to install the system with our solution:

	cd $SONIA_WORKSPACE_ROOT
	git clone https://github.com/sonia-auv/rosjava_ws.git
	cd rosjava_ws
	./install.sh

