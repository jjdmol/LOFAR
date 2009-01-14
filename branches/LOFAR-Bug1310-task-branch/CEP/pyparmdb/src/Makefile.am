lib_LTLIBRARIES         = _parmdb.la

_parmdb_la_SOURCES	= pyparmdb.cc
# Needed for non-standard library name (i.e. not lib...)
_parmdb_la_LDFLAGS	= -module
_parmdb_la_DEPENDENCIES	= $(LOFAR_DEPEND)

lofarpython_PYTHON	= __init__.py
lofarpythondir		= $(pythondir)/lofar/parmdb
libdir			= $(lofarpythondir)

bin_PROGRAMS = versionpyparmdb

versionpyparmdb_SOURCES      = versionpyparmdb.cc Package__Version.cc
versionpyparmdb_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
