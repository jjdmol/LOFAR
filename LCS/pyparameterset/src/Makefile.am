lib_LTLIBRARIES         = _pyparameterset.la

_pyparameterset_la_SOURCES	= pyparameterset.cc
# Needed for non-standard library name (i.e. not lib...)
_pyparameterset_la_LDFLAGS	= -module
_pyparameterset_la_DEPENDENCIES	= $(LOFAR_DEPEND)

python_SCRIPTS = __init__.py
pythondir = $(pythoninstalldir)/parameterset
libdir    = $(pythondir)

bin_PROGRAMS = versionpyparameterset

versionpyparameterset_SOURCES      = versionpyparameterset.cc Package__Version.cc
versionpyparameterset_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
