lib_LTLIBRARIES         = libsourcedb.la

libsourcedb_la_SOURCES = Package__Version.cc  \
SourceValue.cc SourceDBMeta.cc \
SourceDB.cc SourceDBAIPS.cc


bin_PROGRAMS = versionsourcedb sourcedb

versionsourcedb_SOURCES      = versionsourcedb.cc
versionsourcedb_LDADD        = libsourcedb.la
versionsourcedb_DEPENDENCIES = libsourcedb.la $(LOFAR_DEPEND)

sourcedb_SOURCES	= sourcedb_main.cc
sourcedb_LDADD		= libsourcedb.la
sourcedb_DEPENDENCIES	= libsourcedb.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
