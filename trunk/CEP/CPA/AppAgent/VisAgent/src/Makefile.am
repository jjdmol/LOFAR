dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = libvisagent.la

libvisagent_la_SOURCES = AID-VisAgent-Registry.cc \
  InputAgent.cc OutputAgent.cc \
  FileInputAgent.cc FileOutputAgent.cc FileAgentBase.cc
  

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/src


aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h 
 

include $(lofar_sharedir)/Makefile.common

