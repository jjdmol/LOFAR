dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -D_GNU_SOURCE 

check_PROGRAMS		= 
# test_input

bin_PROGRAMS   		= 

# test_input_SOURCES		    = test_input.cc
# test_input_LDADD		      = ../src/libmsvisagent.la
# test_input_DEPENDENCIES	  = ../src/libmsvisagent.la $(LOFAR_DEPEND)

TESTS				=  

EXTRA_DIST			=

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/test

aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
