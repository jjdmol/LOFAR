# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libcs1_run.la

libcs1_run_la_SOURCES =

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcs1_run_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libcs1_run.la
#prg_DEPENDENCIES	= libcs1_run.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 	\
			Commands.py \
			Hosts.py \
			CS1_Hosts.py \
			LOFAR_Parset.py \
			CS1_Parset.py \
			LOFAR_Jobs.py \
			CS1_RSPCtl.py \
			CS1_Run.py \
			CS1_Sections.py

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
