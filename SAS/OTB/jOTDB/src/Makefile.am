# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS   = jOTDB_jOTDBconnection.h \
	     jOTDB_jOTDBcommon.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES         = libjotdb.la

libjotdb_la_SOURCES   = jOTDB_jOTDBconnection.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libjotdb_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(lofar_sharedir)/Makefile.common
