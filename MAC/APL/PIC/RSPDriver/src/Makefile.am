
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-DTOGGLE_LEDS \
	-DWRITE_ACK_VERREAD

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

BUILT_SOURCES = \
	RSP_Protocol.ph \
	EPA_Protocol.ph \
	log4cplus.properties \
	rspdriver.conf

clean-local:
	rm -f \
	RSP_Protocol.ph \
	RSP_Protocol.cc \
	EPA_Protocol.ph \
	EPA_Protocol.cc

EXTRA_DIST = \
	RSP_Protocol.ph \
	EPA_Protocol.ph

sysconf_DATA = rspdriver.conf

# 	log4cplus.properties \
# 	abs.ns \
# 	abs.top

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

lib_LTLIBRARIES    = libaplconfig.la librsp.la libepa.la

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
	Config.h \
	APLConfig.h

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

libaplconfig_la_SOURCES = \
	Config.cc \
	APLConfig.cc

lib_aplconfig_la_LIBS =

bin_PROGRAMS = \
	RSPDriver \
	ViewStats \
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
	StatsRead.h \
	StatsRead.cc \
	VersionsRead.h \
	VersionsRead.cc \
	WGWrite.h \
	WGWrite.cc \
	WGRead.h \
	WGRead.cc

RSPDriver_LDADD = \
	$(top_builddir)/src/librsp.la \
	$(top_builddir)/src/libepa.la \
	$(top_builddir)/src/libaplconfig.la \
	$(PVSS_LDADD)

ViewStats_SOURCES = \
	gnuplot_i.h \
	gnuplot_i.cc \
	ViewStats.h \
	ViewStats.cc

ViewStats_LDADD = \
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
