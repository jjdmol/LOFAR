# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	EPA_Protocol.ph \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph

EXTRA_DIST = \
	EPA_Protocol.ph \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph

phdir=$(includedir)/RSP_Protocol
ph_HEADERS = \
	$(EXTRA_DIST)

lib_LTLIBRARIES         = librsp_protocol.la

librsp_protocol_la_SOURCES = \
	BeamletWeights.cc \
	Clocks.cc \
	RCUSettings.cc \
	Statistics.cc \
	SubbandSelection.cc \
	SystemStatus.cc \
	Versions.cc \
	WGSettings.cc \
	XCStatistics.cc \
	EPA_Protocol.cc \
	RSP_Protocol.cc \
	RSPFE_Protocol.cc \
	MEPHeader.cc \
	MEPData.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# librsp_protocol_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= librsp_protocol.la
#prg_DEPENDENCIES	= librsp_protocol.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
