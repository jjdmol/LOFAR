# Written for GNU -*- makefile -*-.

mydirs = ../lib
#mylibs = -lDTL
mylibs = -lDTL	# ASTRON

# Where to find -lDTL build
libDTLdir = ../lib

$(TARGET): $(OBJECTS) $(mylibs)
