dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = liboctoagent.la

liboctoagent_la_SOURCES = AID-OctoAgent-Registry.cc \
  EventSink.cc \
  EventMultiplexer.cc

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/src


aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h 
 

include $(lofar_sharedir)/Makefile.common

