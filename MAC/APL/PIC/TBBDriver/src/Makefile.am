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
	cp $*.ph $(top_builddir)/include/TBBDriver/TP_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/TBBDriver/TP_Protocol
	
BUILT_SOURCES = \
	TBBDriver.conf \
	RemoteStation.conf \
	tbbctl.conf

sysconf_DATA = \
	TBBDriver.conf \
	RemoteStation.conf \
	tbbctl.conf

%.conf: %.conf.in
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
	BoardCmdHandler.cc \
	ClientMsgHandler.cc \
	AllocCmd.cc \
	FreeCmd.cc \
	RecordCmd.cc \
	StopCmd.cc \
	TrigclrCmd.cc \
	ReadCmd.cc \
	UdpCmd.cc \
	VersionCmd.cc \
	SizeCmd.cc \
	ClearCmd.cc \
	ResetCmd.cc \
	ConfigCmd.cc \
	ErasefCmd.cc \
	ReadfCmd.cc \
	WritefCmd.cc		

noinst_HEADERS = \
	BoardCmdHandler.h \
	ClientMsgHandler.h \
	Message.h	\
	Command.h \
	AllocCmd.h \
	FreeCmd.h \
	RecordCmd.h \
	StopCmd.h \
	TrigclrCmd.h \
	ReadCmd.h \
	UdpCmd.h \
	VersionCmd.h \
	SizeCmd.h \
	ClearCmd.h \
	ResetCmd.h \
	ConfigCmd.h \
	ErasefCmd.h \
	ReadfCmd.h \
	WritefCmd.h		

#NOINSTdir	=

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libtbbdriver_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = TBBDriver

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libtbbdriver.la
#prg_DEPENDENCIES	= libtbbdriver.la $(LOFAR_DEPEND)

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
