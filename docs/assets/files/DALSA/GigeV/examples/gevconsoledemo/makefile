CC= gcc
# For building from subversion source tree.
#IROOT=../..
#INC_PATH = -I. -I$(IROOT)/include -I$(IROOT)/examples/common -I$(IROOT)/externals/LinuxSDK/include \
#            -I$(IROOT)/source/gevapi/linux

# For building from install tree
IROOT=/usr/dalsa/GigeV
INC_PATH = -I. -I$(IROOT)/include -I$(IROOT)/examples/common 

#
# Get the configured include defs file.
# (It gets installed to the distribution tree).
ifeq ($(shell if test -e archdefs.mk; then echo exists; fi), exists)
	include archdefs.mk
else
# Force an error
$(error	archdefs.mk file not found. It gets configured on installation ***)
endif	

DEBUGFLAGS = -g 
#CXX_COMPILE_OPTIONS = -c $(DEBUGFLAGS) -DPOSIX_HOSTPC -D_REENTRANT -fno-for-scope \
			-fno-const-strings -Wall -Wno-parentheses -Wno-missing-braces \
			-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label

CXX_COMPILE_OPTIONS = -c $(DEBUGFLAGS) -DPOSIX_HOSTPC -D_REENTRANT -fno-for-scope \
			-Wall -Wno-parentheses -Wno-missing-braces -Wno-unused-but-set-variable \
			-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label

C_COMPILE_OPTIONS= $(DEBUGFLAGS) -fhosted -Wall -Wno-parentheses -Wno-missing-braces \
		   	-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-label


#LCLLIBS=  -lpthread -lstdc++ -L/usr/X11R6/lib -lXext -lX11 -L/usr/local/lib -lGevApi -lCorW32
#LCLLIBS=  -lpthread -lc -L/usr/X11R6/lib -lXext -lX11 -L/usr/local/lib -lGevApi -lCorW32
LCLLIBS=  -L$(ARCHLIBDIR) -lpthread -lXext -lX11 -L/usr/local/lib -lGevApi -lCorW32

VPATH= . : ../common

%.o : %.cpp
	$(CC) -I. $(INC_PATH) $(CXX_COMPILE_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@

%.o : %.c
	$(CC) -I. $(INC_PATH) $(C_COMPILE_OPTIONS) $(ARCH_OPTIONS) -c $< -o $@

OBJS= gevconsoledemo.o \
      GevUtils.o \
      X_Display_utils.o

gevconsoledemo : $(OBJS)
	$(CC) -g $(ARCH_LINK_OPTIONS) -o gevconsoledemo $(OBJS) $(LCLLIBS)

clean:
	rm *.o gevconsoledemo 


