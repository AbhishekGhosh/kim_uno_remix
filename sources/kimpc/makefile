# Makefile for ArdSim (Arduino simulatorish) for gcc/desktop
#
#  This entire subproject is under MIT license
#
#  Note: this project will let you build for desktop, using the text console as 
#        the serial port
#
#  2015-Feb  Scott Lawrence  yorgle@gmail.com
#

# Main target
#  Set this up as the arduino directory name (which is also the .ino filename)

ARDMAIN := KimUnoRemix
ARDUINOSRCDIR := $(shell pwd)/../$(ARDMAIN)

# target executable name
TARG := KimRemix

# list of all sources, starting with the main.ino
# then going through all of the additional sources 
# in the arduino directory (if any)
SRCPPS := \
	$(ARDMAIN).cpp \
	\
	$(ARDUINOSRCDIR)/keys.cpp \
	$(ARDUINOSRCDIR)/display.cpp

SRCS := \
	$(ARDUINOSRCDIR)/cpu.c \
	$(ARDUINOSRCDIR)/memory.c 

# additional include directories
INCS += -I$(ARDUINOSRCDIR)


########################################
# arduino simulator source for desktop

# list of all .cpp files in the ardsim
SRCSIM := ardsim/main.cpp \
	  ardsim/eesim.cpp \
	  ardsim/sersim.cpp \
	  ardsim/miscsim.cpp \
	  ardsim/iosim.cpp \
	  ardsim/Keypad.cpp

INCS += -I$(shell pwd)/ardsim
	

########################################

# generate a list of .o foles
OBJS := $(SRCS:%.c=%.o) $(SRCPPS:%.cpp=%.o) $(SRCSIM:%.cpp=%.o)

# build flags and such
CFLAGS += -g -DDESKTOP -Wall -pedantic $(INCS)
CPPFLAGS += $(CFLAGS) -std=c99

LDFLAGS +=

LIBS += 


################################################################################
# main build targets

all: $(TARG)

# make object files
%.o: %.cpp
	@echo $(CXX) $<
	@$(CXX) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<


# gcc doesn't know what to do with an INO, so we'll make a 
# symlink .cpp version of it to use.
$(ARDMAIN).cpp: $(ARDUINOSRCDIR)/$(ARDMAIN).ino
	@echo sym-linking INO as a CPP file....
	@-ln -s $< $@


# final build...
$(TARG): $(OBJS) 
	@echo Link $@
	@$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $@


################################################################################

# clean up the directories
clean:
	@echo Remove build files
	-rm -f $(OBJS) $(TARG) $(TARG).exe $(ARDMAIN).cpp 
.PHONY: clean


# clean and rebuild
full: clean
	make test
.PHONY: full

# build and run!
test: $(TARG)
	./$(TARG)
.PHONY: test
