# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS   = jOTDB_jOTDBconnection.h \
	     jOTDB_jTreeMaintenance.h \
	     jOTDB_jOTDBcommon.h \
	     jOTDB_jTreeValue.h \
	     jOTDB_jConverter.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES         = libjotdb.la

libjotdb_la_SOURCES   = $(DOCHDRS) \
		        jOTDB_jOTDBconnection.cc \
		        jOTDB_jTreeMaintenance.cc \
			jOTDB_jTreeValue.cc \
			jOTDB_jConverter.cc

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

include $(top_srcdir)/Makefile.common
