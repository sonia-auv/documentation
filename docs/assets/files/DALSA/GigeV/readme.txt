Release Notes for GigE-V Framework for Linux 
Version 2.00.0.0108

For information on installation, hardware and software requirements, and 
complete API documentation, refer to the GigE-V Framework for Linux 
Programmer's Manual available on the Teledyne DALSA website at: 

https://www.teledynedalsa.com/imaging/support/

What's New?
-----------
This release provides support for feature access using GigE Vision compliant 
XML files.

Supported Hardware Platforms
---------------------------- 
 x86    : Intel/AMD 32-bit and 64-bit CPUs
 ARM7hf : 32-bit ARM7 with hard-float (hardware floating point)
 ARMsf  : 32-bit ARM with soft-float (software emulated floating point)

System Requirements
-------------------
 - Linux OS support for Gigabit NIC hardware (kernel 2.6.24 and later)
 - Support for PF_PACKET with RX_RING capability recommended for best 
    performance (usually available with the Wireshark application and/or 
    the libpcap package which is widely installed by default).
 - libcap-dev package is required to use Linux "capabilities" when running 
   as "root" is not desired.
 - libglade2-dev package is required for building and using the 
   GigE Vision Device Status tool (which uses gtk).
 - libx11-dev / libxext-dev packages are required for using the X11 display 
   window in the example programs.



