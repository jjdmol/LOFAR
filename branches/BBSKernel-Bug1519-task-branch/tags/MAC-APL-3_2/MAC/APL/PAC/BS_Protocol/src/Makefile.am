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
	BS_Protocol.ph

EXTRA_DIST = \
	BS_Protocol.ph

phdir=$(includedir)/APL/BS_Protocol
ph_HEADERS = \
	$(EXTRA_DIST)

lib_LTLIBRARIES         = libbs_protocol.la

libbs_protocol_la_SOURCES = \
	BS_Protocol.cc \
	Beamlet2SubbandMap.cc \
	Pointing.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libbs_protocol_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libbs_protocol.la
#prg_DEPENDENCIES	= libbs_protocol.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
