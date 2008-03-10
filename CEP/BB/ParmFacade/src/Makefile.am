lib_LTLIBRARIES         = _parmdb.la libparmfacade.la

libparmfacade_la_SOURCES = Package__Version.cc

_parmdb_la_SOURCES		= pyparmdb.cc ParmFacade.cc
# Needed for non-standard library name (i.e. not lib...)
_parmdb_la_LDFLAGS		= -module
#_parmdb_la_LIBADD		= ../src/libparmfacade.la
_parmdb_la_DEPENDENCIES	= $(LOFAR_DEPEND)

python_SCRIPTS = __init__.py
pythondir = $(prefix)/lib/python$(PYTHON_VERSION)/site-packages/lofar/parmdb

bin_PROGRAMS = versionparmfacade

versionparmfacade_SOURCES      = versionparmfacade.cc
versionparmfacade_LDADD        = libparmfacade.la
versionparmfacade_DEPENDENCIES = libparmfacade.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
