pkginclude_HEADERS = Package__Version.h


#AM_CPPFLAGS = -DUSE_CAL_THREAD
AM_CPPFLAGS = -DUSE_CAL_THREAD

#-fmessage-length=0 // needed for Eclipse

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	CalServer.conf \
	DipoleModel.conf \
	SourceCatalog.conf \
	LBAntennas.conf \
	HBAntennas.conf \
	AntennaArrays.conf \
	CalServer.log_prop

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	CalServer.conf \
	DipoleModel.conf \
	SourceCatalog.conf \
	LBAntennas.conf \
	HBAntennas.conf \
	AntennaArrays.conf \
	CalServer.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

# protocol library, use this to use the CAL protocol
lib_LTLIBRARIES = libcaldevel.la

# convenience library
# lib_LTLIBRARIES = libcaldevel.la

libcaldevel_ladir = $(prefix)/include

libcaldevel_la_SOURCES = Package__Version.cc  \
	CalibrationAlgorithm.h \
	CalibrationAlgorithm.cc \
	CalibrationThread.h \
	CalibrationThread.cc \
	DipoleModel.h \
	DipoleModel.cc \
	DipoleModelData.h \
	DipoleModelData.cc \
	Documentation.h \
	RemoteStationCalibration.h \
	RemoteStationCalibration.cc \
	Source.h \
	Source.cc \
	SourceData.h \
	SourceData.cc \
	SubArrays.h \
	SubArrays.cc \
	SubArraySubscription.h \
	SubArraySubscription.cc 

bin_PROGRAMS = versioncalserver \
	CalServer \
	juggle_acc \
	calinfo

CalServer_SOURCES = \
	CalServer.h \
	CalServer.cc \
	ACMProxy.h \
	ACMProxy.cc

CalServer_LDADD = \
	$(top_builddir)/src/libcaldevel.la

#	-lrtccommon

juggle_acc_SOURCES = \
	juggle_acc.cc

calinfo_SOURCES = \
	calinfo.cc

CalServer_DEPEND = \
	$(top_builddir)/src/libcaldevel.la \
	$(top_builddir)/src/libcal_protocol.la \
	$(LOFAR_DEPEND)

bin_SCRIPTS = \
	calserver

clean-local:
	rm -f *.ph

versioncalserver_SOURCES      = versioncalserver.cc
versioncalserver_LDADD        = libcaldevel.la
versioncalserver_DEPENDENCIES = libcaldevel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
