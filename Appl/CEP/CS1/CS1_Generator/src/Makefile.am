# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libcs1_generator.la

libcs1_generator_la_SOURCES   = StationData.cc \
		 DH_RSP.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcs1_generator_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libcs1_generator.la
#prg_DEPENDENCIES	= libcs1_generator.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
