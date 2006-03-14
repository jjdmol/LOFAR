lib_LTLIBRARIES         = libparmdb.la

libparmdb_la_SOURCES = \
ParmValue.cc ParmDomain.cc ParmDBMeta.cc \
ParmDB.cc ParmDBAIPS.cc


# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

bin_PROGRAMS = parmdb

parmdb_SOURCES		= parmdb.cc
parmdb_LDADD		= libparmdb.la
parmdb_DEPENDENCIES	= libparmdb.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = lsm2db.g

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = lsm2db

include $(top_srcdir)/Makefile.common
