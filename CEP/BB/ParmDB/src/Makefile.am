lib_LTLIBRARIES         = libparmdb.la

libparmdb_la_SOURCES = Package__Version.cc  \
Axis.cc AxisMapping.cc Box.cc Grid.cc \
ParmValue.cc ParmSet.cc ParmMap.cc ParmCache.cc \
ParmDBMeta.cc ParmDB.cc ParmDBCasa.cc ParmDBLocker.cc \
Parm.cc ParmFacade.cc \
SourceDB.cc SourceDBCasa.cc
#ParmDBPostgres.cc ParmDBPostgresTransactors.cc


# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

bin_PROGRAMS = versionparmdb \
		parmdb makesourcedb

parmdb_SOURCES		= parmdb_main.cc
parmdb_LDADD		= libparmdb.la
parmdb_DEPENDENCIES	= libparmdb.la $(LOFAR_DEPEND)

makesourcedb_SOURCES		= makesourcedb.cc
makesourcedb_LDADD		= libparmdb.la
makesourcedb_DEPENDENCIES	= libparmdb.la $(LOFAR_DEPEND)

pythondir = $(bindir)
dist_python_SCRIPTS = 

versionparmdb_SOURCES      = versionparmdb.cc
versionparmdb_LDADD        = libparmdb.la
versionparmdb_DEPENDENCIES = libparmdb.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
