# <a name="title"></a> Installation

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

- [Configure Production Environment](#production)
	- [Use `root` as default user](#prod_user)
	- [Installing the dependencies](#prod_deps)
	- [Install Devices Modules](#prod_modules)
	- [Configure Log disk](#prod_disk)
	- [Installing the Network](#prod_network)
	- [Installing Git](#prod_git)
	- [Installing the Drivers](#prod_drivers)
	- [Setup directories](#prod_dirs)
	- [Disable few startup checks](#prod_checks)
	- [Configure kernel core dumps](#prod_cores)
	- [Configure CLI and aliases](#prod_cli)
	- [Loading scripts](#prod_scripts)
- [Configure Development Environment](#development)
    - [Installing the dependencies](#dev_deps)
    - [Installing the drivers](#dev_drivers)
    - [Installing Git](#dev_git)
    - [Working with Eclipse](#dev_eclipse)
- [Install our softwares](#software)
    - [Installing AUV6](#soft_auv6)
    - [Installing AUV7](#soft_auv7)

## Configure Production Environment <a name="production"></a>

### <a name="prod_user"></a> Use `root` as default user

Set the password of the `root` user:

	sudo passwd root
	sudo usermod -U root
	sudo bash -c "echo -e \"[SeatDefaults]\ngreeter-show-manual-login=true\" >> /etc/lightdm/lightdm.conf"cd

Then login with `root` and delete the user `sonia`:

	userdel sonia
	rm -rf /home/sonia

### <a name="prod_deps"></a> Installing the dependencies

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
	    coriander\
	    xserver-xorg-lts-utopic\
	    vim\
	    tree

### <a name="prod_modules"></a> Install Devices Modules

Type this command in order to list the modules needed by your system:

	sensors-detect

Add the recommended modules into your `/etc/modules`.

You can now check the temperature of the CPU by using `sensors` command.

### <a name="prod_disk"></a> Configure Log disk

Fist, mount the disk and create a symlink to the mount point:

	ln -s $HOME /sonia
	mkdir -p /sonia/log
	mount /dev/sdb1 /sonia/log

In order to configure the log disk, simply add this line at the end of you `/etc/fstab` file:

	echo "/dev/sdb1    /sonia/log    ext4    errors=remount-ro    0    0" >> /etc/fstab

### <a name="prod_network"></a> Installing the Network

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

### <a name="prod_git"></a> Installing Git

	ssh-keygen -t rsa -b 4096 -C "sonia@ens.etsmtl.ca"
	eval "$(ssh-agent -s)"
	ssh-add ~/.ssh/id_rsa
	git config --global user.name "S.O.N.I.A. Guest"
	git config --global user.email sonia@ens.etsmtl.ca
	git config --global push.default simple

You can now add the content of `~/.ssh/id_rsa.pub` to your Github/Gitlab: `cat ~/.ssh/id_rsa.pub`

We also need to setup a git server in order to be able to push changes on the submarine when there is no internet access. To do so, create a git user and setup the `.ssh` directory:

	touch .ssh/authorized_keys && chmod 600 .ssh/authorized_keys
	
When this has been done, you can add your ssh key to the server. If your public key is located in `~/.ssh/id_rsa.pub`, the following command will add your key on the submarine:

	cat ~/.ssh/id_rsa.pub | ssh root@192.168.0.11 'dd of=.ssh/authorized_keys oflag=append conv=notrunc'
	
Finally, on the production environment, you will have to enable pushing on non-bar repository, do to so, type:

	git config --global receive.denyCurrentBranch warn

### <a name="prod_drivers"></a> Installing the drivers

	cd /tmp/
    wget http://www.kvaser.com/software/7330130980754/V5_12_0/linuxcan.tar.gz
    tar zxvf linuxcan.tar.gz
	cd ./linuxcan
	make -j8
	make install
    cd ../
    rm -r linuxcan linuxcan.tar.gz
	
During the installation of the CAN Libraries, if you have any trouble with the `-Werror=date-time` CFLAG, you can delete these line as a workaround :

	sed -i -e '2293d' ./leaf/leafHWIf.c
	sed -i -e '2293d' ./leaf/leafHWIf.c
	sed -i -e '2123d' ./mhydra/mhydraHWIf.c
	sed -i -e '2123d' ./mhydra/mhydraHWIf.c
	sed -i -e '1757d' ./usbcanII/usbcanHWIf.c
	sed -i -e '1757d' ./usbcanII/usbcanHWIf.c
	sed -i -e '900d' ./pciefd/pciefd_hwif.c

### <a name="prod_dirs"></a> Setup directories

Create a directory called `Workspace` in your home directory and link `/sonia` to it:

	cd ~
	mkdir sonia_ws
    mkdir ros_sonia_ws
	
### <a name="prod_checks"></a> Disable few startup checks

In order to disable the partition checks on startup, type this:

	tune2fs -c 0 /dev/sda1
	tune2fs -c 0 /dev/sdb1

### <a name="prod_cores"></a> Configure kernel core dumps

For configuring the kernel core dumps, simply use these commands:

	mkdir -p /sonia/cores
	echo "kernel.core_pattern = /sonia/cores" >> /etc/sysctl.conf

### <a name="prod_cli"></a> Configure CLI and aliases

Now install bash_it in order to have a better command line interface:

	git clone --depth=1 https://github.com/Bash-it/bash-it.git ~/.bash_it
	~/.bash_it/install.sh
	rm ~/.bashrc.bak
	sed -i -e 's/bobby/nwinkler/g' ~/.bashrc

Now edit your `~/.bashrc` and add the following configuration at the beginning of the file:

	# If not running interactively, con't do anything
	case $- in
	    *i*) ;;
	      *) return;;
	esac

	if ! shopt -oq posix; then
	  if [ -f /usr/share/bash-completion/bash_completion ]; then
	    . /usr/share/bash-completion/bash_completion
	  elif [ -f /etc/bash_completion ]; then
	    . /etc/bash_completion
	  fi
	fi

And source the other bash files at the end of your file:

	# Load common aliases
	if [ -f ~/.bash_aliases ]; then
	    . ~/.bash_aliases
	fi
	
	# Load SONIA Configuration
	if [ -f ~/.bash_sonia ]; then
	    . ~/.bash_sonia
	fi

You can download the sample bashrc file here:

	wget http://sonia-auv.readthedocs.org/assets/files/bashrc -O ~/.bashrc

You can now add the file [`~/.bash_aliases`](assets/files/bash_aliases) with the following configuration:

	wget http://sonia-auv.readthedocs.org/assets/files/bash_aliases -O ~/.bash_aliases

And then add the file [`~/.bash_sonia`](assets/files/bash_sonia) with this configuration:

	wget http://sonia-auv.readthedocs.org/assets/files/bash_sonia -O ~/.bash_sonia

!!!Note
	If you are running a development environment, prefere adding the following configuration:

		wget http://sonia-auv.readthedocs.org/assets/files/bash_sonia_dev -O ~/.bash_sonia

Then resource your `.bashrc`:

	source ~/.bashrc

### <a name="prod_scripts"></a> Loading scripts

We have several scripts that must launch at startup time, here they are:

	ln -sf $SONIA_SCRIPTS/canserver.sh /etc/init.d/can-server
	ln -sf $SONIA_SCRIPTS/sonia-init.sh /etc/init.d/sonia-init
	update-rc.d sonia-init defaults 80
	update-rc.d can-server defaults 81

## Configure Development Environment <a name="development"></a>

### <a name=""></a> Installing the dependencies

Add the partner repositories in the source list:

	sed -i '/http:\/\/archive.canonical.com\/ubuntu trusty partner/s/^# //g' /etc/apt/sources.list

You can now install the upgrade and required packages:

	aptitude update
	aptitude upgrade -y
	aptitude dist-upgrade -y
	aptitude install -y \
	    make\
	    cmake\
	    git\
	    tig\
	    tree

### <a name=""></a> Installing the drivers

	cd /tmp/
    wget http://www.kvaser.com/software/7330130980754/V5_12_0/linuxcan.tar.gz
    tar zxvf linuxcan.tar.gz
	cd ./linuxcan
	make -j8
	make install
    cd ../
    rm -r linuxcan linuxcan.tar.gz
	
During the installation of the CAN Libraries, if you have any trouble with the `-Werror=date-time` CFLAG, you can delete these line as a workaround :

	sed -i -e '2293d' ./leaf/leafHWIf.c
	sed -i -e '2293d' ./leaf/leafHWIf.c
	sed -i -e '2123d' ./mhydra/mhydraHWIf.c
	sed -i -e '2123d' ./mhydra/mhydraHWIf.c
	sed -i -e '1757d' ./usbcanII/usbcanHWIf.c
	sed -i -e '1757d' ./usbcanII/usbcanHWIf.c
	sed -i -e '900d' ./pciefd/pciefd_hwif.c

### <a name=""></a> Installing Git

	ssh-keygen -t rsa -b 4096 -C "you@email.ext"
	eval "$(ssh-agent -s)"
	ssh-add ~/.ssh/id_rsa
	git config --global user.name "Your Name"
	git config --global user.email you@email.ext
	git config --global push.default simple

You can now add the content of `~/.ssh/id_rsa.pub` to your Github/Gitlab: `cat ~/.ssh/id_rsa.pub`

### <a name=""></a> Working with Eclipse

We use Eclipse for our Java development and we also need the C++ plugin that allows us to run and debug C++ code.

First, you need to download the last version on [eclipse website](https://www.eclipse.org/downloads/):

	cd
	wget http://eclipse.mirror.rafal.ca/technology/epp/downloads/release/mars/1/eclipse-java-mars-1-linux-gtk-x86_64.tar.gz
	tar zxvf eclipse-java-mars-1-linux-gtk-x86_64.tar.gz
	sudo mv eclipse /opt/eclipse

!!! note

	You will need these plugin to work with our AUV6/AUV7 software:
	
	- maven - http://download.eclipse.org/technology/m2e/releases
		- m2e - Maven Integration for Eclipse
		- m2e connector for xmlbeans
	- pydev - http://pydev.org/updates
		- pydev for eclipse
	- egit - http://download.eclipse.org/egit/updates
		- Eclipse Git Team provider
	- cdt - http://download.eclipse.org/tools/cdt/releases/8.5
		- C/C++ development tools
	- windows build - outil pour interface swing - http://dl.google.com/eclipse/inst/d2wbpro/latest/3.7
		- Swing Designer
		- WindowsBuild Engine (Required)

You can also configure Eclipse to use our formatting file. You can find it here:

	wget http://sonia-auv.readthedocs.org/assets/files/sonia-formatter.epf

## Install S.O.N.I.A. Software <a name="software"></a>

### <a name=""></a> Installing AUV6

First of all, install all AUV6 dependencies:

	 aptitude install -y openjdk-7-jdk scons build-essential maven2

You can now clone the AUV6 script repository and launch the install script:

	mkdir -p $SONIA_WORKSPACE_ROOT
	cd $SONIA_WORKSPACE_ROOT
	git clone ssh://git@sonia.etsmtl.ca:4223/logiciel/sonia-scripts.git
	sh sonia-scripts/git_update_repo.sh

You must change your maven [`settings.xml`](assets/files/settings.xml) file for adding our repo/mirrors:

	mkdir -p ~/.m2
	wget http://sonia-auv.readthedocs.org/assets/files/settings.xml -O ~/.m2/settings.xml
	    
	
You can now build the CAN server:

	cd $SONIA_WORKSPACE_ROOT/can-server
	scons

And now build the whole AUV6 system:

	cd $SONIA_WORKSPACE_ROOT/sonia-jaus-library
    mvn clean install
    cd $SONIA_WORKSPACE_ROOT/octets-common
    mvn clean install
    cd $SONIA_WORKSPACE_ROOT/auv-mission-library
    mvn clean install
    cd $SONIA_WORKSPACE_ROOT/auv6
	mvn clean install

That's it ! You can now work with AUV6

### <a name=""></a> Installing AUV7

First of all, you must install ROS and its dependencies:

	sudo sh -c \
	    'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" >\
	    /etc/apt/sources.list.d/ros-latest.list'
	sudo apt-key adv --keyserver hkp://pool.sks-keyservers.net:80 --recv-key 0xB01FA116
	sudo apt-get update

We recommend that you install the desktop-full ditribution of ROS even if you do not use the GUI provided. This distribution provides several packages that we may use in our softwares.

	sudo apt-get install -y \
	    ros-indigo-desktop-full\
	    python-rosinstall\
	    ros-indigo-rosjava
	sudo rosdep init
	rosdep update
	source /opt/ros/indigo/setup.bash

You can now clone the AUV7 workspace repository and build it:

	mkdir -p $ROS_SONIA_WS
	git clone git@github.com:sonia-auv/ros_sonia_ws.git $ROS_SONIA_WS
    cd $ROS_SONIA_WS
    ./git_update.sh -d
	catkin_make -j8
	source devel/setup.bash

That's it ! You can now work with AUV7
