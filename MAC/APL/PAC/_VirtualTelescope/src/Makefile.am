noinst_PROGRAMS 		= AVTLogicalDeviceServer

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen -L $(top_builddir)/PAC/BeamServer/src $<

AVTLogicalDeviceServer_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src \
	-I$(top_srcdir)/PAC/BeamServer/src \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

AVTLogicalDeviceServer_SOURCES 		= \
	AVTUtilities.cc \
	AVTAPCAnswer.cc \
	AVTLogicalDevice.cc \
	AVTLogicalDeviceScheduler.cc \
	AVTPropertySetAnswer.cc \
	AVTStationBeamformer.cc \
	AVTVirtualTelescope.cc \
	AVTStationReceptor.cc \
	AVTStationReceptorGroup.cc \
	AVTLogicalDeviceServerMain.cc \
	LogicalDevice_Protocol.cc \
	ABS_Protocol.cc
	
AVTLogicalDeviceServer_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la
AVTLogicalDeviceServer_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
	$(top_builddir)/PAC/BeamServer/src/ABS_Protocol.ph \
	LogicalDevice_Protocol.ph \
	avt.ns \
	avt.top \
	log4cplus.properties	

EXTRA_DIST = \
  $(top_builddir)/PAC/BeamServer/src/ABS_Protocol.ph \
  LogicalDevice_Protocol.ph \
	avt.ns \
	avt.top \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
