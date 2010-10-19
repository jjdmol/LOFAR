
if HAVE_SYS_CAPABILITY_H
CAPLIBS=-lnetraw -lcap
else
CAPLIBS=
endif

#
# Define ENABLE_RSPFE to enable the rspctl frontend
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-DENABLE_RSPFE

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	RSP_Protocol.ph \
	rspctlfe/src/RSPFE_Protocol.ph \
	EPA_Protocol.ph \
	RemoteStation.conf \
	RSPDriverPorts.conf \
	RSPDriver.conf \
	Tuner.conf \
	SetWG.conf \
	rspctl.conf

clean-local:
	rm -f *.ph

EXTRA_DIST = \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph \
	EPA_Protocol.ph \
	$(BUILT_SOURCES)

sysconf_DATA = \
	RemoteStation.conf \
	RSPDriverPorts.conf \
	RSPDriver.conf \
	Tuner.conf \
	SetWG.conf \
	rspctl.conf

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

lib_LTLIBRARIES = librsp.la libepa.la librspfe.la 

include_HEADERS = \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph \
	BeamletWeights.h \
	SubbandSelection.h \
	RCUSettings.h \
	WGSettings.h \
	SystemStatus.h \
	Statistics.h \
	XCStatistics.h \
	Versions.h \
	Clocks.h \
	\
	EPA_Protocol.ph \
	MEPHeader.h \
	MEPData.h

librsp_la_SOURCES = \
	BeamletWeights.cc \
	SubbandSelection.cc \
	RCUSettings.cc \
	WGSettings.cc \
	SystemStatus.cc \
	Statistics.cc \
	XCStatistics.cc \
	Versions.cc \
	Clocks.cc \
	RSP_Protocol.cc

librspfe_la_SOURCES = \
	RSPFE_Protocol.cc

libepa_la_SOURCES = \
	MEPHeader.cc \
	MEPData.cc \
	EPA_Protocol.cc \
	RawEvent.h \
	RawEvent.cc

bin_SCRIPTS = \
	rspdriver

bin_PROGRAMS = \
	RSPDriver \
	Tuner \
	SetWG \
	rspctl

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
	GetXCStatsCmd.h \
	GetXCStatsCmd.cc \
	UpdXCStatsCmd.h \
	UpdXCStatsCmd.cc \
	SetClocksCmd.h \
	SetClocksCmd.cc \
	GetClocksCmd.h \
	GetClocksCmd.cc \
	UpdClocksCmd.h \
	UpdClocksCmd.cc \
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
	XstRead.h \
	XstRead.cc \
	VersionsRead.h \
	VersionsRead.cc \
	WGWrite.h \
	WGWrite.cc \
	WGRead.h \
	WGRead.cc

RSPDriver_LDADD = \
	$(top_builddir)/src/librsp.la \
	$(top_builddir)/src/libepa.la \
	$(RTCCOMMON_LDADD) \
	$(CAPLIBS)

Tuner_SOURCES = \
	Tuner.h \
	Tuner.cc

Tuner_LDADD = \
	$(top_builddir)/src/librsp.la

SetWG_SOURCES = \
	SetWG.h \
	SetWG.cc

SetWG_LDADD = \
	$(top_builddir)/src/librsp.la

rspctl_SOURCES = \
	rspctl.h \
	rspctl.cc

rspctl_LDADD = \
	$(top_builddir)/src/librsp.la \
	$(top_builddir)/src/librspfe.la \
	-lgnuplot_i

#	$(RTCCOMMON_LDADD)

include $(lofar_sharedir)/Makefile.common
