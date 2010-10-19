
#
# Define ENABLE_RSPFE to enable the rspctl frontend
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src 

#	-DENABLE_RSPFE

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	RemoteStation.conf \
	RSPDriver.conf \
	rspctl.conf \
	RSPDriver.log_prop \
	rspctl.log_prop

clean-local:
	rm -f *.ph

EXTRA_DIST = \
	$(BUILT_SOURCES)

sysconf_DATA = \
	RemoteStation.conf \
	RSPDriver.conf \
	rspctl.conf \
	RSPDriver.log_prop \
	rspctl.log_prop

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

lib_LTLIBRARIES = libepa.la

include_HEADERS =

libepa_la_SOURCES = \
	RawEvent.h \
	RawEvent.cc

bin_SCRIPTS = \
	rspdriver

bin_PROGRAMS = \
	RSPDriver \
	rspctl \
	geni2c

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
	StationSettings.h \
	StationSettings.cc \
	\
	SetWeightsCmd.h \
	SetWeightsCmd.cc \
	GetWeightsCmd.h \
	GetWeightsCmd.cc \
	SetSubbandsCmd.h \
	SetSubbandsCmd.cc \
	GetSubbandsCmd.h \
	GetSubbandsCmd.cc \
	UpdSubbandsCmd.h \
	UpdSubbandsCmd.cc \
	GetStatusCmd.h \
	GetStatusCmd.cc \
	UpdStatusCmd.h \
	UpdStatusCmd.cc \
	SetRCUCmd.h \
	SetRCUCmd.cc \
	GetRCUCmd.h \
	GetRCUCmd.cc \
	UpdRCUCmd.h \
	UpdRCUCmd.cc \
	SetHBACmd.h \
	SetHBACmd.cc \
	GetHBACmd.h \
	GetHBACmd.cc \
	UpdHBACmd.h \
	UpdHBACmd.cc \
	SetRSUCmd.h \
	SetRSUCmd.cc \
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
	GetTDStatusCmd.h \
	GetTDStatusCmd.cc \
	UpdTDStatusCmd.h \
	UpdTDStatusCmd.cc \
	GetRegisterStateCmd.h \
	GetRegisterStateCmd.cc \
	UpdRegisterStateCmd.h \
	UpdRegisterStateCmd.cc \
	SetTBBCmd.h \
	SetTBBCmd.cc \
	GetTBBCmd.h \
	GetTBBCmd.cc \
	\
	SyncAction.h \
	SyncAction.cc \
	WriteReg.h \
	WriteReg.cc \
	ReadReg.h \
	ReadReg.cc \
	XWWrite.h \
	XWWrite.cc \
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
	RCUProtocolWrite.h \
	RCUProtocolWrite.cc \
	RCUResultRead.h \
	RCUResultRead.cc \
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
	WGRead.cc \
	CDOWrite.h \
	CDOWrite.cc \
	BSWrite.h \
	BSWrite.cc \
	RSUWrite.h \
	RSUWrite.cc \
	TDSi2cdefs.h \
	TDSProtocolWrite.cc \
	TDSProtocolWrite.h \
	TDSResultWrite.cc \
	TDSResultWrite.h \
	TDSResultRead.cc \
	TDSResultRead.h \
	TDSStatusWrite.h \
	TDSStatusWrite.cc \
	TDSStatusRead.h \
	TDSStatusRead.cc \
	HBAProtocolWrite.h \
	HBAProtocolWrite.cc \
	HBAResultRead.h \
	HBAResultRead.cc \
	RADWrite.h \
	RADWrite.cc \
	TimestampWrite.h \
	TimestampWrite.cc \
	TBBSettingsWrite.h \
	TBBSettingsWrite.cc \
	TBBBandselWrite.h \
	TBBBandselWrite.cc \
	Sequencer.h \
	Sequencer.cc

RSPDriver_LDADD = \
	$(top_builddir)/src/libepa.la \
	$(RTCCOMMON_LDADD)

rspctl_SOURCES = \
	rspctl.h \
	rspctl.cc

geni2c_SOURCES = \
	geni2c.cc

rspctl_LDADD = -lgnuplot_i

#	$(RTCCOMMON_LDADD)

include $(top_srcdir)/Makefile.common
