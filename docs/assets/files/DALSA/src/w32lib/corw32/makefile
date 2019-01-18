#-----------------------------------------------------------------------------
# MAKEFILE               						
#					Copyright(C) Coreco Inc. 2003
#						All rights reserved.
#
# Description: 
#       CorW32 Library for Linux (Posix/Win32 compatibility layer).
#
# 		CorW32 is a shared library
#
# History:
#  $Log: makefile $
#  Revision 1.6  2006/03/07 09:50:36  PARHUG
#  Removed "-lc" link to C-run-time library. This is so that those systems with 
#  C runtime integrated into a C++ library do not end up with 2 different C 
#	runtimes linked to the same program.
#  Revision 1.5  2005/01/20 14:28:39
#  - Put back EXTRALIBS
#  Revision 1.4  2005/01/20 12:14:06
#  - Comment out EXTRALIBS (not needed for create *_a libraries)
#  Revision 1.3  2005/01/07 11:39:55 
#  Added -lm -ldl -lc to library dependencies. All Sapera libraries now depend on and link to CorW32.
#  Revision 1.2  2004/10/20 14:12:29
#  Add pthread library to link.
#  Revision 1.1  2004/08/19 12:26:09
#  Initial revision
#  Initial revision
#
# Initial revision
#-----------------------------------------------------------------------------
#
# Standard dependencies.
#
DEBUGFLAGS=-g
CC=gcc
RM=/bin/rm -f
RANLIB=ranlib
LD=ld
LDCONFIG=/sbin/ldconfig

INC_PATH = -I../include


#
# API Version control information to be embedded.
#
TARGETNAME=CorW32
LIBVERSION=1
LIBBUILDID=0

#
# Path dependency for this library.
#
CURDIR = $(shell pwd)
VPATH = $(CURDIR)

#
# Object definitions: Specific files for this library.
#
OBJS= corw32lib.o \
	dynaqueue.o \
	fileutils.o \
	iniutils.o \
	objutils.o \
	ioctlutils.o \
	csection.o \
	timeutils.o \
	ipcutils.o \
	threadutils.o \
	waitutils.o \
	archutils.o 

#
# Libraries required.
#
EXTRALIBS=-lpthread -lm -ldl -lrt

#
# Dynamic libraries need position independent code.
#
C_COMPILE_OPTIONS= $(DEBUGFLAGS) -fPIC -fhosted -Wall -Wno-parentheses -Wno-missing-braces \
	-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label -Wno-unused-variable -Wno-unused-but-set-variable

# c++ compiler options.
CXX_COMPILE_OPTIONS= $(DEBUGFLAGS) -fPIC -fno-for-scope -fno-const-strings -Wall \
	-Wno-parentheses -Wno-missing-braces -Wno-unknown-pragmas -Wno-cast-qual \
	-Wno-unused-function -Wno-unused-label -Wno-unused-variable -Wno-unused-but-set-variable

#BASE_OPTIONS= -DPOSIX_HOSTPC -DHOSTPC -DCOR_TRACE_LIBINIT 
BASE_OPTIONS= -DPOSIX_HOSTPC -DHOSTPC 

# See if this is 2.6.18 or later (named posix semaphores - process shared)
# (Hopefully kernel is name 2.6.###-### and not 2.6.###text )
CORKERNEL_MAJOR:=$(shell uname -r | cut -d '.' -f 1)
CORKERNEL_MINOR:=$(shell uname -r | cut -d '.' -f 2)
CORKERNEL_LEVEL_TMP:=$(shell uname -r | cut -d '.' -f 3)
CORKERNEL_LEVEL:=$(shell echo $(CORKERNEL_LEVEL_TMP) | cut -d '-' -f 1)
#CORKERNEL_LEVEL:=$(shell echo $(CORKERNEL_LEVEL_TMP) | cut -c '1-2')

ifeq ($(CORKERNEL_MAJOR), 2)
	ifeq ($(CORKERNEL_MINOR), 6)
		ifeq ($(shell if test $(CORKERNEL_LEVEL) -ge  18; then echo ok; fi), ok)
			#BASE_OPTIONS += -DUSE_POSIX_SEMAPHORES
		endif
	endif
endif

ifndef ARCH
  ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/x86_64/x86_64/ )
endif

ifeq  ($(ARCH), x86_64)
	ARCH_OPTIONS= -Dx86_64 -D_REENTRANT
else
	ifeq  ($(ARCH), i386)
		ARCH_OPTIONS= -Dx86_64 -D_REENTRANT
	else
# Force an error
$(error	Unknown architecture ***)
	endif
endif

#
# Use the monotonic clock if it is present.
#
ifeq ($(shell if test -f /usr/include/linux/time.h; then echo exists; fi), exists)
CLOCK_MONOTONIC_PRESENT = $(shell grep CLOCK_MONOTONIC /usr/include/linux/time.h)
	ifneq ($(CLOCK_MONOTONIC_PRESENT),)
		BASE_OPTIONS += -DUSE_MONOTONIC_CLOCK
	endif
endif


.SUFFIXES: .o .c .cpp

%.o : %.c
	$(CC) -I. $(INC_PATH) $(C_COMPILE_OPTIONS) $(BASE_OPTIONS) $(TARGET_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@

%.o : %.cpp
	$(CC) -I. $(INC_PATH) $(CXX_COMPILE_OPTIONS) $(BASE_OPTIONS) $(TARGET_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@


#
# Define the installation paths
#
INSTALL_PREFIX	= /usr/local
BINDIR	= $(INSTALL_PREFIX)/bin
MANDIR	= $(INSTALL_PREFIX)/man
LIBDIR	= $(INSTALL_PREFIX)/lib
INCDIR	= $(INSTALL_PREFIX)/include
CNFDIR	= $(INSTALL_PREFIX)/share


LIBNAME=lib$(TARGETNAME)_$(LIBVERSION)_$(LIBBUILDID)_a
SONAME=lib$(TARGETNAME).so
SHAREDLIBLINK=$(SONAME).$(LIBVERSION)
SHAREDLIBNAME= $(SONAME).$(LIBVERSION).$(LIBBUILDID)

#
# Main target creation section.
#
all:	$(LIBNAME) $(SHAREDLIBNAME)

release:	$(SHAREDLIBNAME)

$(SHAREDLIBNAME): $(OBJS)
	$(CC) -shared -Wl,-Bsymbolic -Wl,-allow-shlib-undefined -Wl,-soname,$(SONAME).$(LIBVERSION) $(LINK_OPTIONS) -o $(SHAREDLIBNAME) $(OBJS) $(EXTRALIBS)


$(LIBNAME): $(OBJS)
	$(LD) -r $^ -o $@

clean: distclean
	-$(RM) $(OBJS) *~ *.bak

distclean: 
	-$(RM) $(SHAREDLIBNAME)
	-$(RM) $(LIBNAME)

relink:
	-$(RM) $(SHAREDLIBNAME)

install: all
	cp $(SHAREDLIBNAME) $(LIBDIR)/$(SHAREDLIBNAME)
	-$(RM) $(LIBDIR)/$(SHAREDLIBLINK)
	ln -s $(LIBDIR)/$(SHAREDLIBNAME) $(LIBDIR)/$(SHAREDLIBLINK)
	-$(LDCONFIG) -l $(LIBDIR)/$(SHAREDLIBNAME)
	-$(RM) $(LIBDIR)/$(SONAME)
	ln -s $(LIBDIR)/$(SHAREDLIBLINK) $(LIBDIR)/$(SONAME)





