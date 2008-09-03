lib_LTLIBRARIES         = libparmdb.la

libparmdb_la_SOURCES = Package__Version.cc  \
ParmValue.cc ParmDomain.cc ParmDBMeta.cc \
ParmDB.cc ParmDBAIPS.cc ParmDBPostgres.cc ParmDBPostgresTransactors.cc


# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

bin_PROGRAMS = versionparmdb parmdb

parmdb_SOURCES		= parmdb_main.cc
parmdb_LDADD		= libparmdb.la
parmdb_DEPENDENCIES	= libparmdb.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = lsm2db.g

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = lsm2db

versionparmdb_SOURCES      = versionparmdb.cc
versionparmdb_LDADD        = libparmdb.la
versionparmdb_DEPENDENCIES = libparmdb.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
