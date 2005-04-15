
if HAVE_SYS_CAPABILITY_H
CAPLIBS=$(top_builddir)/src/libnetraw.la -lcap
else
CAPLIBS=
endif

#
# Define ENABLE_CAP_NET_RAW to let RSPDriver run as setuid root
# with only cap_net_raw capability enabled.
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src

INSTALL=install -C

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

BUILT_SOURCES = \
	RSP_Protocol.ph \
	EPA_Protocol.ph \
	RemoteStation.conf \
	RSPDriverPorts.conf \
	RSPDriver.conf \
	ViewStats.conf \
	CaptureStats.conf \
	CaptureStats.log_prop \
	Tuner.conf \
	SetWG.conf

clean-local:
	rm -f \
	RSP_Protocol.ph \
	RSP_Protocol.cc \
	EPA_Protocol.ph \
	EPA_Protocol.cc

EXTRA_DIST = \
	RSP_Protocol.ph \
	EPA_Protocol.ph \
	$(BUILT_SOURCES)

sysconf_DATA = \
	RemoteStation.conf \
	RSPDriverPorts.conf \
	RSPDriver.conf \
	ViewStats.conf \
	CaptureStats.conf \
	CaptureStats.log_prop \
	Tuner.conf \
	SetWG.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

lib_LTLIBRARIES = librsp.la libepa.la libnetraw.la

include_HEADERS = \
	RSP_Protocol.ph \
	Timestamp.h \
	BeamletWeights.h \
	SubbandSelection.h \
	RCUSettings.h \
	WGSettings.h \
	SystemStatus.h \
	Statistics.h \
	Versions.h \
	Marshalling.h \
	\
	EPA_Protocol.ph \
	MEPHeader.h \
	MEPData.h \
	\
	PSAccess.h

librsp_la_SOURCES = \
	Timestamp.cc \
	BeamletWeights.cc \
	SubbandSelection.cc \
	RCUSettings.cc \
	WGSettings.cc \
	SystemStatus.cc \
	Statistics.cc \
	Versions.cc \
	RSP_Protocol.cc

libepa_la_SOURCES = \
	MEPHeader.cc \
	MEPData.cc \
	EPA_Protocol.cc \
	RawEvent.h \
	RawEvent.cc

libnetraw_la_SOURCES = \
	netraw.cc \
	netraw.h

bin_PROGRAMS = \
	RSPDriver \
	ViewStats \
	CaptureStats \
	Tuner \
	SetWG

RSPDriver_SOURCES = \
	RefCount.h \
	RSPDriver.h \
	RSPDriver.cc \
	Scheduler.h \
	Scheduler.cc \
	Command.h \
	Command.cc \
	Cache.h \
	Cache.cc \
	\
	SetWeightsCmd.h \
	SetWeightsCmd.cc \
	GetWeightsCmd.h \
	GetWeightsCmd.cc \
	SetSubbandsCmd.h \
	SetSubbandsCmd.cc \
	GetSubbandsCmd.h \
	GetSubbandsCmd.cc \
	GetStatusCmd.h \
	GetStatusCmd.cc \
	UpdStatusCmd.h \
	UpdStatusCmd.cc \
	SetRCUCmd.h \
	SetRCUCmd.cc \
	GetRCUCmd.h \
	GetRCUCmd.cc \
	SetWGCmd.h \
	SetWGCmd.cc \
	GetWGCmd.h \
	GetWGCmd.cc \
	GetVersionsCmd.h \
	GetVersionsCmd.cc \
	GetStatsCmd.h \
	GetStatsCmd.cc \
	UpdStatsCmd.h \
	UpdStatsCmd.cc \
	\
	SyncAction.h \
	SyncAction.cc \
	WriteReg.h \
	WriteReg.cc \
	BWWrite.h \
	BWWrite.cc \
	BWRead.h \
	BWRead.cc \
	SSWrite.h \
	SSWrite.cc \
	SSRead.h \
	SSRead.cc \
	RCUWrite.h \
	RCUWrite.cc \
	RCURead.h \
	RCURead.cc \
	StatusRead.h \
	StatusRead.cc \
	SstRead.h \
	SstRead.cc \
	BstRead.h \
	BstRead.cc \
	VersionsRead.h \
	VersionsRead.cc \
	WGWrite.h \
	WGWrite.cc \
	WGRead.h \
	WGRead.cc

RSPDriver_LDADD = \
	$(top_builddir)/src/librsp.la \
	$(top_builddir)/src/libepa.la \
	$(CAPLIBS)

ViewStats_SOURCES = \
	gnuplot_i.h \
	gnuplot_i.cc \
	ViewStats.h \
	ViewStats.cc

ViewStats_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

CaptureStats_SOURCES = \
	CaptureStats.h \
	CaptureStats.cc

CaptureStats_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

Tuner_SOURCES = \
	Tuner.h \
	Tuner.cc

Tuner_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

SetWG_SOURCES = \
	gnuplot_i.h \
	gnuplot_i.cc \
	SetWG.h \
	SetWG.cc

SetWG_LDADD = \
	$(top_builddir)/src/librsp.la \
	-lsuite

include $(lofar_sharedir)/Makefile.common
