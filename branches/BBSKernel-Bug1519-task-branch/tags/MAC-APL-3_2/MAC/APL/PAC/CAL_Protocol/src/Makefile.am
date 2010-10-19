
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	CAL_Protocol.ph

EXTRA_DIST = \
	CAL_Protocol.ph

phdir=$(includedir)/APL/CAL_Protocol
ph_HEADERS = \
	$(EXTRA_DIST)

lib_LTLIBRARIES         = libcal_protocol.la

libcal_protocol_la_SOURCES = \
	CAL_Protocol.cc \
	AntennaArray.cc \
	AntennaArrayData.cc \
	AntennaGains.cc \
	SpectralWindow.cc \
	SubArray.cc \
	ACC.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcal_protocol_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libcal_protocol.la
#prg_DEPENDENCIES	= libcal_protocol.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
