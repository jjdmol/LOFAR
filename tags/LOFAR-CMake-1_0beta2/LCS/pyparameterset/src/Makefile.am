lib_LTLIBRARIES         = _pyparameterset.la

_pyparameterset_la_SOURCES	= pyparameterset.cc
# Needed for non-standard library name (i.e. not lib...)
_pyparameterset_la_LDFLAGS	= -module
_pyparameterset_la_DEPENDENCIES	= $(LOFAR_DEPEND)

lofarpython_PYTHON = __init__.py
lofarpythondir = $(pythondir)/lofar/parameterset
libdir         = $(lofarpythondir)

pkginclude_HEADERS = Package__Version.h

bin_PROGRAMS = versionpyparameterset

versionpyparameterset_SOURCES      = versionpyparameterset.cc Package__Version.cc
versionpyparameterset_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
