
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-DN_PROTO2_BLP=1 \
	-DTOGGLE_LEDS

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
	rsp.cfg

clean-local:
	rm -f \
	RSP_Protocol.ph \
	RSP_Protocol.cc \
	EPA_Protocol.ph \
	EPA_Protocol.cc

EXTRA_DIST = \
	RSP_Protocol.ph \
	EPA_Protocol.ph \
	rsp.cfg

# 	log4cplus.properties \
# 	abs.ns \
# 	abs.top

include_HEADERS = \
	RSP_Protocol.ph \
	EPA_Protocol.ph

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.cfg: %.cfg.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

noinst_LTLIBRARIES = librsp.la libepa.la

librsp_la_SOURCES = \
	Timestamp.h \
	Timestamp.cc \
	BeamletWeights.h \
	BeamletWeights.cc \
	SubbandSelection.h \
	SubbandSelection.cc \
	RCUSettings.h \
	RCUSettings.cc \
	WGSettings.h \
	WGSettings.cc \
	SystemStatus.h \
	SystemStatus.cc \
	Statistics.h \
	Statistics.cc \
	Versions.h \
	Versions.cc \
	RSP_Protocol.cc

libepa_la_SOURCES = \
	MEPHeader.h \
	MEPHeader.cc \
	MEPData.h \
	MEPData.cc \
	EPA_Protocol.cc \
	RawEvent.h \
	RawEvent.cc

bin_PROGRAMS = RSPDriver

RSPDriver_SOURCES = \
	Config.h \
	Config.cc \
	RSPConfig.h \
	RSPConfig.cc \
	RSPDriverTask.h \
	RSPDriverTask.cc \
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
	GetStatusCmd.h \
	GetStatusCmd.cc \
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
	-lgcf

include $(lofar_sharedir)/Makefile.common
