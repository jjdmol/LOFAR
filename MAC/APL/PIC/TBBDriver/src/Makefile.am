# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif
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


lib_LTLIBRARIES         = libtbbdriver.la

EXTRA_DIST = \
	TP_Protocol.prot 

libtbbdriver_la_SOURCES = \
	$(NOINST_HEADERS)  \
	TP_Protocol.cc \
	StationSettings.cc \
	RawEvent.cc \
	BoardCmdHandler.cc \
	ClientMsgHandler.cc \
	VersionCmd.cc
	

NOINST_HEADERS = \
	TP_Protocol.ph \
	StationSettings.h \
	RawEvent.h \
	BoardCmdHandler.h \
	ClientMsgHandler.h \
	Message.h	\
	Command.h \
	VersionCmd.h			

NOINSTdir	=

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

TBBDriver_LDADD		= libtbbdriver.la
TBBDriver_DEPENDENCIES	= libtbbdriver.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
