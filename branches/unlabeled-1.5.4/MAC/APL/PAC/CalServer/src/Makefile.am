

#AM_CPPFLAGS = \
#	-I$(top_srcdir)/src

#-fmessage-length=0 // needed for Eclipse

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(lofar_root)/MAC/MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(lofar_root)/MAC/MACCommon/autogen $<

BUILT_SOURCES = \
	CAL_Protocol.ph \
	CalServer.conf \
	SpectralWindow.conf \
	DipoleModel.conf \
	SourceCatalog.conf \
	LBAntennas.conf \
	HBAntennas.conf \
	AntennaArrays.conf \
	CalServer.log_prop

#	ACC.conf

EXTRA_DIST = \
	CAL_Protocol.ph

sysconf_DATA = \
	CalServer.conf \
	SpectralWindow.conf \
	DipoleModel.conf \
	SourceCatalog.conf \
	LBAntennas.conf \
	HBAntennas.conf \
	AntennaArrays.conf \
	CalServer.log_prop

#	ACC.conf 

include_HEADERS = \
	$(top_builddir)/src/CAL_Protocol.ph

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

# protocol library, use this to use the CAL protocol
lib_LTLIBRARIES = libcal.la

libcal_la_SOURCES = \
	$(top_builddir)/src/CAL_Protocol.cc \
	AntennaGains.h \
	AntennaGains.cc

# convenience library
noinst_LTLIBRARIES = libcaldevel.la

libcaldevel_la_SOURCES = \
	Documentation.h \
	SharedResource.h \
	ACC.h \
	ACC.cc \
	Source.h \
	Source.cc \
	SourceData.h \
	SourceData.cc \
	DipoleModel.h \
	DipoleModel.cc \
	DipoleModelData.h \
	DipoleModelData.cc \
	SpectralWindow.h \
	SpectralWindow.cc \
	AntennaArray.h \
	AntennaArray.cc \
	AntennaArrayData.h \
	AntennaArrayData.cc \
	SubArray.h \
	SubArray.cc \
	SubArraySubscription.h \
	SubArraySubscription.cc \
	CalibrationInterface.h \
	CalibrationAlgorithm.h \
	CalibrationAlgorithm.cc \
	RemoteStationCalibration.h \
	RemoteStationCalibration.cc

bin_PROGRAMS = \
	CalServer

CalServer_SOURCES = \
	CalConstants.h \
	CalServer.h \
	CalServer.cc \
	ACMProxy.h \
	ACMProxy.cc

CalServer_LDADD = \
	$(top_builddir)/src/libcal.la \
	$(top_builddir)/src/libcaldevel.la \
	-lrtccommon

CalServer_DEPEND = $(LOFAR_DEPEND)
#	libcal.la \
#	libcaldevel.la \
#	librtccommon.la

#	-lrsp

include $(lofar_sharedir)/Makefile.common
