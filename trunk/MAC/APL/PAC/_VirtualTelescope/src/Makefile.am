bin_PROGRAMS 		= LogicalDeviceServer

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

LogicalDeviceServer_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

LogicalDeviceServer_SOURCES 		= \
	AVTUtilities.cc \
	AVTAPCAnswer.cc \
	AVTLogicalDevice.cc \
	AVTLogicalDeviceScheduler.cc \
	AVTResourceManager.cc \
	AVTPropertySetAnswer.cc \
	AVTStationBeamformer.cc \
	AVTVirtualTelescope.cc \
	AVTStationReceptor.cc \
	AVTStationReceptorGroup.cc \
	AVTLogicalDeviceServerMain.cc \
	LogicalDevice_Protocol.cc
	
LogicalDeviceServer_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	-labs \
	-lboost_date_time-gcc-1_31
	
LogicalDeviceServer_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
	LogicalDevice_Protocol.ph \
	avt.ns \
	avt.top \
	log4cplus.properties	

EXTRA_DIST = \
  LogicalDevice_Protocol.ph \
	avt.ns \
	avt.top \
	log4cplus.properties

sysconf_DATA = \
	avt.ns \
	avt.top
	
%.properties: %.properties.in
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
