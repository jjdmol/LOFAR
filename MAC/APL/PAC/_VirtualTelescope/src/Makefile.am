bin_PROGRAMS 		= LogicalDeviceServer

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

LogicalDeviceServer_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

LogicalDeviceServer_SOURCES 		= \
	AVTUtilities.cc \
	AVTLogicalDevice.cc \
	AVTLogicalDeviceScheduler.cc \
	AVTResourceManager.cc \
	AVTPropertySetAnswer.cc \
	AVTStationBeamformer.cc \
	AVTVirtualTelescope.cc \
	AVTStationReceptor.cc \
	AVTStationReceptorGroup.cc \
	AVTLogicalDeviceServerMain.cc \
	$(top_builddir)/src/LogicalDevice_Protocol.cc
	
LogicalDeviceServer_LDADD		= \
	-labs \
	-lboost_date_time-gcc-1_31
	
include_HEADERS = \
	$(top_builddir)/src/LogicalDevice_Protocol.ph

BUILT_SOURCES = \
	LogicalDevice_Protocol.ph
	
EXTRA_DIST = \
  LogicalDevice_Protocol.ph 
  
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	LogicalDeviceScheduler.conf \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
