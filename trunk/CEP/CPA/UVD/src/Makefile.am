dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -Wno-unknown-pragmas -pg

lib_LTLIBRARIES = libfillers.la

libfillers_la_SOURCES = \
  AID-UVD-Registry.cc \
  MSIntegratorWP.cc \
  UVSorterWP.cc
  
# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/src

aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h 
 

include $(lofar_sharedir)/Makefile.common

