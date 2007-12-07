# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif
# SUBDIRS = test
AM_CPPFLAGS = \
	-I$(top_builddir)/include
	
AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/TBBDriver/TP_Protocol
	
BUILT_SOURCES = \
	TP_Protocol.ph \
	TP_Protocol.cc 

sysconf_DATA = \
	TBBDriver.conf \
	TBBDriver.log_prop \
	tbbctl.log_prop

%.conf: %.conf.in
	cp $< $@
	
%.log_prop: %.log_prop.in
	cp $< $@

lib_LTLIBRARIES = \
	libtbbdriver.la \
	libtp_protocol.la

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	TP_Protocol.prot 

	
libtp_protocol_la_SOURCES = \
	TP_Protocol.ph \
	TP_Protocol.cc \
	RawEvent.h \
	RawEvent.cc

libtbbdriver_la_SOURCES = \
	$(noinst_HEADERS) \
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
	TrigCoefCmd.cc\
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
	ReadxCmd.cc		

pkginclude_HEADERS	= $(BUILT_SOURCES)

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
	TrigCoefCmd.h\
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
	ReadxCmd.h

#NOINSTdir	=

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libtbbdriver_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = \
	TBBDriver \
	tbbctl

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libtbbdriver.la
#prg_DEPENDENCIES	= libtbbdriver.la $(LOFAR_DEPEND)

tbbctl_SOURCES = \
	tbbctl.h \
	tbbctl.cc

TBBDriver_SOURCES = \
	TBBDriver.h \
	TBBDriver.cc

TBBDriver_LDADD = \
	libtbbdriver.la \
	libtp_protocol.la
	
TBBDriver_DEPENDENCIES = \
	libtbbdriver.la \
	libtp_protocol.la \
	$(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
