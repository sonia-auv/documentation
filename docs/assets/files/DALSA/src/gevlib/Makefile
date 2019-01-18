#-----------------------------------------------------------------------------
# MAKEFILE               						
#					Copyright(C) Teledyne DALSA Inc 2009-2011
#						All rights reserved.
#
# Description: 
#       GigE-V Framework API Library for Linux (x86 architectures)
#
# 		GigE-V Framework API is a shared library
#
#-----------------------------------------------------------------------------
#
# Standard dependencies.
#
CC=gcc
RM=/bin/rm
RANLIB=ranlib
LD=ld
LDCONFIG=/sbin/ldconfig

#
# Architecture specific environment defs for self-hosted environment 
#
ifndef ARCH
  ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/x86_64/x86_64/)
endif

ifeq  ($(ARCH), x86_64)
	ARCHNAME=x86_64
	ARCH_GENICAM_BIN=Linux64_x64
	ARCH_OPTIONS= -Dx86_64 -D_REENTRANT
	ARCH_GCCVER=421
else
	ARCHNAME=i386
	ARCH_GENICAM_BIN=Linux32_i86
	ARCH_OPTIONS= -D__i386__ -D_REENTRANT
	ARCH_GCCVER=421
endif


#
# Arch dependent GenICam library specification
#
GENICAM_PATH_VERSION=v3_0
GENICAM_PATH:=$(GENICAM_ROOT_V3_0)
GENICAM_LIBS=-L$(GENICAM_PATH)/bin/$(ARCH_GENICAM_BIN)\
					-lGenApi_gcc$(ARCH_GCCVER)_$(GENICAM_PATH_VERSION)\
					-lGCBase_gcc$(ARCH_GCCVER)_$(GENICAM_PATH_VERSION)

INC_PATH = -I. -I../../GigeV/include -I$(GENICAM_PATH)/library/CPP/include
#
# API Version control information to be embedded.
#
TARGETNAME=GevApi
LIBVERSION=2
LIBBUILDID=0

#
# Path dependency for this library.
#
CURDIR = $(shell pwd)
VPATH = $(CURDIR)

#
# Object definitions: Specific files for this library.
#
OBJS= gev_linux.o \
	cameraregdata.o \
	gev_message_linux.o \
	gev_packet_mmap.o \
	gev_packet_linux.o \
	gev_xml_utils.o \
	gevapi_utils.o \
	gevgenapi.o

#
# Libraries required.
#
EXTRALIBS=-L/usr/local/lib -lCorW32 $(GENICAM_LIBS)

#
# Dynamic libraries need position independent code.
#
C_COMPILE_OPTIONS= $(DEBUGFLAGS) -fPIC -fhosted -Wall -Wno-parentheses -Wno-missing-braces \
	-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label

# c++ compiler options.
CXX_COMPILE_OPTIONS= $(DEBUGFLAGS) -fPIC -fno-for-scope -Wall \
	-Wno-parentheses -Wno-missing-braces -Wno-unknown-pragmas -Wno-cast-qual \
	-Wno-unused-function -Wno-unused-label

BASE_OPTIONS= -DPOSIX_HOSTPC -DHOSTPC  -D_USE_GNU

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

GENIE_DIR = ../../GigeV
GENIE_LIB_DIR = $(GENIE_DIR)/lib

LIBNAME=lib$(TARGETNAME)_$(LIBVERSION)_$(LIBBUILDID)_$(ARCHNAME)_a
SONAME=lib$(TARGETNAME).so
SHAREDLIBLINK=$(SONAME).$(LIBVERSION)
SHAREDLIBNAME= $(SONAME).$(LIBVERSION).$(LIBBUILDID)

#
# Main target creation section.
#
all:	$(SHAREDLIBNAME)

$(SHAREDLIBNAME): $(OBJS)
	$(CC) -shared -Wl,-Bsymbolic -Wl,-allow-shlib-undefined \
	-Wl,-soname,$(SONAME).$(LIBVERSION) $(LINK_OPTIONS) \
	-o $(GENIE_LIB_DIR)/$(SHAREDLIBNAME) \
	$(OBJS) $(GENIE_LIB_DIR)/$(LIBNAME) $(EXTRALIBS)

clean: 
	-$(RM) $(GENIE_LIB_DIR)/$(SHAREDLIBNAME) $(OBJS) *~ *.bak

install: all
	cp $(GENIE_LIB_DIR)/$(SHAREDLIBNAME) $(LIBDIR)/$(SHAREDLIBNAME)
	-$(RM) $(LIBDIR)/$(SHAREDLIBLINK)
	ln -s $(LIBDIR)/$(SHAREDLIBNAME) $(LIBDIR)/$(SHAREDLIBLINK)
	-$(LDCONFIG) -l $(LIBDIR)/$(SHAREDLIBNAME)
	-$(RM) $(LIBDIR)/$(SONAME)
	ln -s $(LIBDIR)/$(SHAREDLIBLINK) $(LIBDIR)/$(SONAME)

