CC= gcc
# IROOT directory based on installed distribution tree (not archive/development tree). 
# IROOT=../..
IROOT=/usr/dalsa/GigeV

#
# Get the configured include defs file.
# (It gets installed to the distribution tree).
ifeq ($(shell if test -e archdefs.mk; then echo exists; fi), exists)
	include archdefs.mk
else
# Force an error
$(error	archdefs.mk file not found. It gets configured on installation ***)
endif

INC_PATH = -I. -I$(IROOT)/include -I$(IROOT)/examples/common $(INC_GENICAM)
                          
DEBUGFLAGS = -g 

CXX_COMPILE_OPTIONS = -c $(DEBUGFLAGS) -DPOSIX_HOSTPC -D_REENTRANT -fno-for-scope \
			-Wall -Wno-parentheses -Wno-missing-braces \
			-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label

C_COMPILE_OPTIONS= $(DEBUGFLAGS) -fhosted -Wall -Wno-parentheses -Wno-missing-braces \
		   	-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label


LCLLIBS=  -L$(ARCHLIBDIR) -lpthread -L/usr/local/lib -lGevApi -lCorW32

VPATH= . : ../common

%.o : %.cpp
	$(CC) -I. $(INC_PATH) $(CXX_COMPILE_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@

%.o : %.c
	$(CC) -I. $(INC_PATH) $(C_COMPILE_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@

all: dumpfeatures savefeatures loadfeatures c_loadfeatures

dumpfeatures : featuretree.o
	$(CC) -g $(ARCH_LINK_OPTIONS) -o dumpfeatures featuretree.o $(LCLLIBS) $(GENICAM_LIBS) -L$(ARCHLIBDIR) -lstdc++

savefeatures : savefeatures.o
	$(CC) -g $(ARCH_LINK_OPTIONS) -o savefeatures savefeatures.o $(LCLLIBS) $(GENICAM_LIBS) -L$(ARCHLIBDIR) -lstdc++

loadfeatures : loadfeatures.o
	$(CC) -g $(ARCH_LINK_OPTIONS) -o loadfeatures loadfeatures.o $(LCLLIBS) $(GENICAM_LIBS) -L$(ARCHLIBDIR) -lstdc++

c_loadfeatures : feature_loader.o
	$(CC) -g $(ARCH_LINK_OPTIONS) -o c_loadfeatures feature_loader.o $(LCLLIBS) 

clean:
	rm *.o dumpfeatures savefeatures loadfeatures c_loadfeatures


