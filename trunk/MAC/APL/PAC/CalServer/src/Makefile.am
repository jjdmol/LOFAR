

AM_CPPFLAGS = -DUSE_CAL_THREAD

#-fmessage-length=0 // needed for Eclipse

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	CAL_Protocol.ph \
	CalServer.conf \
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
	DipoleModel.conf \
	SourceCatalog.conf \
	LBAntennas.conf \
	HBAntennas.conf \
	AntennaArrays.conf \
	CalServer.log_prop

#	ACC.conf 

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

# protocol library, use this to use the CAL protocol
lib_LTLIBRARIES = libcal.la libcaldevel.la

libcal_ladir = $(prefix)/include
libcal_la_HEADERS = \
	AntennaGains.h \
	AntennaArray.h \
	AntennaArrayData.h \
	SpectralWindow.h \
	SubArray.h \
	SharedResource.h

libcal_la_SOURCES = \
	$(top_builddir)/src/CAL_Protocol.cc \
	AntennaGains.cc \
	AntennaArray.cc \
	AntennaArrayData.cc \
	SpectralWindow.cc \
	SubArray.cc

# convenience library
# lib_LTLIBRARIES = libcaldevel.la

libcaldevel_ladir = $(prefix)/include

libcaldevel_la_SOURCES = \
	Documentation.h \
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
	SubArraySubscription.h \
	SubArraySubscription.cc \
	CalibrationInterface.h \
	CalibrationAlgorithm.h \
	CalibrationAlgorithm.cc \
	RemoteStationCalibration.h \
	RemoteStationCalibration.cc \
	CalibrationThread.h \
	CalibrationThread.cc

include_HEADERS = \
	$(top_builddir)/src/CAL_Protocol.ph

bin_PROGRAMS = \
	CalServer \
	juggle_acc

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

juggle_acc_SOURCES = \
	juggle_acc.cc

CalServer_DEPEND = $(LOFAR_DEPEND)
#	libcal.la \
#	libcaldevel.la \
#	librtccommon.la

#	-lrsp

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common
