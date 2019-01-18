CC= gcc
IROOT=/usr/dalsa/GigeV

INC_PATH = -I. -I$(IROOT)/include  

CXX_COMPILE_OPTIONS = -c $(DEBUGFLAGS) -DPOSIX_HOSTPC -D_REENTRANT -fno-for-scope \
			-Wall -Wno-parentheses -Wno-missing-braces \
			-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label

C_COMPILE_OPTIONS= $(DEBUGFLAGS) -fhosted -Wall -Wno-parentheses -Wno-missing-braces \
		   	-Wno-unknown-pragmas -Wno-cast-qual -Wno-unused-function -Wno-unused-label


LCLLIBS=  -lpthread -L/usr/local/lib -lGevApi -lCorW32

VPATH= . 

%.o : %.cpp
	$(CC) -I. $(INC_PATH) $(CXX_COMPILE_OPTIONS) -D_i386_ -c $< -o $@

%.o : %.c
	$(CC) -I. $(INC_PATH) $(C_COMPILE_OPTIONS) -D_i386_ -c $< -o $@

OBJS= gevipconfig.o 

gevipconfig : $(OBJS)
	$(CC) -g -o gevipconfig $(OBJS) $(LCLLIBS)

clean:
	rm *.o gevipconfig
	
install: gevipconfig
	cp ./gevipconfig /usr/local/bin

uninstall: clean 
	rm -f /usr/local/bin/gevipconfig


