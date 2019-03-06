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

- [Install Bash_IT](#prod_cli)
- [Installing AUV7](#soft_auv7)

### <a name="prod_cli"></a> Install Bash_IT

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

Then resource your `.bashrc`:

	source ~/.bashrc

## Install S.O.N.I.A. Software <a name="software"></a>

### <a name="soft_auv7"></a> Installing AUV7

Installing software is really simple, just execute the following command and enjoy the show (Be aware that at some point you might need to do some actions[ i.e press ENTER]):

	cd ~
	wget http://sonia-auv.readthedocs.org/assets/files/melodic_18.04_new/sonia-install.sh
	sudo chmod +x sonia-install.sh
	./sonia-install.sh
	# SYSTEM WILL REBOOT. AFTER IT, EXECUTE THE FOLLOWING :
	./sonia-install.sh
	
	# Then remove the file
	rm sonia-install.sh
