### <a name="dev_eclipse"></a> Working with Eclipse

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

  ### <a name="soft_auv6"></a> Installing AUV6

  First of all, install all AUV6 dependencies:

  	 aptitude install -y openjdk-7-jdk scons build-essential maven2

  You can now clone the AUV6 script repository and launch the install script:

  	mkdir -p $SONIA_WORKSPACE_ROOT
  	cd $SONIA_WORKSPACE_ROOT
  	git@bitbucket.org:sonia-auv/sonia-scripts.git
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
