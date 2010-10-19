# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libcs1_generator.la

libcs1_generator_la_SOURCES   = 	\
				DH_RSP.cc \
				WH_Signal.cc \
				WH_Wrap.cc \
				WH_FakeStation.cc \
				WH_Strip.cc
# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcs1_generator_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = CS1_Generator CS1_Recorder

CS1_Generator_SOURCES		= AH_FakeStation.cc
CS1_Generator_LDADD		= libcs1_generator.la
CS1_Generator_DEPENDENCIES	= libcs1_generator.la $(LOFAR_DEPEND)

CS1_Recorder_SOURCES		= AH_Recorder.cc
CS1_Recorder_LDADD		= libcs1_generator.la
CS1_Recorder_DEPENDENCIES	= libcs1_generator.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
