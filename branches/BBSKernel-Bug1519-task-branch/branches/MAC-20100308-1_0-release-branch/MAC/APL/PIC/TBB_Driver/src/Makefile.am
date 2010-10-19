AM_CPPFLAGS = \
	-I$(top_builddir)/include
	
AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

#	cp $*.ph $(top_builddir)/include/APL/TBBDriver/
	
BUILT_SOURCES = \
	TP_Protocol.ph \
	TP_Protocol.cc 

# TBBDriver.conf is generated from MAC/Deployment/data/StaticMetadata
#sysconf_DATA = TBBDriver.conf 

#%.conf: %.conf.in
#	cp $< $@
	
lib_LTLIBRARIES = \
	libtbbdriver.la \
	libtp_protocol.la

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	TP_Protocol.prot 

libtbbdriver_la_SOURCES = Package__Version.cc \
	DriverSettings.cc \
	Command.cc \
	AllocCmd.cc \
	BoardCmdHandler.cc \
	MsgHandler.cc \
	FreeCmd.cc \
	RecordCmd.cc \
	StopCmd.cc \
	TrigReleaseCmd.cc \
	TrigGenCmd.cc\
	TrigSetupCmd.cc\
	TrigSetupSameCmd.cc\
	TrigCoefCmd.cc\
	TrigCoefSameCmd.cc\
	TrigInfoCmd.cc\
	ReadCmd.cc \
	UdpCmd.cc \
	PageperiodCmd.cc \
	VersionCmd.cc \
	SizeCmd.cc \
	StatusCmd.cc \
	ClearCmd.cc \
	ResetCmd.cc \
	ConfigCmd.cc \
	ErasefCmd.cc \
	ReadfCmd.cc \
	WritefCmd.cc \
	ImageInfoCmd.cc \
	ReadwCmd.cc \
	WritewCmd.cc \
	ReadrCmd.cc \
	WriterCmd.cc \
	ReadxCmd.cc	\
	ArpCmd.cc \
	ArpModeCmd.cc \
	StopCepCmd.cc	\
	CepStatusCmd.cc	\
	CepDelayCmd.cc	\
	WatchDogCmd.cc \
	TempLimitCmd.cc

libtp_protocol_la_SOURCES = \
	TP_Protocol.ph \
	TP_Protocol.cc \
	RawEvent.h \
	RawEvent.cc

pkginclude_HEADERS = Package__Version.h  $(BUILT_SOURCES)

noinst_HEADERS = \
	DriverSettings.h \
	Command.h \
	AllocCmd.h \
	BoardCmdHandler.h \
	MsgHandler.h \
	FreeCmd.h \
	RecordCmd.h \
	StopCmd.h \
	TrigReleaseCmd.h \
	TrigGenCmd.h\
	TrigSetupCmd.h\
	TrigSetupSameCmd.h\
	TrigCoefCmd.h\
	TrigCoefSameCmd.h\
	TrigInfoCmd.h\
	ReadCmd.h \
	UdpCmd.h \
	PageperiodCmd.h \
	VersionCmd.h \
	SizeCmd.h \
	StatusCmd.h \
	ClearCmd.h \
	ResetCmd.h \
	ConfigCmd.h \
	ErasefCmd.h \
	ReadfCmd.h \
	WritefCmd.h	\
	ImageInfoCmd.h \
	ReadwCmd.h \
	WritewCmd.h	\
	ReadrCmd.h \
	WriterCmd.h \
	ReadxCmd.h	\
	ArpCmd.h \
	ArpModeCmd.h \
	StopCepCmd.h \
	CepStatusCmd.h \
	CepDelayCmd.h \
	WatchDogCmd.h \
	TempLimitCmd.h	\
	UdpIpTools.h

#NOINSTdir	=

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libtbbdriver_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = versiontbb_driver \
	TBBDriver \
	tbbctl

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libtbbdriver.la
#prg_DEPENDENCIES	= libtbbdriver.la $(LOFAR_DEPEND)

tbbctl_SOURCES 			= tbbctl.h tbbctl.cc

TBBDriver_SOURCES 		= TBBDriver.h TBBDriver.cc
TBBDriver_LDADD 		= libtbbdriver.la libtp_protocol.la
TBBDriver_DEPENDENCIES  = libtbbdriver.la libtp_protocol.la $(LOFAR_DEPEND)

versiontbb_driver_SOURCES      = versiontbb_driver.cc
versiontbb_driver_LDADD        = libtbbdriver.la
versiontbb_driver_DEPENDENCIES = libtbbdriver.la $(LOFAR_DEPEND)


glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
