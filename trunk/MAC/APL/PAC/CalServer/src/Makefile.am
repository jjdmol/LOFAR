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
	AntennaArrays_default.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	CalServer.conf \
	DipoleModel.conf \
	SourceCatalog.conf

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
	calinfo

sbin_PROGRAMS = juggle_acc

CalServer_SOURCES = \
	CalServer.h \
	CalServer.cc \
	ACMProxy.h \
	ACMProxy.cc

CalServer_LDADD = \
	libcaldevel.la

#	-lrtccommon

juggle_acc_SOURCES = \
	juggle_acc.cc

calinfo_SOURCES = \
	calinfo.cc

CalServer_DEPENDENCIES = \
	libcaldevel.la \
	$(LOFAR_DEPEND)

clean-local:
	rm -f *.ph

versioncalserver_SOURCES      = versioncalserver.cc
versioncalserver_LDADD        = libcaldevel.la
versioncalserver_DEPENDENCIES = libcaldevel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
