AM_CPPFLAGS 	= -I$(top_srcdir)/src 

AUTOGEN 		= autogen
SUFFIXES 		= .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

sysconf_DATA 		= rspctl.conf 
EXTRA_DIST 			= $(sysconf_DATA)
include_HEADERS		=
pkginclude_HEADERS	= Package__Version.h

lib_LTLIBRARIES		= libepa.la
libepa_la_SOURCES	= Package__Version.cc RawEvent.h RawEvent.cc

sbin_PROGRAMS 		= geni2c
bin_PROGRAMS 		= RSPDriver rspctl versionrspdriver

RSPDriver_SOURCES	= \
	RSPDriver.cc \
	Scheduler.cc \
	Sequencer.cc \
	Cache.cc \
	StationSettings.cc \
	RCUCables.cc \
	CableAttenuation.cc \
	CableSettings.cc \
	SerdesBuffer.cc \
	\
	SetWeightsCmd.cc \
	GetWeightsCmd.cc \
	SetSubbandsCmd.cc \
	GetSubbandsCmd.cc \
	UpdSubbandsCmd.cc \
	GetStatusCmd.cc \
	UpdStatusCmd.cc \
	SetRCUCmd.cc \
	GetRCUCmd.cc \
	UpdRCUCmd.cc \
	SetHBACmd.cc \
	GetHBACmd.cc \
	ReadHBACmd.cc \
	UpdHBACmd.cc \
	SetRSUCmd.cc \
	SetWGCmd.cc \
	GetWGCmd.cc \
	GetVersionsCmd.cc \
	GetStatsCmd.cc \
	UpdStatsCmd.cc \
	GetXCStatsCmd.cc \
	UpdXCStatsCmd.cc \
	SetClocksCmd.cc \
	GetClocksCmd.cc \
	UpdClocksCmd.cc \
	GetTDStatusCmd.cc \
	UpdTDStatusCmd.cc \
	GetRegisterStateCmd.cc \
	UpdRegisterStateCmd.cc \
	SetTBBCmd.cc \
	GetTBBCmd.cc \
	SetBypassCmd.cc \
	GetBypassCmd.cc \
	GetSPUStatusCmd.cc \
	SetRawBlockCmd.cc \
	GetRawBlockCmd.cc \
	SetSplitterCmd.cc \
	GetSplitterCmd.cc \
	\
	SyncAction.cc \
	WriteReg.cc \
	ReadReg.cc \
	XWWrite.cc \
	BWWrite.cc \
	BWRead.cc \
	SSWrite.cc \
	SSRead.cc \
	RCUWrite.cc \
	RCUProtocolWrite.cc \
	RCUResultRead.cc \
	RCURead.cc \
	RawBlockRead.cc \
	RawBlockWrite.cc \
	StatusRead.cc \
	SstRead.cc \
	BstRead.cc \
	XstRead.cc \
	BypassRead.cc \
	VersionsRead.cc \
	WGWrite.cc \
	WGRead.cc \
	CDOWrite.cc \
	BSWrite.cc \
	RSUWrite.cc \
	TDSProtocolWrite.cc \
	TDSResultWrite.cc \
	TDSResultRead.cc \
	TDSStatusWrite.cc \
	TDSStatusRead.cc \
	HBAProtocolWrite.cc \
	HBAResultRead.cc \
	RADWrite.cc \
	TimestampWrite.cc \
	TBBSettingsWrite.cc \
	TBBBandselWrite.cc \
	BypassWrite.cc \
	SerdesWrite.cc \
	SerdesRead.cc

INSTHDRS	=
NOINSTHDRS	= \
	RefCount.h \
	RSPDriver.h \
	Scheduler.h \
	Sequencer.h \
	Command.h \
	Cache.h \
	StationSettings.h \
	RCUCables.h \
	CableAttenuation.h \
	CableSettings.h \
	SerdesBuffer.h \
	\
	SetWeightsCmd.h \
	GetWeightsCmd.h \
	SetSubbandsCmd.h \
	GetSubbandsCmd.h \
	UpdSubbandsCmd.h \
	GetStatusCmd.h \
	UpdStatusCmd.h \
	SetRCUCmd.h \
	GetRCUCmd.h \
	UpdRCUCmd.h \
	SetHBACmd.h \
	GetHBACmd.h \
	ReadHBACmd.h \
	UpdHBACmd.h \
	SetRSUCmd.h \
	SetWGCmd.h \
	GetWGCmd.h \
	GetVersionsCmd.h \
	GetStatsCmd.h \
	UpdStatsCmd.h \
	GetXCStatsCmd.h \
	UpdXCStatsCmd.h \
	SetClocksCmd.h \
	GetClocksCmd.h \
	UpdClocksCmd.h \
	GetTDStatusCmd.h \
	UpdTDStatusCmd.h \
	GetRegisterStateCmd.h \
	UpdRegisterStateCmd.h \
	SetTBBCmd.h \
	GetTBBCmd.h \
	SetBypassCmd.h \
	GetBypassCmd.h \
	GetSPUStatusCmd.h \
	SetRawBlockCmd.h \
	GetRawBlockCmd.h \
	SetSplitterCmd.h \
	GetSplitterCmd.h \
	\
	SyncAction.h \
	WriteReg.h \
	ReadReg.h \
	XWWrite.h \
	BWWrite.h \
	BWRead.h \
	SSWrite.h \
	SSRead.h \
	RCUWrite.h \
	RCUProtocolWrite.h \
	RCUResultRead.h \
	RCURead.h \
	RawBlockRead.h \
	RawBlockWrite.h \
	StatusRead.h \
	SstRead.h \
	BstRead.h \
	XstRead.h \
	BypassRead.h \
	VersionsRead.h \
	WGWrite.h \
	WGRead.h \
	CDOWrite.h \
	BSWrite.h \
	RSUWrite.h \
	TDSi2cdefs.h \
	TDSProtocolWrite.h \
	TDSResultWrite.h \
	TDSResultRead.h \
	TDSStatusWrite.h \
	TDSStatusRead.h \
	HBAProtocolWrite.h \
	HBAResultRead.h \
	RADWrite.h \
	TimestampWrite.h \
	TBBSettingsWrite.h \
	TBBBandselWrite.h \
	BypassWrite.h  \
	SerdesWrite.h \
	SerdesRead.h

RSPDriver_LDADD					= libepa.la $(RTCCOMMON_LDADD)
RSPDriver_DEPENDENCIES			= libepa.la $(LOFAR_DEPEND)

rspctl_SOURCES					= rspctl.h rspctl.cc
rspctl_LDADD					= -lgnuplot_i

geni2c_SOURCES					= geni2c.cc

versionrspdriver_SOURCES		= versionrspdriver.cc
versionrspdriver_LDADD			= libepa.la
versionrspdriver_DEPENDENCIES	= libepa.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
