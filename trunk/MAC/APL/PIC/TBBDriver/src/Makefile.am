# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libtbbdriver.la

libtbbdriver_la_SOURCES = \
	BoardCmdHandler.h \
	BoardCmdHandler.cc \
	Command.h \
	Command.cc			

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

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
