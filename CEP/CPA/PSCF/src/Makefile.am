dmi_dir = $(lofar_sharedir)/../DMI

INCLUDES = -I$(dmi_dir)/src

CXXFLAGS += -Wno-unknown-pragmas 

lib_LTLIBRARIES = libpscf.la

libpscf_la_SOURCES = Dispatcher.cc  Message.cc  MsgAddress.cc  \
	WPInterface.cc WorkProcess.cc Timestamp.cc PSCFDebugContext.cc \
  GWClientWP.cc GWServerWP.cc GatewayWP.cc \
  Socket.cc Subscriptions.cc \
  AID-PSCF-Registry.cc
  
# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/src

aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h
 

include $(lofar_sharedir)/Makefile.common

