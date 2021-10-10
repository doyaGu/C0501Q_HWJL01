Build environment for Linux.

To build projects on Ubuntu, you need

1. Install following package on Ubuntu.

   sudo apt-get install cmake
   sudo apt-get install qtbase5-dev

   sudo apt-get install git p7zip-full (option)

Notes: If you do not need the graphical configuration interface, and just need
to build code on the Linux environment. It only requires following package to
build it.

   sudo apt-get install cmake libc6-i386

2. install toolchains to /opt/ITEGCC

3. Run projects on build/linux/*.sh

   For example, run build/linux/doorbell_indoor_al.sh to indoor project.

   TO autobuild the project,

   export AUTOBUILD=1
   build/linux/doorbell_indoor_all.sh


PS: It's fully test on Ubuntu 14.04 64-bit linux version. For othres Linux
distribution, you need install proper shared libraries.

