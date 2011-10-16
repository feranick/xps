# use "gcc" to compile source files.
CC = g++
# the linker is also "gcc". It might be something else with other compilers.
LD = g++
# Default Compiler flags: dynamic build
CFLAGS = -Wall -O2 -fomit-frame-pointer -pipe
LDFLAGS = -Wall -s -O2
#---------------------------------
# Default Compiler flags: static build
#CFLAGS = -Wall -O2 -fomit-frame-pointer -pipe -static
#LDFLAGS = -Wall -s -O2 -static
#---------------------------------
#CLFAGS for Intel Atom
#CHOST=i686-pc-linux-gnu
#CFLAGS=-O2 -march=core2 -mtune=native -fomit-frame-pointer -pipe
#CXXFLAGS=${CFLAGS}
#---------------------------------
#CFLAGS = -Wall -O2 -march=native
#CFLAGS = -O2 -march=native -pipe -fomit-frame-pointer -mfpmath=sse -msse -msse2 -msse3 -mmmx -m3dnow
# Linker flags go here. Currently there aren't any, but if we'll switch to
# code optimization, we might add "-s" here to strip debug info and symbols.
#LDFLAGS = -Wall -s -O2 -march=core2
#LDFLAGS = -Wall -s -O2 -march=native
#LDFLAGS = -O2 -march=native -pipe -fomit-frame-pointer -mfpmath=sse -msse -msse2 -msse3 -mmmx -m3dnow
# use this command to erase files.
RM = rm -f 
# list of souces
SOURCE = xps.cpp
# list of generated object files.
OBJS = xps.o 
# OBJS2 = xps.cfg
# command to port program in working folder
PORT = sudo cp 
DIR = /usr/local/bin/

# CD
CD = cd

# program executable file name.
PROG = xps

# top-level rule, to compile everything.
all: $(PROG)
	$(RM) $(OBJS) 

# rule to link the program
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)
#	$(PORT) $(PROG) $(DIR)

# rule for file "OBJS".
$(OBJS): $(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE)

# rule for final build
build: $(PROG)
	$(RM) $(OBJS) 

# rule for cleaning re-compilable files.
clean:
	$(RM) $(PROG) $(OBJS)
#	$(RM) $(DIR)$(PROG)

# rule to port ipadd in the working folder
install:
	$(PORT) $(PROG) $(DIR)
	sudo chmod 755 $(PROG)

# rule to port ipadd in the working folder
uninstall:
	sudo $(RM) $(DIR)$(PROG) 

