# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libjparmfacade.la

libjparmfacade_la_SOURCES = $(DOCHDRS) \
			    nl_astron_lofar_java_cep_jparmfacade_jParmFacade.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

if HAVE_SHMEM
libjparmfacade_la_LIBADD     = $(prefix)/lib/libshmem.la
endif

bin_PROGRAMS =

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libjparmfacade.la
#prg_DEPENDENCIES	= libjparmfacade.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
