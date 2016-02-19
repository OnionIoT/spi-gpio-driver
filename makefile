
# main compiler
CC := gcc
# CC := clang --analyze # and comment out the linker last line for sanity

# define the directories
SRCDIR := src
INCDIR := include
BUILDDIR := build
BINDIR := bin
LIBDIR := lib
PYLIBDIR := lib/python


# define common variables
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -maxdepth 1 -type f \( -iname "*.$(SRCEXT)" ! -iname "*main-*.$(SRCEXT)" \) )
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g # -Wall
INC := $(shell find $(INCDIR) -maxdepth 1 -type d -exec echo -I {}  \;)

#PYINC := "-I/usr/include/python2.7"
INC += $(PYINC)

# define specific binaries to create
LIB0 := libonionspi
SOURCE_LIB0 := src/onion-spi.$(SRCEXT)
OBJECT_LIB0 := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCE_LIB0:.$(SRCEXT)=.o))
TARGET_LIB0 := $(LIBDIR)/$(LIB0).so
LIB_LIB0 := -L$(LIBDIR) -loniondebug

APP0 := spi-tool
SOURCE_APP0 := $(SRCDIR)/main-$(APP0).$(SRCEXT)
OBJECT_APP0 := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCE_APP0:.$(SRCEXT)=.o))
LIB_APP0 := -L$(LIBDIR) -loniondebug -lonionspi
TARGET_APP0 := $(BINDIR)/$(APP0)

PYLIB0 := onionSpi
SOURCE_PYLIB0 := src/python/python-onion-spi.c
OBJECT_PYLIB0 := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCE_PYLIB0:.$(SRCEXT)=.o))
TARGET_PYLIB0 := $(PYLIBDIR)/$(PYLIB0).so
LIB_PYLIB0 := -L$(LIBDIR) -loniondebug -lonionspi -lpython2.7


all: info $(TARGET_LIB0) $(TARGET_APP0) $(TARGET_PYLIB0)


# libraries
$(TARGET_LIB0): $(OBJECT_LIB0)
	@echo " Compiling $@"
	@mkdir -p $(LIBDIR)
	$(CC) -shared -o $@  $^ $(LIB_LIB0)

# python libraries
$(TARGET_PYLIB0): $(OBJECT_PYLIB0)
	@echo " Compiling $@"
	@mkdir -p $(PYLIBDIR)
	$(CC) -shared -o $@  $^ $(LIB_PYLIB0)

# application binaries
$(TARGET_APP0): $(OBJECT_APP0)
	@echo " Compiling $(APP0)"
	@mkdir -p $(BINDIR)
	@echo " Linking..."
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $(TARGET_APP0) $(LIB) $(LIB_APP0)


# generic: build any object file required
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	$(RM) -r $(BUILDDIR) $(BINDIR) $(LIBDIR)

info:
	@echo "CC: $(CC)"
	@echo "CCFLAGS: $(CCFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIB: $(LIB)"
	@echo "INC: $(INC)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"

# Tests
tester:
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

# Spikes
#ticket:
#  $(CC) $(CFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket

.PHONY: clean
